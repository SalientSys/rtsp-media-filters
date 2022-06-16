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
/// @class LiveH265VideoDeviceSource
///
/// Modified 08/16/2019
/// Modified by: M. Kinzer
///

#pragma once

#include "LiveDeviceSource.h"
#include "CommonRtsp.h"

namespace CvRtsp
{
	class LiveH265VideoDeviceSource : public LiveDeviceSource
	{
	public:
		///
		/// Destructor.
		virtual ~LiveH265VideoDeviceSource() = default;

		///
		/// Named constructor.
		///
		/// @param[in] env						Usage environment.
		/// @param[in] clientId					Client identifier.
		/// @param[in] parentSubsession			Live media subsession parent class.
		/// @param[in] vps						Video parameter set.
		/// @param[in] sps						Sequence parameter set.
		/// @param[in] pps						Picture parameter set.
		/// @param[in] frameGrabber				Frame grabber.
		/// @param[in] rateAdaptationFactory	Rate adaptation factory.
		/// @param[in] globalRateControl		Rate controller.
		///
		/// @return Live video device source.
		static LiveH265VideoDeviceSource* CreateNew(UsageEnvironment& env, unsigned clientId,
			LiveMediaSubsession* parentSubsession, const std::string& vps, const std::string& sps, const std::string& pps,
			IFrameGrabber* frameGrabber, IRateAdaptationFactory* rateAdaptationFactory,
			IRateController* globalRateControl);

		///
		/// Method to add data to the device. Overridden from LiveDeviceSource base class.
		///
		/// @return True	Is media sample is retrieved from the buffer.
		bool RetrieveMediaSampleFromBuffer() override;

	protected:
		///
		/// Default constructor.
		///
		/// @param[in] env						Usage environment.
		/// @param[in] clientId					Client identifier.
		/// @param[in] parentSubsession			Live media subsession parent class.
		/// @param[in] vps						Video parameter set.
		/// @param[in] sps						Sequence parameter set.
		/// @param[in] pps						Picture parameter set.
		/// @param[in] frameGrabber				Frame grabber.
		/// @param[in] rateAdaptationFactory	Rate adaptation factory.
		/// @param[in] globalRateControl		Rate controller.
		LiveH265VideoDeviceSource(UsageEnvironment& env, unsigned clientId,
			LiveMediaSubsession* parentSubsession,
			const std::string& vps, const std::string& sps, const std::string& pps,
			IFrameGrabber* frameGrabber, IRateAdaptationFactory* rateAdaptationFactory,
			IRateController* globalRateControl);

	private:
		///
		/// Split the payload into multiple media samples for sending via live555 pipeline.
		///
		/// @param[in] dataBuffer	Data to be sent.
		/// @param[in] bufferSize	Size of the data buffer.
		/// @param[in] startTime	Media sample start time.
		///
		/// @return	Queue of media samples.
		std::deque<std::shared_ptr<MediaSample>> splitPayloadIntoMediaSamples(BYTE* dataBuffer,
			uint32_t bufferSize, double startTime);

		// --- Data
				/// True if waiting for intra-random-access-point-picture (~ keyframe, where we can start decoding).
		bool m_isWaitingForIRAP;
	};
}
