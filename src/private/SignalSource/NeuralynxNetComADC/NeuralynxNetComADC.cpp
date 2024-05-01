////////////////////////////////////////////////////////////////////////////////
// Authors:
// Description: NeuralynxNetComADC implementation
////////////////////////////////////////////////////////////////////////////////

#include "NeuralynxNetComADC.h"
#include "StringUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////

// Make the source filter known to the framework.
RegisterFilter(NeuralynxNetComADC, 1.A); // ADC filters must be registered at location "1" in order to work.
//RegisterFilter(IIRBandpass, 1.E); // ADC filters must be registered at location "1" in order to work.

static std::wstring c2ws(const char* cstr)
{
	return StringUtils::FromNarrow(cstr);
}

static std::string ws2s(const std::wstring& wstr)
{
	return StringUtils::FromWide(wstr);
}

static std::wstring s2ws(const std::string& str)
{
	return StringUtils::FromNarrow(str);
}

NeuralynxNetComADC::NeuralynxNetComADC()
	: mDeviceHandle(NlxNetCom::GetNewNetComClient()), _useDirectUDPStream(false), _udpConverter(NULL), _netcom_buffer() 
{
}

NeuralynxNetComADC::~NeuralynxNetComADC()
{
	// The destructor is where to make sure that all acquired resources are released.
	// Memory deallocation (calling delete[] on a NULL pointer is OK).
	Halt();
	clearQueues();
	// Closing connection to device.

	if (mDeviceHandle != NULL)
	{
		mDeviceHandle->DisconnectFromServer();
		NlxNetCom::DeleteNetComClient(mDeviceHandle);
	}
	delete _udpConverter;

}

void NeuralynxNetComADC::clearQueues()
{
	_netcom_buffer.ClearQueues();
}

void NeuralynxNetComADC::flushQueues()
{
	_netcom_buffer.FlushQueues();
}

void
NeuralynxNetComADC::Publish()
{
	// Declare any parameters that the filter needs....

	BEGIN_PARAMETER_DEFINITIONS

		"Source:Signal%20Properties int SourceCh= auto "
		"auto 1 % // number of digitized and stored channels",

		"Source:Signal%20Properties int SampleBlockSize= auto "
		"auto 1 % // number of samples transmitted at a time",

		"Source:Signal%20Properties float SamplingRate= auto "
		"auto 0.0 % // sample rate ",

		"Source:Signal%20Properties list SourceChGain= 1 auto "
		" // physical units per raw A/D unit (noedit)",

		"Source:Signal%20Properties list SourceChOffset= 1 auto "
		" // raw A/D offset to subtract (noedit)",

		"Source:Signal%20Properties list ChannelNames= 1 auto "
		" // names of CSC channels",


		"Source:NeuralynxNetCom string ServerName= 127.0.0.1 "
		"% // Connection string for Neuralynx server",

		"Source:NeuralynxNetCom string DigitalIODevice= % AcqSystem1_0 "
		"% % // Name of digital IO device for digital output",

		"Source:NeuralynxNetCom string DigitalIOPort= 0 0 "
		"% % // Digital IO port digital output",

		"Source:NeuralynxNetCom string DigitalIOExpression= % StimulusCode>0 "
		"% % // Expression that determines digital output",


		"Source:NeuralynxUDPStream int Boards= 16 16 1 16 "
		" // Use Direct Neuralynx UDP stream",

		"Source:NeuralynxUDPStream list SourceChList= 1 auto"
		" // List of channels transmitted ",

		"Source:NeuralynxUDPStream int UseDirectUDPStream= 0 auto "
		" // Use Direct Neuralynx UDP stream (boolean)",

		"Source:NeuralynxUDPStream int UDPStreamPort= auto "
		" // Port for low level stream",

		"Source:NeuralynxUDPStream int DecimationFactor= auto 1 1 %"
		" // Decimation factor for UDP Stream ",

		"Source:NeuralynxUDPStream int UDPSamplingRate= auto 1 1 %"
		" // Hardware Sampling rate for UDP Stream (noedit)",

		"Source:NeuralynxUDPStream string NRDStreamingFile= % "
		" // Use NRD File instead of UDP (inputfile)",

	END_PARAMETER_DEFINITIONS

	BEGIN_STREAM_DEFINITIONS

		"NeuralynxTimestamp 32 0", // This is a truncated version of a 64 bit HPC timestamp, shifted right by 32 bits.
								   // There is no more information available about that timestamp. The header file claims
								   // it's microseconds but that is wrong in case of the direct UDP connection.
								   // In case of the direct UDP connection, the timestamp is per sample.
		                           // Otherwise, it is per Neuralynx block.
		"NeuralynxLostSamples 16 0", // This is based on the difference between timestamps of subsequent native (non-decimated) samples.
								     // The number of lost samples is given in terms of native samples.
								     // This counter is only available with a direct connection.

	END_STREAM_DEFINITIONS

	for (int i = 0; i < 32; ++i)
	{
		std::ostringstream oss;
		oss << "NeuralynxDigitalInput" << i + 1 << " 1 0";
		BEGIN_STREAM_DEFINITIONS
			oss.str(),
		END_STREAM_DEFINITIONS
	}
}




