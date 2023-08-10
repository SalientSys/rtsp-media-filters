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
/// @class LiveSourceTaskScheduler0
///
/// Modified 08/21/2019
/// Modified by: M. Kinzer
///
#include "pch.h"

#include <boost/uuid/uuid_io.hpp>

#include <rtsp-logger/RtspServerLogging.h>

#include "LiveSourceTaskScheduler0.h"
#include "LiveMediaSubsession.h"
#include "ChannelManager.h"

using namespace CvRtsp;

LiveSourceTaskScheduler0::
LiveSourceTaskScheduler0(ChannelManager& channelManager)
	: BasicTaskScheduler(m_maxDelayTimeMicroSec),
	/*m_exitEvent(::CreateEventA(nullptr, TRUE, FALSE, nullptr)),*/
	m_channelManager(channelManager),
	m_samplesReceived(0),
	m_hasRun(false)
{

}

void
LiveSourceTaskScheduler0::
doEventLoop(char volatile* watchVariable)
{
	//Start();
	// Repeatedly loop, handling readable sockets and timed events:
	while (true)
	{
		if (watchVariable != nullptr)
		{
			assert(atoi(const_cast<char*>(watchVariable)) >= 0);
			if (*watchVariable != 0)
			{
				auto bExitEventLoop = false;
				switch (*watchVariable)
				{
					// Exit event loop
				case 1:
				{
					bExitEventLoop = true;
					break;
				}
				default:
					break;
				}

				if (bExitEventLoop)
				{
					// Break out the while
					break;
				}
			}
		}
		log_rtsp_debug("Calling processLiveSources.");
		processLiveSources();
		processMediaSubsessions();

		log_rtsp_debug("Done processLiveSources, calling SingleStep.");
		// run once
		//if (m_hasRun)
		//{
		//	std::this_thread::sleep_for(std::chrono::seconds(60));
		//	continue;
		//}
		SingleStep(m_maxDelayTimeMicroSec);

		log_rtsp_debug("Done SingleStep.");
	}
}


void
LiveSourceTaskScheduler0::
RegisterMediaSubsession(const boost::uuids::uuid& channelId, const std::string& channelName,
	uint32_t sourceId, LiveMediaSubsession* mediaSubsession)
{
	{
		std::stringstream ss;
		ss << "Registering media subsession with task-scheduler -"
			<< " Channel Id: " << channelId
			<< " Channel Name: " << channelName
			<< " Source Id: " << sourceId;
		log_rtsp_debug(ss.str());
	}

	// already exist?
	const auto sessionMapIterator = m_mediaSubSessionsMap.find(std::make_pair(channelId, channelName));
	if (sessionMapIterator == m_mediaSubSessionsMap.end())
	{
		// add media subsession
		m_mediaSubSessionsMap.emplace(std::make_pair(channelId, channelName), mediaSubsession);
	}
}

void
LiveSourceTaskScheduler0::
DeRegisterMediaSubsession(const boost::uuids::uuid& channelId, const std::string& channelName,
	uint32_t sourceId, LiveMediaSubsession* mediaSubsession)
{
	{
		std::stringstream ss;
		ss << "De-registering media subsession from task-scheduler -"
			<< " Channel Id: " << channelId
			<< " Channel Name: " << channelName
			<< " Source Id: " << sourceId;
		log_rtsp_debug(ss.str());
	}

	// exist?
	const auto sessionMapIterator = m_mediaSubSessionsMap.find(std::make_pair(channelId, channelName));
	if (sessionMapIterator != cend(m_mediaSubSessionsMap))
	{
		// remove media subsession
		m_mediaSubSessionsMap.erase(std::make_pair(channelId, channelName));
	}
}

LiveMediaSubsession*
LiveSourceTaskScheduler0::
GetMediaSubsession(const boost::uuids::uuid& channelId,
	const std::string& channelName, uint32_t sourceId)
{
	auto mediaSubSession = m_mediaSubSessionsMap.find(std::make_pair(channelId, channelName));
	if (mediaSubSession != m_mediaSubSessionsMap.end())
	{
		return mediaSubSession->second;
	}
	return nullptr;
}

