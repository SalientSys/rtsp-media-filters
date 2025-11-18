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
/// @class LiveMediaSubsession::
///
/// Modified 08/16/2019
/// Modified by: M. Kinzer
///

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <boost/uuid/uuid.hpp>

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include <live555/OnDemandServerMediaSubsession.hh>
#endif
#include "MediaSample.h"


namespace CvRtsp
{
	/// Forward class declarations.
	class IMediaSampleBuffer;
	class LiveDeviceSource;
	class LiveRtspServer;
	class IRateAdaptationFactory;
	class IRateController;

#pragma region typedefs
	/// Live device source vector.
	using LiveDeviceSourceList = std::vector<LiveDeviceSource*>;

	/// Channel id, source id, client id, IP address
	using ClientJoinHandler = std::function<void(const boost::uuids::uuid&, uint32_t, uint32_t, std::string&)>;

	/// Channel id, source id, client id, channel index
	using ClientUpdateHandler = std::function<void(const boost::uuids::uuid&, uint32_t, uint32_t, uint32_t)>;

	/// Channel id, source id, client id
	using ClientLeaveHandler = std::function<void(const boost::uuids::uuid&, uint32_t, uint32_t)>;
#pragma endregion

	/// 
	/// Base class for live media subsessions.
	///
	/// This class is the entry point into the live555 class hierarchy:
	/// - createNewStreamSource and createNewRTPSink allow overriding of
	///	source and sink creation.
	///
	/// - getStreamParameters and deleteStream allow overriding for the
	/// purpose of client connection management createSubsessionSpecificSource() 
	/// and createSubsessionSpecificRTPSink() .
	class LiveMediaSubsession : public OnDemandServerMediaSubsession
	{
		friend class LiveDeviceSource;

	private:
		///
		/// Helper function to cleanup before destroying this subsession.
		void cleanup();

	public:
		/// Destructor
		virtual ~LiveMediaSubsession();

		///
		/// Getter for source-id, video / audio source ?
		///
		/// @return Source id.
		inline uint32_t GetSourceId() const
		{
			return m_sourceId;
		}

		///
		/// Retrieve RTSP session name.
		///
		/// @return RTSP session name.
		std::string GetSessionName() const
		{
			return m_sessionName;
		}

		///
		/// Is this a video subsession.
		///
		/// @return True if this is a video subsession.
		bool IsVideo() const
		{
			return m_isVideo;
		}

		///
		/// Is this a audio subsession.
		///
		/// @return True if this is a video subsession.
		bool IsAudio() const
		{
			return !m_isVideo;
		}

		///
		/// Subsession is switchable if there is more than one channel.
		/// 
		/// @return True if subsession is switchable.
		bool IsSwitchable() const
		{
			return m_totalChannels > 1;
		}

		///
		/// Add a media sample to the subsession.
		///
		/// @param[in] mediaSample Media sample.
		virtual void AddMediaSample(const std::shared_ptr<MediaSample>& mediaSample);

		///
		/// This method processes the received receiver reports.
		void ProcessClientStatistics();

		///
		/// Getter for connected client ids
		///
		/// @return Vector of connected client ids.
		std::vector<uint32_t> GetConnectedClientIds() const;

		///
		/// Setter for client join callbacks
		///
		/// @param[in] onJoin Client join handler.
		void SetClientJoinHandler(ClientJoinHandler onJoin)
		{
			m_onJoin = onJoin;
		}

		///
		/// Setter for client update callbacks
		///
		/// @param[in] onUpdate Client update handler.		
		void SetClientUpdateHandler(ClientUpdateHandler onUpdate)
		{
			m_onUpdate = onUpdate;
		}

		///
		/// Setter for client leave callbacks
		///
		/// @param[in] onLeave Client leave handler.
		void SetClientLeaveHandler(ClientLeaveHandler onLeave)
		{
			m_onLeave = onLeave;
		}

		///
		/// Check if there are any device-sources using this subsession.
		/// 
		/// @return true if there is any active device-source, else false.
		inline bool IsAnyActiveDeviceSourcePresent() const
		{
			return m_deviceSources.empty() == false;
		}

		///
		/// Getter for if this subsession has served any video device-source with mediasample.
		inline bool HasServedVideoDeviceSource() const
		{
			return m_hasServedAnyVideoDeviceSource;
		}

		///
		/// Ends channel associated with this subsession.
		void KillChannel();

		///
		/// Getter for whether this subsession has been processed to kill.
		inline bool HasBeenProcessedToKill() const
		{
			return m_hasBeenProcessedToKill;
		}

	protected:
		///
		/// Register live device source with subsession.
		///
		/// @param[in] deviceSource Live device source.
		void addDeviceSource(LiveDeviceSource* deviceSource);

		///
		/// De-register live device source from subsession.
		///
		/// @param[in] deviceSource Live device source.
		void removeDeviceSource(LiveDeviceSource* deviceSource);

		///
		/// Constructor
		///
		/// The channelId + sourceID is used to allow this register itself with the scheduler on construction
		/// and to deregister itself from the scheduler on destruction
		///
		/// @param[in] env Usage environment object.
		/// @param[in] liveParentRtspServer Parent rtsp server.
		/// @param[in] channelId Channel id.
		/// @param[in] sourceId Source id.
		/// @param[in] sessionName Session Name.
		/// @param[in] isVideo True if live media is of type video.
		/// @param[in] totalChannels Total channels in the subsession.
		/// @param[in] isSwitchableFormat True if formats are switchable.
		/// @param[in] rateAdaptationFactory Factory used to create rate adaptation module.
		/// @param[in] rateController Rate control to be used for subsession. This allows the subsession to
		/// create different rate-control mechanisms based on the type of media subsession.
		LiveMediaSubsession(UsageEnvironment& env, LiveRtspServer& liveParentRtspServer, const boost::uuids::uuid &channelId,
			uint32_t sourceId, const std::string& sessionName, bool isVideo, unsigned totalChannels = 1,
			bool isSwitchableFormat = false, IRateAdaptationFactory* rateAdaptationFactory = nullptr,
			IRateController* rateController = nullptr);

