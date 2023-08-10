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
/// @class LiveRtspServer
///
/// Modified 08/28/2019
/// Modified by: M. Kinzer
///
#include "pch.h"

#include <iterator>
#include <set>
#include <string>
#include <ctime>

#include <rtsp-logger/RtspServerLogging.h>

#include "LiveRtspServer.h"
#include "LiveMediaSubsession.h"
#include "LiveMediaSubsessionFactory.h"

using namespace CvRtsp;

/// DUP from RTSPServer.cpp
// Generate a "Date:" header for use in a RTSP response:
static char const* _dateHeader() {
	static char buf[200];
#if !defined(_WIN32_WCE)
	time_t tt = time(NULL);
	struct tm timeinfo;
	gmtime_s(&timeinfo, &tt);
	//strftime(buf, sizeof buf, "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", std::to_string(tt));
#else
	// WinCE apparently doesn't have "time()", "strftime()", or "gmtime()",
	// so generate the "Date:" header a different, WinCE-specific way.
	// (Thanks to Pierre l'Hussiez for this code)
	SYSTEMTIME SystemTime;
	GetSystemTime(&SystemTime);
	WCHAR dateFormat[] = L"ddd, MMM dd yyyy";
	WCHAR timeFormat[] = L"HH:mm:ss GMT\r\n";
	WCHAR inBuf[200];
	DWORD locale = LOCALE_NEUTRAL;

	int ret = GetDateFormat(locale, 0, &SystemTime,
		(LPTSTR)dateFormat, (LPTSTR)inBuf, sizeof inBuf);
	inBuf[ret - 1] = ' ';
	ret = GetTimeFormat(locale, 0, &SystemTime,
		(LPTSTR)timeFormat,
		(LPTSTR)inBuf + ret, (sizeof inBuf) - ret);
	wcstombs(buf, inBuf, wcslen(inBuf));
#endif
	return buf;
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
	char const* streamName) {
	char* url = rtspServer->rtspURL(sms);
	UsageEnvironment& env = rtspServer->envir();
	env << "\n\"" << streamName << "\" stream\n";
	env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
}

static char const* allowedCommandNames
= "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER";

//LiveRtspServer*
//LiveRtspServer::
//CreateNew(UsageEnvironment& env, Port rtspPort,
//	UserAuthenticationDatabase* authDatabase, IRateAdaptationFactory* rateFactory,
//	IRateController* rateController)
//{
//	auto socket = -1;
//
//	do
//	{
//		socket = setUpOurSocket(env, rtspPort);
//		if (socket == -1)
//		{
//			break;
//		}
//
//		const auto rtspServer = new LiveRtspServer(env, socket, rtspPort, authDatabase, rateFactory, rateController);
//		return rtspServer;
//
//	} while (false);
//
//	if (socket != -1)
//	{
//		::closeSocket(socket);
//	}
//	return nullptr;
//}

LiveRtspServer::LiveRtspServer(UsageEnvironment& env, int socket, Port rtspPort,
	UserAuthenticationDatabase* authDatabase,
	IRateAdaptationFactory* rateFactory, IRateController* rateController) :
	RTSPServer(env, socket, rtspPort, authDatabase, 45),
	m_checkClientSessionTask(nullptr),
	m_maxConnectedClients(0),
	m_rateFactory(rateFactory),
	m_rateController(rateController)
{
	checkClientSessions();
}

LiveRtspServer::
~LiveRtspServer()
{
	// In cases where the client is still streaming when
	// the server is shutdown, the super class (RTSPServer)
	// deletes the client session objects resulting in them
	// trying to deregister themselves from LiveRtspServer.
	// We need to orphan them before to avoid crashes!
	for (const auto& rtspClientSession : m_rtspClientSessions)
	{
		rtspClientSession.second->Orphan();
	}

	// Turn off any liveness checking:
	envir().taskScheduler().unscheduleDelayedTask(m_checkClientSessionTask);
}

// fwd
static ServerMediaSession* createNewSMS(UsageEnvironment& env, LiveRtspServer& rtspServer,
	const RtspChannel& channel, IRateAdaptationFactory* rateFactory, IRateController* rateController);

void
LiveRtspServer::
AddRtspMediaSession(const RtspChannel& channel)
{
	const auto sSessionName = channel.ChannelName;

	// Next, check whether we already have an RTSP "ServerMediaSession" for this media stream:
	auto serverMediaSession = RTSPServer::lookupServerMediaSession(sSessionName.c_str());
	const auto smsExists = serverMediaSession != nullptr;

	if (!smsExists)
	{
		log_rtsp_debug("Creating Session " + sSessionName + " on RTSP server.");
		// Create a new "ServerMediaSession" object for streaming from the named file.
		serverMediaSession = createNewSMS(envir(), *this, channel, m_rateFactory, m_rateController);

		log_rtsp_debug("Adding ServerMediaSession " + sSessionName + ".");
		addServerMediaSession(serverMediaSession);

		announceStream(this, serverMediaSession, sSessionName.c_str());
	}
	else
	{
		log_rtsp_warning("Session " + sSessionName + " already exists on RTSP server.");
	}
}

