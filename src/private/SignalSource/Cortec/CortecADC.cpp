////////////////////////////////////////////////////////////////////////////////
// Authors: Alexander Belsten, Markus Adamek (belsten/adamek @neurotechcenter.org)
// Description: CortecADC implementation
////////////////////////////////////////////////////////////////////////////////

#include "CortecADC.h"
#include "BCIStream.h"
#include "BCIEvent.h"
#include "RunManager.h"
#include "FileUtils.h"

#define bcidbgOn
//#define NODEVICE

// In order to help you write a source module, exchange of information
// between amplifier and the BCI2000 source module is indicated by the use of
// macros.
// Once you are done with writing the source module, each occurrence of
// GET_FROM_AMP_API(), CALL_AMP_API(), and AMP_API_SYNC_GET_DATA() should
// have been replaced with actual calls to the amplifier API, or constants.
// By removing or disabling those macros' definitions below, you can then
// make sure that the compiler will notify you of any oversights.

// Depending on the kind of amplifier you have, occurrences of GET_FROM_AMP_API()
// may be read through the amplifier API, or obtained from documentation.

using namespace std;

// Make the source filter known to the framework.
RegisterFilter(CortecADC, 1); // ADC filters must be registered at location "1" in order to work.

const uint16_t IMPLANT_VOLTAGE_MULTIPLIER        = 1000;
const uint16_t IMPLANT_HUMIDITY_MULTIPLIER       = 100;
const uint16_t IMPLANT_CONTROLVALUE_MULTIPLIER   = 100;
const uint16_t IMPLANT_PRIMARYCURRENT_MULTIPLIER = 1000;
const uint16_t IMPLANT_TEMPERATURE_MULTIPLIER    = 100;

CortecADC::CortecDataListener::CortecDataListener(CortecADC * p)
{
	parent         = p;
	mResetCounter  = false;
	mSampleCounter = 0;
}


void CortecADC::CortecDataListener::onData(const std::vector<CSample>* samples)
{
#ifdef OUTPUT_BUFFER_TIMES
	chrono::time_point<chrono::system_clock> start_time = chrono::system_clock::now(); 
#endif
	for (int el = 0; el < samples->size(); ++el)
	{
		// if this is the first call to onData we need to initalize the counter
		if (mResetCounter)
		{
			mSampleCounter = samples->at(el).getMeasurementCounter()-1;
			mResetCounter = false;
		}
		// if observed sample's counter is not the current counter += 1, append NULL for each sample missed
		uint32_t thisCounter = samples->at(el).getMeasurementCounter();
		for (int i = 1; i < thisCounter - mSampleCounter; i++)
		{
			parent->mDataQueue.push(NULL);
		}
		mSampleCounter = thisCounter;
		if (samples->at(el).getNumberOfMeasurements() != parent->mNumberOfSignalChannels)
			bcierr << "CortecADC Error: Number of channels received from implant does not match BCI2000 number of channels" << endl;
		parent->mDataQueue.push(samples->at(el).getMeasurements());

	}
	//check if we have enough data for output
	if (parent->mDataQueue.unsafe_size() >= parent->mElementSize)
		SetEvent(parent->mDataLock);
#ifdef OUTPUT_BUFFER_TIMES
	int push_ms = chrono::duration_cast<std::chrono::microseconds>(chrono::system_clock::now() - start_time).count();
	// this could be called every 1ms, print every second
	if (parent->mprintcounterpush % 1000 == 0) 
		bciout << "\tPush time: " << push_ms << "us" << endl;
	parent->mprintcounterpush += 1;
#endif
	delete samples;
}

CortecADC::CortecADC() :
	mImplantFactory(createImplantFactory(false, "")),
	mImplant(nullptr), 
	mImplantInfo(nullptr),
	mStimCommandFactory(createStimulationCommandFactory()),
	mListener(this), 
	mPrevSample(nullptr),
	mIsMeasuring(false),
	mNumberOfSignalChannels(0),
	mBlockSize_ms(0),
	mSamplingRate(0),
	mImplantVoltage(0),
	mImplantHumidity(0),
	mImplantControlValue(0),
	mImplantPrimaryCoilCurrent(0),
	mImplantTemperature(0),
	mImplantStimulation(false),
  mImplantStimulationBursts(false),
	mTimeOutOccurred(false),
	mStimulationEnabled(false),
	mStimulationTriggered(false),
	mStimId(0),
	mRecording(false),
  mStimMode(StimulationMode::STIM_MODE_COUNT),
	mVis("Cortec Impedances")
{
	mDataLock = CreateEvent(NULL, false, false, NULL);
}

CortecADC::~CortecADC()
{
	this->ClearPulseMap();
	this->ClearCommandList();
	this->DisconnectFromImplant();
	if (mPrevSample != nullptr)
		delete[] mPrevSample;
	CloseHandle(mDataLock);
}

