#include "NatusClient.h"
#include <stdexcept>
#include <cstdint>
#include <thread>
#include <chrono>
#include <cstring>
#include <cmath>

#if !_WIN32
#include <netdb.h>
#endif

#include "BCIStream.h"

using namespace std::chrono_literals;
/**************************************************
*
*	NATUS Client IMPLEMENTATION
*****************************************************/

NatusClient::NatusClient() : _receivequeue(), _numChannels(0), _acqThread(NULL), 
	_numSamples(1), _doAquire(false), mSyncCondition(false)
{
	_prevblock.Samples = NULL;
	initWSA();
}

NatusClient::~NatusClient()
{
	/*while (!_receivequeue.empty())
	{
		DeleteSampleBlock(_receivequeue.front());
		_receivequeue.pop();
	}*/
	emptyReceiveQueue();
	deinitWSA();
}

std::uint8_t NatusClient::Ping(std::uint8_t ping)
{
	send_data(CMD_Ping, &ping, sizeof(uint8_t));
	CmdStruct* strct = NULL;
	strct = blockForPackage(1000);
	if (strct->cmd_code != CMD_Ping)
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Unknown or unexpected command code");
	}
	if (strct->is_error)
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Command returned with error!");
	}
	if (strct->payload_len != sizeof(uint8_t))
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Command returned unexpected payload");
	}

	memcpy(&ping, strct->payload, sizeof(uint8_t));
	delete_cmdstruct(strct);
	return ping;
}


bool NatusClient::GetSamples(void* buffer, uint32_t numSamples, uint32_t buffersize)
{
	if (buffersize < numSamples * sizeof(float) * (_numChannels + 1)) //we need to have an item in the queue to check number of channels
		throw std::runtime_error("Buffer is too small for requested data");

	_numSamples = numSamples;

	if (_acqQueue.unsafe_size() < _numSamples)
	{
		std::unique_lock<std::mutex> lock(mConditionMutex);
		bool waitResult = mConditionVariable.wait_for(
			lock, std::chrono::milliseconds(1000 * TIMEOUT_UDP),
			[this]{ return mSyncCondition; }
		);
		if (!waitResult)
			return false;
	}
	NatusSampleBlock* block = NULL;
	float* buf = reinterpret_cast<float*>(buffer);
	for (uint32_t i = 0; i < numSamples; i++)
	{
		while (!_acqQueue.try_pop(block))
			;
		//bciout << block->BlockNumber << std::endl;
		memcpy(buf, block->Samples, sizeof(float) * block->NumChannels);
		buf += block->NumChannels;
		(*buf) = block->Interpolated;
		buf++;

		DeleteSampleBlock(block);
		//elapsed_secs = 0;

	}
	std::lock_guard<std::mutex> lock(mConditionMutex);
	mSyncCondition = false;
	return true;
}


NatusDeviceInformation NatusClient::GetInformation()
{
	send_data(CMD_GetInformation, NULL, 0, false);
	CmdStruct* strct = NULL;
	int i = 0;
	strct = blockForPackage(1000);
	if (strct->is_error)
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Command returned with error!");
	}
	if (strct->cmd_code != CMD_GetInformation)
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Unknown or unexpected command code");
	}

	if (strct->payload_len != sizeof(NatusDeviceInformation))
		throw std::runtime_error("Command returned unexpected payload");
	NatusDeviceInformation information;

	memcpy(&information, strct->payload, sizeof(NatusDeviceInformation));
	//memcpy (&information.NumberOfChannels, strct->payload+ sizeof (uint16_t), sizeof (uint16_t));
	//memcpy (&information.Identifier, strct->payload + 2* sizeof (uint16_t), sizeof (char)*40);
	delete_cmdstruct(strct);
	information.Identifier[IDENTIFIER_LEN - 1] = '\0'; //make sure string is null terminated
	return information;
}



