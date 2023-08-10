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
/// @class LiveDeviceSource
///
/// Modified 08/18/2019
/// Modified by: M. Kinzer
///

#pragma once
#include <deque>
#include <live555/FramedSource.hh>
#include <live555/RTPSink.hh>
#include "MediaSample.h"
//#include "RtspServerLogging.h"

namespace CvRtsp
{
	class IMediaSampleBuffer;
	class IFrameGrabber;
	class LiveMediaSubsession;
	class IRateAdaptationFactory;
	class IRateAdaptation;
	class IRateController;

	///
	/// The LiveDeviceSource class allows the insertion of media samples into the liveMedia stack.
	/// 
	/// While the live media library is single-threaded it is often necessary to obtain data from a live
	/// source or framework which has it's own event loop. The LiveDeviceSource is based on the live555
	/// DeviceSource class and comprises a mechanism to get data into the liveMedia stack for delivery.
	/// 
	/// @note We do not use the provided trigger mechanism as these extensions are being designed for a 
	/// use case in which each RTPSink may receive media of a different quality. For this reason, we can
	/// not use the reuseFirstSource option provided by OnDemandServerMediaSession. This results in a 
	/// LiveDeviceSource being created per RTSP client. The trigger mechanism in the BasicTaskScheduler
	/// is designed for up to MAX_NUM_EVENT_TRIGGERS=32 triggers making the trigger mechanism incompatible
	/// with our use case.
	class LiveDeviceSource : public FramedSource
	{
	public:
		///
		/// Named constructor
		///
		/// @param env Usage environment.
		/// @param clientId Client identifier.
		/// @param parentSubsession Live media subsession parent class.
		/// @param sampleBuffer Sample buffer.
		/// @param rateAdaptationFactory Rate adaptation factory.
		/// @param rateControl Rate controller.
		///
		/// @return Live video device source.
		static LiveDeviceSource* CreateNew(UsageEnvironment& env, unsigned clientId, LiveMediaSubsession* parentSubsession,
			IMediaSampleBuffer* sampleBuffer, IRateAdaptationFactory* rateAdaptationFactory, IRateController* rateControl);

		///
		/// Destructor
		virtual ~LiveDeviceSource();

		LiveDeviceSource() = delete;

		///
		/// Static method used in live555 callback mechanism
		/// to call deliverFrame()
		///
		/// @param instance Source for the video frame.
		static void DoDeliverFrame(void* instance);

		///
		/// Delivers frame into the live555 pipeline.
		void DeliverFrame();

		///
		/// Retrieves a sample from the media buffer.
		///
		/// @return Returns true if a frame is retrieved from the live source buffer.
		virtual bool RetrieveMediaSampleFromBuffer();

		///
		/// Can be called periodically to process receiver reports.
		void ProcessReceiverReports();

		///
		/// Unique client id.
		///
		/// @return Client id.
		unsigned GetClientId() const
		{
			return m_clientId;
		}

		///
		/// RTPSink.
		///
		/// @return RtpSink.
		RTPSink* GetRtpSink()
		{
			return m_sink;
		}

		///
		/// RTPSink setter.
		///
		/// @param sink RtpSink.
		void SetRtpSink(RTPSink* sink)
		{
			m_sink = sink;
		}

		///
		/// Is media playing.
		///
		/// @return True if media is playing.
		bool IsPlaying() const
		{
			return m_isPlaying;
		}

	protected:
		/// Live555 environment.
		UsageEnvironment& m_env;

		/// Unique client id.
		unsigned m_clientId;

		/// Media subsession.
		LiveMediaSubsession* m_parentSubsession;

		/// Framegrabber.
		IFrameGrabber* m_frameGrabber;

		/// Outgoing sample queue.
		std::deque<std::shared_ptr<MediaSample>> m_mediaSampleQueue;

		/// Live555 RTP sink.
		RTPSink* m_sink;

		/// Start time offsets.
		bool m_useTimeOffset;

		/// Offset time structure.
		struct timeval m_offsetTimeStruct {};

		/// Offset time in double.
		double m_offsetTimeDouble;

		/// True if live device is playing media.
		bool m_isPlaying;

		/// Factory to get optional IRateAdaptation.
		IRateAdaptationFactory* m_rateAdaptationFactory;

		/// Rate adaptation.
		IRateAdaptation* m_rateAdaptation;

		/// Rate control class.
		IRateController* m_rateControl;

		/// Used to determine if RTP transmission stats have new information.
		uint32_t m_lastPacketNumReceived;

		///
		/// Protected constructor.
		///
		/// @param env Usage environment.
		/// @param clientId Client identifier.
		/// @param parentSubsession Live media subsession parent class.
		/// @param frameGrabber Frame grabber.
		/// @param rateAdaptationFactory Rate adaptation factory.
		/// @param rateControl Rate controller.
		LiveDeviceSource(UsageEnvironment& env, unsigned clientId, LiveMediaSubsession* parentSubsession,
			IFrameGrabber* frameGrabber, IRateAdaptationFactory* rateAdaptationFactory, IRateController* rateControl);

		/// Redefined virtual functions from live555 FramedSource.
		void doGetNextFrame() override;
	};
}
