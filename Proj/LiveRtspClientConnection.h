#pragma once

#ifndef _RTSP_SERVER_HH
#include <live555/RTSPServer.hh>
#endif
#include "LiveRtspServer.h"

namespace CvRtsp
{
	///
	/// Subclassing this to make the client address accessible and add handleCmd_notEnoughBandwidth.
	class LiveRtspClientConnection : 
		public RTSPServer::RTSPClientConnection
	{
		friend class LiveRtspServer;
		friend class LiveRtspClientSession;

	public:
		///
		/// Constructor.
		LiveRtspClientConnection(LiveRtspServer& ourServer, int clientSocket, struct sockaddr_storage const& clientAddr);

		///
		/// Default destructor.
		virtual ~LiveRtspClientConnection() = default;

	protected:
		/// Rtsp parent server.
		LiveRtspServer* m_rtspParentServer;

		///
		/// Getter for client address
		struct sockaddr_storage getClientAddr() const
		{
			return fClientAddr;
		}

		///
		/// This method can be called to respond to requests where there is insufficient bandwidth
		/// to handle them.
		virtual void handleCmd_notEnoughBandwidth();

		///
		/// Overriding for handling options command.
		void handleCmd_OPTIONS() override;

		///
		/// Overriding this to limit the maximum number of clients that can connect to the RTSP server
		/// 
		/// @param[in]	urlPreSuffix	Suffix before the url.
		/// @param[in]	urlSuffix		Suffix after the url.
		/// @param[in]	fullRequestStr	Full url request.
		void handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr) override;
	};
}