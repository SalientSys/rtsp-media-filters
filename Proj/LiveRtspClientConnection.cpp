#include "pch.h"

#include "LiveRtspClientSession.h"
#include <iostream>
#include <boost/algorithm/string/constants.hpp>
#include <boost/tokenizer.hpp>
#include <boost/variant/detail/substitute.hpp>


using namespace CvRtsp;
using namespace boost::algorithm;


LiveRtspClientConnection::
LiveRtspClientConnection(LiveRtspServer& ourServer, int clientSocket, struct sockaddr_storage const& clientAddr) :
	RTSPServer::RTSPClientConnection(ourServer, clientSocket, clientAddr),
	m_rtspParentServer(&ourServer)
{
}


void
LiveRtspClientConnection::
handleCmd_notEnoughBandwidth()
{
	setRTSPResponse("453 Not Enough Bandwidth");
}

void
LiveRtspClientConnection::
handleCmd_OPTIONS()
{
	RTSPClientConnection::handleCmd_OPTIONS();
}


void
LiveRtspClientConnection::
handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr)
{
	assert(false);
	// Should be overriden in child class.
}