void
LiveSourceTaskScheduler0::
processLiveSources()
{
	log_rtsp_debug("Processing live sources.");

	uint32_t sessionCount = 0;
	uint32_t sampleCount = 0;
	auto exit = false;

	// TODO - MK may need to rework this 
	// try and retrieve a sample for each channel
	// warning: the if condition of the for needs to be re-thought if there are more sessions
	// i.e. some sessions may never be reached if the for always starts at begin and exits when 
	// MaxSamplesToBeProcessedInEventLoop is reached. For the purpose of the single
	// live media session this should suffice
	while (sessionCount < MaxRevolutions)
	{
		for (auto mediaSubSessionPair : m_mediaSubSessionsMap)
		{
			sampleCount = 0;
			while (sampleCount < 15)
			{
				auto mediaSample = m_channelManager.GetMedia(mediaSubSessionPair.first.first,
					mediaSubSessionPair.first.second, mediaSubSessionPair.second->GetSourceId());
				
				if (mediaSample == nullptr)
				{
					break;
				}

				// make sure channel-id, channel-name and source-id are set
				mediaSample->SetChannelId(mediaSubSessionPair.first.first);
				mediaSample->SetChannelName(mediaSubSessionPair.first.second);
				mediaSample->SetSourceId(mediaSubSessionPair.second->GetSourceId());
				
				mediaSubSessionPair.second->AddMediaSample(mediaSample);

				++sampleCount;
			}
		}
		++sessionCount;
	}
}


void 
LiveSourceTaskScheduler0::
processMediaSubsessions()
{
	if (!m_mediaSubSessionsMap.empty())
	{
		auto mediaSubsessionIt = m_mediaSubSessionsMap.begin();
		while (mediaSubsessionIt != end(m_mediaSubSessionsMap))
		{
			if (!mediaSubsessionIt->second->IsAnyActiveDeviceSourcePresent()
				&& mediaSubsessionIt->second->HasServedVideoDeviceSource()
				&& !mediaSubsessionIt->second->HasBeenProcessedToKill())
			{
				// kill the channel(remote rtsp-session in camera-server for this associated subsession)
				mediaSubsessionIt->second->KillChannel();
			}

			// if there was only one obj in map & we just deleted it?
			if (m_mediaSubSessionsMap.empty())
			{
				break; // exit here
			}
			else
			{
				mediaSubsessionIt++;
			}
		}
	}
}

void
LiveSourceTaskScheduler0::
ProcessLiveMediaSessions()
{
	log_rtsp_debug("LiveSourceTaskScheduler0::ProcessLiveMediaSessions()");

	for (const auto& mediaSession : m_mediaSubSessionsMap)
	{
		mediaSession.second->ProcessClientStatistics();
	}
}

void
LiveSourceTaskScheduler0::
OnMediaReceived(std::vector<std::shared_ptr<MediaSample>> mediaSamples, CvRtsp::LiveMediaSubsession* liveMediaSubsession)
{
	//std::cout << "Media Received. Channel Id: " << mediaSample->GetChannelId() << ", Source Id: " << mediaSample->GetSourceId() << std::endl;
	if (liveMediaSubsession)
	{
		for (auto mediaSample : mediaSamples)
		{
			//std::cout << "Media Received. Channel Id: " << mediaSample->GetChannelId() << ", Source Id: " << mediaSample->GetSourceId() << std::endl;
			liveMediaSubsession->AddMediaSample(mediaSample);
		}
	}
	//std::cout << "Single Step" << std::endl;
	SingleStep(m_maxDelayTimeMicroSec);
	m_hasRun = true;

	//auto mediaSubSession = m_mediaSubSessionsMap.find(std::make_pair(mediaSample->GetChannelId(), mediaSample->GetSourceId()));
	//if (mediaSubSession != m_mediaSubSessionsMap.end())
	//{
	//	mediaSubSession->second->AddMediaSample(mediaSample);
	//	//std::cout << "Send Sample" << std::endl;
	//}
	//if (m_samplesReceived > MaxRevolutions)
	//{
	//	std::cout << "Single Step" << std::endl;
	//	SingleStep(m_maxDelayTimeMicroSec);
	//	m_hasRun = true;
	//	m_samplesReceived = 0;
	//}
	//else
	//{
	//	++m_samplesReceived;
	//}
}