void
CortecADC::Publish()
{
	// Declare any parameters that the filter needs....

	stringstream stateinfoparm;
	stateinfoparm << "Source:Info matrix StateInfo= " 
				  << "{ Voltage Humidity Temperature ControlValue PrimaryCoilCurrent} "
			  	  << "{ modifier unit } ";
	stateinfoparm << IMPLANT_VOLTAGE_MULTIPLIER        << " V "
				  << IMPLANT_HUMIDITY_MULTIPLIER	   << " percent%20rh "
			  	  << IMPLANT_TEMPERATURE_MULTIPLIER	   << " deg%20C "
			   	  << IMPLANT_CONTROLVALUE_MULTIPLIER   << " percent "
				  << IMPLANT_PRIMARYCURRENT_MULTIPLIER << " mA "
				  << " // (readonly)(noedit)";

  stringstream stimmodeparm;
  stimmodeparm << "Stimulation:Stimulation int StimulationMode= 0 0 0 2 "
    << "// Stimulation mode - each has certain limitations: "
    << (int)StimulationMode::STIM_MODE_VOLATILE_CMD_PRELOADING << ": volatile commands,"
    << (int)StimulationMode::STIM_MODE_PERSISTENT_CMD_PRELOADING << ": persistant command, "
    << (int)StimulationMode::STIM_MODE_PERSISTENT_FUNC_PRELOADING << ": persistant functions, "
    << " (enumeration)";
  BEGIN_PARAMETER_DEFINITIONS

    "Source:Signal%20Properties int SourceCh= auto "
    "auto 1 % // number of digitized and stored channels (noedit)",

    "Source:Signal%20Properties int SampleBlockSize= auto "
    "auto 1 % // number of samples transmitted at a time",

    "Source:Signal%20Properties float SamplingRate= auto "
    "auto 0.0 % // sample rate, defined by connected device (noedit)",

    "Source:Signal%20Properties list ReferenceCh= 1 auto % % "
    " // list of reference channels",

    "Source:Signal%20Properties list SourceChGain= 1 auto "
    " // physical units per raw A/D unit",

    "Source:Signal%20Properties list SourceChOffset= 1 auto "
    " // raw A/D offset to subtract, typically 0",

    "Source:Signal%20Properties list ChannelNames= 1 auto "
    " // names of amplifier channels",

    "Source:Signal%20Properties int AmplificationFactor= 0 "
    "0 0 3 // factor applied to the recorded data on the implant: "
    "0 57.5dB, 1 51.5dB, 2 45.5dB, 3 39.5dB (enumeration)",

    "Source:Signal%20Properties int UseGround= 0 "
    "0 0 1 // use ground electrode while measuring (boolean)",

    // ================ Stimulation Parameters ================
    "Stimulation:Stimulation int EnableStimulation= 0 0 0 1 "
    " //Enable/Disable Stimulation (boolean)",

    stimmodeparm.str(),

    "Stimulation:Stimulation int MeasureImpedance= 0 0 0 1 "
    "// Enable to measure impedance before recording (boolean)",

		"Stimulation:Stimulation matrix StimulationPulses= "
			"{ PulseID Pulse%20amplitude%20[uA] Pulse%20duration%20[us] Dead%20zone%200%20[us] Dead%20zone%201%20[us]} " // row labels
			"{ Pulse1 Pulse2 Pulse3 Pulse4 } "							 // column labels
			"1 2 3 4  "													 // StimulatonId
			"250 200 150 100 "											 // amplitude in microA
			"500 500 500 500 "											 // duration in microseconds
			"10 10 10 10 "												 // dz0 in microseconds
			"2550 2550 2550 2550 ",										 // dz1 in microseconds
			   
		"Stimulation:Stimulation matrix StimulationTriggers= "
			"{ Trigger PulseID Source%20ch Destination%20ch Pulse%20Repetition Train%20Frequency Train%20Repetition} "// row labels
			"{ Sequence1 Sequence2 } "									   // column labels
			"StimulusCode==1 StimulusCode==2 "						 // Trigger
			"1 4 "														             // PulseID
			"{ list 2 1 2 } { list 2 3 4 } "							 // +%20ch
			"{ list 2 3 4 } { list 2 5 6 } "							 // -%20ch
			"10 20 "													             // Repetition
			"1 2 "																				 // Train Frequency
			"3 4 ",																				 // Train Repetition

		// ================ READ ONLY PARAMETERS ==========================
		"Source:Info matrix DeviceInfo= 1 1 auto "
			"// Device information (readonly)(noedit)",
								
		stateinfoparm.str(),

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
		BEGIN_STATE_DEFINITIONS

			"ImplantVoltage 16 0 ",
			"ImplantHumidity 16 0 ",
			"ImplantControlValue 16 0 ",
			"ImplantPrimaryCoilCurrent 16 0 ",
			"ImplantTemperature 16 0 ",
			"RequestedStimulation 1 0"

		END_STATE_DEFINITIONS

    BEGIN_STREAM_DEFINITIONS

      "ImplantLostSample 1 0 ",

    END_STREAM_DEFINITIONS

		BEGIN_EVENT_DEFINITIONS

			"ImplantStimulation 1 0 ",
      "ImplantStimulationBursts 32 0 ",

		END_EVENT_DEFINITIONS
}

void
CortecADC::OnAutoConfig()
{
	// The user has pressed "Set Config" and some parameters may be set to "auto",
	// indicating that they should be set from the current amplifier configuration.
	// In this handler, we behave as if any parameter were set to "auto", and the
	// framework will transparently make sure to preserve user-defined values.

	Parameter("SourceCh") = 32;
	Parameter("SampleBlockSize") = 100;
	mElementSize = Parameter("SampleBlockSize").ToNumber();
#ifndef NODEVICE
  if (!IsConnected())
  {
    int tries = 0;
    bciwarn << "CortecADC: Attempting to connect to implant..." << endl;
    //try the whole connection process 10 times
    while (!ConnectToImplant() && tries < 10)
    {
      mImplant.reset(nullptr);
      tries++;
      bciwarn << "CortecADC: Continuing the connection attempt..." << endl;
      Sleep(500);
      DisconnectFromImplant();
    }
    if (!IsConnected()) {
      bcierr << "Could not connect to device!" << endl;
      return;
    }
  }
	mTimeOutOccurred = false;
	auto implantInfo=mImplant->getImplantInfo();
	Parameter("SamplingRate") = implantInfo->getSamplingRate();

	// populate device info parameter
	Parameter("DeviceInfo")->SetDimensions(3, 2);
	Parameter("DeviceInfo")(0, 0) = "Device Type";
	Parameter("DeviceInfo")(1, 0) = "Device ID";
	Parameter("DeviceInfo")(2, 0) = "Firmware Version";
	Parameter("DeviceInfo")(0, 1) = implantInfo->getDeviceType();
	Parameter("DeviceInfo")(1, 1) = implantInfo->getDeviceId();
	Parameter("DeviceInfo")(2, 1) = implantInfo->getFirmwareVersion();
	
	int channels = implantInfo->getChannelCount(); //number of channels?
	delete implantInfo;
#else
  int channels = 16;
  mTimeOutOccurred = true;
  Parameter("SamplingRate") = 1000;
#endif

	Parameter("ReferenceCh")->SetNumValues(0);

	Parameter("SourceCh") = channels; // Set SourceCh in case of "auto"
	// If SourceCh != auto but e.g. SourceChGain != auto, we need to use the actual
	// rather than the auto-set value of SourceCh in order to consistently configure
	// SourceChGain.
	// For this purpose, an ActualParameter() call will retrieve a parameter's current
	// value, no matter whether auto-configured or user-configured.
	channels = ActualParameter("SourceCh");
	Parameter("ChannelNames")->SetNumValues(channels);
	Parameter("SourceChGain")->SetNumValues(channels);
	Parameter("SourceChOffset")->SetNumValues(channels);
	for (int i = 0; i < channels; ++i)
	{
		// For EEG amplifiers, channel names should use 10-20 naming if possible.
		const char* name = 0;
		Parameter("ChannelNames")(i) << "CH" << i; // Omit "<< i" if channel names are unique.

		double gainFactor = 1.0;
		Parameter("SourceChGain")(i) << gainFactor << "muV"; // Always provide correct physical unit for documentation and display purposes.
		// For most amplifiers, offset removal is done within the amp hardware or driver. Otherwise, you may set this to a nonzero value.
		Parameter("SourceChOffset")(i) = 0;

	}
}