void NatusClient::data_received(CmdStruct* payload)
{
	bool iserror = payload->is_error;
	uint8_t cmd_code = payload->cmd_code;
	delete_cmdstruct(payload);
	printMessage("Received new data from server! (should not happen...)");
	if (iserror)
		throw std::runtime_error("Command returned error");
	switch (cmd_code)
	{
	case CMD_ERR:
		throw std::runtime_error("Command received error response!");

	default:
		throw std::runtime_error("Unknown or unexpected command code");
	}
}

CmdStruct* NatusClient::blockForPackage(uint32_t timeout)
{
	CmdStruct* strct = NULL;
	printMessage("Waiting for package...");
	while ((strct = getNextPackage()) == NULL)
	{
		std::this_thread::sleep_for(1ms);
		timeout--;
		if (timeout == 0)
			throw std::runtime_error("Timeout while waiting for command response");
	};

	return strct;

}

bool NatusClient::init_udp(int port)
{

	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET)
	{
		throw std::runtime_error("Failed to create UDP Socket");
	}


	_udpReceiver.sin_family = AF_INET;
	_udpReceiver.sin_port = htons(port);
	_udpReceiver.sin_addr.s_addr = htonl(INADDR_ANY);

	int iResult = bind(udpSocket, (SOCKADDR*)&_udpReceiver, sizeof(_udpReceiver));
	if (iResult == SOCKET_ERROR)
	{
		std::string errormsg = "Failed to bind UDP Socket: " + std::to_string(errno);
		return false;
	}
	return true;
}

void NatusClient::interpolateMissingBlocks(NatusSampleBlock* block)
{
	if (block->BlockNumber < _prevblock.BlockNumber)
		return;
	uint32_t num_missed_blocks = (uint32_t)(block->BlockNumber - _prevblock.BlockNumber) - 1;
	uint32_t blockNumber = _prevblock.BlockNumber;
	for (uint32_t i = 1; i <= num_missed_blocks; i++)	//interpolate all the missing blocks
	{
		NatusSampleBlock* interpolated_block = new NatusSampleBlock();

		++blockNumber;

		//Fill up block details
		interpolated_block->BlockNumber = blockNumber;
		interpolated_block->NumChannels = _prevblock.NumChannels;
		interpolated_block->Interpolated = 1;
		interpolated_block->Samples = new float[interpolated_block->NumChannels];
		for (uint32_t j = 0; j < interpolated_block->NumChannels; j++)	//Interpolating missing values
			interpolated_block->Samples[j] = NAN;// (((block->Samples[j] - _prevblock.Samples[j]) * ((float)i)) / ((float)num_missed_blocks)+1.0) + _prevblock.Samples[i];

		//Push block in queue
		_receivequeue.push(interpolated_block);
	}
}

void NatusClient::emptyReceiveQueue()
{
	while (!_receivequeue.empty())
	{
		//DeleteSampleBlock(_receivequeue.front());
		NatusSampleBlock* b = _receivequeue.front();
		if (b != NULL)
		{
			delete[] b->Samples;
			delete b;
		}
		_receivequeue.pop();
	}
	while (!_acqQueue.empty())
	{
		NatusSampleBlock* b = NULL;
		if (_acqQueue.try_pop(b))
		{
			if (b != NULL)
			{
				delete[] b->Samples;
				delete b;
			}
		}

	}

}

bool NatusClient::IsConnected()
{
	try {
		if (connection != INVALID_SOCKET && Ping(0xff) == 0xff)
			return true;
		else
			return false;
	}
	catch (std::exception e)
	{
		return false;
	}
}


void NatusClient::Connect(std::string server_name, int port)
{
	printMessage("Connecting to " + server_name + ":" + std::to_string(port));
	int iResult = 0;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(server_name.c_str(), std::to_string(port).c_str(), &hints, &result);
	if (iResult != 0)
	{
		throw std::runtime_error("getaddrinfo failed: " + std::to_string(iResult));

	}

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	connection = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (connection == INVALID_SOCKET)
	{

		freeaddrinfo(result);
		throw std::runtime_error("Error at socket(): " + std::to_string(errno));
	}

	iResult = connect(connection, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(connection);
		connection = INVALID_SOCKET;
		freeaddrinfo(result);
		throw std::runtime_error("Not able to connect to server!");
	}

	if (!init_udp(port + 1))
		throw std::runtime_error("Could not connect with UDP protocol");



}