void NeuralynxNetComADC::ConnectToDevice()
{
	if (mDeviceHandle == NULL)
	{
		mDeviceHandle = NlxNetCom::GetNewNetComClient();
	}
	if (!mDeviceHandle->AreWeConnected())
	{
		std::string srvrname = Parameter("ServerName");
		if (!mDeviceHandle->ConnectToServer(std::wstring(srvrname.begin(), srvrname.end()).c_str(), true)) // if connect fails, the ComClient is in an undefined state
		{
			NlxNetCom::DeleteNetComClient(mDeviceHandle);
			mDeviceHandle = NlxNetCom::GetNewNetComClient();//get a new client for the next attempt 
			bcierr << "Not able to connect to server: " << srvrname << std::endl;
		}
		else
			mDeviceHandle->SetApplicationName(L"BCI2000");
	}
}

void
NeuralynxNetComADC::AutoConfig(const SignalProperties&)
{

	Parameter("UDPStreamPort") = "26090";
	Parameter("UseDirectUDPStream") = false;
	Parameter("DecimationFactor") = 1;
	
	int numchannels = 0;
	delete _udpConverter;
	_udpConverter = NULL;
	_useDirectUDPStream = ActualParameter("UseDirectUDPStream");


	if (!_useDirectUDPStream)
		ConfigureNetComStream(numchannels);
	else
		ConfigureUDPStream(numchannels);


	bciout << "Incoming data is sampled at " << std::to_string(_sampling_rate) << "Hz " << std::endl;
	Parameter("SamplingRate") = _sampling_rate;
	if (_sampling_rate == 0)
	{
		Parameter("SampleBlockSize") = 1;
		return;
	}

	uint32_t desired_blck_size = std::ceil(ActualParameter("SamplingRate") * 30e-3); //default block size 30ms
	Parameter("SampleBlockSize") = desired_blck_size;

	mDigitalIODevice = s2ws(Parameter("DigitalIODevice"));
	if (!mDigitalIODevice.empty())
	{
		std::vector<std::wstring> devices;
		if (!NetComCommands::GetDigitalIOBoardList(mDeviceHandle, devices))
			throw bcierr << "Could not obtain list of digital IO devices";
		auto i = std::find(devices.begin(), devices.end(), mDigitalIODevice);
		if (i == devices.end()) {
			std::string deviceList;
			for (const auto& dev : devices)
				deviceList += '\n' + ws2s(dev);
			throw bcierr << "Unknown DigitalIODevice: " << ws2s(mDigitalIODevice)
						<< ", available devices are: " << deviceList;
		}
		int mDigitalIOPort = Parameter("DigitalIOPort");
		if (!NetComCommands::SetDigitalIOPortDirection(mDeviceHandle,
				mDigitalIODevice, mDigitalIOPort, NetComCommands::Output))
			throw bcierr << "Could not set IO port direction to \"Output\"";
	}
}