void
CortecADC::OnPreflight(SignalProperties& Output) const
{
	//State("ImplantLostSample");
	State("ImplantVoltage");
	State("ImplantHumidity");
	State("ImplantControlValue");
	State("ImplantPrimaryCoilCurrent");
	State("ImplantTemperature");
	State("Running");
	State("Recording");
	State("RequestedStimulation");

	// Internally, signals are always represented by GenericSignal::ValueType == double.
	// Here, you choose the format in which data will be stored, which may be
	// int16, int32, or float32.
	// Typically, you will choose the format that your amplifier natively provides, in
	// order to avoid loss of precision when converting, e.g., int32 to float32.
	SignalType sigType = SignalType::float32;

	int samplesPerBlock        = Parameter("SampleBlockSize");
	int numberOfSignalChannels = Parameter("SourceCh");
	Output = SignalProperties(numberOfSignalChannels + 1, samplesPerBlock, sigType);
	Output.ChannelLabels()[numberOfSignalChannels] = "@ImplantLostSample";

	// do not subtract one because we are just operating in BCI2000 channel space, not uploading 
	// to device
	set<uint32_t> ref_loc = BCI2000ListToSet(Parameter("ReferenceCh"), false);
	for (auto itr = ref_loc.begin(); itr != ref_loc.end(); itr++)
	{
		if (*itr < 1 || *itr > numberOfSignalChannels)
		{
			bcierr << "CortecADC Error: Reference channels must be in range 1 to "
				<< numberOfSignalChannels << endl;
			return;
		}
	}

}

void
CortecADC::Preflight(const SignalProperties& Input, SignalProperties& Output) const
{
	BufferedADC::Preflight(Input, Output);
	int numberOfSignalChannels = Parameter("SourceCh");
	GenericSignal signal(Output);

	State("RequestedStimulation");
	if (Parameter("EnableStimulation"))
	{
    StimulationMode stimMode = (StimulationMode)Parameter("StimulationMode").ToNumber();
		// ==== check atoms ====
		vector<int> pulseIDs;
		auto pulseParm = Parameter("StimulationPulses");
		for (int atom = 0; atom < pulseParm->NumColumns(); atom++)
		{
			// check for valid stim id (>-1)
			if (pulseParm(0, atom) < 0)
			{
				bcierr << "CortecADC Stimulation Error: StimulationID must be positive or zero" << endl;
				return;
			}
			pulseIDs.push_back(pulseParm(0, atom));
			// check for valid amplitude ... ? [0, 6120] uA
			if (pulseParm(1, atom) > 6120)
			{
				bcierr << "CortecADC StimulationPulses Error: Pulse amplitude must be less than or equal to 6120 uA" << endl;
				return;
			}
			if (pulseParm(1, atom) < 0)
			{
				bcierr << "CortecADC StimulationPulses Error: Pulse amplitude must be greater than or equal to 0 uA" << endl;
				return;
			}

			// check for valid duration ([10, 2550] us)
			if (pulseParm(2, atom) < 10)
			{
				bcierr << "CortecADC StimulationPulses Error: Pulse duration must be greater than or equal to 10 us" << endl;
				return;
			}
			if (pulseParm(2, atom) > 2550)
			{
				bcierr << "CortecADC StimulationPulses Error: Pulse duration must be less than or equal to 2550 us" << endl;
				return;
			}
			// check for valid dz0 [10, 2550] us
			if (pulseParm(3, atom) < 10)
			{
				bcierr << "CortecADC StimulationPulses Error: Dead zone 0 must be greater than or equal to 10 us" << endl;
				return;
			}
			if (pulseParm(3, atom) > 2550)
			{
				bcierr << "CortecADC StimulationPulses Error: Dead zone 0 must be less than or equal to 2550 us" << endl;
				return;
			}
			// check for valid dz1 [10, 20400] us
			if (pulseParm(4, atom) < 10)
			{
				bcierr << "CortecADC StimulationPulses Error: Dead zone 1 must be greater than or equal to 10 us" << endl;
				return;
			}
			if (pulseParm(4, atom) > 20400)
			{
				bcierr << "CortecADC StimulationPulses Error: Dead zone 1 must be less than or equal to 20400 us" << endl;
				return;
			}
		}
		// check that pulseIDs are unique
		if (unique(pulseIDs.begin(), pulseIDs.end()) != pulseIDs.end())
		{
			bcierr << "CortecADC StimulationPulses Error: PulseIDs must be unique" << endl;
			return;
		}

    // ==== check triggers ====
		auto trigParm = Parameter("StimulationTriggers");

    //can only have one row if persistant command desired
    if (stimMode == StimulationMode::STIM_MODE_PERSISTENT_CMD_PRELOADING)
    {
      if (pulseParm->NumColumns() > 1)
        bcierr << "CortecADC Stimulation Error: If Stimulation Mode is Persistant Command, StimulationPulses "
               << "must only contain one configuration (one column)." << endl;
    }
    else if (stimMode == StimulationMode::STIM_MODE_PERSISTENT_FUNC_PRELOADING)
    {
      if (trigParm->NumColumns() > 16)
        bcierr << "CortecADC Stimulation Error: In Persistent Function Stimulation Mode, StimulationTriggers "
               << "must have less than or equal to 16 columns" << endl;
      if (trigParm->NumRows() > 5)
        bcierr << "CortecADC Stimulation Error: In Persistent Function Stimulation Mode, there cannot be any trains. "
               << " Delete the last 2 rows of StimulationTriggers or change your stimulation mode" << endl;
    }

		for (int trig = 0; trig < trigParm->NumColumns(); trig++)
		{
			// check for valid trigger expression
			Expression trigExpression = trigParm(0, trig);
			trigExpression.Compile();
			trigExpression.Evaluate(&signal);

			// check for valid pulseID 
			vector<int>::iterator location = find(pulseIDs.begin(), pulseIDs.end(), trigParm(1, trig));
			if (location == pulseIDs.end())
			{
				bcierr << "CortecADC Stimulation Error: Invalid PulseID in StimulationTrigger list" << endl;
				return;
			}
			
			// check that +/- ch lists are disjoint
			for (int pos_ch = 0; pos_ch < trigParm(2, trig)->NumValues(); pos_ch++)
			{
				for (int neg_ch = 0; neg_ch < trigParm(3, trig)->NumValues(); neg_ch++)
				{
					if (trigParm(2, trig)(pos_ch) == trigParm(3, trig)(neg_ch))
					{
						bcierr << "CortecADC Stimulation Error: Source and destination channel lists are not " <<
							"disjoint for a sequence." << endl;
						return;
					}
				}
			}
			for (int pos_ch = 0; pos_ch < trigParm(2, trig)->NumValues(); pos_ch++)
			{
				if (trigParm(2, trig)(pos_ch) < 1 || trigParm(2, trig)(pos_ch) > numberOfSignalChannels)
				{
					bcierr << "CortecADC Stimulation Error: Source channels out of range. Channels must be in range 1 to "
						<< numberOfSignalChannels << endl;
					return;
				}
			}
			for (int neg_ch = 0; neg_ch < trigParm(3, trig)->NumValues(); neg_ch++)
			{
				if (trigParm(3, trig)(neg_ch) < 0 || trigParm(3, trig)(neg_ch) > numberOfSignalChannels)
				{
					bcierr << "CortecADC Stimulation Error: Destination channel out of range. Channels must be in range 1 to "
						<< numberOfSignalChannels << endl;
					return;
				}
			}
      //num pulses
      if (trigParm(4, trig) < 1 || trigParm(4, trig) > 255)
      {
        bcierr << "CortecADC Stimulation Error: Pulse Repetition must be between 1 and 255" << endl;
        return;
      }
      //quick stimulation check
      if (stimMode == StimulationMode::STIM_MODE_PERSISTENT_CMD_PRELOADING)
        if (trigParm(4, trig) != trigParm(4, 0))
          bcierr << "CortecADC Stimulation Error: With Persistant Command Stimulation Mode, all pulse repetitions must be equal" << endl;
			//train checks
			if (trigParm->NumRows() > 5)
			{
				if ((trigParm(5, trig) == 0) ^ (trigParm(6, trig) == 0)) //xor
				{
					bcierr << "CortecADC Stimulation Error: Train frequency and duration both need to be zero, or both non-zero" << endl;
					return;
				}
				vector<int>::iterator location = find(pulseIDs.begin(), pulseIDs.end(), trigParm(1, trig));
				int pulseId = location - pulseIDs.begin();
				double totalPulseDuration = trigParm(4, trig) *																				//pulse repetitions *
					(5 * pulseParm(2, pulseId) + 2 * pulseParm(3, pulseId) + pulseParm(4, pulseId));		//5*pulse dur + 2*dead zone 0 + dead zone 1
				double trainDuration = 1e6 / trigParm(5, trig); //microseconds
				if (totalPulseDuration > trainDuration)
				{
					bcierr << "CortecADC Stimulation Error: Train frequency of " << trigParm(5, trig) << " is too fast for Pulse "
						<< pulseId << " with " << trigParm(4, trig) << " repetition(s)" << endl;
					return;
				}
        double minTrainFreq = 1e6 / (57600000.0 - totalPulseDuration);
        if (trigParm(5, trig) < minTrainFreq)
        {
          bcierr << "CortecADC Stimulation Error: Train frequency must be more than " << minTrainFreq << endl;
          return;
        }
				if (trigParm(6, trig) < 0)
				{
					bcierr << "CortecADC Stimulation Error: Number of trains must be greater or equal to 0" << endl;
					return;
				}
        if (trigParm(6, trig) > (pow(2, 16) - 1))
        {
          bcierr << "CortecADC Stimulation Error: Number of trains must be less than 65536" << endl;
          return;
        }
				if (fmod(trigParm(6, trig), 1.0) > 1e-6)
				{
					bcierr << "CortecADC Stimulation Error: Number of trains must be an integer" << endl;
					return;
				}
        //persistant command stimulation check
        if (stimMode == StimulationMode::STIM_MODE_PERSISTENT_CMD_PRELOADING)
          if ((trigParm(5, trig) != trigParm(5, 0)) || (trigParm(6, trig) != trigParm(6, 0)))
            bcierr << "CortecADC Stimulation Error: With Persistant Command Stimulation Mode, all train parameters must be equal" << endl;
			}
		}
	}
}

