#include "pch.h"

#include "SingleMediaSampleBuffer.h"
#include "MediaSample.h"

using namespace CvRtsp;

SingleMediaSampleBuffer::
SingleMediaSampleBuffer(unsigned maxFrameSize) :
	m_currentBufferSize(maxFrameSize),
	m_buffer(std::make_unique<BYTE[]>(maxFrameSize)),
	m_size(0),
	m_currentChannel(0),
	m_startTime(0.0)
{
}

unsigned
SingleMediaSampleBuffer::
GetCurrentChannel()
{
	return 0;
}

void
SingleMediaSampleBuffer::
SetCurrentChannel(unsigned channelId)
{
	// Not implemented
}

void
SingleMediaSampleBuffer::
AddMediaSample(const std::shared_ptr<MediaSample>& mediaSample)
{
	const auto mediaData = mediaSample->GetDataBuffer().Data();

	m_startTime = mediaSample->StartTime();
	// Grow buffer if too small
	m_size = mediaSample->GetSize();
	if (m_size > m_currentBufferSize)
	{
		// Increase the current buffer size
		if (m_buffer)
		{
			m_buffer.reset();
		}
		m_buffer = std::make_unique<BYTE[]>(m_size);
		m_currentBufferSize = m_size;
	}
	memcpy(m_buffer.get(), mediaData, m_size);
}

unsigned
SingleMediaSampleBuffer::
GetNumberOfChannels()
{
	return 1;
}

unsigned
SingleMediaSampleBuffer::
GetCurrentSize()
{
	return m_size;
}

double
SingleMediaSampleBuffer::
GetCurrentStartTime()
{
	return m_startTime;
}

BYTE*
SingleMediaSampleBuffer::
GetCurrentBuffer()
{
	if (m_buffer)
	{
		return m_buffer.get();
	}
	return nullptr;
}


BYTE*
CvRtsp::SingleMediaSampleBuffer::
GetBufferAt(unsigned index)
{
	assert(false);	//Note - Use GetCurrentBuffer() instead.

	return nullptr;
}
