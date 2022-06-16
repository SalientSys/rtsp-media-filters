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
/// @class LiveSourceTaskScheduler
///
/// Modified 08/21/2019
/// Modified by: M. Kinzer
///
#pragma once
#include "LiveSourceTaskScheduler0.h"

namespace CvRtsp
{
	/// Live media task scheduler that also handles live sources.
	class LiveSourceTaskScheduler : public LiveSourceTaskScheduler0
	{
	public:
		///
		/// Named constructor.
		///
		/// @param channelManager Channel manager.
		static LiveSourceTaskScheduler* createNew(ChannelManager& channelManager);

		///
		/// Destructor.
		virtual ~LiveSourceTaskScheduler();

	protected:
		///
		/// Protected constructor.
		///
		/// @param[in] channelManager Channel manager.
		///
		///	@remark Called only by "createNew()"
		LiveSourceTaskScheduler(ChannelManager& channelManager);

		///
		/// Redefined virtual function from BasicTaskScheduler.hh.
		///
		/// @param[in] maxDelayTimeMicroSec Maximum delay time in microseconds.
		void SingleStep(unsigned maxDelayTimeMicroSec) override;
	};
}