bool CortecADC::ConnectToImplant()
{
	// do three connection attempts.... this is not an elegant solution at all
	//	but appears to work okay. 
	std::vector<CExternalUnitInfo*> externalDevices;
	std::unique_ptr<CImplantInfo> impInfo;
	try {
		externalDevices = mImplantFactory->getExternalUnitInfos();
		if (externalDevices.size() == 0)
		{
			mImplant.reset(nullptr);
			bcidbg << "CortecADC: Could not find an external Cortec implant" << std::endl;
			return false;
		}

		if (externalDevices.size() > 1)
			bciwarn << "CortecADC: Found more than 1 connected external Cortec implants, first implant will be used: (ID):" << externalDevices[0]->getDeviceId() << std::endl;

		impInfo = std::unique_ptr<CImplantInfo>(mImplantFactory->getImplantInfo(*externalDevices.at(0)));
	} catch (const std::runtime_error& e)
	{
		bciwarn << "CortecADC Runtime Error: Unable to find implant" << endl;
		bciout << "CortecADC Exception: " << e.what() << std::endl;
		mImplant = nullptr;
		return false;
	} catch (const std::exception& e)
	{
		bciwarn << "CortecADC Error: Unable to find implant" << endl;
		bciout << "CortecADC Exception: " << e.what() << std::endl;
		mImplant = nullptr;
		return false;
	}
	for (int i = 0; i < 3; i++)
	{
		try
		{
			mImplant.reset(mImplantFactory->create(*externalDevices.at(0), *impInfo));
			// cleanup
			for (int i = 0; i < externalDevices.size(); ++i)
				delete externalDevices[i];
			bcidbg << "Connected to implant" << endl;

			mImplant->registerListener(&mListener);
			bciwarn << "CortecADC: Connected to implant." << endl;
			return true;
		} catch (const std::runtime_error& e)
		{
			mImplant = nullptr;
		} catch (const std::exception& e)
		{
			mImplant = nullptr;
		}
	}
	bcidbg << "CortecADC Error: Unable to connect to implant" << endl;
	return false;
}

void CortecADC::DisconnectFromImplant()
{
	try 
	{
		if (this->IsConnected())
		{
			// Listener should have already been unregistered by Halt()
			mImplant->setImplantPower(false);
			mImplant = nullptr;
		}
	} catch (const std::runtime_error& e)
	{
		mImplant = nullptr;
	} catch (const std::exception& e)
	{
		mImplant = nullptr;
	}
}

bool CortecADC::IsConnected()
{
	return (mImplant != nullptr);
}

