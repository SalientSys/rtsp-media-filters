#pragma once

#ifndef _RTSP_SERVER_HH
#include <live555/RTSPServer.hh>
#endif
#include "LiveRtspServer.h"

namespace CvRtsp
{
	/// Override implementation of RTSPClientSession in Live555 RtspServer.
	class LiveRtspClientSession : public RTSPServer::RTSPClientSession
	{
		friend class LiveRtspServer;
		friend class LiveRtspClientConnection;

	public:
		///
		/// Constructor.
		///
		/// @param[in] rtspParentServer Live rtsp server.
		/// @param[in] sessionId Session id.
		LiveRtspClientSession(LiveRtspServer& rtspParentServer, uint32_t sessionId);

		///
		/// Destructor.
		virtual ~LiveRtspClientSession();

		///
		/// Invalidates the pointer to the LiveRtspServer object.
		void Orphan();

	protected:
		/// Rtsp parent server.
		LiveRtspServer* m_rtspParentServer;

		/// Session id.
		uint32_t m_sessionId;

		///
		/// Overriding this to limit the maximum number of clients that can connect to the RTSP server.
		///
		/// @param[in] ourClientConnection Our client connection.
		/// @param[in] urlPreSuffix Suffix before the url.
		/// @param[in] urlSuffix Suffix after the url.
		/// @param[in] fullRequestStr Full url request.
		void handleCmd_SETUP(RTSPServer::RTSPClientConnection* ourClientConnection,
			char const* urlPreSuffix, char const* urlSuffix,
			char const* fullRequestStr) override;

		///
		/// Overriding this to fire off a callback informing the RTSP service that a new client has PLAYed the stream.
		///
		/// This is useful in the case of a live media pipeline as it allows e.g. IDR frame generation.
		void handleCmd_PLAY(RTSPServer::RTSPClientConnection* ourClientConnection,
			ServerMediaSubsession* subsession, char const* fullRequestStr) override;
	};
}