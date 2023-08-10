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
#pragma once

#include <map>
#include <vector>

#ifndef _RTSP_SERVER_HH
#include <live555/RTSPServer.hh>
#endif
#include <live555/BasicUsageEnvironment.hh>

#include "AudioChannelDescriptor.h"
#include "VideoChannelDescriptor.h"
#include "LiveRtspClientConnection.h"
#include "LiveRtspClientSession.h"

namespace CvRtsp
{
	/// Forward 
	class IRateAdaptationFactory;
	class IRateController;

	/// Callback for when a client session is PLAYed.
	/// Parameter is the client session id.
	using OnClientSessionPlayHandler = std::function<void(unsigned)>;

	/// RtspChannel descriptor
	struct RtspChannel
	{
		RtspChannel() :
			ChannelId(0)
		{

		}

		RtspChannel(uint32_t channelId, const std::string& channelName,
			VideoChannelDescriptor video, AudioChannelDescriptor audio) :
			ChannelId(channelId),
			ChannelName(channelName),
			VideoDescriptor(video),
			AudioDescriptor(audio)
		{

		}

		RtspChannel(uint32_t channelId, const std::string& channelName, VideoChannelDescriptor video) :
			ChannelId(channelId),
			ChannelName(channelName),
			VideoDescriptor(video)
		{

		}

		RtspChannel(uint32_t channelId, const std::string& channelName, AudioChannelDescriptor audio) :
			ChannelId(channelId),
			ChannelName(channelName),
			AudioDescriptor(audio)
		{

		}

		uint32_t ChannelId;
		std::string ChannelName;
		VideoChannelDescriptor VideoDescriptor;
		AudioChannelDescriptor AudioDescriptor;
	};

	/// Our RTSP server class is derived from the liveMedia RTSP server. It extends the live555 RTSP server
	/// to stream live media sessions.
	///
	/// It also adds the capability to set the maximum number of connected clients.
	/// It adds the ability to kick clients off the server.
	class LiveRtspServer : public RTSPServer
	{
		friend class LiveRtspClientSession;

	public:
		///
		/// Constructor: called only by createNew();
		LiveRtspServer(UsageEnvironment& env, int socket, Port rtspPort,
			UserAuthenticationDatabase* authDatabase, IRateAdaptationFactory* rateFactory,
			IRateController* rateController);

		///
		/// Destructor.
		virtual ~LiveRtspServer();

		/////
		///// Named constructor
		/////
		///// @param[in] env Usage environment.
		///// @param[in] rtspPort Rtsp port. Default if not provided, is 554.
		///// @param[in] authDatabase Authentication mechanism.
		///// @param[in] rateFactory Rate adaptation factory.
		///// @param[in] rateController Rate controller.
		/////
		///// @return Live, dynamic Rtsp server.
		//static LiveRtspServer* CreateNew(UsageEnvironment& env, Port rtspPort = 554,
		//	UserAuthenticationDatabase* authDatabase = nullptr,
		//	IRateAdaptationFactory* rateFactory = nullptr, IRateController* rateController = nullptr);

		///
		/// Get the number of maximum allowed connected clients.
		///
		/// @return Maximum allowed connected clients.
		uint32_t GetMaxConnectedClients() const
		{
			return m_maxConnectedClients;
		}

		///
		/// Set the number of maximum allowed connected clients.
		///
		/// @param[in] maxAllowedClients Maximum allowed clients.
		void SetMaxConnectedClients(uint32_t maxAllowedClients)
		{
			m_maxConnectedClients = maxAllowedClients;
			/// TODO: if val is less than previous value kick oldest clients to meet requirement)
		}

		///
		/// Get the number of connected clients.
		///
		/// @return Number of connected clients.
		uint32_t GetNumberOfConnectedClients() const
		{
			return static_cast<uint32_t>(m_rtspClientSessions.size());
		}

		///
		/// Adds the Rtsp session described by channel to the Rtsp server if it does not already exist.
		///
		/// @param[in] channel RtspChannel to add to the Rtsp media session.
		void AddRtspMediaSession(const RtspChannel& channel);

		///
		/// Removes the Rtsp session described by channel from the Rtsp server.
		///
		/// @param[in] channel RtspChannel to remove to the Rtsp media session.
		void RemoveRtspMediaSession(const RtspChannel& channel);

