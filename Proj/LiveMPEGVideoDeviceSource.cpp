///
/// @class LiveMPEGVideoDeviceSource
///
/// Modified 08/16/2019
/// fied by: M. Kinzer
///
/// 
#include "pch.h"

#include <bitset>

#include "LiveMPEGVideoDeviceSource.h"
#include "IFrameGrabber.h"
#include "CommonRtsp.h"

using namespace CvRtsp;

LiveMPEGVideoDeviceSource::
LiveMPEGVideoDeviceSource(UsageEnvironment& env, unsigned clientId,
	LiveMediaSubsession* parentSubsession, IFrameGrabber* frameGrabber,
	IRateAdaptationFactory* rateAdaptationFactory, IRateController* globalRateControl) :
	LiveDeviceSource(env, clientId, parentSubsession, frameGrabber, rateAdaptationFactory, globalRateControl),
	m_isWaitingForKeyFrame(true) {}


bool
LiveMPEGVideoDeviceSource::
checkIsKeyFrame(const BYTE* dataBuffer, uint32_t bufferSize, int offset)
{
	assert(dataBuffer != nullptr);

	if (memcmp(dataBuffer + offset, CvRtsp::MPEG4_Codes::VOP_START_CODE, sizeof(CvRtsp::MPEG4_Codes::VOP_START_CODE) != 0))
	{
		return false;
	}
	else
	{
		// Now extract higher 2 bits & check.
		const auto searchPos = sizeof(CvRtsp::MPEG4_Codes::VOP_START_CODE);
		const std::bitset<8> byteToReadBitFrom(static_cast<int>(dataBuffer[searchPos]));

		return !(byteToReadBitFrom.test(7) || byteToReadBitFrom.test(6));
	}
}


LiveMPEGVideoDeviceSource*
LiveMPEGVideoDeviceSource::
CreateNew(UsageEnvironment& env, unsigned clientId,
	LiveMediaSubsession* parentSubsession, IFrameGrabber* frameGrabber,
	IRateAdaptationFactory* rateAdaptationFactory, IRateController* globalRateControl)
{
	// When constructing a 'simple' LiveDeviceSource we'll just create a simple frame grabber
	auto videoDeviceSource = new LiveMPEGVideoDeviceSource(env, clientId, parentSubsession,
		frameGrabber, rateAdaptationFactory, globalRateControl);
	OutPacketBuffer::increaseMaxSizeTo(300000); // bytes
	return videoDeviceSource;
}

