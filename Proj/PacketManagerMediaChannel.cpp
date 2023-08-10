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
/// @class PacketManagerMediaChannel
///
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///
#include "pch.h"

#include "PacketManagerMediaChannel.h"

using namespace CvRtsp;

PacketManagerMediaChannel::
PacketManagerMediaChannel(uint32_t channelId)
	: MediaChannel(channelId)
{
	m_videoSamples.set_capacity(QueueCapacity);
	m_audioSamples.set_capacity(QueueCapacity);
}

bool
PacketManagerMediaChannel::
deliverVideo(uint32_t channelId,
	const std::vector<std::shared_ptr<MediaSample>>& mediaSamples)
{
	for (const auto& mediaSample : mediaSamples)
	{
		m_videoSamples.try_push(mediaSample);
	}

	return true;
}

bool
PacketManagerMediaChannel::
deliverAudio(uint32_t channelId,
	const std::vector<std::shared_ptr<MediaSample>>& mediaSamples)
{
	for (const auto& mediaSample : mediaSamples)
	{
		m_audioSamples.try_push(mediaSample);
	}
	return true;
}

std::shared_ptr<MediaSample>
PacketManagerMediaChannel::
GetVideo()
{
	std::shared_ptr<MediaSample> mediaSample;
	if (m_videoSamples.try_pop(mediaSample))
	{
		return mediaSample;
	}
	return nullptr;
}

std::shared_ptr<MediaSample>
PacketManagerMediaChannel::
GetAudio()
{
	std::shared_ptr<MediaSample> mediaSample;
	if (m_audioSamples.try_pop(mediaSample))
	{
		return mediaSample;
	}
	return nullptr;
}
