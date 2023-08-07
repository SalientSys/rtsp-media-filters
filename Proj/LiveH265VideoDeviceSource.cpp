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
/// @class LiveH265VideoDeviceSource
///
/// Modified 08/16/2019
/// Modified by: M. Kinzer
///
#include "pch.h"

#include <rtsp-logger/RtspServerLogging.h>

#include "LiveH265VideoDeviceSource.h"
#include "IFrameGrabber.h"
#include "CommonRtsp.h"

using namespace CvRtsp;

LiveH265VideoDeviceSource::
LiveH265VideoDeviceSource(UsageEnvironment& env, unsigned clientId,
	LiveMediaSubsession* parentSubsession,
	const std::string& vps, const std::string& sps, const std::string& pps,
	IFrameGrabber* frameGrabber, IRateAdaptationFactory* rateAdaptationFactory,
	IRateController* globalRateControl) :
	LiveDeviceSource(env, clientId, parentSubsession, frameGrabber, rateAdaptationFactory, globalRateControl),
	m_isWaitingForIRAP(true) {}


LiveH265VideoDeviceSource*
LiveH265VideoDeviceSource::
CreateNew(UsageEnvironment& env, unsigned clientId,
	LiveMediaSubsession* parentSubsession,
	const std::string& vps, const std::string& sps, const std::string& pps,
	IFrameGrabber* frameGrabber, IRateAdaptationFactory* rateAdaptationFactory,
	IRateController* globalRateControl)
{
	// When constructing a 'simple' LiveDeviceSource we'll just create a simple frame grabber
	auto videoDeviceSource = new LiveH265VideoDeviceSource(env, clientId, parentSubsession,
		vps, sps, pps, frameGrabber, rateAdaptationFactory, globalRateControl);
	OutPacketBuffer::increaseMaxSizeTo(300000); // bytes
	return videoDeviceSource;
}


std::deque<std::shared_ptr<MediaSample>>
LiveH265VideoDeviceSource::
splitPayloadIntoMediaSamples(BYTE* dataBuffer, uint32_t bufferSize, double startTime)
{
	std::deque<std::shared_ptr<MediaSample>> mediaSamples;
	if (!dataBuffer || bufferSize == 0)
	{
		return mediaSamples;
	}

	auto startingPosition = NalUnitPrefixWithoutZeroBitSize;
	auto nalUnitPrefixSize = NalUnitPrefixWithoutZeroBitSize;
	auto hasZeroBit = false;
	auto isNalUnitPrefix = false;

	// check for first start code
	if (memcmp(NalUnitPrefixWithZeroBit, dataBuffer,
		NalUnitPrefixWithZeroBitSize) == 0)
	{
		hasZeroBit = true;
		nalUnitPrefixSize = NalUnitPrefixWithZeroBitSize;
		startingPosition = NalUnitPrefixWithZeroBitSize;
	}
	if (memcmp(NalUnitPrefix, dataBuffer, NalUnitPrefixWithoutZeroBitSize) == 0)
	{
		isNalUnitPrefix = true;
	}

	if (hasZeroBit || isNalUnitPrefix)
	{
		// Search for start codes and create samples.
		for (auto i = startingPosition; i < static_cast<int>(bufferSize) - nalUnitPrefixSize; ++i)
		{
			hasZeroBit = isNalUnitPrefix = false;

			if (memcmp(NalUnitPrefixWithZeroBit, dataBuffer + i, NalUnitPrefixWithZeroBitSize) == 0)
			{
				nalUnitPrefixSize = NalUnitPrefixWithZeroBitSize;
				hasZeroBit = true;
			}
			else if (memcmp(NalUnitPrefix, dataBuffer + i, NalUnitPrefixWithoutZeroBitSize) == 0)
			{
				nalUnitPrefixSize = NalUnitPrefixWithoutZeroBitSize;
				hasZeroBit = true;
			}

			if (hasZeroBit || isNalUnitPrefix)
			{
				const auto nalUnitSize = i - startingPosition;
				const auto mediaSample = MediaSample::
					CreateMediaSample(dataBuffer + startingPosition, nalUnitSize, startTime);

				mediaSamples.push_back(mediaSample);
				startingPosition = i += nalUnitPrefixSize;
			}
		}

		// Push last remaining sample
		const auto nalUnitSize = static_cast<int>(bufferSize) - startingPosition;
		const auto mediaSample = MediaSample::CreateMediaSample(dataBuffer + startingPosition,
			nalUnitSize, startTime);

		mediaSamples.push_back(mediaSample);
	}

	return mediaSamples;
}


bool
LiveH265VideoDeviceSource::
RetrieveMediaSampleFromBuffer()
{
	unsigned bufferSize = 0;
	auto startTime = 0.0;
	const auto dataBuffer = m_frameGrabber->GetNextFrame(bufferSize, startTime);

	// Make sure there's data, the frame grabber should return null if it doesn't have any
	if (!dataBuffer)
	{
		return false;
	}

	auto mediaSamples = splitPayloadIntoMediaSamples(dataBuffer, bufferSize, startTime);

	if (m_isWaitingForIRAP)
	{
		// Search for IRAP.
		for (const auto& sample : mediaSamples)
		{
			const auto payload = sample->GetDataBuffer().Data();

			if (isH265Vps(payload[0]))
			{
				// Found VPS
				log_rtsp_debug("Found VPS NAL unit.");
			}
			if (isH265Sps(payload[0]))
			{
				// Found SPS
				log_rtsp_debug("Found SPS NAL unit.");
			}
			if (isH265Pps(payload[0]))
			{
				// Found PPS
				log_rtsp_debug("Found PPS NAL unit.");
			}
			if (isH265RandomAccessPointPicture(payload[0]))
			{
				// Found IRAP
				log_rtsp_debug("Found IRAP.");
				m_isWaitingForIRAP = false;
				break;
			}
		}

		if (!m_isWaitingForIRAP)
		{
			m_mediaSampleQueue.insert(m_mediaSampleQueue.end(), mediaSamples.begin(), mediaSamples.end());
			return true;
		}
		return false;
	}

	// We got our IRAP, push subsequent samples.
	m_mediaSampleQueue.insert(m_mediaSampleQueue.end(), mediaSamples.begin(), mediaSamples.end());

	// If the media queue size exceeds the max that can be stored in the deque, remove enough samples
	// from the deque until reaching one that contains an i frame.
	if (m_mediaSampleQueue.size() > MaxMediaPackets)
	{
		// Dump frames until hit an Idr
		auto foundIFrame{ false };
		while (!foundIFrame && !m_mediaSampleQueue.empty())
		{
			const auto mediaSample = m_mediaSampleQueue.front();
			foundIFrame = mediaSample->GetIsKeyFrame();
			if (!foundIFrame)
			{
				m_mediaSampleQueue.pop_front();
			}
		}
	}

	std::stringstream ss;
	ss << " Media Sample queue size : " << m_mediaSampleQueue.size();
	log_rtsp_debug(ss.str());

	return true;
}
