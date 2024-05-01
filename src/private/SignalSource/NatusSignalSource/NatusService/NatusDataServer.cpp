#include "stdafx.h"
#include "NatusDataServer.h"
#include <exception>
#include <string>


/**************************************************
*
*	NATUS Data Server IMPLEMENTATION
*****************************************************/

NatusDataServer::NatusDataServer() : _infoCallback(NULL), _errorCallback(NULL), _currPackage(0), _sendBuffer(NULL), _sendBufferSize(DEFAULT_BUFFER_LEN), _srvSocket(INVALID_SOCKET),
_infoCallbackParent(NULL), _errorCallbackParent(NULL), _decimationFactor(1), _requiredAvialableBlocks(1), _runServer(true), _blockSize(1), _dataQueue()
{
	_sendBuffer = new uint8_t[_sendBufferSize];
	_blockSize = 1;
	m_sync_lock = CreateEvent(NULL, false, false, NULL);

}

NatusDataServer::~NatusDataServer()
{
	_runServer = false;
	SetEvent(m_sync_lock);
	_networkThread.join();
	_udpThread.join();
	delete[] _sendBuffer;
	emptySendQueue();
	CloseHandle(m_sync_lock);
}

void NatusDataServer::Startup(std::uint16_t port)
{
	printMessage("Starting server on port " + std::to_string(port));
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	initWSA();
	// Resolve the local address and port to be used by the server
	int iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
	if (iResult != 0)
	{
		WSACleanup();
		throw std::exception("getaddrinfo failed: %d\n", iResult);
	}

	_srvSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (_srvSocket == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		WSACleanup();
		throw std::exception("Error at socket(): %ld\n", WSAGetLastError());
	}

	iResult = bind(_srvSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(_srvSocket);
		WSACleanup();
		throw std::exception("bind failed with error: %ld\n", WSAGetLastError());
	}
	start_UDP_server(port + 1);

	_networkThread = std::thread(&NatusDataServer::tcpWorker, this);
	_udpThread = std::thread(&NatusDataServer::udpWorker, this);
	//::SetThreadPriority(_networkThread.native_handle(), THREAD_PRIORITY_HIGHEST);



}

void NatusDataServer::CheckForConnection()
{

	connection = INVALID_SOCKET;
	//printMessage ("Checking for incoming connection");
	if (listen(_srvSocket, 1) == SOCKET_ERROR)
	{
		std::string errstring = "Listen failed with error: " + std::to_string(WSAGetLastError());
		closesocket(_srvSocket);
		WSACleanup();
		throw std::exception(errstring.c_str());
	}

	timeval t;
	t.tv_sec = 0;
	t.tv_usec = 1;
	fd_set test_sockets;
	FD_ZERO(&test_sockets);
	FD_CLR(0, &test_sockets);
	FD_SET(_srvSocket, &test_sockets);

	if (select(0, &test_sockets, NULL, NULL, &t) == 0) //no pending connection
		return;

	//	if (pollr == INVALID_SOCKET)
	//	{
	//		std::string errstring = "Failed to poll server socket! " + std::to_string (WSAGetLastError ());
	//		closesocket (_srvSocket);
	//		WSACleanup ();
	//		throw std::exception (errstring.c_str());
	//	}

		// Accept a client socket

	connection = accept(_srvSocket, NULL, NULL);
	if (connection == INVALID_SOCKET)
	{
		closesocket(connection);
		WSACleanup();
		throw std::exception("accept failed: %d\n", WSAGetLastError());
	}



	_udpReceiver.sin_family = AF_INET;
	socklen_t len = sizeof(sockaddr_in);
	getpeername(connection, (sockaddr *)&_udpReceiver, &len);
	_udpReceiver.sin_port = htons(_udpport);

	char *s = (char*)std::malloc(INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &(_udpReceiver.sin_addr), s, INET_ADDRSTRLEN);
	printMessage("Connected to new client from: " + std::string(s));
	free(s);


}



