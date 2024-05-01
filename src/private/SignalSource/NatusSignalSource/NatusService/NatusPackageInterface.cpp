#if _WIN32
#include "stdafx.h"
#endif
#include "NatusPackageInterface.h"
#include <stdexcept>
#include <string>
#include <cstring>

/**************************************************
*
*	Baseclass IMPLEMENTATION
*****************************************************/

NatusCommunication::NatusCommunication() : connection(INVALID_SOCKET), _vstream(&std::cout), udpSocket(INVALID_SOCKET)
{
	currBufferLen = DEFAULT_BUFFER_LEN;
	curr_cmd = new uint8_t[currBufferLen];
}

NatusCommunication::~NatusCommunication()
{
	delete curr_cmd;
}

void NatusCommunication::Verbose(bool v)
{
	_verbose = v;
	_vstream = &std::cout;
}

void NatusCommunication::Verbose(bool v, std::ostream& s)
{
	_verbose = v;
	_vstream = &s;
}

void NatusCommunication::Verbose(bool v, std::ostream *s)
{
	_verbose = v;
	_vstream = s;
}




void NatusCommunication::send_data(uint8_t cmd, const uint8_t* payload, uint32_t len_payload, bool isError, PackageType t)
{

	//auto start = std::chrono::high_resolution_clock::now();

	uint32_t datalen = 0;
	if (isError)
	{
		datalen = 2 * sizeof(uint8_t) + len_payload + sizeof(uint8_t); //possible overflow!
		checkAndResizeBuffer(datalen + sizeof(uint32_t));

		reinterpret_cast<uint32_t*>(curr_cmd)[0] = datalen;
		curr_cmd[sizeof(uint32_t)] = (uint8_t)0xFF;
		curr_cmd[sizeof(uint32_t) + sizeof(uint8_t)] = cmd;
		memcpy(curr_cmd + sizeof(uint32_t) + 2 * sizeof(uint8_t), payload, len_payload);
		curr_cmd[sizeof(uint32_t) + 2 * sizeof(uint8_t) + len_payload] = 0;// _crc8(reinterpret_cast<uint8_t*>(curr_cmd), len_payload + sizeof(uint32_t) + 2 * sizeof(uint8_t));
	}
	else
	{
		datalen = sizeof(uint8_t) + len_payload + sizeof(uint8_t);
		checkAndResizeBuffer(datalen + sizeof(uint32_t));
		reinterpret_cast<uint32_t*>(curr_cmd)[0] = datalen;
		curr_cmd[sizeof(uint32_t)] = cmd;
		memcpy(curr_cmd + sizeof(uint32_t) + sizeof(uint8_t), payload, len_payload);
		curr_cmd[sizeof(uint32_t) + sizeof(uint8_t) + len_payload] = 0; //_crc8(reinterpret_cast<uint8_t*>(curr_cmd), sizeof(uint32_t) + sizeof(uint8_t) + len_payload);
	}
	int transmitError = NO_ERROR;
	switch (t)
	{
	case Control:
		transmitError = send(connection, reinterpret_cast<char*>(curr_cmd), datalen + sizeof(uint32_t), 0);
		break;
	case Stream:
		transmitError = sendto(udpSocket, reinterpret_cast<char*>(curr_cmd), datalen + sizeof(uint32_t), 0, (SOCKADDR *)&_udpReceiver, sizeof(_udpReceiver));
		break;
	default:
		throw std::runtime_error("Package type identifier wrong");
	}

	if (transmitError == SOCKET_ERROR)
		throw std::runtime_error("Problem during transmission of data");

	//auto finish = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> elapsed = finish - start;
	//std::cout << "Transmitting data took " << std::to_string(elapsed.count()) << std::endl;

}



void NatusCommunication::Poll()
{
	CmdStruct* newcmd = NULL;
	try
	{
		newcmd = getNextPackage();
		//printMessage(newcmd);
	}
	catch (CmdException e)
	{
		printMessage("Sending Error message,  CmdException on poll received");
		send_data(CMD_ERR, NULL, 0, true);
	}

	if (newcmd != NULL)
	{

		data_received(newcmd);


		delete_cmdstruct(newcmd);
	}
}


