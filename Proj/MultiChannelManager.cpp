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
SetVideoSourceId(const std::string& channelName, const uint32_t videoSourceId)
{
	// Do we have a packet-manager with the said `channelName` ?
	const auto packetManager = m_packetManagerMediaChannelMap.find(channelName);
	if (packetManager != std::end(m_packetManagerMediaChannelMap))
	{
		packetManager->second.SetVideoSourceId(videoSourceId);
	}
	else
	{
		// Create and associate packet-manager with the `channelName`.
		PacketManager manager(channelName);
		manager.SetVideoSourceId(videoSourceId);
		m_packetManagerMediaChannelMap.emplace(channelName, manager);
	}
}

void
MultiChannelManager::
SetAudioSourceId(const std::string& channelName, const uint32_t audioSourceId)
{
	// Do we have a packet-manager with the said `channelName` ?
	const auto packetManager = m_packetManagerMediaChannelMap.find(channelName);
	if (packetManager != std::end(m_packetManagerMediaChannelMap))
	{
		packetManager->second.SetAudioSourceId(audioSourceId);
	}
	else
	{
		// Create and associate packet-manager with the `channelName`.
		PacketManager manager(channelName);
		manager.SetAudioSourceId(audioSourceId);
		m_packetManagerMediaChannelMap.emplace(channelName, manager);
	}
}

const std::shared_ptr<PacketManagerMediaChannel>
MultiChannelManager::
GetPacketManager(const std::string& channelName) const
{
	auto packetManager = m_packetManagerMediaChannelMap.find(channelName);
	if (packetManager != std::end(m_packetManagerMediaChannelMap))
	{
		return packetManager->second.GetPacketManager();
	}
	return nullptr;
}


std::shared_ptr<MediaSample>
MultiChannelManager::
GetMedia(const std::string& channelName, uint32_t sourceId)
{
	const auto packetManager = m_packetManagerMediaChannelMap.find(channelName);
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
