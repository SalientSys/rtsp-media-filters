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
/// @class PacketManagerMediaChannel
///
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///
#pragma once

#include <tbb/concurrent_queue.h>
#include "MediaChannel.h"

namespace CvRtsp
{
	///
	/// Implementation of packet manager-based media channel
	///
	/// This implementation will use the lock-free queue implementations
	/// of the tbb libraries.
	class PacketManagerMediaChannel : public MediaChannel
	{
	public:
		///
		/// Constructor
		PacketManagerMediaChannel(uint32_t channelId);

		///
		/// This method returns a video media sample if available.
		///
		/// @return Media sample, if found, nullptr if not.
		std::shared_ptr<MediaSample> GetVideo();

		///
		/// This method returns a video media sample if available, and a null pointer otherwise
		///
		/// @return Media sample, if found, nullptr if not.
		std::shared_ptr<MediaSample> GetAudio();

	private:
		/// Queue maximum capacity.
		const int QueueCapacity = 10240;

		/// Lock free queue to store video media samples.
		tbb::concurrent_bounded_queue<std::shared_ptr<MediaSample>> m_videoSamples;

		/// Lock free queue to store audio media samples.
		tbb::concurrent_bounded_queue<std::shared_ptr<MediaSample>> m_audioSamples;

		///
		/// The subclass must implement delivery of video media samples to the media sink.
		///
		/// @param[in] channelId Channel id.
		/// @param[in] mediaSamples Media samples to send.
		///
		/// @return True if sample sent.
		bool deliverVideo(uint32_t channelId, const std::vector<std::shared_ptr<MediaSample>>& mediaSamples) override;

		///
		/// The subclass must implement delivery of audio media samples to the media sink.
		///
		/// @param[in] channelId Channel id.
		/// @param[in] mediaSamples Media samples to send.
		///
		/// @return True if sample sent.
		bool deliverAudio(uint32_t channelId, const std::vector<std::shared_ptr<MediaSample>>& mediaSamples) override;
	};
}
