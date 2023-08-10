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
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///

#pragma once

#include "LiveMediaSubsession.h"

namespace CvRtsp
{
	class LiveRtspServer;

	class LiveH265Subsession : public LiveMediaSubsession
	{
	public:
		///
		/// Default constructor.
		///
		/// @param[in] env						Usage environment.
		/// @param[in] rParent					Parent Rtsp server.
		/// @param[in] uiChannelId			    Channel id
		/// @param[in] uiSourceId				Source id.
		/// @param[in] sSessionName				Session name.
		/// @param[in] vps						Video parameter set.
		/// @param[in] sps						Sequence parameter set.
		/// @param[in] pps						Picture parameter set.
		/// @param[in] pFactory					Rate adaptation factory.
		/// @param[in] pGlobalRateControl		Rate controller.
		///
		/// @return Live video device source.
		LiveH265Subsession(UsageEnvironment& env, LiveRtspServer& rParent,
			const unsigned uiChannelId, unsigned uiSourceId,
			const std::string& sSessionName,
			const std::string& sVps, const std::string& sSps, const std::string& sPps,
			IRateAdaptationFactory* pFactory,
			IRateController* pGlobalRateControl);

		///
		/// Default destructor.
		virtual ~LiveH265Subsession() = default;

	protected:
#pragma region override from LiveMediaSubsession
		///
		/// Overridden from LiveMediaSubsession
		/// 
		/// @param[in] clientSessionId			The id assigned to the client by live555 media
		/// @param[in] pMediaSampleBuffer		The media sample buffer that the device
		///										source will retrieve sample from.
		/// @param[in] pRateAdaptationFactory	Factory used to create rate adaptation module.
		/// @param[in] pRateControl				Rate control to be used for subsession. This allows the subsession to
		///										create different rate-control mechanisms based on the type of media subsession.
		///
		/// @return	Framed source.
		FramedSource* createSubsessionSpecificSource(unsigned clientSessionId,
			IMediaSampleBuffer* pMediaSampleBuffer,
			IRateAdaptationFactory* pRateAdaptationFactory,
			IRateController* pRateControl) override;

		/// Overridden from RtvcLiveMediaSubsession
		///
		/// @param[in] estBitrate	Estimated bitrate.
		void setEstimatedBitRate(unsigned& estBitrate) override;

		/// Overriding from LiveMediaSubsession
		///
		/// @param[in] rtpGroupsock				Rtp group socket.
		/// @param[in] rtpPayloadTypeIfDynamic	Specifies the payload type if the payload is dynamic.
		/// @param[in] inputSource				Framed input source.
		///
		/// @return	Rtp sink.
		RTPSink* createSubsessionSpecificRTPSink(Groupsock* rtpGroupsock,
			unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;
#pragma endregion 

	private:
		/// VPS used by decoder for this h265 stream.
		std::string m_sVps;

		/// SPS used by decoder for this h265 stream.
		std::string m_sSps;

		/// PPS used by decoder for this h265 stream.
		std::string m_sPps;
	};
}
