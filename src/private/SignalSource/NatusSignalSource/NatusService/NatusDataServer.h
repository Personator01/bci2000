#ifndef __NatusServer_H_
#define __NatusServer_H_

#include "NatusPackageInterface.h"
#include <queue>
#include <mutex>
#include <atomic>
#include <concurrent_queue.h>

using namespace concurrency;

#define UNDEFINED_STREAMBLOCK_LEN -1

#define TIMING_RECONSTRUCTION 0

typedef NatusDeviceInformation (*NatusInformationRequest)(void*);
typedef void (*NatusResponse)(void*,uint8_t cmd, uint8_t* payload, uint32_t payload_size);
typedef NatusChannelInformation (*NatusChannelInfoRequest)(void*);

class NatusDataServer :public  NatusCommunication
{

	struct DataToSend
	{
		float * data;
		uint32_t numCh;
		uint32_t packageNumber;

	};

public:
	NatusDataServer ();
	~NatusDataServer ();
	void Startup (std::uint16_t port);
	void CheckForConnection ();
	void RegisterInformationCallback (NatusInformationRequest,void*);
	void RegisterErrorCallback (NatusResponse,void*);
	void UnregisterCallbacks ();
	void SendStreamData (float* data, uint32_t numdata);
	void updateBufferSize(const uint32_t &send_space);
	void Disconnect ();
	bool HasClient ();
	void RegisterChannelInformationCallback (NatusChannelInfoRequest, void*);

protected:
	void data_received (CmdStruct* payload) override;
	void start_UDP_server (std::uint16_t port);
	void tcpWorker();
	void udpWorker();
	void resetConnection() override;
	void udpSendDataPackage();
	

private:
	concurrent_queue<DataToSend*> _dataQueue;
	//std::mutex _dataMutex;
	std::thread _networkThread;
	std::thread _udpThread;
	std::atomic<bool> _runServer;
	HANDLE    m_sync_lock;
	uint32_t _requiredAvialableBlocks;

	void DeleteDataToSend(DataToSend*);
	DataToSend* NewDataToSend(uint32_t numCh);

	std::atomic<bool> _streamData;

	NatusInformationRequest _infoCallback;
	void* _infoCallbackParent;
	NatusResponse _errorCallback;
	void* _errorCallbackParent;
	uint32_t _currSourcePackage;
	uint32_t _currPackage;
	uint8_t* _sendBuffer;
	uint32_t _sendBufferSize;
	SOCKET _srvSocket;
	int _udpport;
	std::atomic<uint32_t> _blockSize;

#if TIMING_RECONSTRUCTION
	float _sendTimer;

#endif
	


	std::atomic<uint32_t> _decimationFactor;
	void emptySendQueue();
	NatusChannelInfoRequest _channelInfoCallback;
	void* _channelInfoCallbackParent;
	int getSize(uint8_t *);
	int select_channels_to_stream(char *,uint32_t);
	void stream_all_channels();
	std::vector<uint32_t> _streamChannels;

};


#endif