void NatusDataServer::data_received(CmdStruct* payload)
{
	bool isErrorResponse = false;
	if (payload->is_error)
	{
		printMessage("Received Payload is error message");
		_errorCallback(_errorCallbackParent, payload->cmd_code, payload->payload, payload->payload_len);
	}
	else
	{
		switch (payload->cmd_code)
		{
		case CMD_Ping:
			printMessage("Answering ping command");
			send_data(CMD_Ping, payload->payload, payload->payload_len, false);
			break;
		case CMD_GetInformation:
		{
			printMessage("Answering GetInformation command");
			NatusDeviceInformation info;
			if (_infoCallback != NULL)
			{

				try { info = _infoCallback(_infoCallbackParent); }
				catch (std::exception) { isErrorResponse = true; }
			}
			else
			{
				isErrorResponse = true;

			}

			stream_all_channels();
			//uint8_t natusinfo[2 * sizeof (uint16_t) + sizeof (char) * 40];
			//memcpy (natusinfo, &info.SamplingRate, sizeof (uint16_t));
			//memcpy (natusinfo+ sizeof (uint16_t), &info.NumberOfChannels, sizeof (uint16_t));
			//memcpy (natusinfo + 2*sizeof (uint16_t), &info.Identifier, sizeof (char)*40);
			send_data(CMD_GetInformation, reinterpret_cast<uint8_t*>(&info), sizeof(NatusDeviceInformation), isErrorResponse);
			break;
		}
		case CMD_GetChannelNames:
		{
			printMessage("Answering GetChannelNames command");
			NatusChannelInformation channelinfo;
			if (_channelInfoCallback != NULL)
			{
				try
				{
					channelinfo = _channelInfoCallback(_channelInfoCallbackParent);
					//if (channelinfo.ChannelNames == NULL) //channel names not set
						//isErrorResponse = true;
				}
				catch (std::exception) { isErrorResponse = true; }
			}
			else
			{
				isErrorResponse = true;
			}
			//Convert data from struct to uint8 to send.
			int chNamesLength = 0;
			int streamNamesLength = 0;
			uint32_t numChannels = channelinfo.ChannelNames.size();
			uint32_t numStreams = channelinfo.StreamNames.size();
			for (int i = 0; i < channelinfo.ChannelNames.size(); i++)
				chNamesLength += channelinfo.ChannelNames[i].length() + 1; // each string will be sent null terminated

			for (int i = 0; i < channelinfo.StreamNames.size(); i++)
				streamNamesLength += channelinfo.StreamNames[i].length() + 1; // each string will be sent null terminated

			int totalLength = sizeof(uint32_t) * 2 + chNamesLength + streamNamesLength;
			uint8_t *data = new uint8_t[totalLength];
			memcpy(data, &numChannels, sizeof(uint32_t));
			memcpy(data + sizeof(uint32_t), &numStreams, sizeof(uint32_t));
			uint8_t* curr_d = data + sizeof(uint32_t) * 2;
			for (int i = 0; i < channelinfo.ChannelNames.size(); i++)
			{
				memcpy(curr_d, channelinfo.ChannelNames[i].c_str(), channelinfo.ChannelNames[i].length() + 1);
				curr_d += channelinfo.ChannelNames[i].length() + 1;
			}
			for (int i = 0; i < channelinfo.StreamNames.size(); i++)
			{
				memcpy(curr_d, channelinfo.StreamNames[i].c_str(), channelinfo.StreamNames[i].length() + 1);
				curr_d += channelinfo.StreamNames[i].length() + 1;
			}
			//Send Response
			send_data(CMD_GetChannelNames, data, totalLength, isErrorResponse);

			break;
		}
		case CMD_StartStream:
			printMessage("Answering StartStream command");


			_streamData = true;

			send_data(CMD_StartStream, NULL, 0, false);
			printMessage("Starting to stream data with " + std::to_string(_streamChannels.size()) + " Channels");
			break;
		case CMD_StopStream:
			printMessage("Answering StopStream command");
			_streamData = false;
			emptySendQueue();
			send_data(CMD_StopStream, NULL, 0, false);
			break;

		case CMD_SetBlockSize:
		{
			uint32_t blocksize;
			if (payload->payload_len != sizeof(uint32_t))
				return send_data(CMD_SetBlockSize, NULL, 0, true);

			printMessage("Received SetBlockSize command");
			memcpy(&blocksize, payload->payload, payload->payload_len);
			_blockSize = blocksize;
			printMessage("Blocksize changed to " + std::to_string(blocksize));
			send_data(CMD_SetBlockSize, NULL, 0, false);
			break;
		}
		case CMD_SetDecimationFactor:
		{
			if (payload->payload_len != sizeof(uint32_t))
				return send_data(CMD_SetDecimationFactor, NULL, 0, true);

			uint32_t decimationFactor;
			memcpy(&decimationFactor, payload->payload, sizeof(uint32_t));
			_decimationFactor = decimationFactor;

			printMessage("Decimation Factor changed to " + std::to_string(decimationFactor));
			send_data(CMD_SetDecimationFactor, NULL, 0, false);
			break;
		}
		case CMD_SelectChannels:
		{
			printMessage("Answering Select Channels command");
			char *channels = new char[payload->payload_len + 1];
			memcpy(channels, payload->payload, payload->payload_len);
			channels[payload->payload_len] = '\0';
			int i = select_channels_to_stream(channels, payload->payload_len + 1);
			if (i == 1)
				send_data(CMD_SelectChannels, NULL, 0, false);
			else
				send_data(CMD_SelectChannels, NULL, 0, true);

			printMessage("Number of Channels to stream: " + std::to_string(_streamChannels.size()));
			break;
		}
		case CMD_GetDecimationFactor:
		{
			printMessage("Answering Get Decimation Factor command");
			uint32_t decimationFactor = _decimationFactor;
			send_data(CMD_GetDecimationFactor, reinterpret_cast<uint8_t*>(&decimationFactor), sizeof(_decimationFactor), false);
			break;
		}
		default:
			send_data(payload->cmd_code, NULL, 0, true);

		}
	}
}

