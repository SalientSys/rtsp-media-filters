/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "CSIR"
// Copyright (c) 2014 CSIR.  All rights reserved.

///
/// @class LiveDeviceSource
///
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///

#include "pch.h"

#include <live555/GroupsockHelper.hh>
#include <rtsp-logger/RtspServerLogging.h>

#include "IRateAdaptation.h"
#include "IRateAdaptationFactory.h"
#include "IRateController.h"
#include "LiveDeviceSource.h"
#include "IFrameGrabber.h"
#include "SimpleFrameGrabber.h"
#include "LiveMediaSubsession.h"

using namespace CvRtsp;

LiveDeviceSource* LiveDeviceSource::
CreateNew(UsageEnvironment& env, unsigned clientId, LiveMediaSubsession* parentSubsession,
	IMediaSampleBuffer* sampleBuffer, IRateAdaptationFactory* rateAdaptationFactory, IRateController* rateControl)
{
	// When constructing a 'simple' LiveDeviceSource we'll just create a simple frame grabber
	return new LiveDeviceSource(env, clientId, parentSubsession,
		new SimpleFrameGrabber(sampleBuffer), rateAdaptationFactory, rateControl);
}

LiveDeviceSource::
LiveDeviceSource(UsageEnvironment& env, unsigned clientId, LiveMediaSubsession* parent,
	IFrameGrabber* frameGrabber, IRateAdaptationFactory* rateAdaptationFactory, IRateController* rateControl) :
	FramedSource(env),
	m_env(env),
	m_clientId(clientId),
	m_parentSubsession(parent),
	m_frameGrabber(frameGrabber),
	m_sink(nullptr),
	m_useTimeOffset(false),
	m_offsetTimeDouble(0.0),
	m_isPlaying(false),
	m_rateAdaptationFactory(rateAdaptationFactory),
	m_rateAdaptation(nullptr),
	m_rateControl(rateControl),
	m_lastPacketNumReceived(0)
{
	m_parentSubsession->addDeviceSource(this); // register with the subsession.
	if (m_rateAdaptationFactory)
	{
		m_rateAdaptation = m_rateAdaptationFactory->getInstance();
	}
}

LiveDeviceSource::
~LiveDeviceSource()
{
	m_parentSubsession->removeDeviceSource(this); // un-register with the subsession.
	if (m_frameGrabber)
	{
		delete m_frameGrabber;
		m_frameGrabber = nullptr;
	}

	if (m_rateAdaptation)
	{
		assert(m_rateAdaptationFactory);
		m_rateAdaptationFactory->releaseInstance(m_rateAdaptation);
	}
}

void
LiveDeviceSource::
doGetNextFrame()
{
	// Arrange here for our "deliverFrame" member function to be called
	// when the next frame of data becomes available from the device.
	// This must be done in a non-blocking fashion - i.e., so that we
	// return immediately from this function even if no data is
	// currently available.
	//
	// If the device can be implemented as a readable socket, then one easy
	// way to do this is using a call to
	//     envir().taskScheduler().turnOnBackgroundReadHandling( ... )
	// (See examples of this call in the "liveMedia" directory.)
	// Check availability
	if (m_mediaSampleQueue.empty())
	{
		m_isPlaying = false;
		return;
	}

	m_isPlaying = true;

	DeliverFrame();
}

bool
LiveDeviceSource::
RetrieveMediaSampleFromBuffer()
{
	unsigned bufferSize = 0;
	auto startTime = 0.0;
	const auto dataBuffer = m_frameGrabber->GetNextFrame(bufferSize, startTime);

	// Make sure there's data, the frame grabber should return null if it doesn't have any data.
	if (!dataBuffer)
	{
		log_rtsp_warning("NULL Sample retrieved from frame grabber");
		return false;
	}

	const auto mediaSample = MediaSample::CreateMediaSample(dataBuffer, bufferSize, startTime);
	m_mediaSampleQueue.push_back(mediaSample);

	return true;
}

