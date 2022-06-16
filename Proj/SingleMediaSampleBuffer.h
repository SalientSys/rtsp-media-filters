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
/// @class SingleMediaSampleBuffer
///
/// Modified 08/22/2019
/// Modified by: M. Kinzer
///
#pragma once
#include "IMediaSampleBuffer.h"

namespace CvRtsp
{
	/// This class abstracts the multiple buffers that a sample gets copied into
	class SingleMediaSampleBuffer : public IMediaSampleBuffer
	{
	public:
		/// Byte array definition for a data buffer.
		typedef std::unique_ptr<BYTE[]> DataBuffer;

		///
		/// Constructor.
		///
		/// @param[in] maxFrameSize Maximum frame size.
		SingleMediaSampleBuffer(unsigned maxFrameSize = 10000);

		///
		/// Default destructor.
		~SingleMediaSampleBuffer() = default;

		///
		/// Overridden from IMediaSampleBuffer to get current channel.
		///
		/// @return Current channel.
		unsigned GetCurrentChannel() override;

		///
		/// Overridden from IMediaSampleBuffer to set the current channel.
		///
		/// @param[in] channelId Channel identifier.
		///
		/// @note This implementation will only have one channel.
		void SetCurrentChannel(unsigned channelId) override;

		///
		/// Overridden from IMediaSampleBuffer to add a media sample.
		///
		/// @param[in] mediaSample Media sample.
		///		
		/// @note This method should copy the media sample in the buffers according to the media type.
		void AddMediaSample(const std::shared_ptr<MediaSample>& mediaSample) override;

		///
		/// Overridden from IMediaSampleBuffer to get number of channels, for this
		/// implementation the value should always be 1.
		///
		/// @return Number of channels.		
		unsigned GetNumberOfChannels() override;

		///
		/// Overriden to return the current buffer size.
		///
		/// @return Current buffer size.
		unsigned GetCurrentSize() override;

		///
		/// Overriden to return the current media sample start time.
		///
		/// @return Current media sample start time.
		double GetCurrentStartTime() override;

		///
		/// Override to return current media buffer.
		///
		/// @return Current media buffer else nullptr.
		BYTE* GetCurrentBuffer() override;

		/// 
		/// Not implemented for this class.
		/// 
		/// @return nullptr.
		BYTE* GetBufferAt(unsigned index) override;

	private:
		/// Current buffer size.
		unsigned m_currentBufferSize;

		/// Data buffer.
		DataBuffer m_buffer;

		/// Media sample size.
		unsigned m_size;

		/// Current channel.
		unsigned m_currentChannel;

		/// Media sample start time.
		double m_startTime;
	};
}