void NatusDataServer::start_UDP_server(std::uint16_t port)
{
	printMessage("Starting UDP server on port " + std::to_string(port));

	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (udpSocket == INVALID_SOCKET)
	{
		WSACleanup();
		throw std::exception("Error at socket(): %ld\n", WSAGetLastError());
	}
	_udpport = port;

}

void NatusDataServer::udpSendDataPackage()
{
	std::vector<DataToSend*> _dataToSendVec;
	uint32_t send_size = 0;
	uint32_t single_block_size = 0;
	int num_ch = -1;
	uint8_t cmd = CMD_StreamPackage;
	while (_dataQueue.unsafe_size() > (_decimationFactor * _blockSize))
	{
		for (int i = 0; i < (_decimationFactor * _blockSize); ++i)
		{
			DataToSend* d = NULL;
			if (_dataQueue.try_pop(d))
			{
				num_ch = (num_ch < 0) ? d->numCh : num_ch;
				if ((d->packageNumber % _decimationFactor) == 0) //decimate
				{
					if (d->numCh != num_ch)
					{
						printMessage("WARNING: not all samples have the same number of channels!");
					}
					_dataToSendVec.push_back(d);
				}
				else
				{
					DeleteDataToSend(d);
				}

			}
			else
				--i;
		}


		single_block_size = num_ch * sizeof(float) + sizeof(uint32_t) + sizeof(uint32_t);

		if (_blockSize == 1)
		{
			updateBufferSize(single_block_size);
			cmd = CMD_StreamPackage;
			send_size = single_block_size;
			memcpy(_sendBuffer, &_currPackage, sizeof(uint32_t));
			memcpy(_sendBuffer + sizeof(uint32_t), &num_ch, sizeof(uint32_t));
			memcpy(_sendBuffer + (2 * sizeof(uint32_t)), _dataToSendVec[0]->data, sizeof(float) * num_ch);
			++_currPackage;

		}
		else
		{

			cmd = CMD_VariableSizeStreamPackage;
			send_size = sizeof(uint32_t) + single_block_size * _blockSize;
			updateBufferSize(send_size);
			memcpy(_sendBuffer, &single_block_size, sizeof(uint32_t));
			for (int i = 0; i < _dataToSendVec.size(); ++i)
			{
				memcpy(_sendBuffer + sizeof(uint32_t) + (i * single_block_size), &_currPackage, sizeof(uint32_t));
				memcpy(_sendBuffer + (i * single_block_size) + (2 * sizeof(uint32_t)), &num_ch, sizeof(uint32_t));
				memcpy(_sendBuffer + (3 * sizeof(uint32_t)) + (i * single_block_size), _dataToSendVec[i]->data, sizeof(float) * num_ch);
				++_currPackage;
			}

		}

		send_data(cmd, _sendBuffer, send_size, false, PackageType::Stream);

		num_ch = -1;
		for (int i = 0; i < _dataToSendVec.size(); ++i)
			DeleteDataToSend(_dataToSendVec[i]);

		_dataToSendVec.clear();
	}

}


void NatusDataServer::udpWorker()
{
	while (_runServer)
	{
		DWORD res = WaitForSingleObject(m_sync_lock, 1000);
		if (res != WAIT_OBJECT_0)
			continue;
		if (_runServer)
			udpSendDataPackage();
		ResetEvent(m_sync_lock);
	}
}
void NatusDataServer::tcpWorker()
{

	while (_runServer)
	{
		Sleep(100);
		if (HasClient())
		{
			try
			{

				Poll();

			}
			catch (std::exception e)
			{
				printMessage(e.what());
			}
		}
		else
		{
			CheckForConnection();
		}
	}
}

void NatusDataServer::resetConnection()
{
	_streamData = false;
	NatusCommunication::resetConnection();

}

void NatusDataServer::DeleteDataToSend(NatusDataServer::DataToSend *o)
{
	delete[] o->data;
	delete o;
}

NatusDataServer::DataToSend* NatusDataServer::NewDataToSend(uint32_t numCh)
{
	DataToSend* o = new DataToSend();
	o->numCh = numCh;
	o->data = new float[numCh];
	return o;
}

void NatusDataServer::emptySendQueue()
{

	while (!_dataQueue.empty())
	{
		DataToSend* d;
		if(_dataQueue.try_pop(d))
			DeleteDataToSend(d);
		
	}

}



