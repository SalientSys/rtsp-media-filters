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
/// @class IMediaSampleBuffer
///
/// Modified 08/22/2019
/// Modified by: M. Kinzer
///
#pragma once
#include "MediaSample.h"

namespace CvRtsp
{
	/// 
	/// This class abstracts the multiple buffers that a (multiplexed) sample gets copied into
	/// The interface tries to cater for both media samples where there is only one channel such 
	/// as PCM or H263.
	class IMediaSampleBuffer
	{
	public:
		///
		/// Virtual destructor.
		virtual ~IMediaSampleBuffer() = default;

		///
		/// Override to add media sample.
		///
		/// @param[in] mediaSample Media sample.
		virtual void AddMediaSample(const std::shared_ptr<MediaSample>& mediaSample) = 0;

		///
		/// Override to return current channel.
		virtual unsigned GetCurrentChannel() = 0;

		///
		/// Override to set current channel.
		virtual void SetCurrentChannel(unsigned channelId) = 0;

		///
		/// Override to return current buffer.
		virtual BYTE* GetCurrentBuffer() = 0;

		///
		/// Override to return the current start time.
		virtual double GetCurrentStartTime() = 0;

		///
		/// Override to return the current buffer size.
		virtual unsigned GetCurrentSize() = 0;

		///
		/// Override to return the number of channels.
		virtual unsigned GetNumberOfChannels() = 0;

		///
		/// Override to return the buffer at an index location.
		virtual BYTE* GetBufferAt(unsigned index) = 0;
	};
}
