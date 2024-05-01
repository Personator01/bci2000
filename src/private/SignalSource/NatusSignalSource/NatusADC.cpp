////////////////////////////////////////////////////////////////////////////////
// Authors:
// Description: NatusADC implementation
////////////////////////////////////////////////////////////////////////////////

#include "NatusADC.h"
#include "BCIStream.h"
#include "BCIEvent.h"


// In order to help you write a source module, exchange of information
// between amplifier and the BCI2000 source module is indicated by the use of
// macros.
// Once you are done with writing the source module, each occurrence of
// GET_FROM_AMP_API(), CALL_AMP_API(), and AMP_API_SYNC_GET_DATA() should
// have been replaced with actual calls to the amplifier API, or constants.
// By removing or disabling those macros' definitions below, you can then
// make sure that the compiler will notify you of any oversights.


#include "ThreadUtils.h"

using namespace std;

// Make the source filter known to the framework.
RegisterFilter(NatusADC, 1); // ADC filters must be registered at location "1" in order to work.

NatusADC::NatusADC() : mbuffer(NULL)
{
}

NatusADC::~NatusADC()
{
	client.Disconnect();
	delete[] mbuffer;
}

void
NatusADC::Publish()
{
	// Declare any parameters that the filter needs....

	BEGIN_PARAMETER_DEFINITIONS

		"Source:Signal%20Properties int SourceCh= auto "
		"auto 1 % // number of digitized and stored channels",

		"Source:Signal%20Properties int SamplingRate= auto  "
		"// Signal Sampling Rate",

		"Source:Signal%20Properties list SourceChGain= 1 auto "
		" // physical units per raw A/D unit",

		"Source:Signal%20Properties list SourceChOffset= 1 auto "
		" // raw A/D offset to subtract, typically 0",

		"Source:Signal%20Properties int SampleBlockSize= auto 1 % % "
		"// number of samples per block",

		"Source:Signal%20Properties list SourceChList= 1 auto "
		" // channel numbers to stream - space separated numbers - channel numbers start from 0",


		"Source:NatusServerSettings list ChannelNames= 1 auto "
		" // names of amplifier channels",

		"Source:NatusServerSettings int ServerBlockSize= auto 1 % % "
		"// number of samples per block transmitted by the server application in each package ",

		"Source:NatusServerSettings int DecimationFactor= auto 1 % % "
		"// number of blocks to aggregate & decimate ",

		"Source:NatusServerSettings string ServerIP= auto "
		"auto 1 % // number of digitized and stored channels",

		"Source:NatusServerSettings string Port= auto "
		"auto 1 % // number of digitized and stored channels",








		END_PARAMETER_DEFINITIONS

		// ...and likewise any state variables.

		// IMPORTANT NOTE ABOUT BUFFEREDADC AND STATES:
		// * BCI2000 States, or "state variables", are additional data channels stored alongside signal data,
		//   with a resolution of one value per source signal sample.
		// * A State is much like a source signal channel, but differs from a source signal channel in the
		//   following respects:
		//   + You choose the number of bits used to represent the State's value, up to 64.
		//   + Unlike source signals, States are transmitted through all stages of processing, and their values
		//     may be modified during processing, allowing all parts of the system to store state information in
		//     data files.
		//   + When loading a BCI2000 data file for analysis, States appear separately, with names, which is
		//     typically more useful for trigger information than being just another channel in the signal.
		//   + States may be set synchronously from inside a filter's Process() handler, or asynchronously using
		//     a "bcievent" interface.
		//   + If your amplifier provides a digital input, or another kind of "trigger" information, it makes sense
		//     to store this information in form of one or more States. From a user perspective, it is probably most
		//     useful if physically distinct amplifier input sockets correspond to States, and single bits to certain
		//     lines or values communicated through such a socket.
		//   + If the amplifier API asynchronously reports trigger information through a callback mechanism, you
		//     should register a callback that uses the "bcievent" interface to set states asynchronously.
		//     This example provides a "MyAsyncTriggers" "event state", and a sample callback function.
		//   + If the amplifier API sends trigger information in the same way as it does for signal data, you should
		//     use a "State channel" to represent it in your source module. This example provides a "MySyncTriggers"
		//     state, and writes to it when acquiring data.
		BEGIN_STREAM_DEFINITIONS
		"Interpolated 1 0 0",
		//"QueueStatus 32 0 0",
		END_STREAM_DEFINITIONS


}