void NeuralynxNetComADC::ConfigureUDPStream(int &numchannels)
{
	Parameter("NRDStreamingFile") = "";
	std::string nrdfile = ActualParameter("NRDStreamingFile");
	
	int dec_factor = ActualParameter("DecimationFactor");
	_streamFile = false;
	numchannels = ActualParameter("Boards") * NUM_CH_PER_BOARD;
	Parameter("SourceCh") = numchannels;
	numchannels = ActualParameter("SourceCh");

	if (!nrdfile.empty())
	{
		bciwarn << "Data will be streamed from file: " << nrdfile << std::endl;
		_udp_conv_factor = 0.0;
		int fileChannels = 0;
		int sampling_rate;
		GetSamplingInformationFromFile(nrdfile, sampling_rate, _udp_conv_factor, fileChannels);

		std::string srvrname = Parameter("ServerName");
		mDeviceHandle->ConnectToServer(std::wstring(srvrname.begin(), srvrname.end()).c_str(), true);
		if (mDeviceHandle->AreWeConnected())
		{
			InitializeUDPSettingsFromNetCom(numchannels);
			if (fileChannels != numchannels)
				bcierr << "The streaming file and Neuralynx information diverge (different number of channels!)" << std::endl;
			if (sampling_rate != _sampling_rate)
				bcierr << "The streaming file and Neuralynx information diverge (different number of channels!)" << std::endl;
		}
		else
		{
			_sampling_rate = sampling_rate;
			numchannels = ActualParameter("Boards") * NUM_CH_PER_BOARD;
			Parameter("SourceCh") = numchannels;
			numchannels = ActualParameter("SourceCh");

			Parameter("SourceCh") = numchannels;
			numchannels = ActualParameter("SourceCh");

		}
		Parameter("UDPSamplingRate") = _sampling_rate;
		numchannels = ActualParameter("Boards") * NUM_CH_PER_BOARD;
		Parameter("SourceCh") = numchannels;
		numchannels = ActualParameter("SourceCh");

		_udpConverter = new  NeuralynxUDPConverter(nrdfile, ActualParameter("Boards"), _sampling_rate);
		_udpConverter->SetDecimation(dec_factor);
		Parameter("SamplingRate") = _sampling_rate/ dec_factor;
		_sampling_rate = ActualParameter("SamplingRate");

		Parameter("ChannelNames")->SetNumValues(numchannels);
		Parameter("SourceChGain")->SetNumValues(numchannels);
		Parameter("SourceChOffset")->SetNumValues(numchannels);
		Parameter("SourceChList")->SetNumValues(numchannels);

		for (int ch = 0; ch < numchannels; ch++)
		{
			Parameter("ChannelNames")(ch) = "Ch" + std::to_string(ch);
			Parameter("SourceChGain")(ch) << _udp_conv_factor << "V";
			Parameter("SourceChOffset")(ch) = 0;
			Parameter("SourceChList")(ch) = ch;
		}


		_streamFile = true;



	}
	else
	{
		ConnectToDevice();

		if (mDeviceHandle->AreWeConnected())
		{
			InitializeUDPSettingsFromNetCom(numchannels);
			dec_factor=ActualParameter("UDPSamplingRate") / ActualParameter("SamplingRate");
			Parameter("DecimationFactor") = dec_factor;
			dec_factor = ActualParameter("DecimationFactor");
			Parameter("Boards") = numchannels/ NUM_CH_PER_BOARD;
			numchannels = ActualParameter("Boards") * NUM_CH_PER_BOARD;
			Parameter("SourceCh") = numchannels;
			uint32_t port = ActualParameter("UDPStreamPort");
			_udpConverter = new  NeuralynxUDPConverter(mDeviceHandle, port, ActualParameter("Boards"));
			_udpConverter->SetDecimation(dec_factor);
			Parameter("SamplingRate") = ActualParameter("SamplingRate") / dec_factor;
			_sampling_rate = ActualParameter("SamplingRate");
		}
		else
		{
			bcierr << "The UDP Stream requires a valid Neuralynx Pegasus/Cheetah NetCom connection!" << std::endl;
			return;
		}



	}
	int samplingrate = Parameter("UDPSamplingRate");

	if (samplingrate != _sampling_rate)
	{
		if ((samplingrate %_sampling_rate) != 0)
			bcierr << "The selected Sampling rate has to be an integer fraction of the Acq. Device sampling rate ( " << std::to_string(samplingrate) << "Hz )" << std::endl;

		//bciwarn << "Data will be decimated; No anti - aliasing filter implemented !!" << std::endl;
	}


}

