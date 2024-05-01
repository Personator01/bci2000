#include "NetComCommands.h"

const std::wstring NetComCommands::START_ACQUISITION = L"-StartAcquisition";
//const std::wstring NetComCommands::STOP_ACQUISITION = L"-StopAcquisition";
const std::wstring NetComCommands::START_RECORDING = L"-StartRecording";
//const std::wstring NetComCommands::STOP_RECORDING = L"-StopRecording";
const std::wstring NetComCommands::GET_PROCESSING_ENTITY_ENABLED = L"-GetAcqEntProcessingEnabled";
const std::wstring NetComCommands::GET_CONVERSION_FACTOR = L"-GetVoltageConversion";

const std::wstring NetComCommands::GET_BLOCKSIZE = L"-GetSubSamplingInterleave";
const std::wstring NetComCommands::SET_BLOCKSIZE = L"-SetSubSamplingInterleave";

const std::wstring NetComCommands::GET_HARDWARE_SUBSYSTEM_INFORMATION = L"-GetHardwareSubSystemInformation";

const std::wstring NetComCommands::SET_NETCOMDATABUFFER_ENABLE = L"-SetNetComDataBufferingEnabled";
const std::wstring NetComCommands::GET_SAMPLINGFREQUENCY = L"-GetSampleFrequency";
const std::wstring NetComCommands::GET_SUBSAMPLING = L"-GetSubSamplingInterleave";
// const wchar_t* NetComCommands::SET_SUBSAMPLING = L"-";

// const wchar_t* NetComCommands::SET_DSP_HIGHPASS_STATE = L"-";
 // const wchar_t* NetComCommands::SET_DSP_LOWPASS_STATE = L"-";

const std::wstring NetComCommands::GET_DSP_HIGHPASS_STATE = L"-GetDspHighCutFilterEnabled";
const std::wstring NetComCommands::GET_DSP_LOWPASS_STATE = L"-GetDspLowCutFilterEnabled";

//const wchar_t* NetComCommands::SET_CHANNEL_STATE = L"-";
const std::wstring NetComCommands::GET_CHANNEL_NUMBER = L"-GetChannelNumber";
const std::wstring NetComCommands::GET_CHANNEL_STATE = L"-GetChannelNumber";

const std::wstring NetComCommands::GET_DIGITAL_IO_PORT_LIST = L"-GetDigitalIOBoardList";
const std::wstring NetComCommands::SET_DIGITAL_IO_PORT_DIRECTION = L"-SetDigitalIOPortDirection";
const std::wstring NetComCommands::SET_DIGITAL_IO_BIT = L"-SetDigitalIOBit";

bool NetComCommands::StartRecording(NlxNetCom::NetComClient * c)
{
	std::wstring reply;
	return c->SendCommand(START_RECORDING.c_str(), reply);
}

bool NetComCommands::GetHardwareSubsystemInformation(NlxNetCom::NetComClient *c, uint32_t& sample_freq, uint32_t& num_boards)
{
	return true;
}

bool NetComCommands::SetNetComDataBuffer(NlxNetCom::NetComClient * c, std::wstring DASobject, bool enable)
{
	std::wstring reply;
	return c->SendCommand((SET_NETCOMDATABUFFER_ENABLE + L" " + DASobject + ((enable)? L" True" : L" False")).c_str(), reply);
}

bool NetComCommands::StartAcquisition(NlxNetCom::NetComClient *c)
{
	std::wstring reply;
	return c->SendCommand(START_ACQUISITION.c_str(), reply);
}

bool NetComCommands::GetBlockSize(NlxNetCom::NetComClient *c, std::wstring DASobject, uint32_t & blocksize)
{
	std::wstring reply;
	if (c->SendCommand((GET_BLOCKSIZE + L" " + DASobject).c_str(), reply))
	{
		auto tokens = _tokenize(reply, L' ');
		if (std::stoi(tokens[0]) != 0)
			return false;

		blocksize = std::stoi(tokens[1]);
	}
	else return false;

	return true;
}

bool NetComCommands::SetBlockSize(NlxNetCom::NetComClient *c, std::wstring DASobject, uint32_t blocksize)
{
	std::wstring reply;
	if (c->SendCommand((SET_BLOCKSIZE + L" " + DASobject + L" " + std::to_wstring(blocksize)).c_str(), reply))
	{
		auto tokens = _tokenize(reply, L' ');
		if (std::stoi(tokens[0]) != 0)
			return false;
	}
	else return false;

	return true;
}

bool NetComCommands::GetSamplingRate(NlxNetCom::NetComClient *c, std::wstring DASobject, uint32_t & samplingrate)
{
	std::wstring reply;
	if (c->SendCommand((GET_SAMPLINGFREQUENCY + L" " + DASobject).c_str(), reply))
	{
		auto tokens = _tokenize(reply, L' ');
		if (std::stoi(tokens[0]) != 0)
			return false;

		samplingrate = std::stoi(tokens[1]);
	}
	else return false;

	return true;
}