void
LiveRtspServer::
RemoveRtspMediaSession(const RtspChannel& channel)
{
	/// code to kick clients before removing session so that there are no outstanding references
	endServerSession(channel.ChannelName);

	// Check whether we already have a "ServerMediaSession" for this media stream:
	const auto serverMediaSession = RTSPServer::lookupServerMediaSession(channel.ChannelName.c_str());

	if (serverMediaSession != nullptr)
	{
		log_rtsp_debug("Removed session RtspChannel Id: " + to_string(channel.ChannelId) + " Name: " + channel.ChannelName + " from RTSP ServerMediaSessions.");
		// "sms" was created for a media stream that the transcoder is no longer sending. Remove it.
		removeServerMediaSession(serverMediaSession);
	}
	else
	{
		log_rtsp_warning("Failed removing session " + channel.ChannelName + " from RTSP ServerMediaSessions");
	}
}

bool
LiveRtspServer::
DoesChannelExist(char const* streamName)
{
	const auto serverMediaSession = RTSPServer::lookupServerMediaSession(streamName);
	return serverMediaSession == nullptr ? false : true;
}

ServerMediaSession*
LiveRtspServer::
lookupServerMediaSession(char const* streamName)
{
	log_rtsp_debug("Looking up new ServerMediaSession: " + string(streamName) + ".");
	return RTSPServer::lookupServerMediaSession(streamName);
}

#define NEW_SMS(description) do {\
	char* descStr = description\
	", streamed by the LIVE555 Media Server";\
	sms = ServerMediaSession::createNew(env, fileName, fileName, descStr);\
} while(0)