void NeuralynxNetComADC::InitializeUDPSettingsFromNetCom(int & numchannels)
{
	std::vector<uint32_t> channelIds;
	std::vector<std::wstring> channelNames;
	QueryNetComDAS();

	if (std::count(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComAcqSourceDataType)) != 1)
		bcierr << "Multiple acquisiton systems found, Direct UDP stream only works with a single UDP source entity" << std::endl;
	//lets find all adc channels to decide the number of udp channels
	
	for (int i = 0; i < _DASObjects.size(); i++)
	{
		std::vector<uint32_t> ch_num;
		if ( //ignore non adc type data
			   _DASTypes.at(i) == s2ws(NlxDataTypes::NetComAcqSourceDataType)
			|| _DASTypes.at(i) == s2ws(NlxDataTypes::NetComEventDataType)
			|| _DASTypes.at(i) == s2ws(NlxDataTypes::NetComVTDataType)
		) 
			continue;

		if (!NetComCommands::GetChannelNumber(mDeviceHandle, _DASObjects.at(i), ch_num))
			bcierr << "Could not get ADC number for DAS Object " << ws2s(_DASObjects.at(i)) << std::endl;


		for (int ch = 0; ch < ch_num.size(); ch++)
		{
			if (std::find(channelIds.begin(), channelIds.end(), ch_num[ch]) == channelIds.end()) //only add channels that are not added yet
			{
				channelIds.push_back(ch_num[ch]);
				if (ch_num.size() > 1)
					channelNames.push_back(_DASObjects.at(i) + L"_" + s2ws(std::to_string(ch)));
				else
					channelNames.push_back(_DASObjects.at(i));
			}
		}
	}
	uint32_t last_ch = *std::max_element(channelIds.begin(), channelIds.end()) + 1;//indexed from 0
	uint32_t num_ch = ceil(last_ch / 32.0) * 32;
	bciout << "NUMBER OF CHANNELS DETECTED: " << std::to_string(last_ch) << " :: Assuming " << std::to_string(num_ch) << " in total (next multiple of 32)";
	Parameter("Boards") = ceil(last_ch / 32.0);
	Parameter("SourceCh") = ActualParameter("Boards") * NUM_CH_PER_BOARD; // number of UDP channels has to be a multiple of 32;


	std::wstring acqSystem = GetAcquistionSystem();
	if (acqSystem.empty())
		return;
	uint32_t samplingrate = 0;
	NetComCommands::GetSamplingRate(mDeviceHandle, acqSystem, samplingrate);
	Parameter("UDPSamplingRate") = samplingrate;



	numchannels = ActualParameter("SourceCh");
	_sampling_rate = ActualParameter("UDPSamplingRate");
	Parameter("SamplingRate") = _sampling_rate;
	_sampling_rate = ActualParameter("SamplingRate");
	_udp_conv_factor = 0.000000015624999960550667; //default accuracy;


	//int acq_sytem_idx = std::distance(_DASTypes.begin(), std::find(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComAcqSourceDataType)));

	Parameter("SourceCh") = numchannels;
	numchannels = ActualParameter("SourceCh");

	Parameter("SourceCh") = numchannels;
	numchannels = ActualParameter("SourceCh");
	Parameter("ChannelNames")->SetNumValues(numchannels);
	Parameter("SourceChGain")->SetNumValues(numchannels);
	Parameter("SourceChOffset")->SetNumValues(numchannels);
	Parameter("SourceChList")->SetNumValues(numchannels);
	for (int i = 0; i < numchannels; i++)
	{
		auto it = std::find(channelIds.begin(), channelIds.end(), i);
		int channel_idx = std::distance(channelIds.begin(), it);
		if (channel_idx >= channelIds.size()) // no entry found
		{
			Parameter("ChannelNames")(i) = "Ch" + std::to_string(i);
			Parameter("SourceChGain")(i) << 1 << "V";
			Parameter("SourceChOffset")(i) = 0;
			bciout << "Initialized ADC Channel " << std::to_string(i) << "With default values, because no information was availble" << std::endl;
		}
		else
		{
			Parameter("ChannelNames")(i) = ws2s(channelNames.at(channel_idx));
			Parameter("SourceChGain")(i) << 1 << "V";
			Parameter("SourceChOffset")(i) = 0;
			bciout << "Initialized ADC Channel " << std::to_string(i) << " with Neuralynx DAS Object " << ws2s(channelNames.at(channel_idx)) << std::endl;
		}
		Parameter("SourceChList")(i) = i;
	}



}