void NatusCommunication::printMessage(std::string s)
{
	std::lock_guard<std::mutex> guard(_printGuard);
	if (_verbose) (*_vstream) << s << std::endl;
}



CmdStruct * NatusCommunication::getNextPackage()
{
	if (connection == INVALID_SOCKET)
	{
		throw std::runtime_error("Tried to receive a package while no active connection is available!");
	}
	CmdStruct* newcmd = NULL;
	int iResult = 0;
	uint32_t getLen = 0;

	timeval t;
	t.tv_sec = 0;
	t.tv_usec = 1000;
	fd_set test_sockets;
	FD_ZERO(&test_sockets);
	FD_SET(connection, &test_sockets);

	if (select(connection + 1, &test_sockets, NULL, NULL, &t) == 0) //no pending data
		return NULL;

	iResult = recv(connection, reinterpret_cast<char*>(&getLen), sizeof(uint32_t), MSG_PEEK); //check if there is a msg in the buffer
	if (iResult == 0) //result 0 means connection has been closed
	{
		resetConnection();
		return NULL;
	}

	if (iResult == SOCKET_ERROR)
	{
		
		std::string errstring = "Error while trying to receive data from socket! Error " + std::to_string(errno);
		resetConnection();
		throw std::runtime_error(errstring.c_str());
	}
	checkAndResizeBuffer(getLen + sizeof(uint32_t));


	iResult = recv(connection, reinterpret_cast<char*>(curr_cmd), getLen + sizeof(uint32_t), MSG_WAITALL); //blocking call that waits until the whole msg is received



	if (iResult == SOCKET_ERROR)
	{
		resetConnection();
		std::string errstring = "Error while trying to receive data from socket! Error " + std::to_string(errno);
		throw std::runtime_error(errstring.c_str());
	}



	newcmd = new CmdStruct();
	if (curr_cmd[sizeof(uint32_t)] == CMD_ERR) //returned error
	{
		newcmd->is_error = true;
		newcmd->cmd_code = curr_cmd[sizeof(uint32_t) + 2 * sizeof(uint8_t)];
		newcmd->crc = curr_cmd[getLen + sizeof(uint32_t) - sizeof(uint8_t)];
		newcmd->payload_len = getLen - 3 * sizeof(uint8_t);
		newcmd->payload = new uint8_t[newcmd->payload_len];
		memcpy(newcmd->payload, curr_cmd + 3 * sizeof(uint8_t), newcmd->payload_len);
	}
	else
	{
		newcmd->is_error = false;
		newcmd->cmd_code = curr_cmd[sizeof(uint32_t)];
		newcmd->crc = curr_cmd[getLen + sizeof(uint32_t) - sizeof(uint8_t)];
		newcmd->payload_len = getLen - 2 * sizeof(uint8_t);
		newcmd->payload = new uint8_t[newcmd->payload_len];
		memcpy(newcmd->payload, curr_cmd + sizeof(uint8_t) + sizeof(uint32_t), newcmd->payload_len);
	}
	//if (_crc8(reinterpret_cast<uint8_t*>(curr_cmd), sizeof(uint32_t) + getLen - sizeof(uint8_t)) != newcmd->crc)//crc is calculated from everything including len, but ofc without the crc itself
	//{
	//	throw CmdException("Error while receiving command: CRC does not match"); //if this happens we should probably discard a byte and try again to check if maybe just a single byte is misaligned
	//}


	return newcmd;
}

void NatusCommunication::resetConnection()
{
	closesocket(connection);

	connection = INVALID_SOCKET;
}