bool NatusClient::TryConnect(std::string server_name, int port)
{
	printMessage("Connecting to " + server_name + ":" + std::to_string(port));
	int iResult = 0;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(server_name.c_str(), std::to_string(port).c_str(), &hints, &result);
	if (iResult != 0)
	{
		return false;
	}

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	connection = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (connection == INVALID_SOCKET)
	{

		freeaddrinfo(result);
		return false;
	}

	iResult = connect(connection, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(connection);
		connection = INVALID_SOCKET;
		freeaddrinfo(result);
		return false;
	}

	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET)
	{
		std::cout << "Failed to create UDP Socket" << std::endl;
		return false;
	}


	return init_udp(port + 1);


}

void NatusClient::Disconnect()
{
	printMessage("Disconnecting...");
	//	if (connection == INVALID_SOCKET)
	//		return;

	if (udpSocket != INVALID_SOCKET)
	{
		int iResult = closesocket(udpSocket);
		if (iResult == SOCKET_ERROR)
		{
			throw std::runtime_error("Could not close UDP socket: " + std::to_string(errno));
		}
		udpSocket = INVALID_SOCKET;
	}

	if (connection != INVALID_SOCKET)
	{
		int iResult = shutdown(connection, SD_SEND);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(connection);
			throw std::runtime_error("shutdown failed: " + std::to_string(errno));
		}
		closesocket(connection);
		connection = INVALID_SOCKET;
	}
}

void NatusClient::StartStream()
{
	printMessage("Sending Start Stream package");
	_doAquire = true;
	emptyReceiveQueue();
	_acqThread = new std::thread(&NatusClient::aquisitionWorker, this);

	send_data(CMD_StartStream, NULL, 0);
	CmdStruct* strct = NULL;

	strct = blockForPackage(1000);
	if (strct->cmd_code != CMD_StartStream)
		throw std::runtime_error("Unknown or unexpected command code");
	if (strct->is_error)
		throw std::runtime_error("Command returned with error!");
	if (strct->payload_len != 0)
		throw std::runtime_error("Command returned unexpected payload");

	printMessage("Received Start Stream response!");
	delete[] _prevblock.Samples;
	_prevblock.Samples = NULL;



}

void NatusClient::StopStream()
{
	printMessage("Sending Stop Stream package");

	send_data(CMD_StopStream, NULL, 0);
	CmdStruct* strct = NULL;
	strct = blockForPackage(1000);
	if (strct->cmd_code != CMD_StopStream)
		throw std::runtime_error("Unknown or unexpected command code");
	if (strct->is_error)
		throw std::runtime_error("Command returned with error!");
	if (strct->payload_len != 0)
		throw std::runtime_error("Command returned unexpected payload");

	printMessage("Stop Stream response received");
	_doAquire = false;
	if (_acqThread != NULL)
		_acqThread->join();

	//while (!_acqQueue.empty()) { delete _acqQueue.front(); _acqQueue.pop(); }
	emptyReceiveQueue();

	delete _acqThread;
	_acqThread = NULL;
}

