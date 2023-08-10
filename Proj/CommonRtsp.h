#ifndef COMMONRTSP_H
#define COMMONRTSP_H

#pragma once

namespace CvRtsp
{
	/// Nal unit prefix.
	const BYTE NalUnitPrefix[3] = { 0x00, 0x00, 0x01 };

	/// nal unit prefix containing a leading zero bit.
	const BYTE NalUnitPrefixWithZeroBit[4] = { 0x00, 0x00, 0x00, 0x01 };

	/// Size of a nal unit prefix.
	const int NalUnitPrefixWithoutZeroBitSize = 3;

	/// Size of a nal unit prefix that contains a leading zero bit.
	const int NalUnitPrefixWithZeroBitSize = 4;

	///
	/// Determine if nal unit header contains an idr frame.
	///
	/// @param[in] nalUnitHeader Nal unit header.
	///
	/// @return True if the nal unit is an Idr frame.
	static bool isH264IdrFrame(unsigned char nalUnitHeader)
	{
		return (nalUnitHeader & 0x1f) == 5;
	}

	///
	/// Determine if nal unit header is a sequence parameter set.
	///
	/// @param[in] nalUnitHeader Nal unit header.
	///
	/// @return True if the nal unit is a sps.
	static bool isH264Sps(unsigned char nalUnitHeader)
	{
		return (nalUnitHeader & 0x1f) == 7;
	}

	///
	/// Determine if nal unit header is a picture parameter set.
	///
	/// @param[in] nalUnitHeader Nal unit header.
	///
	/// @return True if the nal unit is a pps.
	static bool isH264Pps(unsigned char nalUnitHeader)
	{
		return (nalUnitHeader & 0x1f) == 8;
	}

#pragma region h265
	///
	/// Determine if nal unit header is a video parameter set.
	///
	/// @param[in] nalUnitHeader Nal unit header.
	///
	/// @return True if the nal unit is a vps.
	static bool isH265Vps(unsigned char nalUnitHeader)
	{
		return ((nalUnitHeader & 0x7e) >> 1) == 32;
	}

	///
	/// Determine if h265 nal unit header is a sequence parameter set.
	///
	/// @param[in] nalUnitHeader Nal unit header.
	///
	/// @return True if the nal unit is a sps.
	static bool isH265Sps(unsigned char nalUnitHeader)
	{
		return ((nalUnitHeader & 0x7e) >> 1) == 33;
	}

	///
	/// Determine if h265 nal unit header is a picture parameter set.
	///
	/// @param[in] nalUnitHeader Nal unit header.
	///
	/// @return True if the nal unit is a pps.
	static bool isH265Pps(unsigned char nalUnitHeader)
	{
		return ((nalUnitHeader & 0x7e) >> 1) == 34;
	}

	///
	/// Determine if h265 nal unit header contains IRAP.
	///
	/// @param[in] nalUnitHeader Nal unit header.
	///
	/// @return True if the nal unit is an IRAP (BLA/IDR/CRA).
	static bool isH265RandomAccessPointPicture(unsigned char nalUnitHeader)
	{
		int val = (nalUnitHeader & 0x7e) >> 1;
		return (val > 15 && val < 22);
	}
#pragma endregion

#pragma region MPEG4
	namespace MPEG4_Codes
	{
		const BYTE VISUAL_OBJECT_SEQUENCE_START_CODE[] = { 0x00,0x00,0x01,0xb0 };

		const BYTE VISUAL_OBJECT_SEQUENCE_END_CODE[] = { 0x00,0x00,0x01,0xb1 };

		const BYTE USER_DATA_START_CODE[] = { 0x00,0x00,0x01,0xb2 };

		const BYTE GROUP_VOP_START_CODE[] = { 0x00,0x00,0x01,0xb3 };

		const BYTE VISUAL_OBJECT_START_CODE[] = { 0x00,0x00,0x01,0xb5 };

		const BYTE VOP_START_CODE[] = { 0x00,0x00,0x01,0xb6 };
	}
#pragma endregion


#pragma region MediaSubTypes	
	/// Media Types
	namespace MediaSubType
	{
		static const std::string H264("H264");
		static const std::string MPEG4("MPEG4");
		static const std::string MJPEG("MJPEG");
		static const std::string H265("H265");
		static const std::string AMR("AMR");
		static const std::string AAC("AAC");
		static const std::string MMF("MMF");
	}
#pragma endregion


#pragma region Ports
	static const int DEFAULT_RTSP_PORT = 554;
#pragma endregion


#pragma region TimeFunctions
///
/// Converts seconds to micro-seconds.
///
/// @param[in]	seconds	Seconds to convert.
///
/// @return	Converted value in micro-seconds.
template<typename T>
auto SecondToMicroSecond(T seconds)
{
	return seconds * pow(10, 6);
}


///
/// Converts milli-seconds to micro-seconds.
///
/// @param[in]	milliSeconds	Milli-Seconds to convert.
///
/// @return	Converted value in micro-seconds.
template<typename T>
auto MilliSecondToMicroSecond(T milliSeconds)
{
	return SecondToMicroSecond<decltype(milliSeconds* pow(10.0, -3))>(milliSeconds * pow(10.0, -3));
}
#pragma endregion

}
#endif // COMMONRTSP_H