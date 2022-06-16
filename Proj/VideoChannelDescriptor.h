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

#include <cstdint>
#include <string>
#include <vector>

namespace CvRtsp
{
	/// Simple descriptor for video parameters
	struct VideoChannelDescriptor
	{
		/// Video codec
		std::string Codec;

		/// Resolution width
		uint32_t Width;

		/// Resolution height
		uint32_t Height;

		/// H.264/H.265 sequence parameter set
		std::string Sps;

		/// H.264/H.265 picture parameter set
		std::string Pps;

		/// Frame bit limits for bitrate switching multiplexed media types
		std::vector<uint32_t> FrameBitLimits;

		/// Initial channel
		uint32_t InitialChannel;

#pragma region MPEG4 data

		/// Config-bytes from sdp a=ftmp line used by decoder
		std::string ConfigStr;

		/// Profile-Level-id used by decoder (extracted from config-bytes, 5th byte)
		int profileAndLevelIndication;
#pragma endregion

		// TODO - Yet to implement!
//#pragma region MJPEG data
//		u_int8_t jpegType;	// chaeck for restart interval present  or not
//			u_int8_t	jpegQFactor;
//			u_int8_t	jpegWidth;
//			u_int8_t	jpegHeight;
//			u_int8_t	jpegPrecision; // for quant table
//			u_int8_t	jpegLength;	   // for quant table
//#pragma endregion

		/// h265 video parameter set
		std::string vps;

		/// Constructor
		VideoChannelDescriptor() :
			Width(0),
			Height(0),
			InitialChannel(0)
		{
		}
	};
}