NatusSampleBlock* NatusClient::ReadStream()
{
	NatusSampleBlock* block = NULL;
	CmdStruct* strct = NULL;
	if (!_receivequeue.empty())
	{
		block = _receivequeue.front();
		_receivequeue.pop();
		return block;
	}
	
	try
	{
		strct = getNextUDPPackage();
	}
	catch (std::exception e)
	{
		std::cout << "Error during receive package! " << std::endl;
		return NULL;
	}
	if (strct == NULL)
		return block;

	if (strct->is_error)
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Command returned with error!");
	}

	if (strct->payload_len < 2 * sizeof(uint32_t))
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Command returned unexpected payload");
	}
	printMessage("Stream package received!");
	switch (strct->cmd_code)
	{
	case CMD_StreamPackage:
	{
		block = create_natus_sampleblock(strct->payload);
		delete_cmdstruct(strct);
		//check for missing data and create it
		if (!_receivequeue.empty())
			updatePreviousBlock(_receivequeue.back());

		if (_prevblock.Samples != NULL)
		{
			if ((block->BlockNumber - _prevblock.BlockNumber) != 1 && _prevblock.BlockNumber != UINT32_MAX)
				interpolateMissingBlocks(block);
		}
		updatePreviousBlock(block);
		_receivequeue.push(block);
		break;
	}
	case CMD_VariableSizeStreamPackage: //curly braces needed for scope declaration
	{
		uint32_t send_space = reinterpret_cast<uint32_t*>(strct->payload)[0];
		if ((strct->payload_len - sizeof(uint32_t)) % send_space != 0)
			throw std::runtime_error("Received Variable Size data is not composed of full Streaming blocks");

		uint32_t num_blocks = (strct->payload_len - sizeof(uint32_t)) / send_space;
		//first  block
		block = create_natus_sampleblock(strct->payload + sizeof(uint32_t));
		//Check if packet is missed & interpolate values if missed
		if (!_receivequeue.empty())
			updatePreviousBlock(_receivequeue.back());
		if (_prevblock.Samples != NULL)
		{
			if ((block->BlockNumber - _prevblock.BlockNumber) != 1 && _prevblock.BlockNumber != UINT32_MAX)
				interpolateMissingBlocks(block);

		}

		//Push the first block
		_receivequeue.push(block);

		//Add rest of the blocks
		for (int i = 1; i < num_blocks; ++i)
		{
			block = create_natus_sampleblock(strct->payload + sizeof(uint32_t) + send_space * i);
			_receivequeue.push(block);
		}
		updatePreviousBlock(block);
		delete_cmdstruct(strct);
		break;
	}

	default:
		delete_cmdstruct(strct);
		throw std::runtime_error("Unknown or unexpected command code");
		break;
	}

	block = _receivequeue.front();
	_receivequeue.pop();

	if (_receivequeue.empty())	//If queue is empty, store current block value to check if next packet is lost or not
		updatePreviousBlock(block);

	return block;

}

NatusSampleBlock* NatusClient::create_natus_sampleblock(uint8_t* payload)
{
	NatusSampleBlock* block = new NatusSampleBlock();
	block->BlockNumber = reinterpret_cast<uint32_t*>(payload)[0];
	block->NumChannels = reinterpret_cast<uint32_t*>(payload)[1];
	block->Interpolated = 0;
	block->Samples = new float[block->NumChannels];
	memcpy(block->Samples, payload + (2 * sizeof(uint32_t)), block->NumChannels * sizeof(float));
	return block;
}

void NatusClient::DeleteSampleBlock(NatusSampleBlock* b)
{
	if (b != NULL)
		delete b->Samples;

	delete b;
}

void NatusClient::updatePreviousBlock(NatusSampleBlock* b)
{
	delete[] _prevblock.Samples;
	_prevblock.BlockNumber = b->BlockNumber;
	_prevblock.NumChannels = b->NumChannels;
	_prevblock.Interpolated = b->Interpolated;
	_prevblock.Samples = new float[b->NumChannels];
	memcpy(_prevblock.Samples, b->Samples, b->NumChannels * sizeof(float));
}

void NatusClient::aquisitionWorker()
{
	NatusSampleBlock* block = NULL;
	while (_doAquire)
	{
		if ((block = ReadStream()) != NULL)
		{
			_numChannels = block->NumChannels;
			_acqQueue.push(block);
			if (_acqQueue.unsafe_size() > _numSamples)
			{
				std::unique_lock<std::mutex> lock(mConditionMutex);
				mSyncCondition = true;
				lock.unlock();
				mConditionVariable.notify_all();
			}
		}
	}
}

void NatusClient::SetSampleBlockSize(uint32_t blocksize)
{
	printMessage("Sending Set Sampleblocksize package");
	send_data(CMD_SetBlockSize, reinterpret_cast<uint8_t*>(&blocksize), sizeof(uint32_t));
	CmdStruct* strct = NULL;
	strct = blockForPackage(1000);
	if (strct->cmd_code != CMD_SetBlockSize)
		throw std::runtime_error("Unknown or unexpected command code response for SetSampleBlockSize");
	if (strct->is_error)
		throw std::runtime_error("Command returned with error!");
	if (strct->payload_len != 0)
		throw std::runtime_error("Command returned unexpected payload");

	printMessage("Received Start Stream response!");
}

