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
/// @class ChannelManager
///
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///
#pragma once

#include <cstdint>
#include <boost/uuid/uuid.hpp>

#include "MediaSample.h"

namespace CvRtsp
{
	///
	/// Abstract class for channel managers. A channel manager must make media accessible to the liveMedia task scheduler
	/// Media can be retrieved by using the channel id and source id.
	class ChannelManager
	{
	public:
		///
		/// Destructor
		virtual ~ChannelManager() = default;

		///
		/// The implementation must return a media sample if the channel has received a media sample
		/// of the specified channel and source id.
		///
		/// @param[in] channelId	Unique Channel id.
		/// @param[in] sourceId		Source id.
		///
		/// @return Media sample, nullptr if not found.
		virtual std::shared_ptr<MediaSample> GetMedia(const boost::uuids::uuid &channelId, 
			const std::string& channelName, uint32_t sourceId) = 0;
	};
}
