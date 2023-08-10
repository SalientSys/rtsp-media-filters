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
/// @class LiveMediaSubsession
///
/// Modified 08/16/2019
/// Modified by: M. Kinzer
///
#include "pch.h"

#include <cassert>
#include <live555/liveMedia.hh>

#include "LiveMediaSubsession.h"
#include "LiveAMRAudioDeviceSource.h"
#include "LiveDeviceSource.h"
#include "LiveSourceTaskScheduler.h"
#include "LiveRtspServer.h"
#include "IMediaSampleBuffer.h"
#include "MultiMediaSampleBuffer.h"
#include "SingleMediaSampleBuffer.h"

// We don't want to reuse the first source as one usually would for "live" liveMedia sources
// In our case we want a separate device source per client so that we can control the switching on a per client basis
static const bool ReuseFirstSource = false;

using namespace CvRtsp;

LiveMediaSubsession::
LiveMediaSubsession(UsageEnvironment& env, LiveRtspServer& parent,
	const boost::uuids::uuid& channelId, uint32_t sourceId, const std::string& sessionName,
	bool isVideo, const unsigned totalChannels, bool isSwitchableFormat,
	IRateAdaptationFactory* rateAdaptationFactory, IRateController* globalRateControl) :
	OnDemandServerMediaSubsession(env, ReuseFirstSource),
	m_rtspServer(parent),
	m_channelId(channelId),
	m_sourceId(sourceId),
	m_sessionName(sessionName),
	m_isVideo(isVideo),
	m_totalChannels(totalChannels),
	m_sampleBuffer(nullptr),
	m_rateAdaptationFactory(rateAdaptationFactory),
	m_globalRateControl(globalRateControl),
	m_hasServedAnyVideoDeviceSource(false),
	m_hasBeenProcessedToKill(false)
{
	// sanity checks
	assert(m_channelId.is_nil() == false);
	assert(m_sessionName.empty() == false);

	const auto taskScheduler = &(envir().taskScheduler());
	const auto pollingScheduler = dynamic_cast<LiveSourceTaskScheduler*>(taskScheduler);
	if (pollingScheduler)
	{
		pollingScheduler->RegisterMediaSubsession(m_channelId, m_sessionName, m_sourceId, this);
	}

	// Create sample buffer according to number of 'switchable' channels.
	assert(m_totalChannels > 0);
	if (m_totalChannels > 1 || isSwitchableFormat)
	{
		m_sampleBuffer = new MultiMediaSampleBuffer(m_totalChannels);
	}
	else
	{
		m_sampleBuffer = new SingleMediaSampleBuffer();
	}

	assert(m_sampleBuffer);
}


void 
LiveMediaSubsession::
cleanup()
{
	// get handle to task-scheduler.
	const auto pScheduler = &(envir().taskScheduler());
	const auto pPollingScheduler = dynamic_cast<LiveSourceTaskScheduler*>(pScheduler);
	if (pPollingScheduler)
	{
		// unregister this subsession from task scheduler.
		pPollingScheduler->DeRegisterMediaSubsession(m_channelId, m_sessionName, m_sourceId, this);
	}

	// release media buffer.
	if (m_sampleBuffer)
	{
		delete m_sampleBuffer;
		m_sampleBuffer = nullptr;
	}
}


LiveMediaSubsession::
~LiveMediaSubsession()
{
	cleanup();
}


void
LiveMediaSubsession::
AddMediaSample(const std::shared_ptr<MediaSample>& mediaSample)
{
	assert(m_sampleBuffer);

	// Add the sample to the buffer where it will be parsed
	m_sampleBuffer->AddMediaSample(mediaSample);
	
	// Iterate over all device sources and deliver the samples to the clients
	for (const auto& deviceSource : m_deviceSources)
	{
		if (!m_hasServedAnyVideoDeviceSource)
		{
			// mark as this subsession has served media samples.
			m_hasServedAnyVideoDeviceSource = true;
		}

		if (deviceSource->RetrieveMediaSampleFromBuffer())
		{
			if (!deviceSource->IsPlaying())
			{
				deviceSource->DeliverFrame();
			}
		}
	}
}

FramedSource*
LiveMediaSubsession::
createNewStreamSource(uint32_t clientSessionId, uint32_t& estBitrate)
{
	assert(m_sampleBuffer);

	// Delegating creation call to subclasses of LiveMediaSubsession: pass in the global rate control in the case the
	// subclass wants to use it. It may decide to create it's own rate control method based on the type of the stream
	const auto streamSource = createSubsessionSpecificSource(clientSessionId,
		m_sampleBuffer, m_rateAdaptationFactory, m_globalRateControl);

	// make sure subclasses were able to construct source
	assert(streamSource);

	// Call virtual subclass method to set estimated bit rate
	setEstimatedBitRate(estBitrate);
	return streamSource;
}