void NatusClient::SetDecimationFactor(uint32_t decimationFactor)
{
	printMessage("Sending Set DecimationFactor package");
	send_data(CMD_SetDecimationFactor, reinterpret_cast<uint8_t*>(&decimationFactor), sizeof(uint32_t));
	CmdStruct* strct = NULL;
	strct = blockForPackage(1000);
	if (strct->cmd_code != CMD_SetDecimationFactor)
		throw std::runtime_error("Unknown or unexpected command code response for SetDecimationFactor");
	if (strct->is_error)
		throw std::runtime_error("Command returned with error!");
	if (strct->payload_len != 0)
		throw std::runtime_error("Command returned unexpected payload");

	printMessage("Received Start Stream response!");
}

NatusChannelInformation NatusClient::GetChannelInfo()
{
	send_data(CMD_GetChannelNames, NULL, 0, false);
	CmdStruct* strct = NULL;
	int i = 0;
	strct = blockForPackage(1000);
	if (strct->is_error)
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Command returned with error!");
	}
	if (strct->cmd_code != CMD_GetChannelNames)
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Unknown or unexpected command code");
	}

	NatusChannelInformation channelInformation;
	uint32_t numChannels = 0;
	uint32_t numStreams = 0;

	memcpy(&numChannels, strct->payload, sizeof(uint32_t));
	memcpy(&numStreams, strct->payload + sizeof(uint32_t), sizeof(uint32_t));
	char* curr_d = reinterpret_cast<char*>(strct->payload) + sizeof(uint32_t) * 2;
	for (uint32_t i = 0; i < numChannels; i++)
	{
		std::string buf(curr_d);
		channelInformation.ChannelNames.push_back(buf);
		curr_d += buf.length() + 1;

	}
	for (uint32_t i = 0; i < numStreams; i++)
	{
		std::string buf(curr_d);
		channelInformation.StreamNames.push_back(buf);
		curr_d += buf.length() + 1;

	}

	delete_cmdstruct(strct);

	return channelInformation;

}

int NatusClient::GetDecimationFactor()
{
	send_data(CMD_GetDecimationFactor, NULL, 0, false);
	CmdStruct* strct = NULL;
	int i = 0;
	strct = blockForPackage(1000);
	if (strct->is_error)
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Command returned with error!");
	}
	if (strct->cmd_code != CMD_GetDecimationFactor)
	{
		delete_cmdstruct(strct);
		throw std::runtime_error("Unknown or unexpected command code");
	}
	if (strct->payload_len <= 0) {
		delete_cmdstruct(strct);
		throw std::runtime_error("Command returned unexpected payload");
	}
	uint32_t decimationFactor;
	memcpy(&decimationFactor, strct->payload, strct->payload_len);
	delete_cmdstruct(strct);
	return decimationFactor;

}

int NatusClient::SelectChannelsToStream(std::vector<uint32_t> channels)
{
	printMessage("Sending Channels to Stream package");
	std::string send_string;
	for (int i = 0; i < channels.size(); i++)
		send_string += std::to_string(channels[i]) + " ";
	send_data(CMD_SelectChannels, reinterpret_cast<const uint8_t*>(send_string.c_str()), send_string.length());
	CmdStruct* strct = NULL;
	strct = blockForPackage(1000);
	if (strct->is_error)
		throw std::runtime_error(("Command returned with error! CMD_CODE: " + std::to_string(strct->cmd_code)).c_str());
	if (strct->cmd_code != CMD_SelectChannels)
		throw std::runtime_error(("Unknown or unexpected command code response for SelectChannelsToStream " + std::to_string(strct->cmd_code)).c_str());

	if (strct->payload_len != 0)
		throw std::runtime_error("Command returned unexpected payload");

	return channels.size();

	printMessage("Received Start Stream response!");
}
