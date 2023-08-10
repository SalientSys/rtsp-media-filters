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

#include <live555/H264VideoRTPSink.hh>
#include <live555/Base64.hh>
#include <live555/H264VideoStreamDiscreteFramer.hh>
#include <live555/H264VideoStreamFramer.hh>
#include <rtsp-logger/RtspServerLogging.h>

#include "LiveDeviceSource.h"
#include "LiveH264VideoDeviceSource.h"
#include "LiveH264Subsession.h"
#include "SimpleFrameGrabber.h"

using namespace CvRtsp;

LiveH264Subsession::LiveH264Subsession(UsageEnvironment& env, LiveRtspServer& rParent,
    const unsigned uiChannelId, unsigned uiSourceId,
    const std::string& sSessionName,
    const std::string& sSps, const std::string& sPps,
    IRateAdaptationFactory* pFactory,
    IRateController* pGlobalRateControl)
    :LiveMediaSubsession(env, rParent, uiChannelId, uiSourceId, sSessionName, true, 1, false, pFactory, pGlobalRateControl),
    m_sSps(sSps),
    m_sPps(sPps)
{
    log_rtsp_debug("LiveH264Subsession() SPS: " + m_sSps + " PPS: " + m_sPps + ".");
}


FramedSource* LiveH264Subsession::createSubsessionSpecificSource(unsigned clientSessionId,
    IMediaSampleBuffer* pMediaSampleBuffer,
    IRateAdaptationFactory* pRateAdaptationFactory,
    IRateController* pRateControl)
{
    FramedSource* pLiveDeviceSource = LiveH264VideoDeviceSource::CreateNew(envir(), clientSessionId, this, m_sSps, m_sPps,
        new SimpleFrameGrabber(pMediaSampleBuffer),
        pRateAdaptationFactory, pRateControl);
    // wrap framer around our device source
    H264VideoStreamDiscreteFramer* pFramer = H264VideoStreamDiscreteFramer::createNew(envir(), pLiveDeviceSource);
    return pFramer;
}


void LiveH264Subsession::setEstimatedBitRate(unsigned& estBitrate)
{
    // Set estimated session band width
    estBitrate = 500;
}


RTPSink* LiveH264Subsession::createSubsessionSpecificRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
    // HACKERY
    std::string sPropParameterSets = m_sSps + "," + m_sPps;
    H264VideoRTPSink* pSink = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, sPropParameterSets.c_str());
    pSink->setPacketSizes(1000, 1400);
    return pSink;
}