RTPSink*
LiveMediaSubsession::
createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	const auto rtpSink = createSubsessionSpecificRTPSink(rtpGroupsock, rtpPayloadTypeIfDynamic, inputSource);
	LiveDeviceSource* deviceSource = nullptr;
	if (inputSource->isH264VideoStreamFramer() || inputSource->isH265VideoStreamFramer())
	{
		auto test = inputSource->MIMEtype();

		auto h264Or5VideoStreamFramer = static_cast<H264or5VideoStreamFramer*>(inputSource);
		// unsafe cast! We assume that the input source is a LiveDeviceSource as all source and sink construction 
		// is happening inside this LiveMediaSubsession!
		// get the framed LiveDeviceSource
		deviceSource = dynamic_cast<LiveDeviceSource*>(h264Or5VideoStreamFramer->inputSource());
		test = deviceSource->MIMEtype();
	}
	else
	{
		// unsafe cast! We assume that the input source is a LiveDeviceSource as all source and sink construction 
		// is happening inside this LiveMediaSubsession!
		deviceSource = static_cast<LiveDeviceSource*>(inputSource);
	}
	assert(deviceSource);
	deviceSource->SetRtpSink(rtpSink);
	return rtpSink;
}

void
LiveMediaSubsession::
ProcessClientStatistics()
{
	for (const auto& deviceSource : m_deviceSources)
	{
		deviceSource->ProcessReceiverReports();
	}
}

void
LiveMediaSubsession::
addDeviceSource(LiveDeviceSource* deviceSource)
{
	m_deviceSources.emplace_back(deviceSource);
}

void
LiveMediaSubsession::
removeDeviceSource(LiveDeviceSource* deviceSource)
{
	for (auto source = m_deviceSources.begin(); source != m_deviceSources.end(); ++source)
	{
		if (*source == deviceSource)
		{
			m_deviceSources.erase(source);

			// is this the last device-source/client that was using this subsession?
			if (m_deviceSources.empty())
			{
				// kill this subsession.
				//cleanup(); - @TODO check if this needs to be called, it looks like its called when we call 
				// CvRtspServer::RemoveChannel()

				// kill the channel associated with this subsession.
				//m_rtspServer.OnRtspDestroyChannel(m_sessionName);
			}

			break;
		}
	}
}

void
LiveMediaSubsession::
deleteStream(uint32_t clientSessionId, void*& streamToken)
{
	// Call super class method
	OnDemandServerMediaSubsession::deleteStream(clientSessionId, streamToken);

	// Handle client management.
	if (m_onLeave)
	{
		m_onLeave(m_channelId, m_sourceId, clientSessionId);
	}
}

void
LiveMediaSubsession::
getStreamParameters(uint32_t clientSessionId, netAddressBits clientAddress,
	Port const& clientRTPPort, Port const& clientRTCPPort, int tcpSocketNum, unsigned char rtpChannelId,
	unsigned char rtcpChannelId, netAddressBits& destinationAddress, u_int8_t& destinationTTL,
	Boolean& isMulticast, Port& serverRTPPort, Port& serverRTCPPort, void*& streamToken)
{
	if (destinationAddress == 0)
	{
		destinationAddress = clientAddress;
	}

	struct in_addr destinationAddr;
	destinationAddr.s_addr = destinationAddress;

	// client management
	if (m_onJoin)
	{
		// Convert IpAddress to readable format
		std::string ipAddress(inet_ntoa(destinationAddr));
		m_onJoin(m_channelId, m_sourceId, clientSessionId, ipAddress);
	}

	OnDemandServerMediaSubsession::getStreamParameters(clientSessionId, clientAddress, clientRTPPort, clientRTCPPort, tcpSocketNum, rtpChannelId,
		rtcpChannelId, destinationAddress, destinationTTL, isMulticast, serverRTPPort, serverRTCPPort, streamToken);
}

std::vector<uint32_t>
LiveMediaSubsession::
GetConnectedClientIds() const
{
	std::vector<uint32_t> clientIds;
	for (auto deviceSource : m_deviceSources)
	{
		clientIds.push_back(deviceSource->GetClientId());
	}
	return clientIds;
}


void 
LiveMediaSubsession::
KillChannel()
{
	m_hasBeenProcessedToKill = true;
	m_rtspServer.OnRtspDestroyChannel(std::make_pair(m_channelId, m_sessionName));
}