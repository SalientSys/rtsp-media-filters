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
#include "RtpTransmissionStats.h"

namespace CvRtsp
{
	enum SwitchDirection
	{
		STAY = 0,             // no advice
		SWITCH_DOWN,          // switch down a single step
		SWITCH_UP,            // switch up a single step
		SWITCH_DOWN_MULTIPLE, // switch down multiple application defined steps
		SWITCH_UP_MULTIPLE,   // switch up multiple application defined steps
		SWITCH_DOWN_MIN,      // switch down to the minimum rate
		SWITCH_UP_MAX         // switch down to the maximum rate
	};

	///
	/// The IRateAdaptation abstracts the rate adaptation process. 
	class IRateAdaptation
	{
	public:
		/// Virtual destructor
		virtual ~IRateAdaptation()
		{

		}

		///
		/// The subclass must implement the update of the RTCP statistics database
		/// and return the advice based on the updated model.
		///
		/// @param[in] transmissionStats Transmission stats.
		///
		/// @return Direction in which to switch rate adaptation.
		virtual SwitchDirection GetRateAdaptAdvice(const RtpTransmissionStats& transmissionStats) = 0;

	protected:
		///	
		/// Constructor
		IRateAdaptation()
		{
		}
	};
}