void NeuralynxNetComADC::ConfigureNetComStream(int &numchannels)
{
	QueryNetComDAS();
	numchannels = std::count(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComCSCDataType));

	bciout << "Found " << std::to_string(numchannels) << " Continously sampled Channels" << std::endl;

	bciout << "Found " << std::to_string(std::count(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComSTDataType))) << " Stereotrode Spike Channels" << std::endl;
	bciout << "Found " << std::to_string(std::count(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComSEDataType))) << " Single Electrode Spike Channels" << std::endl;
	bciout << "Found " << std::to_string(std::count(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComTTDataType))) << " Tetrode Spike Channels" << std::endl;
	bciout << "Found " << std::to_string(std::count(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComEventDataType))) << " Event Channels" << std::endl;


	std::vector<std::wstring>::iterator it = std::find(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComCSCDataType));

	bool processing_enabled = true;
	uint32_t new_ch_number = 0;
	for (int i = 0; i < numchannels; i++)//check on which channels processing is enabled
	{
		int index = std::distance(_DASTypes.begin(), it + i);//get the correct name from the subset of CSCs
		NetComCommands::GetAcqEntityProcessingEnabled(mDeviceHandle, _DASObjects.at(index), processing_enabled);
		if (processing_enabled)
		{
			new_ch_number++;

		}
	}

	Parameter("SourceCh") = new_ch_number;
	Parameter("ChannelNames")->SetNumValues(new_ch_number);
	Parameter("SourceChGain")->SetNumValues(new_ch_number);
	Parameter("SourceChOffset")->SetNumValues(new_ch_number);
	Parameter("SourceChList")->SetNumValues(new_ch_number);
	int k = 0;
	
	for (int i = 0; i < numchannels; i++)
	{
		int index = std::distance(_DASTypes.begin(), it + i);//get the correct name from the subset of CSCs
		NetComCommands::GetAcqEntityProcessingEnabled(mDeviceHandle, _DASObjects.at(index), processing_enabled);
		
		if (processing_enabled)
		{
			Parameter("ChannelNames")(k) = ws2s(_DASObjects.at(index));
			std::vector<float> conversion;
			if (NetComCommands::GetVoltageConversionFactor(mDeviceHandle, _DASObjects.at(index), conversion))
			{
				Parameter("SourceChGain")(k) << conversion[0] << "V";
				Parameter("SourceChOffset")(k) = 0;
			}
			else
			{
				bciwarn << "Could not get conversion factor for channel " << ws2s(_DASObjects.at(index));
			}
			Parameter("SourceChList")(k) = k+1;
			k++;
		}
	}

	numchannels = ActualParameter("SourceCh");

	_recordStreamObjects.clear();
	_recordStreamObjects_ordered.clear();
	for (int i = 0; i < Parameter("ChannelNames")->NumValues(); ++i)
	{
		_recordStreamObjects.insert(s2ws(Parameter("ChannelNames")(i)));
		_recordStreamObjects_ordered.push_back(s2ws(Parameter("ChannelNames")(i)));
	}

	//mDeviceHandle->SetCallbackFunctionCSC(ConstSampledRecord, this);


	std::wstring acqSystem = GetAcquistionSystem();
	if (acqSystem.empty())
		return;

	GetSamplingRateFromNetCom(_sampling_rate, acqSystem);
	Parameter("SamplingRate") = _sampling_rate;
	_sampling_rate = ActualParameter("SamplingRate");

	uint32_t hardwareSamplingRate = 0;
	NetComCommands::GetSamplingRate(mDeviceHandle, acqSystem, hardwareSamplingRate);
	if ((hardwareSamplingRate % _sampling_rate) != 0)
	{
		bcierr << "Sampling rate has to be an integer number fraction of the Hardware sampling rate of " << std::to_string(hardwareSamplingRate) << std::endl;
	}

	for (auto recObj : _recordStreamObjects)
		NetComCommands::SetBlockSize(mDeviceHandle, recObj, hardwareSamplingRate / _sampling_rate);
}