void
CortecADC::OnInitialize(const SignalProperties& Output)
{
	// The user has pressed "Set Config" and all Preflight checks have passed.
	// The system will now transition into an "Initialized" state.

#ifndef NODEVICE
	if (!IsConnected())
		bcierr << "CortecADC: Implant not connected" << std::endl;
#endif

	// subtract one because set will be sent to device
	mReferenceLocations = BCI2000ListToSet(Parameter("ReferenceCh"), true);

	mStimulationEnabled = Parameter("EnableStimulation");

#ifndef NODEVICE
	auto impInfo = mImplant->getImplantInfo();
	mSamplingRate = impInfo->getSamplingRate();

	mBlockSize_ms = (((float)Parameter("SampleBlockSize") * 1000) / mSamplingRate) ;
	mNumberOfSignalChannels = Parameter("SourceCh");
	mBufferSizeMs = Parameter("SourceBufferSize").ToNumber() * 1000;

	stringstream memostream;
	memostream << "\tDevice Type: "				  << impInfo->getDeviceType() << endl;
	memostream << "\tDevice ID: "				  << impInfo->getDeviceId() << endl;
	memostream << "\tFirmware Version: "		  << impInfo->getFirmwareVersion() << endl;
	memostream << "\tChannel Count: "			  << impInfo->getChannelCount() << endl;
	memostream << "\tSampling Rate: "			  << impInfo->getSamplingRate() << " Hz" << endl;
	memostream << "\tMeasurement Channel Count: " << impInfo->getMeasurementChannelCount() << endl;
	memostream << "\tStimulation Channel Count: " << impInfo->getStimulationChannelCount() << endl;
	bciout << memostream.str() << endl;
	delete impInfo;
#else
  mSamplingRate = Parameter("SamplingRate");
  mBlockSize_ms = (((float)Parameter("SampleBlockSize") * 1000) / mSamplingRate);
  mNumberOfSignalChannels = Parameter("SourceCh");
#endif

	mDataQueue.clear();
	//initialize array in case the first sample is invalid
	if (mPrevSample == nullptr)
		mPrevSample = new double[mNumberOfSignalChannels] {};
	mListener.ResetCounter();
	int ampFactor = Parameter("AmplificationFactor");
	bool useGnd = Parameter("UseGround");

	if (Parameter("MeasureImpedance") == 1) //impedance measurement
	{
		GetImpedances();
	}
#ifndef NODEVICE
	mImplant->startMeasurement(mReferenceLocations, (RecordingAmplificationFactor)ampFactor, useGnd); // reference locations can be uploaded empty
#endif
	mIsMeasuring = true;
	mStimulationTriggered = false;

}

void
CortecADC::Initialize(const SignalProperties& Input, const SignalProperties& Output)
{
	BufferedADC::Initialize(Input, Output);
  if (mStimulationEnabled)
  {
    auto pulseParm = Parameter("StimulationPulses");
    auto trigParm = Parameter("StimulationTriggers");
    mStimMode = (StimulationMode)Parameter("StimulationMode").ToNumber();


    this->ClearPulseMap();
    this->ClearCommandList();
    // construct pulses and add to the pulse map
    for (int atom = 0; atom < pulseParm->NumColumns(); atom++)
    {
      if (!ConstructStimulationPulse(
        pulseParm(0, atom),
        pulseParm(1, atom),
        pulseParm(2, atom),
        pulseParm(3, atom),
        pulseParm(4, atom))
        )
      {
        bcierr << "CortecADC Error: Unable to construct pulse" << endl;
        mStimulationEnabled = false;
      }
    }

    // construct commands and add to list
#ifndef NODEVICE
    IStimulationCommand* cmd = mStimCommandFactory->createStimulationCommand();
#else
    IStimulationCommand* cmd = nullptr;
#endif
    for (int trig = 0; trig < trigParm->NumColumns(); trig++)
    {
      // construct commands
      double trainFreq = 0;
      int trainRep = 0;
      if (trigParm->NumRows() > 5)
      {
        trainFreq = (double)trigParm(5, trig);
        trainRep = (int)trigParm(6, trig);
      }

      //stimulation trigger parameters
      Expression exp = trigParm(0, trig);
      PulseID pID = (PulseID)trigParm(1, trig);
      set<uint32_t> sourceChs = BCI2000ListToSet(trigParm(2, trig), true);
      set <uint32_t> destChs = BCI2000ListToSet(trigParm(3, trig), false);
      int pulseReps = (int)trigParm(4, trig);

      //add functions to one command 
      if (mStimMode == StimulationMode::STIM_MODE_PERSISTENT_FUNC_PRELOADING)
      {
        if (!InitializeStimulationFunction(cmd, pID, sourceChs, destChs, pulseReps))
        {
          bcierr << "CortecADC: Unable to create Stimulation command" << endl;
          mStimulationEnabled = false;
        }
        StimCommands c;
        c.exp = (Expression)trigParm(0, trig);
        c.cmd = cmd;
        c.triggered = false;
        c.functionId = trig; //increment function ids
        mStimCommands.push_back(c);
      }
      else {
        //create separate commands to allow for trains
        if (!ConstructStimulationCommand(exp, pID, sourceChs, destChs, pulseReps,
          trainFreq,
          trainRep)
          )
        {
          bcierr << "CortecADC: Unable to create Stimulation command" << endl;
          mStimulationEnabled = false;
        }
      }
    }
#ifndef NODEVICE
    if (mStimMode == StimulationMode::STIM_MODE_PERSISTENT_FUNC_PRELOADING)
      mImplant->enqueueStimulationCommand(cmd, StimulationMode::STIM_MODE_PERSISTENT_FUNC_PRELOADING);
#endif
  }
}

ofstream
CortecADC::FindFile(string filePath, int fileNum)
{
  string name = filePath + "_impedances" + to_string(fileNum) + ".txt";
  ifstream file(name);
  if (file.good())
  {
    //file exists, try new one
    //file.close();
    return FindFile(filePath, fileNum + 1);
  }
  else return ofstream(name);
}

void
CortecADC::GetImpedances()
{
	bciwarn << "Printing impedance measurements in separate window" << endl;
	mVis.Send("Starting impedance measurements...");
	string filePath = FileUtils::StripExtension(RunManager()->CurrentRun());
	int fileNum = 0;
	ofstream file = FindFile(filePath, 0);
	//print time stamp
	time_t now = time(0);
	file << "Timestamp:\t" << ctime(&now);
	//get electrode impedance (Ohms)
	file << "Units:\tOhms";
	for (int ch = 0; ch < mNumberOfSignalChannels; ch++)
	{
    double z = 0;
#ifndef NODEVICE
    z = mImplant->getImpedance(ch);
#endif
		std::stringstream memostream;
		memostream << "Channel " << ch + 1 << ": " << z << " Ohms\r";
		mVis.Send(memostream.str());

		file << "\nCh " << ch + 1 << "\t" << z;
	}
	mVis.Send("\r");
	file.close();
}