bool NetComCommands::GetSubSampling(NlxNetCom::NetComClient *c, std::wstring DASobject, uint32_t & subsampling)
{
	std::wstring reply;
	if (c->SendCommand((GET_SUBSAMPLING + L" " + DASobject).c_str(), reply))
	{
		auto tokens = _tokenize(reply, L' ');
		if (std::stoi(tokens[0]) != 0)
			return false;
		subsampling = std::stoi(tokens[1]);
	}
	else return false;

	return true;
}

bool NetComCommands::GetEffectiveSamplingRate(NlxNetCom::NetComClient *c, std::wstring DASACqObj, std::wstring DASchannel, int & samplingrate)
{
	uint32_t sr;
	uint32_t subs;

	if (NetComCommands::GetSamplingRate(c, DASACqObj, sr) && NetComCommands::GetSubSampling(c, DASchannel, subs))
	{
		samplingrate = float(sr) / float(subs);
	}
	else
		return false;

	return true;
}

bool NetComCommands::GetVoltageConversionFactor(NlxNetCom::NetComClient *c, std::wstring DASobject, std::vector<float> & conversionFactor)
{
	std::wstring reply;
	conversionFactor.clear();
	if (c->SendCommand((GET_CONVERSION_FACTOR + L" " + DASobject).c_str(), reply))
	{
		auto tokens = _tokenize(reply, L' ');
		if (std::stoi(tokens[0]) != 0)
			return false;

		for(int i=0; i < tokens.size()-1;i++)
		conversionFactor.push_back(std::stof(tokens[i+1]));
	}
	else return false;

	return true;
}

bool NetComCommands::GetHighpassStatus(NlxNetCom::NetComClient *c, std::wstring DASobject, bool & status)
{
	std::wstring reply;
	if (c->SendCommand((GET_DSP_HIGHPASS_STATE + L" " + DASobject).c_str(), reply))
	{
		auto tokens = _tokenize(reply, L' ');
		if (std::stoi(tokens[0]) != 0)
			return false;

		status = (tokens[1] == L"True");
	}
	else
		return false;

	return true;
}

bool NetComCommands::GetLowpassStatus(NlxNetCom::NetComClient *c, std::wstring DASobject, bool & status)
{
	std::wstring reply;
	if (c->SendCommand((GET_DSP_LOWPASS_STATE + L" " + DASobject).c_str(), reply))
	{
		auto tokens = _tokenize(reply, L' ');
		if (std::stoi(tokens[0]) != 0)
			return false;

		status = (tokens[1] == L"True");
	}
	else
		return false;

	return true;
}

bool NetComCommands::GetAcqEntityProcessingEnabled(NlxNetCom::NetComClient *c, std::wstring DASobject, bool & status)
{
	std::wstring reply;
	if (c->SendCommand((GET_PROCESSING_ENTITY_ENABLED + L" " + DASobject).c_str(), reply))
	{
		auto tokens = _tokenize(reply, L' ');
		if (std::stoi(tokens[0]) != 0)
			return false;

		status = (tokens[1] == L"True");
	}
	else
		return false;

	return true;
}

bool NetComCommands::GetChannelNumber(NlxNetCom::NetComClient *c, std::wstring DASobject, std::vector<uint32_t>& ch)
{
	std::wstring reply;
	ch.clear();
	if (c->SendCommand((GET_CHANNEL_NUMBER + L" " + DASobject).c_str(), reply))
	{
		auto tokens = _tokenize(reply, L' ');
		if (std::stoi(tokens[0]) != 0)
			return false;
		for (int i = 0; i < tokens.size() - 1; i++)
			ch.push_back(std::stoi(tokens[i + 1]));
	}
	else
		return false;

	return true;
}

bool NetComCommands::GetDigitalIOBoardList(NlxNetCom::NetComClient* c, std::vector<std::wstring>& list)
{
	list.clear();
	std::wstring reply;
	if (!c->SendCommand(GET_DIGITAL_IO_PORT_LIST.c_str(), reply))
		return false;
	list = _tokenize(reply, ' ');
	return true;
}

bool NetComCommands::SetDigitalIOPortDirection(NlxNetCom::NetComClient* c, std::wstring device, int port, Direction dir)
{
	std::wstring command = SET_DIGITAL_IO_PORT_DIRECTION + L" " + device
	                        + L" " + std::to_wstring(port) + L" ";
	if (dir == Input)
	  command += L"Input";
	else if (dir == Output)
	  command += L"Output";
	std::wstring reply;
	return c->SendCommand(command.c_str(), reply);
}

bool NetComCommands::SetDigitalIOBit(NlxNetCom::NetComClient* c, std::wstring device, int port, int bit,  bool value)
{
	std::wstring command = SET_DIGITAL_IO_BIT + L" " + device
		+ L" " + std::to_wstring(port) + L" " + std::to_wstring(bit) + L" ";
	if (value)
		command += L"On";
	else
		command += L"Off";
	std::wstring reply;
	return c->SendCommand(command.c_str(), reply);
}

std::vector<std::wstring> NetComCommands::_tokenize(std::wstring s, wchar_t delim)
{
	std::vector<std::wstring> ret;
	std::wstring temp;
	std::wstringstream wss(s);
	while (std::getline(wss, temp, delim))
		ret.push_back(temp);

	return ret;
}
