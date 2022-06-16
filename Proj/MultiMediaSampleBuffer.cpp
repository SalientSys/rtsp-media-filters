#include "pch.h"

#include <iostream>

#include <rtsp-logger/RtspServerLogging.h>

#include "MultiMediaSampleBuffer.h"

using namespace CvRtsp;

MultiMediaSampleBuffer::
MultiMediaSampleBuffer(unsigned uiChannels, unsigned uiMaxFrameSize) :
	m_uiChannels(uiChannels),
	m_uiMaxFrameSize(uiMaxFrameSize),
	m_uiCurrentChannel(0),
	m_dStartTime(-1.0)
{
	// Create the static media sample buffers at this point
	for (auto i = 0; i < static_cast<int>(m_uiChannels); i++)
	{
		BYTE* pBuffer = new BYTE[m_uiMaxFrameSize];
		memset(pBuffer, 0, m_uiMaxFrameSize);
		m_vSampleBuffers.push_back(pBuffer);
		m_vSizes.push_back(0);
		// Store current buffer size for this channel since it might need to get bigger
		m_mBufferSizes[i] = m_uiMaxFrameSize;
	}
}

MultiMediaSampleBuffer::
~MultiMediaSampleBuffer()
{
	for (auto i = 0; i < static_cast<int>(m_uiChannels); i++)
	{
		delete[] m_vSampleBuffers[i];
	}
	m_vSampleBuffers.clear();
	m_vSizes.clear();
}

void
MultiMediaSampleBuffer::
AddMediaSample(const std::shared_ptr<MediaSample>& mediaSample)
{
	// Get start time
	m_dStartTime = mediaSample->StartTime();

	// Get raw data stream
	const uint8_t* pMediaData = mediaSample->GetDataBuffer().Data();

	MultiplexedMediaHeader pMediaHeader = MultiplexedMediaHeader::read(pMediaData, mediaSample->GetSize());

	if (&pMediaHeader == nullptr)
	{
		log_rtsp_error("Invalid multiplexed media header received.");
		return;
	}

	// Read media stream information
	int nNumberOfStreams = pMediaHeader.getStreamCount();
	if (nNumberOfStreams != static_cast<int>(m_uiChannels))
	{
		log_rtsp_error("Unexpected error : Number of Streams : " + std::to_string(nNumberOfStreams) + " Number of Channels : " + std::to_string(m_uiChannels) + ".");
		return;
	}

	// For now let's just extract the first stream and pass this back
	const uint8_t* pSource = pMediaData + pMediaHeader.getHeaderLength();

	for (auto i = 0; i < static_cast<int>(m_vSampleBuffers.size()); i++)
	{
		BYTE* pDestination = m_vSampleBuffers[i];
		unsigned uiLengthOfStream = static_cast<int>(pMediaHeader.getStreamLength(i));
		unsigned uiBufferSize = static_cast<int>(m_mBufferSizes[i]);
		while (uiLengthOfStream > m_mBufferSizes[i])
		{
			// Double the buffer size
			m_mBufferSizes[i] <<= 1;
			log_rtsp_warning("Framesize was too large : Size: " + std::to_string(uiLengthOfStream)
				+ " Prev Max: " + std::to_string(m_mBufferSizes[i] / 2) + " New Max: " + std::to_string(m_mBufferSizes[i]) + ".");
		}
		if (uiBufferSize != m_mBufferSizes[i])
		{
			// free old memory
			delete[] pDestination;
			pDestination = new BYTE[m_mBufferSizes[i]];
			m_vSampleBuffers[i] = pDestination;
		}

		assert(uiLengthOfStream <= m_mBufferSizes[i]);
		// Copy data			
		memcpy(pDestination, pSource, uiLengthOfStream);
		m_vSizes[i] = uiLengthOfStream;

		// Point to next media sample
		pSource += uiLengthOfStream;
	}
}

unsigned
MultiMediaSampleBuffer::
GetCurrentChannel()
{
	return m_uiCurrentChannel;
}

void
MultiMediaSampleBuffer::
SetCurrentChannel(unsigned channel)
{
	if (channel < m_uiChannels)
	{
		m_uiCurrentChannel = channel;
	}
}

unsigned
MultiMediaSampleBuffer::
GetNumberOfChannels()
{
	return m_uiChannels;
}

unsigned
MultiMediaSampleBuffer::
GetCurrentSize()
{
	if (m_uiCurrentChannel < m_vSampleBuffers.size())
	{
		return m_vSizes[m_uiCurrentChannel];
	}
	return 0;
}

double
MultiMediaSampleBuffer::
GetCurrentStartTime()
{
	return m_dStartTime;
}

BYTE*
MultiMediaSampleBuffer::
GetCurrentBuffer()
{
	if (m_uiCurrentChannel < m_vSampleBuffers.size())
	{
		return m_vSampleBuffers[m_uiCurrentChannel];
	}
	return nullptr;
}

BYTE*
MultiMediaSampleBuffer::
GetBufferAt(unsigned index)
{
	if (index < m_vSampleBuffers.size())
	{
		return m_vSampleBuffers[index];
	}
	return nullptr;
}