/*
	  *  What is a pulse?
	  *                   ____
	  *  Pulse      _   _|    |_ _____
	  *              | |
	  *              |_|
	  *
	  *  Atom         1 2   3  4   5
	  *
	  * Pulse Definition: Pulses are made up of atoms.
	  *
	  * 1) Main pulse:    Holds the amplitude and duration of the main pulse in uA and us.
	  *                   The acceptable value range goes from -6120 to 0 uA. The granularity changes for smaller amplitudes as follows:
	  *                     amplitude >= -3060 uA: step size of 12
	  *                     amplitude <  -3060 uA: step size of 24
	  *                   This leads to a set of acceptable values that looks like: [-6120, -6096, ..., -3084, -3060, -3048, ..., -12, 0 ]
	  *                   Values for the pulse duration can be set in steps of 10 us. The acceptable range is between 10 and 2550 us.
	  * 2) Dead zone 0:   Holds the duration of the pause between main and counter pulse in us. Must have an amplitude of 0.
	  *                   Values can be set in steps of 10 us. The acceptable range is between 10 and 2550 us.
	  * 3) Counter pulse: Holds the amplitude and duration of the counter pulse in uA and us.
	  *                   The counter amplitude strength must be -1/4 * main_pulse_amplitude.
	  *                   The counter amplitude duration must be 4 * main_pulse_duration.
	  * 4) Dead zone 0:   Must be identical to the atom 2)
	  * 5) Dead zone 1:   Holds the duration of the pause after the pulse was delivered. Must have an amplitude of 0.
	  *                   Values can be set in steps of 80 us. The acceptable range goes from 10 to 20400 us.
	  *                   Note that the steps are starting from 0 while the minimal value is 10 us. This leads to
	  *                   a set of acceptable values that looks like: [10, 80, 160, 240, ... , 20400]
*/
bool CortecADC::ConstructStimulationPulse(int pulseID, double amplitude, double pulse_duration, double dead_zone_0, double dead_zone_1) 
{
	///
	/// amplitude      - corresponds to first pulse (large negative pulse)
	/// pulse_duration - corresponds to first pulse (large negative pulse)
	///					 The counter pulse are dictated by these two parameters
	/// 
	IStimulationFunction* function    = mStimCommandFactory->createStimulationFunction();
#ifndef NODEVICE
	IStimulationAtom* dead_zone0_atom = mStimCommandFactory->createRect4AmplitudeStimulationAtom(0, 0, 0, 0, dead_zone_0);
	try {
		
		function->append(mStimCommandFactory->createRect4AmplitudeStimulationAtom(amplitude, 0, 0, 0, pulse_duration));
		function->append(dead_zone0_atom->clone());
		function->append(mStimCommandFactory->createRect4AmplitudeStimulationAtom(-1.0 / 4.0 * amplitude, 0, 0, 0, 4.0 * pulse_duration));
		function->append(dead_zone0_atom);
		function->append(mStimCommandFactory->createRect4AmplitudeStimulationAtom(0, 0, 0, 0, dead_zone_1));
		
		// function->append(mStimCommandFactory->createStimulationPauseAtom(1000000));
	} catch (const std::runtime_error& e)
	{
		bciout << "CortecADC Exception: " << e.what() << std::endl;
		return false;
	} catch (const std::exception& e)
	{
		bciout << "CortecADC Exception: " << e.what() << std::endl;
		return false;
	}
#endif
	mStimPulses[pulseID] = function;
	return true;
}

bool CortecADC::InitializeStimulationFunction(IStimulationCommand* cmd, int pulseID, std::set<uint32_t> source_loc, std::set<uint32_t> destination_loc, int repetitions)
{
  // find pulse function according to ID
  map<PulseID, IStimulationFunction*>::iterator pulseitr = mStimPulses.find(pulseID);
  if (pulseitr == mStimPulses.end())
  {
    bcierr << "CortecADC Error: PulseID not found" << endl;
  }
  //see if ground electrode is used for stimulation
  bool useGnd = false;
  set<uint32_t> modDestinationLoc;
  for (auto it = destination_loc.begin(); it != destination_loc.end(); it++)
  {
    if (*it != 0)
      modDestinationLoc.insert(*it - 1);//shift every element by 1 to get [0, ..., channel count - 1]
    else
      useGnd = true;
  }
#ifndef NODEVICE
  try
  {
    // set repetitions
    pulseitr->second->setRepetitions(repetitions, 1);
    // set source and desitination electrodes
    pulseitr->second->setVirtualStimulationElectrodes(source_loc, modDestinationLoc, useGnd);
    cmd->append(pulseitr->second);
  }
  catch (const std::runtime_error& e)
  {
    bciout << "CortecADC Runtime Exception: " << e.what() << std::endl;
    return false;
  }
  catch (const std::exception& e)
  {
    bciout << "CortecADC Exception: " << e.what() << std::endl;
    return false;
  }
#endif
  return true;
}

// creates a command that can be uploaded to the device
bool CortecADC::ConstructStimulationCommand(Expression trigger, int pulseID, std::set<uint32_t> source_loc, std::set<uint32_t> destination_loc, int repetitions, double train_freq, int train_reps)
{
	IStimulationCommand* cmd = mStimCommandFactory->createStimulationCommand();

  if (!InitializeStimulationFunction(cmd, pulseID, source_loc, destination_loc, repetitions))
    return false;

#ifndef NODEVICE
	try
	{
		//create train
		if (train_freq != 0)
		{
			int duration = round(1e6 / train_freq); //microseconds
			uint64_t pulseDur = cmd->getDuration(); //cmd duration = pulse duration at this point
			IStimulationFunction* pauseFunction = mStimCommandFactory->createPauseStimulationFunction(duration - pulseDur);
			cmd->append(pauseFunction);
			cmd->setRepetitions(train_reps);
		}
	} catch (const std::runtime_error& e)
	{
		bciout << "CortecADC Runtime Exception: " << e.what() << std::endl;
		return false;
	} catch (const std::exception& e)
	{
		bciout << "CortecADC Exception: " << e.what() << std::endl;
		return false;
	}
	std::string message;
	if (!mImplant->isStimulationCommandValid(cmd, &message))
	{
		bcidbg << "CortecADC: Stimulation command has warnings. Implant reported:\n"
				<< message  << "\n\nThese warnings may or may not be autocorrected by Cortec implant." << endl;
	}
	mStimId++;
	if (mStimMode == StimulationMode::STIM_MODE_PERSISTENT_CMD_PRELOADING)
		mImplant->enqueueStimulationCommand(cmd, StimulationMode::STIM_MODE_PERSISTENT_CMD_PRELOADING);

#endif
	mStimCommands.push_back({ trigger, cmd, false });
	return true;
}

