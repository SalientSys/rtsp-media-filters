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
/// @class LiveSourceTaskScheduler0
///
/// Modified 08/21/2019
/// Modified by: M. Kinzer
///
#pragma once

#include <map>
#include <live555/BasicUsageEnvironment.hh>

#include "LiveDeviceSource.h"
#include "LiveMediaSubsession.h"

namespace CvRtsp
{
	/// Forward declarations
	class ChannelManager;
	class LiveMediaSubsession;

	using LiveMediaSubSessionMap = std::map<std::pair<std::string, unsigned>, LiveMediaSubsession*>;

	/// Maximum number of cycles in the while loop.
	const unsigned MaxRevolutions = 60;

	/// Maximum number of samples to be processed in the event loop.
	//const unsigned MaxSamplesToBeProcessedInEventLoop = 30;

	///
	/// The LiveSourceTaskScheduler0 class is aware of the media subsessions that
	/// have been created. Each media subsession must register itself on construction
	/// and deregister itself on destruction.
	class LiveSourceTaskScheduler0 : public BasicTaskScheduler/*, public ThreadedObject*/
	{
	public:
		///
		/// Destructor.
		~LiveSourceTaskScheduler0() = default;

		/// Overriding from class BasicTaskScheduler0; this so that we can control the watch variable. 
		/// 
		/// @param[in] watchVariable Variable to control the event loop.
		///
		/// @remark Method naming does not match convention, but instead, that of the base class
		/// that is being overridden.
		void doEventLoop(char volatile* watchVariable) override;

		/// Set the maximum poll delay time. 
		///
		/// @param[in] maxDelayTimeMicroSec Maximum poll delay time in microseconds.
		void SetMaximumPollDelay(unsigned maxDelayTimeMicroSec)
		{
			m_maxDelayTimeMicroSec = maxDelayTimeMicroSec;
		}

		/// 
		/// Registers a LiveMediaSubsession with the scheduler.
		///
		/// @param[in] channelId Channel id.
		/// @param[in] sourceId Source id.
		/// @param[in] mediaSubsession Live media subsession.
		void AddMediaSubsession(const std::string& channelName, uint32_t sourceId, LiveMediaSubsession* mediaSubsession);

		/// 
		/// Deregisters a LiveMediaSubsession from the scheduler.
		///
		/// @param[in] channelId Channel id.
		/// @param[in] sourceId Source id.
		/// @param[in] mediaSubsession Live media subsession.
		void RemoveMediaSubsession(const std::string& channelName, uint32_t sourceId, LiveMediaSubsession* mediaSubsession);

		/// 
		/// Deregisters a LiveMediaSubsession from the scheduler.
		///
		/// @param[in] channelId Channel id.
		/// @param[in] sourceId Source id.
		/// 
		/// @return mediaSubsession Live media subsession.
		LiveMediaSubsession* GetMediaSubsession(const std::string& channelName, uint32_t sourceId);

		/// 
		/// Processes all registered media subsessions.
		void ProcessLiveMediaSessions();

		void OnMediaReceived(std::vector<std::shared_ptr<MediaSample>> mediaSamples, CvRtsp::LiveMediaSubsession*);

#pragma region Thread Processing
		//private:
		//	/// Exit event handle, used to signal class exit.
		//	///
		//	SmartHandle m_exitEvent;

		//public:
		//	/// Start background 
		//	///
		//	virtual void Start()
		//	{
		//		beginThread();
		//	}

		//	/// Stop background 
		//	///
		//	void Stop()
		//	{
		//		SetEvent(m_exitEvent);
		//	}
#pragma endregion

	protected:
		///
		/// Constructor.
		LiveSourceTaskScheduler0(ChannelManager& channelManager);

	private:
		/// "m_maxDelayTimeMicroSec" is in microseconds. Default time of once per second.
		unsigned m_maxDelayTimeMicroSec = 10;

		/// Packet manager that receives media packets from device/network interface.
		ChannelManager& m_channelManager;

		///
		int m_samplesReceived;

		///
		bool m_hasRun;

		/// Map which stores ALL media subsessions. Each subsession is identified via a unique id.
		//MediaSessionMap m_mediaSessions;
		LiveMediaSubSessionMap m_mediaSubSessions;

		/// Helper method to process media samples within the live555 event loop.
		void processLiveSources();
	};
}
