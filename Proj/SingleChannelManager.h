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
/// @class SingleChannelManager
///
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///
#pragma once
#include <climits>
#include "ChannelManager.h"
#include "PacketManagerMediaChannel.h"

namespace CvRtsp
{
	///
	/// This class manages a single channel with audio and video.
	/// The channel id is not used much here. It is however needed when there are 
	/// multiple channels that supply the task manager with data.
	class SingleChannelManager : public ChannelManager
	{
	public:

		/// Constructor
		///
		/// @param[in]	channelId	Channel id.
		SingleChannelManager(uint32_t channelId) :
			m_packetManager(channelId),
			m_channelId(channelId),
			m_videoSourceId(UINT_MAX),
			m_audioSourceId(UINT_MAX)
		{
		}

		/// Constructor
		///
		/// @param[in] channelId		Channel id.
		/// @param[in] videoSourceId	Video source id.
		/// @param[in] audioSourceId	Audio source id.
		SingleChannelManager(uint32_t channelId, uint32_t videoSourceId, uint32_t audioSourceId) :
			m_packetManager(channelId),
			m_channelId(channelId),
			m_videoSourceId(videoSourceId),
			m_audioSourceId(audioSourceId)
		{
		}

		///
		/// Set the video source id.
		///
		/// @param[in]	videoSourceId	Video source id.
		void SetVideoSourceId(const uint32_t videoSourceId)
		{
			assert(videoSourceId != m_audioSourceId);
			m_videoSourceId = videoSourceId;
		}

		///
		/// Set the audio source id.
		///
		/// @param[in]	audioSourceId	Audio source id.
		void SetAudioSourceId(const uint32_t audioSourceId)
		{
			assert(audioSourceId != m_videoSourceId);
			m_audioSourceId = audioSourceId;
		}

		///
		/// Get the packet manager for this channel manager.
		///
		/// @return Media channel packet manager.
		const PacketManagerMediaChannel& GetPacketManager() const
		{
			return m_packetManager;
		}

		///
		/// Get the packet manager for this channel manager.
		///
		/// @return Media channel packet manager.
		PacketManagerMediaChannel& GetPacketManager()
		{
			return m_packetManager;
		}

		///
		/// Get media.
		///
		/// @param[in]	channelId	Channel id.
		/// @param[in]	sourceId	Source id.
		///
		/// @return Media sample from the packet manager.
		std::shared_ptr<MediaSample> GetMedia(uint32_t channelId, uint32_t sourceId) override
		{
			assert(channelId == m_channelId);
			if (sourceId == m_videoSourceId)
			{
				return m_packetManager.GetVideo();
			}
			if (sourceId == m_audioSourceId)
			{
				return m_packetManager.GetAudio();
			}

			assert(false);
			return nullptr;
		}

	protected:
		/// Packet manager.
		PacketManagerMediaChannel m_packetManager;

		/// Channel id.
		uint32_t m_channelId;

		/// Video source id.
		uint32_t m_videoSourceId;

		/// Audio source id.
		uint32_t m_audioSourceId;
	};
}