std::set<uint32_t> 
CortecADC::BCI2000ListToSet(ParamRef ch_parm, bool subtract_one) const
{
	std::set<uint32_t> ch_set = {};
	for (int ch = 0; ch < ch_parm->NumValues(); ch++)
	{
		if(subtract_one)
			ch_set.insert( (uint32_t)ch_parm(ch) - 1);
		else
			ch_set.insert((uint32_t)ch_parm(ch));
	}
	return ch_set;
}

void CortecADC::ClearPulseMap() 
{
	map<PulseID, IStimulationFunction*>::iterator pulseitr = mStimPulses.begin();
	try {
		while (pulseitr != mStimPulses.end())
		{
			IStimulationFunction* temp = pulseitr->second;
			// TODO: this throws an exception ... I dont know why. The documentation says 
			//	I am responsible for its deletion.... for now: MEMORY LEAK
			//delete temp;
			pulseitr->second = nullptr;
			pulseitr++;
		}
		mStimPulses.clear();
	} catch (const std::runtime_error& e)
	{
		bciout << "CortecADC Runtime Exception: " << e.what() << std::endl;
	} catch (const std::exception& e)
	{
		bciout << "CortecADC Exception: " << e.what() << std::endl;
	}
}

void CortecADC::ClearCommandList()
{
	try {
		list<StimCommands>::iterator cmditr = mStimCommands.begin();
		while (cmditr != mStimCommands.end())
		{
			IStimulationCommand* temp = cmditr->cmd;
			// TODO: this throws an exception ... I dont know why. The documentation says 
			//	I am responsible for its deletion.... for now: MEMORY LEAK
			//delete temp;
			cmditr->cmd = nullptr;
			cmditr++;
		}
		mStimCommands.clear();
    mStimId = 0;
	} catch (const std::runtime_error& e)
	{
		bciout << "CortecADC Runtime Exception: " << e.what() << std::endl;
	} catch (const std::exception& e)
	{
		bciout << "CortecADC Exception: " << e.what() << std::endl;
	}
}


void CortecADC::StartStimulation(IStimulationCommand* cmd, uint8_t functionID)
{
	//vector<IStimulationCommand::StimulationCommandFault> stimInfo;
	try
	{
    if (mImplantStimulation)
      mImplant->stopStimulation(); //safety, lets stop old stimulation before we start the new one
		//load stimulation
    switch (mStimMode) {
      case StimulationMode::STIM_MODE_VOLATILE_CMD_PRELOADING:
        mImplant->enqueueStimulationCommand(cmd, StimulationMode::STIM_MODE_VOLATILE_CMD_PRELOADING);
        mImplant->startStimulation();
        break;
      case StimulationMode::STIM_MODE_PERSISTENT_CMD_PRELOADING:
        mImplant->startStimulation();
        break;
      case StimulationMode::STIM_MODE_PERSISTENT_FUNC_PRELOADING:
        mImplant->startStimulation(functionID);
        break;
    }
		mStimulationTriggered = true;
	} catch (const std::runtime_error& e)
	{
		bcierr << "CortecADC Runtime Error: Unable to start stimulation" << endl;
		bciout << "CortecADC Exception: " << e.what() << std::endl;
		mStimulationTriggered = false;
		return;
	} catch (const std::exception& e)
	{
		bciwarn << "CortecADC Error: Unable to start stimulation" << endl;
		bciout << "CortecADC Exception: " << e.what() << std::endl;
		mStimulationTriggered = false;
		return;
	}
	/*for (int i = 0; i < stimInfo.size(); i++)
	{
		if (stimInfo[i].criticality == IStimulationCommand::FaultType::FT_ERROR)
		{
			bciwarn << "CortecADC Error: Implant reported error when trying to stimulate. Stimulation disabled." << endl;
			mStimulationEnabled   = false;
			mStimulationTriggered = false;
		}
	}*/
}

void
CortecADC::OnStartAcquisition()
{

}

void
CortecADC::OnStopAcquisition()
{
	try
	{
		if (this->IsConnected())
		{
			if (mIsMeasuring)
			{
				mImplant->stopMeasurement();
				mIsMeasuring = false;
			}
		}
	}
	catch (const std::runtime_error& e)
	{
		mIsMeasuring = false;
	}
	catch (const std::exception& e)
	{
		mIsMeasuring = false;
	}
}

void
CortecADC::DoAcquire(GenericSignal& Output)
{
	// ====================== Disconnected state ======================
	if (mTimeOutOccurred)
	{
    //try to reconnect
    if (this->ConnectToImplant())
    {
      //we are reconnected
      mTimeOutOccurred = false;
    }
    else
    {
      for (int el = 0; el < Output.Elements(); el++)
      {
        for (int ch = 0; ch < Output.Channels(); ch++)
        {
          Output(ch, el) = 0;
        }
      }
      Sleep(mBlockSize_ms - 1);
      return;
    }
	}


	// ====================== Connected state ======================
	DWORD res = WaitForSingleObject(mDataLock, mBufferSizeMs);
	if (res == WAIT_OBJECT_0)
	{
#ifdef OUTPUT_BUFFER_TIMES
			chrono::time_point<chrono::system_clock> start_pop_time = chrono::system_clock::now();
#endif

			WithThreadPriority(ThreadUtils::Priority::Maximum - 1)
			{
				for (int el = 0; el < Output.Elements(); el++)
				{
					double* ch_array = NULL;
					while (!mDataQueue.try_pop(ch_array));

					if (ch_array)
					{
						for (int ch = 0; ch < Output.Channels(); ch++)
						{
							Output(ch, el) = ch_array[ch];
						}
						//ImplantLostSample stream
						Output(Output.Channels() - 1, el) = 0;
						delete[] mPrevSample;
						mPrevSample = ch_array;
					} 
					//fill with previous sample
					else
					{
						// lost sample!
						for (int ch = 0; ch < Output.Channels(); ch++)
						{
							Output(ch, el) = mPrevSample[ch];
						}
						//ImplantLostSample stream
						Output(Output.Channels() - 1, el) = 1;
					}
				}
			}
#ifdef OUTPUT_BUFFER_TIMES
			int pop_ms = chrono::duration_cast<std::chrono::microseconds>(chrono::system_clock::now() - start_pop_time).count();
			int aq_ms = chrono::duration_cast<std::chrono::microseconds>(chrono::system_clock::now() - start_aq_time).count();
			// this could be called every 100ms, print every second
			if (mprintcounterpop % 10 == 0)
			{
				bciout << "\tPop time: " << pop_ms << "us" << endl; 
				bciout << "\tAqu time: " << aq_ms << "us" << endl; 
			}
			mprintcounterpop += 1;
#endif
			
			return;
		}
	if (mImplantStimulation)
	{
		bciwarn << "CortecADC Error: Timeout occurred during stimulation. "
				<< "\nTimeout could be due to poor connection, or high impedance across electrodes "
				<< "causing applied voltage to exceed maximum. "
				<< "\nDisconnecting from implant. To attempt a reconnect: end the run and click \"Set Config\"." << endl;
	} else {
		bciwarn << "CortecADC Error: Timeout occurred during collection of data. " 
				<< "\nDisconnecting from implant. You will probably need to power cycle "
				<< "the implant to reconnect. To attempt a reconnect: end the run and click \"Set Config\"." << endl;
	}
	mTimeOutOccurred      = true;
	mImplantStimulation   = false;
  mImplantStimulationBursts = false;
	mStimulationTriggered = false;
#ifndef NODEVICE
	this->OnStopAcquisition();
	this->DisconnectFromImplant();
#endif
}