		///
		/// This method must be overridden by subclasses.
		/// 
		/// @param[in] clientSessionId The id assigned to the client by live555
		/// @param[in] mediaSampleBuffer [in] The media sample buffer that the device
		///		source will retrieve sample from
		/// @param[in] rateAdaptationFactory Factory used to create rate adaptation module.
		/// @param[in] rateController Rate control to be used for subsession. This allows the subsession to
		///		create different rate-control mechanisms based on the type of media subsession.
		///
		/// @return Framed source.
		virtual FramedSource* createSubsessionSpecificSource(uint32_t clientSessionId, IMediaSampleBuffer* mediaSampleBuffer,
			IRateAdaptationFactory* rateAdaptationFactory, IRateController* rateController) = 0;

		///
		/// This method must be overridden by subclasses
		///
		/// @param[in] rtpGroupsock Rtp group socket.
		/// @param[in] rtpPayloadTypeIfDynamic Specifies the payload type if the payload is dynamic.
		/// @param[in] inputSource Framed input source.
		///
		/// @return Rtp sink.
		virtual RTPSink* createSubsessionSpecificRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
			FramedSource* inputSource) = 0;

		///
		/// Subclasses must override this instead of createNewStreamSource
		///
		/// @param[in] estBitrate Estimated bitrate.
		virtual void setEstimatedBitRate(uint32_t& estBitrate) = 0;

		///
		/// Overridden from OnDemandServermediaSubsession for RTP source creation
		///
		/// @param[in] clientSessionId Client session id.
		/// @param[in] estBitrate Estimated bitrate.
		///
		/// @return Framed source.
		FramedSource* createNewStreamSource(uint32_t clientSessionId, uint32_t& estBitrate) override;

		///
		/// Overridden from OnDemandServermediaSubsession for RTP sink creation. "estBitrate"
		/// is the stream's estimated bitrate, in kbps
		///
		/// @param[in] rtpPayloadTypeIfDynamic Specifies the payload type if the payload is dynamic.
		/// @param[in] inputSource Framed input source.
		///
		/// @return New rtp sink. 
		RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;

		///
		/// Overridden so that we can store the client connection info of connecting RTP clients
		void getStreamParameters(uint32_t clientSessionId,
			struct sockaddr_storage const& clientAddress,
			Port const& clientRTPPort,
			Port const& clientRTCPPort,
			int tcpSocketNum,
			unsigned char rtpChannelId,
			unsigned char rtcpChannelId,
			TLSState* tlsState,
			struct sockaddr_storage& destinationAddress,
			u_int8_t& destinationTTL,
			Boolean& isMulticast,
			Port& serverRTPPort,
			Port& serverRTCPPort,
			void*& streamToken) override;

		///
		/// Overridden so that we can manage connecting client info
		///
		/// @param[in] clientSessionId Client session id.
		/// @param[in] streamToken Stream token.
		void deleteStream(uint32_t clientSessionId, void*& streamToken) override;

		/// Rtsp server
		LiveRtspServer& m_rtspServer;

		/// Unique channel Id: channels are assigned by the media web server
		boost::uuids::uuid m_channelId;

		/// Session ID that is used to register the media subsession with the scheduler
		/// The channel ID is not sufficient since a single channel will have at least 
		/// one audio and one video subsession
		uint32_t m_sourceId;

		/// RTSP session name
		std::string m_sessionName;

		/// RTP clients listed for this subsession 
		LiveDeviceSourceList m_deviceSources;

		/// video or audio: helps find applicable sessions
		bool m_isVideo;

		/// Total number of channels: more than one implies that a parallel sample buffer will be created
		/// The term 'channel' in this context is not to be confused with the channels in the audio media subtype
		unsigned m_totalChannels;

		/// Buffer for the samples
		IMediaSampleBuffer* m_sampleBuffer;

		/// Rate adaptation factory
		IRateAdaptationFactory* m_rateAdaptationFactory;

		/// Global rate control: depending on the type of rate control i.e. per source or per client.
		IRateController* m_globalRateControl;

		/// Callback for client joins
		ClientJoinHandler m_onJoin;

		/// Callback for client updates
		ClientUpdateHandler m_onUpdate;

		/// Callback for client leaves
		ClientLeaveHandler m_onLeave;

		/// Tracks if this subsession was serving any video device-source.
		bool m_hasServedAnyVideoDeviceSource;

		/// Has this been processed to kill already?
		/// KE @TODO - this is a hack so that we dont break consistency. Not a big fan of this!
		/// So need to update later.
		/// Background on above: In LiveSourceTaskScheduler0::processMediaSubsessions(), it processes
		/// faster than we can stop remote-session in our capture-server & call CvRtspServer::RemoveChannel().
		/// We could do this from deleting this subsession from here, but the calls would be incosistent since we would 
		/// be ending some sessions from here & some from there depending on situation, so this hack.
		bool m_hasBeenProcessedToKill;
	};
}