std::wstring NeuralynxNetComADC::GetAcquistionSystem()
{
	if (std::count(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComAcqSourceDataType)) == 0)
	{
		bcierr << "No Acquisition system found" << std::endl;
		return L"";

	}
	if (std::count(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComAcqSourceDataType)) > 1)
	{
		bcierr << "BCI2000 only supports Acquisition from a single connected Data Acq. System!" << std::endl;
		return L"";
	}
	else
	{
		std::vector<std::wstring>::iterator it = std::find(_DASTypes.begin(), _DASTypes.end(), s2ws(NlxDataTypes::NetComAcqSourceDataType));
		return _DASObjects.at(std::distance(_DASTypes.begin(), it));
	}
}

void NeuralynxNetComADC::QueryNetComDAS()
{
	ConnectToDevice();
	_DASObjects.clear();
	_DASTypes.clear();
	if (mDeviceHandle->GetDASObjectsAndTypes(_DASObjects, _DASTypes))
	{
		for (int i = 0; i < _DASObjects.size(); ++i)
		{
			bciout << "DAS Available: " << ws2s(_DASTypes.at(i)) << "::" << ws2s(_DASObjects.at(i)) << std::endl;

		}
	}
	else
	{
		bcierr << "Failed to get DASObjectsAndTypes" << std::endl;
	}
}

void
NeuralynxNetComADC::Preflight(const SignalProperties& Input, SignalProperties& Output) const
{
	Parameter("ServerName");

	//if (_useDirectUDPStream && ((((int)Parameter("SourceCh")) % 32) != 0))
	//	bcierr << "UDP channels must be a multipe of 32" << std::endl;

	//Parameter("RecordEvents");
	int channels = Parameter("SourceCh");

	if (Parameter("SourceChGain")->NumValues() != channels)
		bcierr << "Number of SourceChGain entries must be equal to the number of channels" << std::endl;

	if (Parameter("SourceChOffset")->NumValues() != channels)
		bcierr << "Number of SourceChOffset entries must be equal to the number of channels" << std::endl;

	if (Parameter("SourceChList")->NumValues() != channels)
		bcierr << "Number of SourceChList entries must be equal to the number of channels" << std::endl;

	if (_useDirectUDPStream)
		if(((int)Parameter("UDPSamplingRate") % (int)Parameter("SamplingRate")) != 0 || 
			((int)Parameter("UDPSamplingRate") / Parameter("DecimationFactor") != (int)Parameter("SamplingRate")))
				bcierr << "Please check your UDPSamplingRate, SamplingRate and DecimationFactor settings! Selected settings contradict each other!" << std::endl;
	//	State("StereoElectrodeSpikes");
	//	State("TetrodeElectrodeSpikes");
	//	State("SingleElectrodeSpikes");
	//State("NeuralynxEventRecord");
	if (!_useDirectUDPStream)
	{
		for (int i = 0; i < Parameter("ChannelNames")->NumValues(); ++i)
		{
			if (std::find(_DASObjects.begin(), _DASObjects.end(), s2ws(Parameter("ChannelNames")(i))) == _DASObjects.end())
				bcierr << "No DASObject available for " << Parameter("ChannelNames")(i) << std::endl;

		}
	}

	Expression digitalIOExpression(Parameter("DigitalIOExpression"));
	digitalIOExpression.Evaluate();

	SignalType sigType = SignalType::float32;
	Output = SignalProperties(channels, Parameter("SampleBlockSize"), sigType);
}

void
NeuralynxNetComADC::Initialize(const SignalProperties& Input, const SignalProperties& Output)
{
	if (!_useDirectUDPStream)
	{
		_netcom_buffer.InitBuffer(mDeviceHandle, _recordStreamObjects_ordered, MeasurementUnits::SampleBlockDuration());
		ConnectToDevice();
	}
	mDigitalIOExpression = Parameter("DigitalIOExpression").ToString();
	StartAcquisition();
	//mWait = Time::Seconds(MeasurementUnits::SampleBlockDuration()).Seconds();
	//mClock.Start();
}

void
NeuralynxNetComADC::StartAcquisition()
{
	flushQueues();

	if (!_useDirectUDPStream)
	{
		for(const auto& item : _recordStreamObjects)
		{
			int idx = std::distance(_DASObjects.begin(), std::find(_DASObjects.begin(), _DASObjects.end(), item));
			if (!mDeviceHandle->OpenStream(item.c_str()))
				bcierr << "Could not start stream: " << ws2s(item) << std::endl;
		}
		NetComCommands::StartAcquisition(mDeviceHandle);
	}
	else
	{
		_chList.clear();
		for (int i = 0; i < Parameter("SourceChList")->NumValues(); i++)
			_chList.push_back(Parameter("SourceChList")(i));

		_udpConverter->StartListening();
	}
}