void NatusDataServer::RegisterInformationCallback(NatusInformationRequest cb, void* parent)
{
	printMessage("NatusInformationRequest callback set");
	_infoCallback = cb;
	_infoCallbackParent = parent;
}

void NatusDataServer::RegisterErrorCallback(NatusResponse cb, void* parent)
{
	printMessage("ErrorCallback callback set");
	_errorCallback = cb;
	_errorCallbackParent = parent;
}

void NatusDataServer::RegisterChannelInformationCallback(NatusChannelInfoRequest cb, void* parent)
{
	printMessage("NatusChannelInformationRequest callback set");
	_channelInfoCallback = cb;
	_channelInfoCallbackParent = parent;
}

void NatusDataServer::UnregisterCallbacks()
{
	printMessage("UnregisterCallbacks");
	_infoCallback = NULL;
	_infoCallbackParent = NULL;
	_errorCallback = NULL;
	_errorCallbackParent = NULL;
	_channelInfoCallback = NULL;
	_channelInfoCallbackParent = NULL;
}

void NatusDataServer::SendStreamData(float * data, uint32_t numdata)
{
	//printMessage ("Sending stream data");
	/*int i;
	int ilen = sizeof(int);
	int iResult = getsockopt(udpSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&i, &ilen);
	printMessage("Max Message size " + std::to_string(i));
	*/
	if (!_streamData || (_streamChannels.size() == 0))
		return;


	DataToSend* buff = NewDataToSend(_streamChannels.size());

	for (int i = 0; i < _streamChannels.size(); i++)
		buff->data[i] = data[_streamChannels[i]];

	buff->packageNumber = _currSourcePackage;
	++_currSourcePackage;
	_dataQueue.push(buff);
	if (_dataQueue.unsafe_size() > (_decimationFactor * _blockSize))
		SetEvent(m_sync_lock);
	//{
	//	
		
//	}


}

void NatusDataServer::updateBufferSize(const uint32_t &send_space)
{
	if (send_space > _sendBufferSize) //resize buffer
	{
		uint8_t* old_buffer = _sendBuffer;
		_sendBufferSize = send_space;
		_sendBuffer = new uint8_t[_sendBufferSize];
		delete[] old_buffer;

	}
}

void NatusDataServer::Disconnect()
{
	printMessage("Disconnecting");
	_runServer = false;
	_networkThread.join();

	// shutdown the send half of the connection since no more data will be sent
	int iResult = shutdown(udpSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{

		closesocket(_srvSocket);
		WSACleanup();
		throw std::exception("shutdown of UDP failed: %d\n", WSAGetLastError());
	}

	closesocket(_srvSocket);
	if (connection != INVALID_SOCKET)
	{
		iResult = shutdown(connection, SD_BOTH);
		if (iResult == SOCKET_ERROR)
		{
			std::string err = "shutdown of client connection failed failed: " + std::to_string(WSAGetLastError());
			closesocket(connection);
			WSACleanup();
			throw std::exception(err.c_str());
		}
	}
}

bool NatusDataServer::HasClient()
{
	return connection != INVALID_SOCKET;
}

int NatusDataServer::getSize(uint8_t *c)
{
	uint8_t *t = c;
	int length = 0;
	if (t == NULL)
		return 0;
	while (*t != '\0')
	{
		length++;
		t++;
	}
	return length;	//Includes length with null character
}

int NatusDataServer::select_channels_to_stream(char *channels, uint32_t len)
{
	if (channels == NULL)
		return -1;	//error

	_streamChannels.clear();
	if (strcmp(channels, "auto") == 0)
	{
		stream_all_channels();
		return 1;
	}
	else
	{
		auto info = _infoCallback(_infoCallbackParent);

		char *next_token = NULL;
		char *channel = strtok_s(channels, " ", &next_token);

		while (channel != NULL)
		{

			int temp = std::atoi(channel);
			if (temp == 0 && channel[0] != '0') // Checking if it is not a number
			{
				stream_all_channels();
				return -1;
			}
			else
			{
				if (temp >= (info.NumberOfChannels) || temp < 0) {	// checking if channel number is within the range
					stream_all_channels();
					return -1;
				}
				//check for duplicates
				if (std::find(_streamChannels.begin(), _streamChannels.end(), temp) != _streamChannels.end())
				{
					stream_all_channels();
					return -1;
				}
				_streamChannels.push_back(temp);
				channel = strtok_s(NULL, " ", &next_token);
			}
		}
		return 1;
	}
}

void NatusDataServer::stream_all_channels()
{
	_streamChannels.clear();
	auto info = _infoCallback(_infoCallbackParent);
	for (int i = 0; i < info.NumberOfChannels; i++)
		_streamChannels.push_back(i);
}