		///
		/// Set the notification callback when a client session is issues the PLAY command to the Rtsp server.
		///
		/// @param[in] onClientSessionPlay Handler for the Client Session Play command.
		void SetOnClientSessionPlayCallback(OnClientSessionPlayHandler onClientSessionPlay)
		{
			m_onClientSessionPlay = onClientSessionPlay;
		}

		///
		/// Handler to be called when clients join.
		///
		/// @param[in] channelId Channel id.
		/// @param[in] sourceId Source id.
		/// @param[in] clientId Client id.
		/// @param[in] ipAddress Ip address.
		void OnClientJoin(uint32_t channelId, uint32_t sourceId, uint32_t clientId, std::string& ipAddress);

		///
		/// Handler to be called when clients update.
		///
		/// @param[in] channelId Channel id.
		/// @param[in] sourceId Source id.
		/// @param[in] clientId Client id.
		/// @param[in] channelIndex Channel index.
		void OnClientUpdate(uint32_t channelId, uint32_t sourceId, uint32_t clientId, uint32_t channelIndex);

		///
		/// Handler to be called when clients leave.
		///
		/// @param[in] channelId Channel id.
		/// @param[in] sourceId Source id.
		/// @param[in] clientId Client id.
		void OnClientLeave(uint32_t channelId, uint32_t sourceId, uint32_t clientId);

		///
		/// Check to see if a requested channel exists.
		///
		/// @param[in] streamName Name of the media channel.
		///
		/// @return True if stream exists.
		bool DoesChannelExist(char const* streamName);
	protected:
		///
		/// Ends the server session.
		///
		/// @param[in] sessionName Server session name.
		void endServerSession(const std::string& sessionName);

		///
		/// This is called when the client session processes the RTSP PLAY request.
		///
		/// @param[in] clientSessionId Client session id.
		void onRtspClientSessionPlay(uint32_t clientSessionId);

		///
		/// If you subclass "RTSPClientConnection", then you must also redefine this virtual function in order
		/// to create new objects of your subclass:
		///
		/// @param[in] clientSocket Client socket.
		/// @param[in] clientAddr Client ip address.
		///
		/// @return Rtsp client connection.
		RTSPClientConnection* createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr) override;

		///
		/// If you subclass "RTSPClientSession", then you must also redefine this virtual function in order
		/// to create new objects of your subclass:
		///
		/// @param[in] sessionId Session id.
		///
		/// @return Rtsp client session.
		RTSPClientSession* createNewClientSession(uint32_t sessionId) override;

		///
		/// Remove existing client session.
		///
		/// @param[in] sessionId Session id.
		void removeClientSession(uint32_t sessionId);

	private:
		/// Live client session map definition.
		using LiveClientSessionMap = std::map<uint32_t, LiveRtspClientSession*>;

		/// Map to store a pointer to client sessions on creation.
		LiveClientSessionMap m_rtspClientSessions;

		/// A task to check for new client sessions.
		TaskToken m_checkClientSessionTask;

		/// Maximum permitted number of clients that can be served by this Rtsp server,
		/// a value of 0 means that no restrictions exist.
		uint32_t m_maxConnectedClients;

		/// Adaptation factory. 
		IRateAdaptationFactory* m_rateFactory;

		/// Rate controller.
		IRateController* m_rateController;

		/// On client session play callback
		OnClientSessionPlayHandler m_onClientSessionPlay;

		/// Redefined virtual functions: this method returns the session identified by streamName provided its valid
		virtual ServerMediaSession* lookupServerMediaSession(char const* streamName);

		///
		/// Kicks clients from the server
		/// this method SHOULD only be called from within the live555 eventloop!
		///
		/// @param[in] clientId Client id.
		void kickClient(uint32_t clientId);

		///
		/// Method to add a task to the task scheduler to add a delayed scheduled task to
		/// check for new client sessions.
		void checkClientSessions();

		///
		/// Check client sessions.
		void doCheckClientSessions();

		///
		/// Method run with the task scheduler runs the delayed task.
		///
		/// @param[in] rtspServer Live rtsp server.
		static void clientSessionsTask(LiveRtspServer* rtspServer);
	};

}