void
CortecADC::Process(const GenericSignal& Input, GenericSignal& Output)
{
  // ====================== check stimulation ======================
  if (mStimulationEnabled)
  {
    // check if were currently stimulating
    if (!mStimulationTriggered)
    {
      if (State("Running"))
      {
        // were not stimulating
        // check if any of the triggers evaluate true
        for (auto itr = mStimCommands.begin(); itr != mStimCommands.end(); itr++)
        {
          if (itr->exp.Evaluate(&Input))
          {
            if (!itr->triggered)
            {
#ifndef NODEVICE  
              // send this command to the device
              this->StartStimulation(itr->cmd, itr->functionId);
#endif
              State("RequestedStimulation") = true;
              itr->triggered = true;
	          }
          }
          else {
            itr->triggered = false;
          }
        }
      }
    }
  }
	BufferedADC::Process(Input, Output);

	State("ImplantVoltage") = mImplantVoltage;
	State("ImplantHumidity") = mImplantHumidity;
	State("ImplantControlValue") = mImplantControlValue;
	State("ImplantPrimaryCoilCurrent") = mImplantPrimaryCoilCurrent;
	State("ImplantTemperature") = mImplantTemperature;
	State("RequestedStimulation") = mStimulationTriggered;
}
void
CortecADC::StartRun()
{
	if (!mStimulationEnabled) return;
	mRecording = true;
  for (auto itr = mStimCommands.begin(); itr != mStimCommands.end(); itr++)
    itr->triggered = false;
}

void
CortecADC::StopRun()
{
	if (!mStimulationEnabled) return;
	mRecording = false;
	if (!this->IsConnected()) return;
	try
	{
		if (mImplantStimulation)
			mImplant->stopStimulation(); 
	} catch (const std::runtime_error& e)
	{
		bcierr << "CortecADC Runtime Error: Unable to stop stimulation" << endl;
		bciout << "CortecADC Exception: " << e.what() << std::endl;
	} catch (const std::exception& e)
	{
		bcierr << "CortecADC Error: Unable to stop stimulation" << endl;
		bciout << "CortecADC Exception: " << e.what() << std::endl;
	} catch (...)
	{
		bcierr << "CortecADC Error: Unknown exception throwm in StopRun()" << endl;
	}
}


void CortecADC::CortecDataListener::onMeasurementStateChanged(const bool isMeasuring)
{
}

void CortecADC::CortecDataListener::onConnectionStateChanged(const connection_info_t & info)
{
	if (info.count(ConnectionType::PC_TO_EXT) > 0)
	{
		const bool isConnected = info.at(ConnectionType::PC_TO_EXT) == ConnectionState::CONNECTED;
		bcidbg << "*** Connection state from PC to external unit changed: "
			   << (isConnected ? "connected" : "disconnected") << std::endl;
	}
	if (info.count(ConnectionType::EXT_TO_IMPLANT) > 0)
	{
		const bool isConnected = info.at(ConnectionType::EXT_TO_IMPLANT) == ConnectionState::CONNECTED;
		bcidbg << "*** Connection state from external unit to implant changed: "
			   << (isConnected ? "connected" : "disconnected") << std::endl;
	}
}

void CortecADC::CortecDataListener::onStimulationStateChanged(const bool isStimulating)
{
	// reset mStimulationTriggered on falling edge of stimulation
	if (!isStimulating && parent->mImplantStimulation)
	{
		parent->mStimulationTriggered = isStimulating;
    bcievent << "ImplantStimulationBursts " << isStimulating << endl;
	}
	if (parent->mRecording)
		bcievent << "ImplantStimulation " << isStimulating << endl;
	parent->mImplantStimulation = isStimulating;
}

void CortecADC::CortecDataListener::onImplantVoltageChanged(const double voltage)
{
	parent->mImplantVoltage =  voltage * IMPLANT_VOLTAGE_MULTIPLIER;
}

void CortecADC::CortecDataListener::onTemperatureChanged(const double temperature)
{
	parent->mImplantTemperature = temperature * IMPLANT_TEMPERATURE_MULTIPLIER;
}

void CortecADC::CortecDataListener::onHumidityChanged(const double humidity)
{
	parent->mImplantHumidity = humidity * IMPLANT_HUMIDITY_MULTIPLIER;
}

void CortecADC::CortecDataListener::onError(const std::exception & err)
{
	bcierr << "CortecADC: Error received from device - " << err.what() << endl;
}

void CortecADC::CortecDataListener::onPrimaryCoilCurrentChanged(const double currentMilliA)
{
	parent->mImplantPrimaryCoilCurrent = currentMilliA * IMPLANT_PRIMARYCURRENT_MULTIPLIER;
}

void CortecADC::CortecDataListener::onImplantControlValueChanged(const double controlValue)
{
	parent->mImplantControlValue = controlValue * IMPLANT_CONTROLVALUE_MULTIPLIER;
}

void CortecADC::CortecDataListener::onDataProcessingTooSlow()
{

}

void CortecADC::CortecDataListener::onStimulationFunctionFinished(const uint64_t numFinishedFunctions)
{
  //number of finished functions or pauses of current function, called during active stim
  if (parent->mRecording)
    bcievent << "ImplantStimulationBursts " << numFinishedFunctions << endl;
}

void CortecADC::CortecDataListener::onRfQualityUpdate(const int8_t antennaQualitydBm,
  const uint16_t validFramesReceived, const uint16_t invalidHandshake,
  const uint16_t radioCrcErrors, const uint16_t otherRxErrors,
  const uint32_t rxQueueOverflows, const uint32_t txQueueOverflows) 
{
}

void CortecADC::CortecDataListener::onChannelUpdate(const uint8_t rfChannel)
{
}