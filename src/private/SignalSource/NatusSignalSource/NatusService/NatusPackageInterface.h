#ifndef __NatusPackageInterface_H_
#define __NatusPackageInterface_H_

#if _WIN32
#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>//needs to be above windows.h to avoid redefinition from inside windows.h ... (https://stackoverflow.com/questions/22517036/socket-errors-cant-get-functions-in-winsock2-h)
#include <ws2tcpip.h> 
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#define errno (WSAGetLastError())
#define socklen_t int
#else // POSIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET (-1)
#define NO_ERROR (0)
#define SOCKET_ERROR (-1)
#define SOCKADDR struct sockaddr
#define SD_SEND (SHUT_WR)
#define closesocket(x) close(x)
#endif
#include <cstdlib>
#include <thread> 
#include <fstream>
#include <vector>

#include <cstdint>
#include <iostream>
#include <mutex>
#include <stdexcept>

#define CMD_Ping							0x01
#define CMD_GetInformation		0x02
#define CMD_GetChannelNames 0x03
#define CMD_StartStream				0x10
#define CMD_StopStream				0x1F
#define CMD_StreamPackage			0x11
#define CMD_VariableSizeStreamPackage 0x12

#define CMD_SelectChannels		0x20
#define CMD_SetBlockSize		0x21
#define CMD_SetDecimationFactor		0x22
#define CMD_GetDecimationFactor 0x23
#define CMD_ERR								0xFF
#define CMD_CloseConnection 0x09;
#define DEFAULT_BUFFER_LEN 512
#define IDENTIFIER_LEN 40


enum PackageType { Control, Stream };


#pragma pack (push, 1) //pragma is supported by gcc
typedef struct natus_inf_t
{
	std::uint16_t NumberOfChannels;
	std::uint16_t SamplingRate;
	char Identifier[IDENTIFIER_LEN];

} NatusDeviceInformation;

typedef struct natus_channel_t
{
	std::vector<std::string> ChannelNames; //stores channel names 
	std::vector<std::string> StreamNames; //stores stream names 
} NatusChannelInformation;
#pragma pack (pop)

typedef struct natus_sample_block_t
{
	std::uint32_t BlockNumber;
	std::uint32_t NumChannels;
	float* Samples;
	bool Interpolated;
} NatusSampleBlock;

typedef struct cmd_struct_t
{
	std::uint8_t cmd_code;
	std::uint32_t payload_len;
	std::uint8_t* payload;
	std::uint8_t crc;
	bool is_error;


} CmdStruct;



class CmdException : public std::runtime_error
{
public:
	CmdException() : std::runtime_error("") {};
	CmdException(const char* msg) : std::runtime_error(msg) {};
};

class NatusCommunication
{
public:
	NatusCommunication();
	~NatusCommunication();
	void Verbose(bool);
	void Verbose(bool, std::ostream&);
	void Verbose(bool, std::ostream*);
	void Verbose(bool, std::string);
	void Poll();
protected:
	void send_data(uint8_t cmd, const uint8_t* payload, uint32_t len_payload, bool isError = false, PackageType t = Control);
	virtual void data_received(CmdStruct* payload) = 0;
	void printMessage(std::string);
	CmdStruct* getNextPackage();
	virtual void resetConnection();
	CmdStruct* getNextUDPPackage();
	void delete_cmdstruct(CmdStruct*);
	void initWSA();
	void deinitWSA();
	SOCKET connection;
	SOCKET udpSocket;
	sockaddr_in _udpReceiver;

	std::mutex _printGuard;

private:
	static uint8_t _crc8(const uint8_t *data, int length);
	uint8_t* curr_cmd;
	uint32_t currBufferLen;
	void checkAndResizeBuffer(uint32_t reqLen);
	bool _verbose;
	std::ostream *_vstream;
};
#endif
