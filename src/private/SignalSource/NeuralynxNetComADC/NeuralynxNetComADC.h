////////////////////////////////////////////////////////////////////////////////
// Authors: 
// Description: NeuralynxNetComADC header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_NEURALYNXNETCOMADC_H  // makes sure this header is not included more than once
#define INCLUDED_NEURALYNXNETCOMADC_H

#include "NeuralynxUDPConverter.h"
#include "GenericADC.h"
#include "lib/include/NetComClient.h"
#include "lib/include/Nlx_DataTypes.h"
#include "NetComCommands.h"
#include "SynchronizedQueue.h"
#include <unordered_map>
#include <unordered_set>
#include "Expression.h"
#include "BCIStream.h"
#include "BCIEvent.h"

#define NUM_CH_PER_BOARD 32

class NeuralynxNetComADC : public GenericADC
{
public:
	NeuralynxNetComADC();
	~NeuralynxNetComADC();
	virtual void Publish() override;
	virtual void AutoConfig(const SignalProperties&) override;
	void ConfigureUDPStream(int &numchannels);
	void InitializeUDPSettingsFromNetCom(int & numchannels);
	void ConfigureNetComStream(int &numchannels);
	std::wstring GetAcquistionSystem();
	void QueryNetComDAS();
	virtual void Preflight(const SignalProperties&, SignalProperties&) const override;

	virtual void Initialize(const SignalProperties&,const SignalProperties&) override;
	void StartAcquisition();
	virtual void StopRun() override;
	virtual void Halt() override;
	virtual void Process(const GenericSignal&,GenericSignal&) override;
	void AcquireFromNetComStream(GenericSignal & Output);

private:
	void ConnectToDevice();

	void clearQueues();
	void flushQueues();
	void AcquireFromDirectUDPStream(GenericSignal& Output);

	void GetSamplingRateFromNetCom(int &sampling_rate, std::wstring acqSystem);
	void GetSamplingInformationFromFile(std::string, int &sampling_rate, float &adc_conversion_factor, int& number_of_channels);



//--------------------------------------------
//UDP STREAM VARIABLES

	bool _useDirectUDPStream = false;
	bool _streamFile = false;
	float _udp_conv_factor = 1.0;
	std::vector<uint32_t> _chList;
//-----------------------------------------------


	NlxNetCom::NetComClient* mDeviceHandle = nullptr;
	int mNumberOfSignalChannels = 0;
	int _sampleCounter = 0;
	int _sampling_rate = 0;
	NeuralynxUDPConverter *_udpConverter = nullptr;

	std::vector<std::wstring> _DASObjects;
	std::vector<std::wstring> _DASTypes;

	std::unordered_set <std::wstring> _recordStreamObjects; //hashmap for O(1) searches
	std::vector <std::wstring> _recordStreamObjects_ordered;

	std::wstring mDigitalIODevice;
	int mDigitalIOPort = 0;
	Expression mDigitalIOExpression;

//-----------------------------------------


//NetComBuffer reads the blocks and breaks it down into single sample blocks for the BufferedADC
	class NetComBuffer
	{		
	public:
		NetComBuffer();
		void InitBuffer(NlxNetCom::NetComClient *client, const std::vector<std::wstring>& entities, double blockDuration);
		void SetRecordingEntities(const std::vector<std::wstring>& entities, double blockDuration);
		void GetData(GenericSignal& output, StateRef& timestamps);
		void FlushQueues();
		void ClearQueues();
		~NetComBuffer();

	private:
		static void csc_callback(void* myClassPtr, NlxDataTypes::CRRec *records, int numRecords, const wchar_t *const objectName);
		NlxNetCom::NetComClient *mDevice;
		std::unordered_map<std::wstring, size_t> mEntityToChannel;

		typedef SynchronizedQueue<NlxDataTypes::CRRec*> PacketQueue;
		struct QueueState
		{
			PacketQueue* pQueue = nullptr;
			NlxDataTypes::CRRec* pCurrentPacket = nullptr;
			int currentSampleIndex = -1;
		};
		std::vector<QueueState> mQueueState;
		TimeUtils::TimeInterval mTimeout = 0;
	};

	NetComBuffer _netcom_buffer;
};

#endif // INCLUDED_NEURALYNXNETCOMADC_H


