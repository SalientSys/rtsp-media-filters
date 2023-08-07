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
// "liveMedia"
// Copyright (c) 1996-2014 Live Networks, Inc.  All rights reserved.
#include "pch.h"

#include <rtsp-logger/RtspServerLogging.h>

#include "LiveAMRAudioDeviceSource.h"
#include "SimpleFrameGrabber.h"


using namespace CvRtsp;

LiveAMRAudioDeviceSource::LiveAMRAudioDeviceSource(UsageEnvironment& env, unsigned uiClientId, LiveMediaSubsession* pParent,
    IMediaSampleBuffer* pSampleBuffer, IRateAdaptationFactory* pRateAdaptationFactory,
    IRateController* pGlobalRateControl)
    :LiveDeviceSource(env, uiClientId, pParent, new SimpleFrameGrabber(pSampleBuffer), pRateAdaptationFactory, pGlobalRateControl),
    fIsWideband(False),
    fNumChannels(1),
    fLastFrameHeader(0)
{

}

LiveAMRAudioDeviceSource::~LiveAMRAudioDeviceSource()
{

}

LiveAMRAudioDeviceSource* LiveAMRAudioDeviceSource::createNew(UsageEnvironment& env, unsigned uiClientId, LiveMediaSubsession* pParent,
    IMediaSampleBuffer* pSampleBuffer,
    IRateAdaptationFactory* pRateAdaptationFactory,
    IRateController* pRateControl)
{
    LiveAMRAudioDeviceSource* pSource = new LiveAMRAudioDeviceSource(env, uiClientId, pParent, pSampleBuffer, pRateAdaptationFactory, pRateControl);
    return pSource;
}

// From liveMedia
#define FT_INVALID 65535
static unsigned short frameSize[16] = {
  12, 13, 15, 17,
  19, 20, 26, 31,
  5, FT_INVALID, FT_INVALID, FT_INVALID,
  FT_INVALID, FT_INVALID, FT_INVALID, 0
};
static unsigned short frameSizeWideband[16] = {
  17, 23, 32, 36,
  40, 46, 50, 58,
  60, 5, FT_INVALID, FT_INVALID,
  FT_INVALID, FT_INVALID, 0, 0
};

// #define DEBUG
bool LiveAMRAudioDeviceSource::retrieveMediaSampleFromBuffer()
{
    unsigned uiSize = 0;
    double dStartTime = 0.0;
    BYTE* pBuffer = m_frameGrabber->GetNextFrame(uiSize, dStartTime);
#if 1
    fLastFrameHeader = pBuffer[0]; // Should be pos 0 or 1???
    if ((fLastFrameHeader & 0x83) != 0)
    {
#ifdef DEBUG
        fprintf(stderr, "Invalid frame header 0x%02x (padding bits (0x83) are not zero)\n", fLastFrameHeader);
        log_rtsp_warning("Invalid frame header " + to_string(fLastFrameHeader) + " (padding bits(0x83) are not zero).");
#endif
    }
    else
    {
        unsigned char ft = (fLastFrameHeader & 0x78) >> 3;
        fFrameSize = fIsWideband ? frameSizeWideband[ft] : frameSize[ft];
        if (fFrameSize == FT_INVALID)
        {
#ifdef DEBUG
            fprintf(stderr, "Invalid FT field %d (from frame header 0x%02x)\n", ft, fLastFrameHeader);
            log_rtsp_warning("Invalid FT field  " + to_string(ft) + " (from frame header " + to_string(fLastFrameHeader) + ".");
#endif
        }
        else
        {
            // The frame header is OK

            fprintf(stderr, "Valid frame header 0x%02x -> ft %d -> frame size %d\n", fLastFrameHeader, ft, fFrameSize);
            log_rtsp_debug("Valid frame header " + to_string(fLastFrameHeader) + " -> ft " + to_string(ft) + " -> frame size " + to_string(fFrameSize) + ".");

        }
    }
#endif

    // Make sure there's data, the frame grabber should return null if it doesn't have any
    if (!pBuffer)
    {
#if 0
        log_rtsp_warning("NULL Sample retrieved from frame grabber.");
#endif
        return false;
    }
    else
    {
        // Without TOC!!!!!!
        //MediaSample mediaSample;
        //mediaSample.setPresentationTime(dStartTime);
        BYTE* pData = new uint8_t[uiSize - 1];
        memcpy(pData, pBuffer + 1, uiSize - 1);

        auto mediaSample = MediaSample::CreateMediaSample(pData, uiSize - 1, dStartTime);
        //mediaSample.setData(Buffer(pData, uiSize - 1));
        m_mediaSampleQueue.push_back(mediaSample);
        return true;
    }
}

void LiveAMRAudioDeviceSource::doGetNextFrame()
{
    LiveDeviceSource::doGetNextFrame();
}

Boolean LiveAMRAudioDeviceSource::isAMRAudioSource() const
{
    return True;
}
