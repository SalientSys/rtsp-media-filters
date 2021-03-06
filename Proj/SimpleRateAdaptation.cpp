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
#include "pch.h"

#include <rtsp-logger/RtspServerLogging.h>

#include "SimpleRateAdaptation.h"

using namespace CvRtsp;

SimpleRateAdaptation::SimpleRateAdaptation() :
    m_dPreviousAverage(-1.0),
    m_lastPacketNumReceived(0),
    nCounter(0),
    nCurrentChannel(0),
    bUp(true)
{

}

SwitchDirection SimpleRateAdaptation::GetRateAdaptAdvice(const RtpTransmissionStats& stats)
{
    // Get packet num
    unsigned packetNum = stats.LastPacketNumReceived;
    if (packetNum == m_lastPacketNumReceived)
    {
        // There's no new information
        log_rtsp_debug("Staying: no new info");
        return STAY;
    }

    SwitchDirection eSwitchDirection = STAY;

    if (bUp)
    {
        ++nCurrentChannel;
        log_rtsp_debug("Switching up");
        eSwitchDirection = SWITCH_UP;
        if (nCurrentChannel == 7)
            bUp = false;
    }
    else
    {
        log_rtsp_debug("Switching down");
        --nCurrentChannel;
        eSwitchDirection = SWITCH_DOWN;
        if (nCurrentChannel == 0)
            bUp = true;
    }
    return eSwitchDirection;
}
