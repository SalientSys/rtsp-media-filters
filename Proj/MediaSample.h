/** @file

MODULE				: MediaSample

FILE NAME			: MediaSample.h

DESCRIPTION			:

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, Meraka Institute
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the Meraka Institute nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===========================================================================
*/
///
/// @class MediaSample
///
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///

#pragma once

#include <string>
#include "Buffer.h"

namespace CvRtsp
{
	/// Encapsulates raw media data, size and start time of the sample
	/// Extended Media Sample to have a sync point: this can be used as a flag to say this is the first i-frame
	/// Or that this is the first sample that has been RTCP synchronized.
	class MediaSample
	{
	public:
		///
		/// Copy constructor.
		MediaSample(const MediaSample& mediaSample);

		///
		/// Disallow assignment constructor.
		MediaSample& operator=(const MediaSample& mediaSample) = delete;

		///
		/// Default destructor.
		~MediaSample() = default;

		/// 
		/// Create a media sample.
		/// 
		/// @param[in] data			Data buffer.
		/// @param[in] size			Size of the data buffer.
		/// @param[in] startTime	Start time of the data stream.
		/// @param[in] isKeyFrame	True, if this a keyframe.
		/// @param[in] channelId	Channel id.
		/// @param[in] sourceId		Source id.
		/// @param[in] isSyncPoint	True if this is a marker.
		///
		/// @return Media sample.
		static std::shared_ptr<MediaSample> CreateMediaSample(BYTE* data, int size, double startTime,
			bool isKeyFrame = false, uint32_t channelId = 0, uint32_t sourceId = 0, bool isSyncPoint = false);

		/// 
		/// Data buffer contained in this media sample.
		///
		/// @return Buffer reference.
		const Buffer& GetDataBuffer() const
		{
			return m_data;
		}

		///
		/// Return size of the media sample.
		///
		/// @return Size of media sample.
		int GetSize() const
		{
			return static_cast<int>(m_data.GetSize());
		}

		/// 
		/// Media start time.
		///
		/// @return Start time.
		double StartTime() const
		{
			return m_startTimeMs;
		}

		///
		/// Get marker.
		///
		/// @return True if marker is set.
		bool IsMarkerSet() const
		{
			return m_marker;
		}

		///
		/// Set marker.
		///
		/// @param[in] isMarker True if marker is set.		
		void SetMarker(bool isMarker)
		{
			m_marker = isMarker;
		}

		/// Get channel id.
		///
		/// @return Channel id.
		uint32_t GetChannelId() const
		{
			return m_channelId;
		}

		/// Set channel id.
		///
		/// @param[in] channelId Channel id.
		void SetChannelId(uint32_t channelId)
		{
			m_channelId = channelId;
		}

		/// Get source id.
		///
		/// @return Source id.
		uint32_t GetSourceId() const
		{
			return m_sourceId;
		}

		/// Set source id.
		///
		/// @param[in] sourceId Source id.
		void SetSourceId(uint32_t sourceId)
		{
			m_sourceId = sourceId;
		}

		///
		/// Getter for isKeyFrame.
		///
		/// @return True if this a key frame.
		const inline bool GetIsKeyFrame() const
		{
			return m_isKeyFrame;
		}

		/// Setter for isKeyFrame.
		///
		/// @param[in] isKeyFrame	Whether a key-frame or not.
		inline void SetIsKeyFrame(bool isKeyFrame)
		{
			m_isKeyFrame = isKeyFrame;
		}


	private:
		///
		/// Default constructor.
		MediaSample() :
			m_startTimeMs(0.0),
			m_marker(false),
			m_channelId(0),
			m_sourceId(0),
			m_isKeyFrame(false)
		{
		};

		///
		/// Private media sample constructor.
		///
		/// @param[in] data			Data buffer.
		/// @param[in] size			Size of the data buffer.
		/// @param[in] startTimeMs	Start time of the data stream.
		/// @param[in] isKeyFrame	True if this a keyframe.
		/// @param[in] channelId	Channel id.
		/// @param[in] sourceId		Source id.
		/// @param[in] isSyncPoint	True if this is a marker.
		MediaSample(BYTE* data, int size, double startTimeMs, bool isKeyFrame, uint32_t channelId, uint32_t sourceId, bool isSyncPoint);

		/// Media data byte stream.
		Buffer m_data;

		/// Media start time.
		double m_startTimeMs;

		/// End of sample marker.
		bool m_marker;

		/// Channel id
		uint32_t m_channelId;

		/// Source id
		uint32_t m_sourceId;

		/// Is this key-frame ?
		bool m_isKeyFrame;

	};
}
