#ifndef INCLUDED_NEURALYNUDPCONVC_H  // makes sure this header is not included more than once
#define INCLUDED_NEURALYNUDPCONVC_H

#include "lib/include/Nlx_DataTypes.h"
#include "lib/include/NetComClient.h"
#include <WinSock2.h>//needs to be above windows.h to avoid redefinition from inside windows.h ... (https://stackoverflow.com/questions/22517036/socket-errors-cant-get-functions-in-winsock2-h)
#include <ws2tcpip.h> 
#include <windows.h>
#include <stdlib.h>
#include <thread> 
#include <fstream>
#include <exception>
#include <queue>
#include <concurrent_queue.h>
#include "ThreadUtils.h"
#include "MeasurementUnits.h"
#include "GenericSignal.h"
#include "CircularBuffer.h"
#include "StateRef.h"

#pragma comment(lib, "Ws2_32.lib")

#include <Thread>

#define FILE_STREAM_BUFFER 256*1024
#define TIMEOUT_UDP 1 //wait maximum of 1 second for a UDP package
#define CIRCULAR_BUFFER_SIZE 1024
#define HARDWARE_SAMPLINGRATE  32000
using namespace concurrency;
class NeuralynxUDPConverter
{



public:
	NeuralynxUDPConverter(NlxNetCom::NetComClient* comClient, uint32_t port, uint32_t boards);
	NeuralynxUDPConverter(std::string, uint32_t n_boards, uint32_t);
	~NeuralynxUDPConverter();
	void StartListening();
	void StopListening();
	int GetNumberOfChannels();
	void SetDecimation(uint32_t dec);
	bool GetData(GenericSignal & Output, std::vector<uint32_t>, StateRef ts, StateRef lost, std::vector<StateRef>& ttl);

protected:
	bool configureUDP();
	void fetchFromUdp();

	private:
		HANDLE    m_sync_lock;
		std::atomic<int> _n_samples;
		NeuralynxUDPConverter();
		void InitializeBuffers();
		int receive(void* buffer, uint32_t receive_size);
		uint32_t _decimationFactor;
		int32_t* _valueBuffer;

		static const uint32_t HEADER_SIZE;
		static const uint32_t DATA_SIZE_PER_BOARD;
		static const uint32_t FOOTER_SIZE;
		static const uint32_t CHANNELS_PER_BOARD;


		std::ifstream* _stream;
		char _fileStreamBuffer[FILE_STREAM_BUFFER];

		//SynchronizedQueue<int32_t*> _receivedData;
//		CircularBuffer<char> _stxBuffer;
		NlxNetCom::NetComClient* _client;
		concurrent_queue<float*> _receiveDataQueue;
		uint32_t _port;
		uint32_t _n_channels;
		uint32_t _n_boards;
		uint32_t _package_size;
		uint32_t _data_size;
		uint32_t _filterIdx;
		SOCKET _udpSocket;
		SOCKADDR_IN _udpReceiver;
		std::thread _udpWorker;
		char* _inpBuffer;
		volatile bool _doFetch;
		volatile bool _runTask;
		uint32_t _expPackageSize, _currBlock;


		
		
		
};

#endif
