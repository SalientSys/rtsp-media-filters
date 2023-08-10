///
/// @class MultiChannelManager
///
/// Created 08/21/2019
/// Modified by: M. Kinzer
///
#pragma once
#include <climits>
#include <unordered_map>
#include "ChannelManager.h"
#include "PacketManagerMediaChannel.h"

namespace CvRtsp
{
	struct PacketManager
	{
		/// Disallow default constructor.
		PacketManager() = delete;

		PacketManager(const std::string& channelName) :
			m_videoSourceId(UINT_MAX),
			m_audioSourceId(UINT_MAX),
			m_packetManager(std::make_shared<PacketManagerMediaChannel>(channelName))
		{
		}

		///
		/// Set the video source id.
		///
		/// @param videoSourceId Video source id.
		void SetVideoSourceId(const uint32_t videoSourceId)
		{
			assert(videoSourceId != m_audioSourceId);
			m_videoSourceId = videoSourceId;
		}

		///
		/// Set the audio source id.
		///
		/// @param audioSourceId Audio source id.
		void SetAudioSourceId(const uint32_t audioSourceId)
		{
			assert(audioSourceId != m_videoSourceId);
			m_audioSourceId = audioSourceId;
		}

		///
		/// Get the video source id.
		///
		/// @return Video source id.
		uint32_t GetVideoSourceId()
		{
			return m_videoSourceId;
		}

		///
		/// Get the audio source id.
		///
		/// @return Audio source id.
		uint32_t GetAudioSourceId()
		{
			return m_audioSourceId;
		}

		///
		/// Get the packet manager for this channel manager.
		///
		/// @return Media channel packet manager.
		const std::shared_ptr<PacketManagerMediaChannel> GetPacketManager() const
		{
			return m_packetManager;
		}

		///
		/// Get the packet manager for this channel manager.
		///
		/// @return Media channel packet manager.
		std::shared_ptr<PacketManagerMediaChannel> GetPacketManager()
		{
			return m_packetManager;
		}

	private:
		/// Video source id.
		uint32_t m_videoSourceId;

		/// Audio source id.
		uint32_t m_audioSourceId;

		/// Packet manager.
		std::shared_ptr<PacketManagerMediaChannel> m_packetManager;
	};

	///
	/// This class manages a single channel with audio and video.
	/// The channel id is not used much here. It is however needed when there are 
	/// multiple channels that supply the task manager with data.
	class MultiChannelManager : public ChannelManager
	{
	public:
		///
		/// Default constructor.
		MultiChannelManager() = default;

		/// 
		/// Set the video source id.
		///
		/// @param[in] channelId Channel id.
		/// @param[in] videoSourceId Video source id.
		/// 
		/// @TODO - change this function name, does not indicate correctly what it does.
		void SetVideoSourceId(const std::string& channelName, const uint32_t videoSourceId);

		///
		/// Set the audio source id.
		///
		/// @param[in] channelId Channel id.
		/// @param audioSourceId Audio source id.
		/// 
		/// @TODO - change this function name, does not indicate correctly what it does.
		void SetAudioSourceId(const std::string& channelName, const uint32_t audioSourceId);

		///
		/// Get the packet manager for this channel.
		///
		/// @param[in] channelId Channel id.
		/// 
		/// @return Media channel packet manager.
		const std::shared_ptr<PacketManagerMediaChannel> GetPacketManager(const std::string& channelName) const;

		///
		/// Get media.
		///
		/// @param channelId Channel id.
		/// @param sourceId Source id.
		///
		/// @return Media sample from the packet manager.
		std::shared_ptr<MediaSample> GetMedia(const std::string& channelName, uint32_t sourceId) override;

	protected:
		///
		/// Maps a packet-manager related to particular channel.
		using PacketManagerChannelMap = std::unordered_map<std::string, PacketManager>;

		PacketManagerChannelMap m_packetManagerMediaChannelMap;
	};
}
