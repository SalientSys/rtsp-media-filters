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
SetVideoSourceId(const boost::uuids::uuid& channelId, const std::string& channelName, 
	const uint32_t videoSourceId)
{
	// Do we have a packet-manager with the said <channelId, channelName> ?
	const auto packetManager = m_packetManagerMediaChannelMap.find(std::make_pair(channelId, channelName));
	if (packetManager != std::end(m_packetManagerMediaChannelMap))
	{
		packetManager->second.SetVideoSourceId(videoSourceId);
	}
	else
	{
		// Create and associate packet-manager with the `channelName`.
		PacketManager manager(channelId, channelName);
		manager.SetVideoSourceId(videoSourceId);
		m_packetManagerMediaChannelMap.emplace(std::make_pair(channelId, channelName), manager);
	}
}

void
MultiChannelManager::
SetAudioSourceId(const boost::uuids::uuid& channelId, const std::string& channelName, 
	const uint32_t audioSourceId)
{
	// Do we have a packet-manager with the said  <channelId, channelName> ?
	const auto packetManager = m_packetManagerMediaChannelMap.find(std::make_pair(channelId, channelName));
	if (packetManager != std::end(m_packetManagerMediaChannelMap))
	{
		packetManager->second.SetAudioSourceId(audioSourceId);
	}
	else
	{
		// Create and associate packet-manager with the <channelId, channelName> ?
		PacketManager manager(channelId, channelName);
		manager.SetAudioSourceId(audioSourceId);
		m_packetManagerMediaChannelMap.emplace(std::make_pair(channelId, channelName), manager);
	}
}

const std::shared_ptr<PacketManagerMediaChannel>
MultiChannelManager::
GetPacketManager(const boost::uuids::uuid &channelId,
	const std::string &channelName) const
{
	const auto packetManagerIt = m_packetManagerMediaChannelMap.find(std::make_pair(channelId, channelName));
	if (packetManagerIt != std::cend(m_packetManagerMediaChannelMap))
	{
		return packetManagerIt->second.GetPacketManager();
	}
	return nullptr;
}


std::shared_ptr<MediaSample>
MultiChannelManager::
GetMedia(const boost::uuids::uuid& channelId, const std::string& channelName,
	uint32_t sourceId)
{
	const auto packetManager = m_packetManagerMediaChannelMap.find(std::make_pair(channelId, channelName));
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

	//assert(false);
	return nullptr;
}