void NeuralynxNetComADC::Process(const GenericSignal& in, GenericSignal& Output)
{
	WithThreadPriority(ThreadUtils::Priority::Maximum - 1)
	{
		if (_useDirectUDPStream)
			AcquireFromDirectUDPStream(Output);
		else
			AcquireFromNetComStream(Output);
	}
	if (!mDigitalIODevice.empty())
	{
		bool value = false;
		for (int sample = 0; !value && sample < Output.Elements(); ++sample)
			if (mDigitalIOExpression.Evaluate(&Output, sample))
				value = true;
		if (!NetComCommands::SetDigitalIOBit(mDeviceHandle, mDigitalIODevice, mDigitalIOPort, 0, value))
			bcierr << "Could not set digital output bit";
	}
}

void NeuralynxNetComADC::AcquireFromNetComStream(GenericSignal & Output)
{
	_netcom_buffer.GetData(Output, State("NeuralynxTimestamp"));
}

void NeuralynxNetComADC::AcquireFromDirectUDPStream(GenericSignal & Output)
{
	std::vector<StateRef> ttl;
	for (int i = 0; i < 32; ++i)
		ttl.push_back(State("NeuralynxDigitalInput" + std::to_string(i+1)));
	if (!_udpConverter->GetData(Output, _chList, State("NeuralynxTimestamp"), State("NeuralynxLostSamples"), ttl))
	{
		bciwarn << "No data received from UDP stream!" << std::endl;
		return;
	}
}

void
NeuralynxNetComADC::StopRun()
{
}

void
NeuralynxNetComADC::Halt()
{
	if (!_useDirectUDPStream)
	{
		for (const auto& item : _recordStreamObjects)
		{
			if (!mDeviceHandle->CloseStream(item.c_str()))
				bcierr << "Could not close stream: " << ws2s(item) << std::endl;
		}
	}
	else
	{
		if (_udpConverter)
			_udpConverter->StopListening();
	}
}

//Get Sampling rate by starting the stream for a short time to get packages from each CSC stream
// Throws an error if streams have different sampling rates, which is not allowed (for now)
void NeuralynxNetComADC::GetSamplingRateFromNetCom(int &sampling_rate, std::wstring acqSystem)
{
	sampling_rate = 0;

	std::vector<float> _chsrates;
	std::vector<uint32_t> _block_sizes;

	for (const auto& obj : _recordStreamObjects)
	{
		if (!NetComCommands::GetEffectiveSamplingRate(mDeviceHandle, acqSystem, obj, sampling_rate))
			bcierr << "Could not get effective sampling rate for Object " << ws2s(obj) << std::endl;

		_chsrates.push_back(sampling_rate);
	}

	if (_chsrates.size() == 0)
	{
		bcierr << "Did not receive CSC sampling rates!" << std::endl;
		return;
	}



	auto it = std::unique(_chsrates.begin(), _chsrates.end());
	_chsrates.resize(std::distance(_chsrates.begin(), it));
	if (_chsrates.size() != 1)
	{
		bciwarn << "Channels have different sampling rates, system will be configured to use largest configured sampling rate for all channels if set to auto" << std::endl;
		int min = *std::max_element(_chsrates.begin(), _chsrates.end());

		sampling_rate = min;
	}
	else
		sampling_rate = _chsrates.at(0);

}

void NeuralynxNetComADC::GetSamplingInformationFromFile(std::string file, int & sampling_rate, float & adc_conversion_factor, int& number_of_channels)
{
	sampling_rate = -1;
	adc_conversion_factor = -1;
	number_of_channels = -1;
	std::ifstream  f(file.c_str());
	std::string currLine;

	std::string SAMPLEFREQ_MARKER = "-SamplingFrequency";
	std::string ADC_CONV_MARKER = "-ADBitVolts";
	std::string NUM_CH_MARKER = "-NumADChannels";
	if (f)
	{
		while ((!f.eof()) || ((sampling_rate < 0) && (adc_conversion_factor < 0) && (number_of_channels == -1)))
		{
			getline(f, currLine);
			if (currLine.rfind(SAMPLEFREQ_MARKER) == 0)
			{
				sampling_rate = stof(currLine.substr(SAMPLEFREQ_MARKER.length()));
			}
			if (currLine.rfind(ADC_CONV_MARKER) == 0)
			{
				adc_conversion_factor = stof(currLine.substr(ADC_CONV_MARKER.length()));
			}
			if (currLine.rfind(NUM_CH_MARKER) == 0)
			{
				number_of_channels = stof(currLine.substr(NUM_CH_MARKER.length()));
			}
		}
		f.close();

	}
	else bcierr << "Could not open streaming file: " << file << std::endl;

}



