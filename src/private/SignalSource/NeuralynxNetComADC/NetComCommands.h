#ifndef INCLUDED_NETCOMCOMMANDS_H  // makes sure this header is not included more than once
#define INCLUDED_NETCOMCOMMANDS_H

#include <string>
#include "lib/include/NetComClient.h"
#include <sstream>
class NetComCommands
{

public:
	static bool StartRecording(NlxNetCom::NetComClient*);
	static bool StartAcquisition(NlxNetCom::NetComClient*);

	static bool GetBlockSize(NlxNetCom::NetComClient*, std::wstring DASobject, uint32_t &blocksize);
	static bool SetBlockSize(NlxNetCom::NetComClient*, std::wstring DASobject, uint32_t blocksize);

	static bool GetSamplingRate(NlxNetCom::NetComClient*, std::wstring DASobject, uint32_t &samplingrate);
	static bool GetSubSampling(NlxNetCom::NetComClient*, std::wstring DASobject, uint32_t &subsampling);
	static bool GetEffectiveSamplingRate(NlxNetCom::NetComClient *c, std::wstring DASACqObj, std::wstring DASchannel, int & samplingrate);

	static bool GetVoltageConversionFactor(NlxNetCom::NetComClient*, std::wstring DASobject, std::vector<float> &conversionFactor);

	static bool GetHighpassStatus(NlxNetCom::NetComClient*, std::wstring DASobject, bool &status);
	static bool GetLowpassStatus(NlxNetCom::NetComClient*, std::wstring DASobject, bool &status);
	static bool GetAcqEntityProcessingEnabled(NlxNetCom::NetComClient*, std::wstring DASobject, bool &status);

	static bool GetChannelNumber(NlxNetCom::NetComClient*, std::wstring DASobject, std::vector<uint32_t>&);
	static bool GetHardwareSubsystemInformation(NlxNetCom::NetComClient *c, uint32_t& sample_freq, uint32_t& num_boards);
	static bool SetNetComDataBuffer(NlxNetCom::NetComClient *c,  std::wstring DASobject, bool enable);
	//bool StopRecording(NlxNetCom::NetComClient*);

	static bool GetDigitalIOBoardList(NlxNetCom::NetComClient*, std::vector<std::wstring>&);
	enum Direction { Input, Output };
	static bool SetDigitalIOPortDirection(NlxNetCom::NetComClient*, std::wstring Device, int port, Direction);
	static bool SetDigitalIOBit(NlxNetCom::NetComClient*, std::wstring Device, int port, int bit, bool value);

private:
	 NetComCommands();
	 static const std::wstring START_RECORDING;

	 static const std::wstring GET_CONVERSION_FACTOR;
	//static const std::wstring STOP_RECORDING;
	 static const std::wstring START_ACQUISITION;
	 //static const std::wstring STOP_ACQUISITION;

	 static const std::wstring GET_BLOCKSIZE;
	 static const std::wstring SET_BLOCKSIZE;
	 static const std::wstring GET_HARDWARE_SUBSYSTEM_INFORMATION;
	 static const std::wstring GET_PROCESSING_ENTITY_ENABLED;

	 static const std::wstring SET_NETCOMDATABUFFER_ENABLE;


	 static const std::wstring GET_SAMPLINGFREQUENCY;
	 static const std::wstring GET_SUBSAMPLING;
	// static const wchar_t* SET_SUBSAMPLING;

	 // static const wchar_t* SET_DSP_HIGHPASS_STATE;
	 // static const wchar_t* SET_DSP_LOWPASS_STATE;

	 static const std::wstring GET_DSP_HIGHPASS_STATE;
	 static const std::wstring GET_DSP_LOWPASS_STATE;

	// static const wchar_t* SET_CHANNEL_STATE;
	 static const std::wstring GET_CHANNEL_NUMBER;
	 static const std::wstring GET_CHANNEL_STATE;

	 static const std::wstring GET_DIGITAL_IO_PORT_LIST;
	 static const std::wstring SET_DIGITAL_IO_PORT_DIRECTION;
	 static const std::wstring SET_DIGITAL_IO_BIT;

	 static std::vector<std::wstring> _tokenize(std::wstring s, wchar_t delim);
};

#endif