void
NatusADC::AutoConfig(const SignalProperties&)
{
	// The user has pressed "Set Config" and some parameters may be set to "auto",
	// indicating that they should be set from the current amplifier configuration.
	// In this handler, we behave as if any parameter were set to "auto", and the
	// framework will transparently make sure to preserve user-defined values.

	// Device availability (or connection parameters) may have changed, so close
	// and reopen the connection to the device before proceeding.
	client.Verbose(false);
	NatusDeviceInformation inf;
	Parameter("Port") = 1030;
	NatusChannelInformation channelInf;
	Parameter("ServerIP") = "127.0.0.1";
	try
	{
		bciout << "Trying to connect to: " << ActualParameter("ServerIP") << ":" << ActualParameter("Port") << std::endl;
		//
		if (!client.IsConnected())
		{
			client.Disconnect();
			client.Connect(ActualParameter("ServerIP"), ActualParameter("Port"));

		}
		inf = client.GetInformation();
		channelInf = client.GetChannelInfo();
		//Requirement to be below 512 Hz is only for a Natus server version 9.2 and below
		if (inf.SamplingRate > 512)
		{
			bciwarn << "Natus server version must be 9.4 or above.\n"
				<< "If not, set the BCI2000 SamplingRate to 512Hz" << std::endl;
			//inf.SamplingRate = 512;
		}
		Parameter("SamplingRate") = inf.SamplingRate;
		Parameter("SourceCh") = inf.NumberOfChannels;

		bciout << "Device has " << std::to_string(inf.NumberOfChannels) << " Channels" << std::endl;
	}
	catch (std::exception e)
	{
		bcierr << e.what() << std::endl;
		return;
	}
	mNumberOfSignalChannels = ActualParameter("SourceCh");

	Parameter("SourceChList")->SetNumValues(mNumberOfSignalChannels);

	for (int i = 0; i < mNumberOfSignalChannels; i++)
		Parameter("SourceChList")(i) = i;

	Parameter("SourceCh") = ActualParameter("SourceChList")->NumValues();



	if (mNumberOfSignalChannels != ActualParameter("SourceChList")->NumValues())
	{
		bcierr << "SourceChList must have SourceCh number of entries!" << std::endl;
		return;
	}

	//mNumberOfSignalChannels = ActualParameter("SourceCh");

	Parameter("SampleBlockSize") = 20;
	Parameter("ServerBlockSize") = 1;
	Parameter("SourceChOffset")->SetNumValues(mNumberOfSignalChannels);
	Parameter("SourceChGain")->SetNumValues(mNumberOfSignalChannels);
	Parameter("ChannelNames")->SetNumValues(mNumberOfSignalChannels);

	Parameter("DecimationFactor") = 1;

	if (ActualParameter("ChannelNames")->NumValues() < mNumberOfSignalChannels)
		mNumberOfSignalChannels = ActualParameter("ChannelNames")->NumValues();

	if (ActualParameter("SourceChGain")->NumValues() < mNumberOfSignalChannels)
		mNumberOfSignalChannels = ActualParameter("SourceChGain")->NumValues();

	if (ActualParameter("SourceChOffset")->NumValues() < mNumberOfSignalChannels)
		mNumberOfSignalChannels = ActualParameter("SourceChOffset")->NumValues();


	int decimationFactor = ActualParameter("DecimationFactor");
	int samplingRate = ActualParameter("SamplingRate");
	bciout << "Decimation Factor is: " << std::to_string(decimationFactor) << std::endl;
	if (decimationFactor < 1)
	{
		bcierr << "Decimation Factor should be 1 or larger." << std::endl;
		return;
	}
	if (samplingRate % decimationFactor != 0)
	{
		bcierr << "Sampling Rate should be a multiple of decimation factor" << std::endl;
		std::string s = "Possible values for decimation factor ";
		for (int i = 1; i < 10; i++)
			if (samplingRate % i == 0)
				s = s + to_string(i) + " ";
		bcierr << s << std::endl;
		return;
	}

	client.SetSampleBlockSize(ActualParameter("ServerBlockSize"));
	client.SetDecimationFactor(ActualParameter("DecimationFactor"));
	//updating sampling rate in accordance with the decimation factor
	float sr = ActualParameter("SamplingRate");
	float df = ActualParameter("DecimationFactor");
	Parameter("SamplingRate") = sr / df;
	bciout << "Device Sampling Rate is: " << std::to_string(inf.SamplingRate) << "\n"
		<< "BCI2000 Sampling Rate is: " << Parameter("SamplingRate").InHertz() << std::endl;



	std::vector<uint32_t> channelsInList;

	for (int i = 0; i < ActualParameter("SourceChList")->NumValues(); i++)
		channelsInList.push_back(ActualParameter("SourceChList")(i));
	try
	{
		client.SelectChannelsToStream(channelsInList);
	}
	catch (std::exception e)
	{
		bcierr << "Could not set channels to stream on server: " << e.what() << std::endl;
	}
	//updating channels to show in accordance with the channels selected

	bciout << "Number of channels " << mNumberOfSignalChannels << std::endl;
	Parameter("ChannelNames")->SetNumValues(mNumberOfSignalChannels);
	Parameter("SourceChOffset")->SetNumValues(mNumberOfSignalChannels);
	Parameter("SourceChGain")->SetNumValues(mNumberOfSignalChannels);


	for (int i = 0; i < mNumberOfSignalChannels; ++i)
	{

		Parameter("SourceChOffset")(i) = 0;
		Parameter("SourceChGain")(i) = "1muV";
		if (channelsInList[i] < channelInf.ChannelNames.size())
			Parameter("ChannelNames")(i) = channelInf.ChannelNames[channelsInList[i]];
		else
			Parameter("ChannelNames")(i) = "CH" + std::to_string(channelsInList[i]);

	}

}