NeuralynxNetComADC::NetComBuffer::NetComBuffer() : mDevice(NULL)
{
}

void NeuralynxNetComADC::NetComBuffer::InitBuffer(NlxNetCom::NetComClient * client, const std::vector<std::wstring>& entities, double blockDuration)
{
	if (client == NULL)
		throw std::exception("NetCom Client is not allowed to be NULL!");

	mDevice = client;
	SetRecordingEntities(entities, blockDuration);
	mDevice->SetCallbackFunctionCSC(&csc_callback, this);
}

void NeuralynxNetComADC::NetComBuffer::SetRecordingEntities(const std::vector<std::wstring>& entities, double blockDuration)
{
	ClearQueues();

	mEntityToChannel.clear();
	int i = 0;
	for (const auto& entity : entities)
		mEntityToChannel[entity] = i++;

	mQueueState.resize(entities.size());
	for (auto& state : mQueueState)
		state.pQueue = new PacketQueue;

	mTimeout = Time::Interval::Seconds(3 * blockDuration);
}

void NeuralynxNetComADC::NetComBuffer::GetData(GenericSignal& output, StateRef& timestamp)
{
	auto* pOutput = output.MutableData();
	bool abort = false;
	for (int ch = 0; ch < output.Channels() && !abort; ++ch)
	{
		QueueState& state = mQueueState[ch];
		for (int sample = 0; sample < output.Elements() && !abort; ++sample)
		{
			if (!state.pCurrentPacket || state.currentSampleIndex >= state.pCurrentPacket->dwNumValidSamples)
			{
				delete state.pCurrentPacket;
				state.pCurrentPacket = nullptr;
				PacketQueue::Consumable packet = state.pQueue->AwaitConsumption(mTimeout);
				if (!packet)
					abort = true;
				else
					state.pCurrentPacket = *packet;
				state.currentSampleIndex = 0;
			}
			if (state.pCurrentPacket)
			{
				pOutput[output.LinearIndex(ch, sample)] = state.pCurrentPacket->snSamples[state.currentSampleIndex++];
				if (ch == 0)
					timestamp.Sample(sample) = (state.pCurrentPacket->qwTimeStamp & 0xffffffff);
			}
			else
			{
				pOutput[output.LinearIndex(ch, sample)] = GenericSignal::NaN;
				if (ch == 0)
					timestamp.Sample(sample) = -1;
			}
		}
	}
}

NeuralynxNetComADC::NetComBuffer::~NetComBuffer()
{
	ClearQueues();
}

void NeuralynxNetComADC::NetComBuffer::csc_callback(void * myClassPtr, NlxDataTypes::CRRec * records, int numRecords, const wchar_t * const objectName)
{
	ThreadUtils::Priority::Set(ThreadUtils::Priority::Maximum);
	NetComBuffer* this_ = static_cast<NetComBuffer*>(myClassPtr);
	auto i = this_->mEntityToChannel.find(objectName);
	if (i == this_->mEntityToChannel.end())
		return;
	auto& pQueue = this_->mQueueState[i->second].pQueue;
	for (int j = 0; j < numRecords; ++j)
		pQueue->Produce(new NlxDataTypes::CRRec(records[j]));
}

void NeuralynxNetComADC::NetComBuffer::FlushQueues()
{
	for (auto& state : mQueueState)
	{
		state.currentSampleIndex = -1;
		delete state.pCurrentPacket;
		state.pCurrentPacket = nullptr;
		while (!state.pQueue->Empty())
		{
			PacketQueue::Consumable el = state.pQueue->Consume();
			if (el)
				delete* el;
		}
	}
}

void NeuralynxNetComADC::NetComBuffer::ClearQueues()
{
	FlushQueues();
	for (auto& state : mQueueState)
		delete state.pQueue;
	mQueueState.clear();
}