void
LiveDeviceSource::
DeliverFrame()
{
	// This would be called when new frame data is available from the device.
	// This function should deliver the next frame of data from the device,
	// using the following parameters (class members):
	// 'in' parameters (these should *not* be modified by this function):
	//     fTo: The frame data is copied to this address.
	//         (Note that the variable "fTo" is *not* modified.  Instead,
	//          the frame data is copied to the address pointed to by "fTo".)
	//     fMaxSize: This is the maximum number of bytes that can be copied
	//         (If the actual frame is larger than this, then it should
	//          be truncated, and "fNumTruncatedBytes" set accordingly.)
	// 'out' parameters (these are modified by this function):
	//     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
	//     fNumTruncatedBytes: Should be set iff the delivered frame would have been
	//         bigger than "fMaxSize", in which case it's set to the number of bytes
	//         that have been omitted.
	//     fPresentationTime: Should be set to the frame's presentation time
	//         (seconds, microseconds).
	//     fDurationInMicroseconds: Should be set to the frame's duration, if known.
	if (!isCurrentlyAwaitingData())
	{
		return; // we're not ready for the data yet
	}

	assert(!m_mediaSampleQueue.empty());

	// Deliver the data here:
	const auto mediaSample = m_mediaSampleQueue.front();
	m_mediaSampleQueue.pop_front();

	const auto startTime = mediaSample->StartTime();
	const auto bufferSize = mediaSample->GetSize();
	const auto dataBuffer = mediaSample->GetDataBuffer().Data();

	// The start time of the first sample is stored as a reference start time for the media samples
	// Similarly we store the current time obtained by gettimeofday in m_offsetTimeStruct.
	// The reason for this is that we need to start timestamping the samples with timestamps starting at gettimeofday
	// This is a requirement that allows us to receive the original timestamp on the RTP client side.
	// The reason is that the RTCP implementation starts running using gettimeofday.
	// If this is not done, samples on the receiver side have really strange time stamps
	if (!m_useTimeOffset)
	{
		// Set initial values
		gettimeofday(&m_offsetTimeStruct, nullptr);
		m_offsetTimeDouble = startTime;
		m_useTimeOffset = true;

		// Set the presentation time of the first sample
		gettimeofday(&fPresentationTime, nullptr);
	}
	else
	{
		// Calculate the difference between this samples start time and the initial samples start time
		const auto timeDifference = startTime - m_offsetTimeDouble;
		const auto timeDifferenceSecs = static_cast<long>(timeDifference);
		const auto timeDifferenceMicroSecs = static_cast<long>((timeDifference - timeDifferenceSecs) * 1000000);

		// Now add these offsets to the initial presentation time obtained through gettimeofday
		fPresentationTime.tv_sec = m_offsetTimeStruct.tv_sec + timeDifferenceSecs;
		fPresentationTime.tv_usec = m_offsetTimeStruct.tv_usec + timeDifferenceMicroSecs;
	}

	if (bufferSize > static_cast<int>(fMaxSize))
	{
		// TODONB
		//TOREVISE/TODO
		fNumTruncatedBytes = bufferSize - fFrameSize;
		fFrameSize = fMaxSize;
		//TODO How do we send the rest in the following packet???
		//TODO How do we delete the frame??? Unless we store extra attributes in the MediaFrame class
		//LOG(WARNING) << "TODO: Truncated packet";
	}
	else
	{
		fFrameSize = bufferSize;
		memcpy(fTo, dataBuffer, fFrameSize);
		// Testing with current time of day
		//gettimeofday(&fPresentationTime, NULL);
		// 04/04/2008 RG: http://lists.live555.com/pipermail/live-devel/2008-April/008395.html
		//Testing with 'live' config
		fDurationInMicroseconds = 0;
	}

	// After delivering the data, inform the reader that it is now available:
	afterGetting(this);
}

void
LiveDeviceSource::
DoDeliverFrame(void* instance)
{
	auto pSource = static_cast<LiveDeviceSource*>(instance);
	if (pSource)
	{
		pSource->DeliverFrame();
	}
}

void
LiveDeviceSource::
ProcessReceiverReports()
{
	if (m_rateAdaptation && m_sink)
	{
		RTPTransmissionStatsDB::Iterator statsIter(m_sink->transmissionStatsDB());
		RTPTransmissionStats* stats = statsIter.next();
		if (stats != NULL)
		{
			if (m_lastPacketNumReceived == stats->lastPacketNumReceived())
			{
				log_rtsp_information("LiveDeviceSource::ProcessReceiverReports(): no new info");
				return;
			}
			m_lastPacketNumReceived = stats->lastPacketNumReceived();

			// convert RtpTransmissionStats to our format
			RtpTransmissionStats rtpStats;
			rtpStats.LastPacketNumReceived = stats->lastPacketNumReceived();
			rtpStats.FirstPacketNumReported = stats->firstPacketNumReported();
			rtpStats.TotalPacketsLost = stats->totNumPacketsLost();
			rtpStats.Jitter = stats->jitter();
			rtpStats.LastSrTime = stats->lastSRTime();
			rtpStats.DiffSrRr = stats->diffSR_RRTime();
			rtpStats.Rtt = stats->roundTripDelay();

			SwitchDirection eRateAdvice = m_rateAdaptation->GetRateAdaptAdvice(rtpStats);

			// get hold of rate control interface
			// If we switch for a single source - single client, this will be ok.
			// In the case of multiple clients being connected, this will screw up the rate control as there is currently
			// only one encoder in the media pipeline.
			if (m_rateControl)
			{
				m_rateControl->controlBitrate(eRateAdvice);
			}
		}
		else
		{
			log_rtsp_information("LiveDeviceSource::ProcessReceiverReports(): stats null");
		}
	}
}