CmdStruct * NatusCommunication::getNextUDPPackage()
{
	if (udpSocket == INVALID_SOCKET)
	{
		throw std::runtime_error("Tried to receive a UDP package while no active connection is available!");
	}

	CmdStruct* newcmd = NULL;
	int iResult = 0;
	uint32_t getLen = 0;

	timeval t;
	t.tv_sec = 0;
	t.tv_usec = 1000;
	fd_set test_sockets;
	FD_ZERO(&test_sockets);
	FD_SET(udpSocket, &test_sockets);

	if (select(udpSocket + 1, &test_sockets, NULL, NULL, &t) == 0) //no pending data
		return NULL;
	socklen_t SenderAddrSize = sizeof(_udpReceiver);
	iResult = recvfrom(udpSocket, reinterpret_cast<char*>(&getLen), sizeof(uint32_t), MSG_PEEK, (SOCKADDR *)&_udpReceiver, &SenderAddrSize); //check if there is a msg in the buffer
	if (iResult == 0)
		return NULL;
	//	if (iResult == SOCKET_ERROR)// DO nothing here, because it will recvfrom will return -1 with a LastError WSAEMSGSIZE because we only read only LEN from the buffer -> only sizeof(uint_32t) bufferlength is used
	//	{							// This is not a problem because we MSG_PEEK, therefore nothing gets lost if we do not read the whole message; we will check for error when we read out the whole msg

			//
	//	}
	checkAndResizeBuffer(getLen + sizeof(uint32_t));


	iResult = recvfrom(udpSocket, reinterpret_cast<char*>(curr_cmd), getLen + sizeof(uint32_t), 0, (SOCKADDR *)&_udpReceiver, &SenderAddrSize); //blocking call that waits until the whole msg is received



	if (iResult == SOCKET_ERROR)
	{
		//resetConnection();
		std::string errstring = "Error while trying to receive data from socket! Error " + std::to_string(errno);
		throw std::runtime_error(errstring.c_str());
	}



	newcmd = new CmdStruct();
	if (curr_cmd[sizeof(uint32_t)] == CMD_ERR) //returned error
	{
		newcmd->is_error = true;
		newcmd->cmd_code = curr_cmd[sizeof(uint32_t) + 2 * sizeof(uint8_t)];
		newcmd->crc = curr_cmd[getLen + sizeof(uint32_t) - sizeof(uint8_t)];
		newcmd->payload_len = getLen - 3 * sizeof(uint8_t);
		newcmd->payload = new uint8_t[newcmd->payload_len];
		memcpy(newcmd->payload, curr_cmd + 3 * sizeof(uint8_t), newcmd->payload_len);
	}
	else
	{
		newcmd->is_error = false;
		newcmd->cmd_code = curr_cmd[sizeof(uint32_t)];
		newcmd->crc = curr_cmd[getLen + sizeof(uint32_t) - sizeof(uint8_t)];
		newcmd->payload_len = getLen - 2 * sizeof(uint8_t);
		newcmd->payload = new uint8_t[newcmd->payload_len];
		memcpy(newcmd->payload, curr_cmd + sizeof(uint8_t) + sizeof(uint32_t), newcmd->payload_len);
	}
	//if (_crc8(reinterpret_cast<uint8_t*>(curr_cmd), sizeof(uint32_t) + getLen - sizeof(uint8_t)) != newcmd->crc)//crc is calculated from everything including len, but ofc without the crc itself
	//{
	//	throw CmdException("Error while receiving command: CRC does not match"); //if this happens we should probably discard a byte and try again to check if maybe just a single byte is misaligned
	//}

	return newcmd;
}

void NatusCommunication::delete_cmdstruct(CmdStruct * strct)
{
	if (strct != NULL)
		delete strct->payload;
	delete strct;
}


void NatusCommunication::initWSA()
{
#if _WIN32
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		throw std::runtime_error("WSA Startup failed: " + std::to_string(iResult));
	}
#endif
}

void NatusCommunication::deinitWSA()
{
#if _WIN32
    WSACleanup();
#endif
}

uint8_t NatusCommunication::_crc8(const uint8_t *data, int length)
{
	uint8_t crc = 0x00;
	uint8_t extract;
	uint8_t sum;
	for (int i = 0; i < length; i++)
	{
		extract = *data;
		for (char tempI = 8; tempI; tempI--)
		{
			sum = (crc ^ extract) & 0x01;
			crc >>= 1;
			if (sum)
				crc ^= 0x8C;
			extract >>= 1;
		}
		data++;
	}
	return crc;
}

void NatusCommunication::checkAndResizeBuffer(uint32_t reqLen)
{

	if (currBufferLen < reqLen) //if the current buffer is too small we need to make it bigger
	{
		currBufferLen = reqLen;
		delete  curr_cmd;
		curr_cmd = new uint8_t[currBufferLen];
	}
}