static ServerMediaSession* createNewSMS(UsageEnvironment& env, LiveRtspServer& rtspServer, const RtspChannel& channel,
	IRateAdaptationFactory* pFactory, IRateController* pGlobalRateControl)
{
	log_rtsp_debug("createNewSMS: " + channel.ChannelName + ".");
	auto sms = ServerMediaSession::createNew(env, channel.ChannelName.c_str(), channel.ChannelName.c_str(),
		"Session streamed by \"SSP\"", False/*SSM*/);

	// at least a video or audio descriptor must be set
	//assert(channel.VideoDescriptor || channel.AudioDescriptor);

	// FIXME: hard-coded in application and lib for now!
	uint32_t videoId = 0;
	uint32_t audioId = 0;

	// Create the live media RTSP subsessions
	if (&channel.VideoDescriptor != nullptr)
	{
		// if there is a video channel, the audio source id increases!
		audioId = 1;
		auto liveMediaSubsession = LiveMediaSubsessionFactory::createVideoSubsession(
			env, rtspServer, channel.ChannelName, channel.ChannelId, videoId, channel.VideoDescriptor,
			pFactory, pGlobalRateControl);
		if (liveMediaSubsession == nullptr)
		{
			log_rtsp_error("Invalid video subsession.");
			Medium::close(sms);
			return nullptr;
		}

		log_rtsp_debug("Added " + channel.ChannelName + " to video ServerMediaSession.");
		sms->addSubsession(liveMediaSubsession);

		liveMediaSubsession->SetClientJoinHandler(std::bind(&LiveRtspServer::OnClientJoin, &rtspServer,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		liveMediaSubsession->SetClientUpdateHandler(std::bind(&LiveRtspServer::OnClientUpdate, &rtspServer,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		liveMediaSubsession->SetClientLeaveHandler(std::bind(&LiveRtspServer::OnClientLeave, &rtspServer,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	// TODO - Need to implement audio support!
	// create the live media RTSP subsessions
	//if (&channel.AudioDescriptor != nullptr)
	//{
	//	LiveMediaSubsession* pLiveMediaSubsession = LiveMediaSubsessionFactory::createAudioSubsession(
	//		env, rtspServer, channel.ChannelName, channel.ChannelId, uiAudioId, channel.AudioDescriptor,
	//		pFactory, pGlobalRateControl);
	//	if (pLiveMediaSubsession == NULL)
	//	{
	//		log_rtsp_error("Invalid audio subsession");
	//		Medium::close(sms);
	//		return NULL;
	//	}
	//	else
	//	{
	//		log_rtsp_debug("Added " + channel.ChannelName + " audio to ServerMediaSession.");
	//		sms->addSubsession(pLiveMediaSubsession);

	//		pLiveMediaSubsession->SetClientJoinHandler(std::bind(&LiveRtspServer::onClientJoin, &rtspServer,
	//			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	//		pLiveMediaSubsession->SetClientUpdateHandler(std::bind(&LiveRtspServer::onClientUpdate, &rtspServer,
	//			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	//		pLiveMediaSubsession->SetClientLeaveHandler(std::bind(&LiveRtspServer::onClientLeave, &rtspServer,
	//			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	//	}
	//}

	return sms;
}

void
LiveRtspServer::
checkClientSessions()
{
	envir().taskScheduler()
		.rescheduleDelayedTask(m_checkClientSessionTask, 1000000, // in microseconds
			reinterpret_cast<TaskFunc*>(clientSessionsTask), this);

}

void
LiveRtspServer::
clientSessionsTask(LiveRtspServer* rtspServer)
{
	rtspServer->doCheckClientSessions();
}

void
LiveRtspServer::
doCheckClientSessions()
{
	// Process all receiver reports

	// schedule next task
	checkClientSessions();
}

void
LiveRtspServer::
kickClient(uint32_t clientId)
{
	// find corresponding client session
	auto clientSessionIterator = m_rtspClientSessions.find(clientId);
	if (clientSessionIterator != m_rtspClientSessions.end())
	{
		log_rtsp_information("Kicking client - " + std::to_string(clientId) + ".");
		// according to live555 mailing list, deletion of a client session cleanly removes the session.
		delete clientSessionIterator->second;
	}
	else
	{
		// couldn't find client in map
		log_rtsp_warning("Unable to kick client - not found " + std::to_string(clientId) + ".");
	}
}

void
LiveRtspServer::
endServerSession(const std::string& session)
{
	// Next, check whether we already have an RTSP "ServerMediaSession" for this media stream:
	const auto serverMediaSession = RTSPServer::lookupServerMediaSession(session.c_str());

	if (serverMediaSession)
	{
		log_rtsp_information("Kicking all clients from RTSP media session: " + session + ".");

		// First kick all clients so that we can remove the subsession and dereg from the scheduler
		// The LiveMediaSubsession objects can only be reclaimed once there are no outstanding references
		ServerMediaSubsessionIterator iter(*serverMediaSession);
		auto mediaSubSession = iter.next();

		// stores set of unique ids
		std::set<unsigned> uniqueClientIds;

		while (mediaSubSession != nullptr)
		{
			// Kick all clients
			LiveMediaSubsession* liveMediaSubSession = static_cast<LiveMediaSubsession*>(mediaSubSession);
			// assuming our server only contains LiveMediaSubsession
			// in the case that it does not, we could use dynamic_cast but this shouldn't be the case
			std::vector<uint32_t> vClientIds = liveMediaSubSession->GetConnectedClientIds();
			// the live555 assigns the same client id to the audio and video subsession
			// this next part could be superfluous since the first subsession SHOULD contain all IDs
			// but playing it on the safe side for now
			std::copy(vClientIds.begin(), vClientIds.end(), inserter(uniqueClientIds, uniqueClientIds.end()));
			mediaSubSession = iter.next();
		}

		// now kick all clients
		for (auto uiId : uniqueClientIds)
		{
			kickClient(uiId);
		}

		log_rtsp_debug("Kicking complete: " + session + ".");
	}
	else
	{
		log_rtsp_warning("Unable to find session " + session + ".");
	}
}

void
LiveRtspServer::
onRtspClientSessionPlay(uint32_t clientSessionId)
{
	if (m_onClientSessionPlay)
	{
		m_onClientSessionPlay(clientSessionId);
	}
}

void
LiveRtspServer::
OnClientJoin(uint32_t channelId, uint32_t sourceId, uint32_t clientId, std::string& ipAddress)
{
	log_rtsp_information("Client joined: RtspChannel: " + std::to_string(channelId) + ":"
		+ std::to_string(sourceId) + " client ID: " + std::to_string(clientId) + " IP: " + ipAddress + ".");
}

void
LiveRtspServer::
OnClientUpdate(uint32_t channelId, uint32_t sourceId, uint32_t clientId, uint32_t channelIndex)
{
	log_rtsp_information("Client joined: RtspChannel: " + std::to_string(channelId) + ":"
		+ std::to_string(sourceId) + " client ID: " + std::to_string(clientId) + " RtspChannel index: " + std::to_string(channelIndex) + ".");
}

void
LiveRtspServer::
OnClientLeave(uint32_t channelId, uint32_t sourceId, uint32_t clientId)
{
	log_rtsp_information("Client left: RtspChannel: " + std::to_string(channelId) + ":"
		+ std::to_string(sourceId) + " client ID: " + std::to_string(clientId) + ".");
}

RTSPServer::RTSPClientConnection*
LiveRtspServer::
createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr)
{
	return new LiveRtspClientConnection(*this, clientSocket, clientAddr);
}

RTSPServer::RTSPClientSession*
LiveRtspServer::
createNewClientSession(uint32_t sessionId)
{
	const auto clientSession = new LiveRtspClientSession(*this, sessionId);
	m_rtspClientSessions[sessionId] = clientSession;
	return clientSession;
}

void
LiveRtspServer::
removeClientSession(uint32_t sessionId)
{
	const auto sessionMapIterator = m_rtspClientSessions.find(sessionId);
	if (sessionMapIterator != m_rtspClientSessions.end())
	{
		log_rtsp_information("Removing client session " + std::to_string(sessionId) + ".");
		m_rtspClientSessions.erase(sessionMapIterator);
	}
	else
	{
		log_rtsp_warning("Unable to remove client session " + std::to_string(sessionId) + ".");
	}
}
