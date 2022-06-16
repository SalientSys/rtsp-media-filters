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
#pragma once
#include <map>
#include "IMediaSampleBuffer.h"
#include "MultiplexedMediaHeader.h"

namespace CvRtsp
{
	/// 
	/// Buffer class for multiplexed media samples
	///
	/// Media samples must contain the MultiplexedMediaHeader format
	class MultiMediaSampleBuffer : public IMediaSampleBuffer
	{
	public:
		// TOREVISE: what default size?
		MultiMediaSampleBuffer(unsigned uiChannels, unsigned uiMaxFrameSize = 10000);

		~MultiMediaSampleBuffer();

		unsigned GetCurrentChannel() override;

		void SetCurrentChannel(unsigned channel) override;

		/// This method should copy the media sample in the buffers according to the media type
		void AddMediaSample(const std::shared_ptr<MediaSample>& mediaSample) override;

		BYTE* GetCurrentBuffer() override;

		BYTE* GetBufferAt(unsigned index) override;

		unsigned GetNumberOfChannels() override;

		unsigned GetCurrentSize() override;

		double GetCurrentStartTime() override;

	private:
		std::vector<BYTE*> m_vSampleBuffers;

		std::vector<int> m_vSizes;

		unsigned m_uiChannels;

		unsigned m_uiMaxFrameSize;

		std::map<unsigned, unsigned> m_mBufferSizes;

		unsigned m_uiCurrentChannel;

		double m_dStartTime;
	};
}
