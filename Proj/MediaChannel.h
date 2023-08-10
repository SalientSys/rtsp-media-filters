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
/// @class MediaChannel
///
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///

#pragma once
#include <cstdint>
#include <vector>

#include "MediaSample.h"

namespace CvRtsp
{
	/// A MediaChannel is comprised of an audio and a video channel.
	/// The MediaChannel abstracts the delivery of media from a source to some
	/// media sink.
	/// 
	/// A MediaChannel is identified by the channel ID. 
	class MediaChannel
	{
	public:

		///Constructor.
		///
		/// @param channelId Media channel id.
		MediaChannel(const std::string& channelName) :
			m_channelName(channelName)
		{
		}

		/// Destructor.
		virtual ~MediaChannel() = default;

		/// The addVideoMediaSamples() can be called to deliver media samples to 
		/// the media sink.
		///
		/// @param mediaSamples Vector containing media samples.
		///
		/// @return True if delivery successful.
		bool AddVideoMediaSamples(const std::vector<std::shared_ptr<MediaSample>>& mediaSamples)
		{
			return deliverVideo(m_channelName, mediaSamples);
		}

		/// The addAudioMediaSamples() can be called to deliver media samples to 
		/// the media sink.
		///
		/// @param mediaSamples Vector containing audio samples.
		///
		/// @return True if delivery successful.
		bool AddAudioMediaSamples(const std::vector<std::shared_ptr<MediaSample>>& mediaSamples)
		{
			return deliverAudio(m_channelName, mediaSamples);
		}

	private:
		/// The subclass must implement delivery of video media samples to the media sink
		///
		/// @param channelId Channel id.
		/// @param mediaSamples Vector containing video samples.
		///
		/// @return True if delivery successful.
		virtual bool deliverVideo(const std::string& channelName, const std::vector<std::shared_ptr<MediaSample>>& mediaSamples) = 0;

		/// The subclass must implement delivery of audio media samples to the media sink
		///
		/// @param channelId Channel id.
		/// @param mediaSamples Vector containing audio samples.
		///
		/// @return True if delivery successful.
		virtual bool deliverAudio(const std::string& channelName, const std::vector<std::shared_ptr<MediaSample>>& mediaSamples) = 0;

		/// Unique channel id
		std::string m_channelName;
	};
}

