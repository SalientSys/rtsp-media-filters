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
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///

#include "pch.h"

#include <live555/H265VideoRTPSink.hh>
#include <live555/Base64.hh>
#include <live555/H265VideoStreamDiscreteFramer.hh>
#include <live555/H265VideoStreamFramer.hh>
#include <rtsp-logger/RtspServerLogging.h>

#include "LiveDeviceSource.h"
#include "LiveH265VideoDeviceSource.h"
#include "LiveH265Subsession.h"
#include "SimpleFrameGrabber.h"

using namespace CvRtsp;

LiveH265Subsession::
LiveH265Subsession(UsageEnvironment& env, LiveRtspServer& rParent,
    const boost::uuids::uuid& uiChannelId, unsigned uiSourceId,
    const std::string& sSessionName,
    const std::string& sVps, const std::string& sSps, const std::string& sPps,
    IRateAdaptationFactory* pFactory,
    IRateController* pGlobalRateControl)
    :LiveMediaSubsession(env, rParent, uiChannelId, uiSourceId, sSessionName, true, 1, false, pFactory, pGlobalRateControl),
    m_sVps(sVps),
    m_sSps(sSps),
    m_sPps(sPps)
{
    log_rtsp_debug("LiveH265Subsession() VPS: " + m_sVps + " SPS: " + m_sSps + " PPS: " + m_sPps + ".");
}


FramedSource*
LiveH265Subsession::
createSubsessionSpecificSource(unsigned clientSessionId,
    IMediaSampleBuffer* pMediaSampleBuffer,
    IRateAdaptationFactory* pRateAdaptationFactory,
    IRateController* pRateControl)
{
    FramedSource* pLiveDeviceSource = LiveH265VideoDeviceSource::CreateNew(envir(), clientSessionId, this, m_sVps, m_sSps, m_sPps,
        new SimpleFrameGrabber(pMediaSampleBuffer),
        pRateAdaptationFactory, pRateControl);
    // wrap framer around our device source
    H265VideoStreamDiscreteFramer* pFramer = H265VideoStreamDiscreteFramer::createNew(envir(), pLiveDeviceSource);
    return pFramer;
}


void
LiveH265Subsession::
setEstimatedBitRate(unsigned& estBitrate)
{
    // Set estimated session band width
    estBitrate = 500;
}


RTPSink*
LiveH265Subsession::
createSubsessionSpecificRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
    // HACKERY
    H265VideoRTPSink* pSink = H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, m_sVps.c_str(), m_sSps.c_str(), m_sPps.c_str());
    pSink->setPacketSizes(1000, 1400);
    return pSink;
}
