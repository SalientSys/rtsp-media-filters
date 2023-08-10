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
AddMediaSubsession(const std::string& channelName, uint32_t sourceId, LiveMediaSubsession* mediaSubsession)
{
	{
		std::stringstream ss;
		ss << "LiveSourceTaskScheduler0::AddMediaSubsession: " << channelName << " source: " << sourceId;
		log_rtsp_debug(ss.str());
	}

	const auto sessionMapIterator = m_mediaSubSessions.find(std::make_pair(channelName, sourceId));
	//auto mediaSessionMapIterator = m_mediaSessions.find(channelId);
	if (sessionMapIterator == m_mediaSubSessions.end())
	{
		// Need to add session.
		m_mediaSubSessions.emplace(std::make_pair(channelName, sourceId), mediaSubsession);
	}
}

void
LiveSourceTaskScheduler0::
RemoveMediaSubsession(const std::string& channelName, uint32_t sourceId, LiveMediaSubsession* mediaSubsession)
{
	{
		std::stringstream ss;
		ss << "Trying to remove media subsession with channel: " << channelName << " source: " << sourceId;
		log_rtsp_debug(ss.str());
	}

	const auto sessionMapIterator = m_mediaSubSessions.find(std::make_pair(channelName, sourceId));
	if (sessionMapIterator != cend(m_mediaSubSessions))
	{
		m_mediaSubSessions.erase(std::make_pair(channelName, sourceId));
	}
}

LiveMediaSubsession*
LiveSourceTaskScheduler0::
GetMediaSubsession(const std::string& channelName, uint32_t sourceId)
{
	auto mediaSubSession = m_mediaSubSessions.find(std::make_pair(channelName, sourceId));
	if (mediaSubSession != m_mediaSubSessions.end())
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
		for (auto mediaSubSession : m_mediaSubSessions)
		{
			sampleCount = 0;
			while (sampleCount < 15)
			{
				auto mediaSample = m_channelManager.GetMedia(mediaSubSession.first.first,
					mediaSubSession.first.second);
				if (mediaSample == nullptr)
				{
					break;
				}
				// make sure channel and source ids are set
				mediaSample->SetChannelName(mediaSubSession.first.first);
				mediaSample->SetSourceId(mediaSubSession.first.second);
				mediaSubSession.second->AddMediaSample(mediaSample);

				++sampleCount;
			}
		}
		++sessionCount;
	}
}

void
LiveSourceTaskScheduler0::
ProcessLiveMediaSessions()
{
	log_rtsp_debug("LiveSourceTaskScheduler0::ProcessLiveMediaSessions()");

	for (const auto& mediaSession : m_mediaSubSessions)
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

	//auto mediaSubSession = m_mediaSubSessions.find(std::make_pair(mediaSample->GetChannelId(), mediaSample->GetSourceId()));
	//if (mediaSubSession != m_mediaSubSessions.end())
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
