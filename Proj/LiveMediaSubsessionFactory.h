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

#include <sstream>

#include <rtsp-logger/RtspServerLogging.h>

#include "AudioChannelDescriptor.h"
#include "VideoChannelDescriptor.h"
#include "LiveAACSubsession.h"
#include "LiveAMRSubsession.h"
#include "LiveH264Subsession.h"
#include "LiveMediaSubsession.h"
#include "CommonRtsp.h"
#include "LiveMPEGSubsession.h"
#include "LiveH265Subsession.h"

namespace CvRtsp
{
	class LiveRtspServer;

	/// This class is responsible for creating the Live555 media subsession according to 
	/// passed in media subsession.
	class LiveMediaSubsessionFactory
	{
	public:

		/// This method creates the appropriate live555 media subsession.
		///
		/// @param[in] env The live555 environment.
		/// @param[in] rtspServer The live555 RTSP server.
		/// @param[in] sessionName A human readable string for the session name.
		/// @param[in] channelId The channel Id assigned by the media web server.
		/// @param[in] subsessionId A unique Id: this is currently derived
		///        from the port and subsession id of the incoming media subsession.
		/// @param[in] videoDescriptor Video descriptor.
		/// @param[in] rateAdaptationFactory Rate adaptation factory.
		/// @param[in] rateController Rate controller.
		static LiveMediaSubsession* createVideoSubsession(UsageEnvironment& env, LiveRtspServer& rtspServer,
			const std::string& sessionName, uint32_t channelId, unsigned subsessionId,
			const VideoChannelDescriptor& videoDescriptor, IRateAdaptationFactory* rateAdaptationFactory,
			IRateController* rateController)
		{
			{
				std::stringstream message;
				message << "Creating new LiveMediaSubsession " << sessionName << " Channel Id: " << channelId
					<< " Subsession Id: " << subsessionId;
				log_rtsp_debug(message.str());
			}

			LiveMediaSubsession* pMediaSubsession = nullptr;
			if (videoDescriptor.Codec == MediaSubType::H264)
			{
				pMediaSubsession = new LiveH264Subsession(env, rtspServer,
					channelId, subsessionId, sessionName,
					videoDescriptor.Sps, videoDescriptor.Pps,
					rateAdaptationFactory, rateController);
			}
			else if (videoDescriptor.Codec == MediaSubType::MPEG4)
			{
				pMediaSubsession = new LiveMPEGSubsession(env, rtspServer,
					channelId, subsessionId, sessionName,
					rateAdaptationFactory, rateController, videoDescriptor.ConfigStr, videoDescriptor.profileAndLevelIndication);
			}
			else if (videoDescriptor.Codec == MediaSubType::MJPEG)
			{
				// TODO - Yet to implement!
				//pMediaSubsession = new LiveJPEGSubsession(env, rtspServer,
				//	channelId, subsessionId, sessionName, rateAdaptationFactory, rateController);
			}
			else if (videoDescriptor.Codec == MediaSubType::H265)
			{
				pMediaSubsession = new LiveH265Subsession(env, rtspServer,
					channelId, subsessionId, sessionName,
					videoDescriptor.vps, videoDescriptor.Sps, videoDescriptor.Pps,
					rateAdaptationFactory, rateController);
			}
			else
			{
				std::stringstream message;
				message << "Invalid media subtype: " << videoDescriptor.Codec;
				log_rtsp_warning(message.str());
				pMediaSubsession = nullptr;
			}

			return pMediaSubsession;
		}

		/// This method creates the appropriate live555 media audio subsession.
		///
		/// @param[in] env The live555 environment.
		/// @param[in] rtspServer The live555 RTSP server.
		/// @param[in] sessionName A human readable string for the session name.
		/// @param[in] channelId The channel Id assigned by the media web server.
		/// @param[in] subsessionId A unique Id: this is currently derived
		///        from the port and subsession id of the incoming media subsession.
		/// @param[in] audioDescriptor Audio descriptor.
		/// @param[in] rateAdaptationFactory Rate adaptation factory.
		/// @param[in] rateController Rate controller.
		static LiveMediaSubsession* createAudioSubsession(UsageEnvironment& env, LiveRtspServer& rtspServer,
			const std::string& sessionName, const unsigned channelId, unsigned subsessionId,
			const AudioChannelDescriptor& audioDescriptor, IRateAdaptationFactory* rateAdaptationFactory,
			IRateController* rateController)
		{
			{
				std::stringstream message;
				message << "Creating new audio LiveMediaSubsession " << sessionName << " Channel Id: " << channelId
					<< " Subsession Id: " << subsessionId;
				log_rtsp_debug(message.str());
			}
			LiveMediaSubsession* pMediaSubsession = nullptr;
			if (audioDescriptor.Codec == MediaSubType::AMR)
			{
				std::stringstream message;
				message << "Adding AMR subsession: channels: " << audioDescriptor.Channels
					<< " bits per sample: " << audioDescriptor.BitsPerSample
					<< " sampling frequency: " << audioDescriptor.SamplingFrequency;
				log_rtsp_debug(message.str());

				pMediaSubsession = new LiveAMRSubsession(env, rtspServer,
					channelId, subsessionId, sessionName,
					audioDescriptor.Channels, audioDescriptor.BitsPerSample, audioDescriptor.SamplingFrequency,
					rateAdaptationFactory, rateController);
			}
			else if (audioDescriptor.Codec == MediaSubType::AAC)
			{
				std::stringstream message;
				message << "Adding AAC subsession: channels: " << audioDescriptor.Channels
					<< " bits per sample: " << audioDescriptor.BitsPerSample
					<< " sampling frequency: " << audioDescriptor.SamplingFrequency
					<< " AAC config string: " << audioDescriptor.ConfigString;
				log_rtsp_debug(message.str());

				pMediaSubsession = new LiveAACSubsession(env, rtspServer,
					channelId, subsessionId, sessionName,
					audioDescriptor.Channels, audioDescriptor.BitsPerSample, audioDescriptor.SamplingFrequency,
					audioDescriptor.ConfigString, rateAdaptationFactory, rateController);
			}
			else
			{
				std::stringstream message;
				message << "Invalid media subtype: " << audioDescriptor.Codec;
				log_rtsp_warning(message.str());
				pMediaSubsession = nullptr;
			}
			return pMediaSubsession;
		}
	};
}