// @TODO - Remove below "Usecase" comments after merge
std::deque<std::shared_ptr<MediaSample>>
LiveMPEGVideoDeviceSource::
splitPayloadIntoMediaSamples(BYTE* dataBuffer, uint32_t bufferSize, double startTime)
{
	std::deque<std::shared_ptr<MediaSample>> mediaSamples;
	if (!dataBuffer || bufferSize == 0)
	{
		return mediaSamples;
	}

	auto startingPos = 0;
	auto currPos = -1;
	auto isConfigStrPresent = false;
	auto isParsedOnce = false;
	auto keyFramePosCheck = -1;	// Using this, since if `configStr_StartCode` is present (Usecase - 001b ... 001b6 ... 001b6 ...)
								// then we wont be able to  use `startingPos` as the seekPos(atleast for the 1st iteration) for searching I-frame bit.
								// TodDo - can be removed by palacing a cond. 

	// Parse for start codes here.
	if (memcmp(dataBuffer + 0, CvRtsp::MPEG4_Codes::VISUAL_OBJECT_SEQUENCE_START_CODE,
		sizeof(CvRtsp::MPEG4_Codes::VISUAL_OBJECT_SEQUENCE_START_CODE)) == 0)	// (Usecase - 001b ... 001b6 ... 001b6 ...)
	{
		// Found config-string. 
		isConfigStrPresent = true;
		currPos = sizeof(CvRtsp::MPEG4_Codes::VISUAL_OBJECT_SEQUENCE_START_CODE);
	}
	else if (memcmp(dataBuffer + 0, CvRtsp::MPEG4_Codes::VOP_START_CODE,
		sizeof(CvRtsp::MPEG4_Codes::VOP_START_CODE)) == 0) // (Usecase - 001b6 ... 001b6 ... 001b6 ... )
	{
		// Found frame start code.
		currPos = sizeof(CvRtsp::MPEG4_Codes::VOP_START_CODE);
		keyFramePosCheck = 0;
	}

	assert(currPos != -1);

	for (int i = currPos; i < static_cast<int>(bufferSize - sizeof(CvRtsp::MPEG4_Codes::VOP_START_CODE));)
	{
		if (memcmp(dataBuffer + i, CvRtsp::MPEG4_Codes::VOP_START_CODE, sizeof(CvRtsp::MPEG4_Codes::VOP_START_CODE)) == 0)
		{
			if (!isParsedOnce && isConfigStrPresent) // (Usecase - 001b ... 001b6 ... 001b6 ...) 
			{
				isParsedOnce = true;
				keyFramePosCheck = i;
				i += sizeof(CvRtsp::MPEG4_Codes::VOP_START_CODE);
			}
			else									// (Usecase - 001b6 ... 001b6 ... 001b6 ... ) or (Usecase - 001b ... 001b6 ... 001b6 ...) 
			{
				// We have upper & lower limit for a frame.
				auto offset = i - startingPos;
				auto isKeyFrame = checkIsKeyFrame(dataBuffer, bufferSize, keyFramePosCheck);

				auto tempSample = MediaSample::CreateMediaSample(dataBuffer + startingPos, offset, startTime, isKeyFrame);
				mediaSamples.push_back(tempSample);

				startingPos = i;
				keyFramePosCheck = startingPos;
				i += sizeof(CvRtsp::MPEG4_Codes::VOP_START_CODE);
			}
		}
		else
		{
			i++;
		}
	}

	// Push last remaining tempSample.
	auto isKeyFrame = checkIsKeyFrame(dataBuffer, bufferSize, keyFramePosCheck);

	auto tempSample = MediaSample::CreateMediaSample(dataBuffer + startingPos, bufferSize - startingPos, startTime, isKeyFrame);
	mediaSamples.push_back(tempSample);

	return mediaSamples;
}


bool
LiveMPEGVideoDeviceSource::
RetrieveMediaSampleFromBuffer()
{
	unsigned bufferSize = 0;
	auto startTime = 0.0;
	const auto dataBuffer = m_frameGrabber->GetNextFrame(bufferSize, startTime);

	// Make sure there's data, the frame grabber should return null if it doesn't have any
	if (!dataBuffer)
	{
		return false;
	}

	auto mediaSamples = splitPayloadIntoMediaSamples(dataBuffer, bufferSize, startTime);

	//KE @TODO - need to optimize I-frame parsing above in splitPayloadIntoMediaSamples() then below `false`
	//in the if condtion can be removed, else takes ~30 seconds to display first frame in vlc client if we wait.

	// Have we not sent a key frame until now ?
	if (/*m_isWaitingForKeyFrame*/false) 
	{
		// We need to be able to push a key frame first.
		// @Note - there will be some delay before the video starts.
		for (const auto& tempSample : mediaSamples)
		{
			if (tempSample->GetIsKeyFrame())
			{
				m_isWaitingForKeyFrame = false;
				break;
			}
		}

		if (!m_isWaitingForKeyFrame)
		{
			m_mediaSampleQueue.insert(m_mediaSampleQueue.end(), mediaSamples.begin(), mediaSamples.end());
			return true;
		}
		return false;
	}

	m_mediaSampleQueue.insert(m_mediaSampleQueue.end(), mediaSamples.begin(), mediaSamples.end());

	return true;
}
