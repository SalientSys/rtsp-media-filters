///
/// @class MultiChannelManager
///
/// Created 08/21/2019
/// Modified by: M. Kinzer
///
#include "pch.h"
#include "MultiChannelManager.h"

using namespace CvRtsp;

void
MultiChannelManager::
SetVideoSourceId(const uint32_t channelId, const uint32_t videoSourceId)
{
	const auto packetManager = m_packetManagerMediaChannelMap.find(channelId);
	if (packetManager != std::end(m_packetManagerMediaChannelMap))
	{
		packetManager->second.SetVideoSourceId(videoSourceId);
	}
	else
	{
		PacketManager manager(channelId);
		manager.SetVideoSourceId(videoSourceId);
		m_packetManagerMediaChannelMap.emplace(channelId, manager);
	}
}

void
MultiChannelManager::
SetAudioSourceId(const uint32_t channelId, const uint32_t audioSourceId)
{
	const auto packetManager = m_packetManagerMediaChannelMap.find(channelId);
	if (packetManager != std::end(m_packetManagerMediaChannelMap))
	{
		packetManager->second.SetAudioSourceId(audioSourceId);
	}
	else
	{
		PacketManager manager(channelId);
		manager.SetAudioSourceId(audioSourceId);
		m_packetManagerMediaChannelMap.emplace(channelId, manager);
	}
}

const std::shared_ptr<PacketManagerMediaChannel>
MultiChannelManager::
GetPacketManager(uint32_t channelId) const
{
	auto packetManager = m_packetManagerMediaChannelMap.find(channelId);
	if (packetManager != std::end(m_packetManagerMediaChannelMap))
	{
		return packetManager->second.GetPacketManager();
	}
	return nullptr;
}


std::shared_ptr<MediaSample>
MultiChannelManager::
GetMedia(uint32_t channelId, uint32_t sourceId)
{
	const auto packetManager = m_packetManagerMediaChannelMap.find(channelId);
	if (packetManager != std::end(m_packetManagerMediaChannelMap))
	{
		if (sourceId == packetManager->second.GetVideoSourceId())
		{
			return packetManager->second.GetPacketManager()->GetVideo();
		}
		if (sourceId == packetManager->second.GetAudioSourceId())
		{
			return packetManager->second.GetPacketManager()->GetAudio();
		}
	}

	assert(false);
	return nullptr;
}