void
NatusADC::Preflight(const SignalProperties&, SignalProperties& Output) const
{
	State("Interpolated");
	int samplesPerBlock = Parameter("SampleBlockSize");
	if (Parameter("ServerBlockSize") < 1)
		bcierr << "ServerBlockSize cannot be smaller than 1!" << std::endl;

	if (Parameter("SourceChOffset")->NumValues() != mNumberOfSignalChannels)
		bcierr << "A SourceChOffset has to be defined for each Channel!" << std::endl;

	if (Parameter("SourceChGain")->NumValues() != mNumberOfSignalChannels)
		bcierr << "A SourceChGain has to be defined for each Channel!" << std::endl;

	if (Parameter("ChannelNames")->NumValues() != mNumberOfSignalChannels)
		bcierr << "A ChannelNames has to be defined for each Channel!" << std::endl;



	bciout << "Setting blocksize to " << std::to_string(samplesPerBlock) << std::endl;
	Output = SignalProperties(mNumberOfSignalChannels, samplesPerBlock, SignalType::float32);
	//Output.ChannelLabels()[Output.Channels() - 1] = "@Interpolated";
	//Output.ChannelLabels()[Output.Channels() - 1] = "@QueueStatus";
	//TODO -- we need to figure out which channels of the real system are state channels!

	//char *channelStreamCopy = new char[client.channelInfo.StreamNamesLength + 1];
	//memcpy(channelStreamCopy, client.channelInfo.StreamNames, client.channelInfo.StreamNamesLength + 1);
	//char *split = strtok(channelStreamCopy, " ");
	//while (split != NULL)
	//{
	//	State(std::string(split));
	//	Output.SetChannels(Output.Channels() + 1);
	//	Output.ChannelLabels()[Output.Channels() - 1] = "@" + std::string(split);
	//	split = strtok(NULL, " ");
	//}
	//delete channelStreamCopy;

	// Note that the NatusADC instance itself, and its members, are read-only during the
	// Preflight phase---note the "const" at the end of the OnPreflight prototype above.
	// Any methods called by OnPreflight must also be declared as "const" in the same way.

}

void
NatusADC::Initialize(const SignalProperties&, const SignalProperties& Output)
{
	//mNumberOfSignalChannels = Parameter ("SourceCh");

	try
	{
		if (!client.IsConnected())
		{
			client.Disconnect();
			client.Connect(Parameter("ServerIP"), Parameter("Port"));
		}
		if (0xff != client.Ping(0xff))
			bcierr << "Wrong ping response received" << std::endl;
		//client.SetSampleBlockSize (Parameter ("ServerBlockSize"));
		//client.SetDecimationFactor (Parameter ("DecimationFactor"));
		//client.SelectChannelsToStream(Parameter ("ChannelNumbersToStream"));
		//bciout << "Decimation Factor - queried from tester: " << client.GetDecimationFactor();
		//client.SetSampleBlockSize (1);
		bufferSize = Output.Elements()*(Output.Channels()+1);
		delete[] mbuffer;
		mbuffer = new float[bufferSize];

	}
	catch (std::exception e)
	{
		bcierr << e.what() << std::endl;
		return;
	}

	client.StartStream();

}




void NatusADC::Process(const GenericSignal&, GenericSignal& Output)
//void NatusADC::Process(const GenericSignal& input , GenericSignal& Output)
{
	//float*buff = reinterpret_cast<float*>(mbuffer);
	//
	//for (int i = 0; i < bufferSize; i++)
	//{
	//	buff[i] = 100;

	//}
	WithThreadPriority(ThreadUtils::Priority::Maximum - 1)
	{


		if (!client.GetSamples(mbuffer, Output.Elements(), bufferSize * sizeof(float)))
		{
			bciwarn << "Timout during data acquisition" << std::endl;
			for (int sample = 0; sample < Output.Elements(); sample++)

			{
				for (int ch = 0; ch < Output.Channels(); ch++)
				{
					Output(ch, sample) = GenericSignal::NaN;

				}

				return;
			}
		}

		uint32_t numI = 0;
		uint32_t i = 0;
		for (int sample = 0; sample < Output.Elements(); sample++)
		{
			for (int ch = 0; ch < Output.Channels(); ch++)
			{
				Output(ch, sample) = mbuffer[i++];

			}
			//Output(Output.Channels()-1, sample) = client.GetSamplesInQueue();
			numI += mbuffer[i];
			State("Interpolated").Sample(sample) = mbuffer[i++];

		}
		if (numI > (Output.Elements() / 0.1))
			bciwarn << "Loosing a lot of packages in current block (" << numI << " per block of " << Output.Elements() << ") if this issue persists increase ServerBlockSize " << std::endl;
	}

}






void NatusADC::Halt()
{
	if (client.IsConnected())
		client.StopStream();
	NatusSampleBlock *p = NULL;

}

