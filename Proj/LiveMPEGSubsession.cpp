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

#include <live555/MPEG4ESVideoRTPSink.hh>
#include <live555/Base64.hh>
#include <live555/MPEG4VideoStreamDiscreteFramer.hh>
#include <live555/MPEG4VideoStreamFramer.hh>
#include <rtsp-logger/RtspServerLogging.h>

#include "LiveDeviceSource.h"
#include "LiveMPEGSubsession.h"
#include "SimpleFrameGrabber.h"
#include "LiveMPEGVideoDeviceSource.h"

using namespace CvRtsp;

class LiveMPEGVideoDeviceSource;

LiveMPEGSubsession::LiveMPEGSubsession(UsageEnvironment& env, LiveRtspServer& rParent,
    const unsigned uiChannelId, unsigned uiSourceId,
    const std::string& sSessionName,
    IRateAdaptationFactory* pFactory,
    IRateController* pGlobalRateControl, std::string configStr, int profileAndLevelIndication)
    :LiveMediaSubsession(env, rParent, uiChannelId, uiSourceId, sSessionName, true, 1, false, pFactory, pGlobalRateControl),
    m_configStr(configStr),
    m_profileAndLevelIndication(profileAndLevelIndication)
{
    log_rtsp_debug("LiveMPEGSubsession() sdp config-bytes: " + m_configStr + " profile & level id: " + to_string(m_profileAndLevelIndication));
}


FramedSource* LiveMPEGSubsession::createSubsessionSpecificSource(unsigned clientSessionId,
    IMediaSampleBuffer* pMediaSampleBuffer,
    IRateAdaptationFactory* pRateAdaptationFactory,
    IRateController* pRateControl)
{
    FramedSource* pLiveDeviceSource = LiveMPEGVideoDeviceSource::CreateNew(envir(), clientSessionId, this,
        new SimpleFrameGrabber(pMediaSampleBuffer), pRateAdaptationFactory, pRateControl);

    // wrap framer around our device source
    MPEG4VideoStreamDiscreteFramer* pFramer = MPEG4VideoStreamDiscreteFramer::createNew(envir(), pLiveDeviceSource);
    return pFramer;
}


void LiveMPEGSubsession::setEstimatedBitRate(unsigned& estBitrate)
{
    // Set estimated session band width
    estBitrate = 500;
}


RTPSink* LiveMPEGSubsession::createSubsessionSpecificRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
    // HACKERY   
    u_int32_t rtpTimestampFrequency = 90000;
    MPEG4ESVideoRTPSink* pSink = MPEG4ESVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
        rtpTimestampFrequency, static_cast<u_int8_t>(m_profileAndLevelIndication), std::string(m_configStr).c_str());
    pSink->setPacketSizes(1000, 1400);

    return pSink;
}
