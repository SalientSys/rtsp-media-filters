#include "pch.h"

#include <cassert>

#include <rtsp-logger/RtspServerLogging.h>

#include "LiveRtspClientSession.h"
#include "LiveRtspClientConnection.h"

using namespace CvRtsp;

LiveRtspClientSession::
LiveRtspClientSession(LiveRtspServer& rtspParentServer, uint32_t sessionId) :
	RTSPClientSession(rtspParentServer, sessionId),
	m_rtspParentServer(&rtspParentServer),
	m_sessionId(sessionId)
{

}

LiveRtspClientSession::
~LiveRtspClientSession()
{
	// We need to check if the parent is still valid
	// in the case where the client session outlives the
	// RTSPServer child class implementation! In that case
	// the RTSPServer destructor deletes all the client 
	// sessions, but at this point m_rtspParentServer is not valid 
	// anymore. This is the reason for the orphan method.
	if (m_rtspParentServer)
	{
		m_rtspParentServer->removeClientSession(m_sessionId);
	}
}

void
LiveRtspClientSession::
Orphan()
{
	m_rtspParentServer = nullptr;
}

void
LiveRtspClientSession::
handleCmd_SETUP(RTSPServer::RTSPClientConnection* ourClientConnection,
	char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr)
{
	// "urlPreSuffix" should be the session (stream) name, and
	// "urlSuffix" should be the subsession (track) name.

	if (m_rtspParentServer->GetMaxConnectedClients() == 0)
	{
		// no limitations set: let base class handle it
		RTSPClientSession::handleCmd_SETUP(ourClientConnection, urlPreSuffix, urlSuffix, fullRequestStr);
		return;
	}

	auto streamName = urlPreSuffix;
	auto trackId = urlSuffix;

	// Check whether we have existing session state, and, if so, whether it's
	// for the session that's named in "streamName".  (Note that we don't
	// support more than one concurrent session on the same client connection.) #####
	if (fOurServerMediaSession == nullptr)
	{
		// this is a new session: reject the user if we have reached the server limit
		if (m_rtspParentServer->GetNumberOfConnectedClients() <= m_rtspParentServer->GetMaxConnectedClients())
		{
			RTSPClientSession::handleCmd_SETUP(ourClientConnection, urlPreSuffix, urlSuffix, fullRequestStr);
			//VLOG(2) << "TODO: parse request str to get user: " << fullRequestStr;
			return;
		}

		auto clientConnection = dynamic_cast<LiveRtspClientConnection*>(ourClientConnection);
		assert(clientConnection);

		// Convert IpAddress to readable format
		struct in_addr destinationAddr; destinationAddr.s_addr = clientConnection->getClientAddr().sin_addr.s_addr;
		std::string sIpAddress(inet_ntoa(destinationAddr));
		log_rtsp_warning("Max connections (" + std::to_string(m_rtspParentServer->GetMaxConnectedClients()) + ") reached. Rejecting connection request from " + sIpAddress + ".");
		clientConnection->handleCmd_notEnoughBandwidth();
	}
	else
	{
		// SETUP in existing session: let base class handle it
		RTSPClientSession::handleCmd_SETUP(ourClientConnection, urlPreSuffix, urlSuffix, fullRequestStr);
		//VLOG(2) << "TODO: parse request str to get user: " << fullRequestStr;
	}
}

void
LiveRtspClientSession::
handleCmd_PLAY(RTSPServer::RTSPClientConnection* ourClientConnection, ServerMediaSubsession* subsession,
	char const* fullRequestStr)
{
	m_rtspParentServer->onRtspClientSessionPlay(m_sessionId);
	RTSPClientSession::handleCmd_PLAY(ourClientConnection, subsession, fullRequestStr);
}
