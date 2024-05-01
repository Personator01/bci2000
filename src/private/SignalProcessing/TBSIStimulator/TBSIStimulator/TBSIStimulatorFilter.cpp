////////////////////////////////////////////////////////////////////////////////
// Authors:
// Description: TBSIStimulatorFilter implementation
////////////////////////////////////////////////////////////////////////////////

#include "TBSIStimulatorFilter.h"
#include "BCIStream.h"

using namespace std;

RegisterFilter( TBSIStimulatorFilter, 2.A );
     // Change the location as appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations witin SignalProcessing modules begin with "2."
     //       (NB: Filter() commands in a separate PipeDefinition.cpp file may override the default location set here with RegisterFilter)
     //  - filter locations within Application modules begin with "3."

// C++ does not initialize simple types such as numbers, or pointers, by default.
// Rather, they will have random values.
// Take care to initialize any member variables here, so they have predictable values
// when used for the first time.
TBSIStimulatorFilter::TBSIStimulatorFilter()
: mCurrentBlock(0), mDeviceOpened(false), apiResult(NO_ERROR_API), mhDevice(NULL), mEnableHWTrigger(false), stimAwake(false)
{
}

TBSIStimulatorFilter::~TBSIStimulatorFilter()
{
  CloseDevice(mhDevice);
  if (mDeviceOpened) 
	  CloseDevice(mhDevice);

}

void
TBSIStimulatorFilter::Publish()
{
  // Define any parameters that the filter needs....

 BEGIN_PARAMETER_DEFINITIONS

	 "Filtering:TBSI%20Stimulator string StimPattern= 555500000100000000000100000000000000320000006200FA0090010300EE02000000000000A4085C076117010001000100000000080000000000000100000000000000000064000000000064000000000032000000E7032003FFFF00000000000000000000A4085C070A00010001000000000000080000000000000100AAAA "
	 "555500000100000000000100000000000000320000006200FA0090010300EE02000000000000A4085C076117010001000100000000080000000000000100000000000000000064000000000064000000000032000000E7032003FFFF00000000000000000000A4085C070A00010001000000000000080000000000000100AAAA "
	 "% % // Encoded pattern obtained from StimWare",	// more detail about the default pattern is available on www.bci2000.org/mediawiki/index.php/Contributions:TBSIStimulator
	 "Filtering:TBSI%20Stimulator int PI1= 500 500 1 1000 // PI1 value in uA",
	 "Filtering:TBSI%20Stimulator int PD1= 2000 2000 50 65535 // PD1 value in us",
	 "Filtering:TBSI%20Stimulator int EditStimPattern= 0 1 0 1 // edit StimPattern based on PI1 and PD1 (boolean)",
	 "Filtering:TBSI%20Stimulator string StimInterval= 3s 3s % % // Stimulation interval",
	 "Filtering:TBSI%20Stimulator int ExternalHWTrigger= 0 0 0 1 // Enable external hardware triggers (boolean)",
	 "Filtering:TBSI%20Stimulator string StimAddress= 0024 0024 % % // Stimulator Address",
	 "Filtering:TBSI%20Stimulator int VerboseOutput= 1 1 0 1 // Enable debug output (boolean)",

 END_PARAMETER_DEFINITIONS


 BEGIN_STATE_DEFINITIONS

	 "StimState 1 0 0 0",    // binary state that is set to 1 when a stimulus is administered, otherwise it takes the value of 0

 END_STATE_DEFINITIONS

}

void
TBSIStimulatorFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
	// The user has pressed "Set Config" and we need to sanity-check everything.
   // For example, check that all necessary parameters and states are accessible:
	Parameter("StimPattern");
	Parameter("PI1");
	Parameter("PD1");
	Parameter("EditStimPattern");
	Parameter("ExternalHWTrigger");
	Parameter("StimInterval");
	Parameter("StimAddress");
	Parameter("VerboseOutput");
	State("StimState");

	Output = Input;

  // ... or alternatively, we could modify that info here:

  // Let's imagine this filter has only one output, namely the amount of stuff detected in the signal:
  // Output.SetChannels( 1 );
  // Output.ChannelLabels()[0] = "Stuff";

  // Imagine we want to output twice as many samples (or bins) as we receive from the input:
  // Output.SetElements( Input.Elements() * 2 );

  // Note that the TBSIStimulatorFilter instance itself, and its members, are read-only during
  // this phase, due to the "const" at the end of the Preflight prototype above.
  // Any methods called by Preflight must also be "const" in the same way.
}


void
TBSIStimulatorFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
	// The user has pressed "Set Config" and all Preflight checks have passed.

	if (Parameter("EditStimPattern"))
	{
		mPulse = Pulse(Parameter("StimPattern"), Parameter("PI1"), Parameter("PD1"));
	}
	else
	{
		mPulse = Pulse(Parameter("StimPattern"));
	}
	mStimInterval = Parameter("StimInterval").InSampleBlocks();
	mEnableHWTrigger = (bool)Parameter("ExternalHWTrigger");
	mStimAddress = (std::string)Parameter("StimAddress");
	//Opens the TBSI device 
	TBSIStimulatorFilter::OpenDevice();
	//Connect with stimulator
	SetupStimulator(mStimAddress);
	//Download pattern
	
	if (apiResult == NO_ERROR_API)
	{
		//Wake up stimulator
		apiResult = SetStimulatorState(mhDevice, awake);
		std::cout << "Wake Stimulator result: " << apiResult << std::endl;
		if (apiResult != NO_ERROR_API)
		{
			bcierr << "can't wake up stimulator" << std::endl;
			return;
		}
		stimAwake = true;
		Pattern p;
		strncpy(p, String(mPulse.Pattern()),PATTERN_LENGTH);
		bciout << p << std::endl;
		apiResult = DownloadPattern(mhDevice, p);
		if (apiResult != NO_ERROR_API)
		{
			bcierr << "Pattern downloading unsuccessful" << std::endl;
			CloseDevice(mhDevice);
			return;
		}
		bciout << "Pattern download successful" << std::endl;
	}
}

void
TBSIStimulatorFilter::StartRun()
{
  // The user has just pressed "Start" (or "Resume")
  bciout << "Start Triggering" << std::endl;
}


void
TBSIStimulatorFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
	if (mDeviceOpened) // be sure to wait until the device is opened
	{
		mCurrentBlock += 1;
		// Send stim command every user supplied interval
		if (mCurrentBlock == mStimInterval)
		{
			mCurrentBlock = 0;
			State("StimState") = 1;
			apiResult = SetTriggerState(mhDevice, true);
		}
		else
		{
			State("StimState") = 0;
		}
	}
	else
	{
		State("StimState") = 0;
	}
	
  // And now we're processing a single SampleBlock of data.
  // Remember not to take too much CPU time here, or you will break the real-time constraint.

	Output = Input; // Pass the signal through unmodified.
                  // ( Obviously this will no longer fly if we modified the shape of the
                  //   output SignalProperties during Preflight ).

  // Or we could do it one value at a time:
  /*
  for( ch = 0; ch < Output.Channels(); ch++ )
  {
    for( el = 0; el < Output.Elements(); el++ )
    {
      Output( ch, el ) = some_function( Input );
    }
  }
  */
}

void
TBSIStimulatorFilter::StopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // or because the run has reached its natural end.
	apiResult = SetTriggerState(mhDevice, false);
	
}

void 
TBSIStimulatorFilter::OpenDevice()
{
	if (apiResult != NO_ERROR_API)
	{
		bcierr << "can't start connection.\n Close current connection" << std::endl;
		return;
	}
	apiResult = SetupDongle(&mhDevice, mDeviceOpened);
	bciout << "Device Open " << mDeviceOpened << std::endl;
	bciout << "Result " << apiResult << std::endl;
	if (apiResult != NO_ERROR_API)
	{
		bcierr << "No device found";
		return;
	}
	bool deviceStatus = false;
	int sleepTime = 5;
	std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
	apiResult = GetDongleStatus(mhDevice, deviceStatus);
	bciout << "Dongle status : " << deviceStatus << std::endl;
	bciout << "Dongle Status result: " << apiResult << std::endl;
	if (apiResult != NO_ERROR_API)
	{
		CloseDevice(mhDevice);
		return;
	}
		
}

void TBSIStimulatorFilter::CloseDevice(DongleHandle devhandle)
{
	if (stimAwake) {
		apiResult = SetTriggerState(mhDevice, sleep);
		stimAwake = false;
	}
	//Closing Dongle
	TBSI_API_RESULT result = CloseDongleConnection(devhandle);
	bciout << "Close dongle result: " << result << std::endl;
	mDeviceOpened = false;
}

void TBSIStimulatorFilter::SetupStimulator(String address)
{
	if (mDeviceOpened)
	{
		int sleepTime = 5;
		bciout << "Finding the following stimulators: " << address << std::endl;
		uint32_t found_stims;
		StimulatorAdd stimAdd[1], found_stimulators[1];
		strncpy_s(stimAdd[0], address , STIMULATOR_LENGTH-1);
		apiResult = FindStimulators(mhDevice, stimAdd, 1, found_stimulators, found_stims);
		bciout << "Find Stimulator result: " << apiResult << std::endl;
		bciout << "Stimulators found: " << found_stims << std::endl;
		for (uint32_t i = 0; i < found_stims; i++)
			bciout << found_stimulators[i] << std::endl;

		if (apiResult != NO_ERROR_API)
		{
			bcierr << "Can't connect to stimulator!" << std::endl;
			CloseDevice(mhDevice);
			return;
		}
		std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
		bciout << stimAdd[0] << std::endl;
		apiResult = ConnectStimulator(mhDevice,stimAdd[0]);
		if (apiResult != NO_ERROR_API)
		{
			bcierr << "Can't connect to stimulator!" << std::endl;
			CloseDevice(mhDevice);
			return;
		}
		bciout << "Connect Stimulator result: " << apiResult << std::endl;
	}
}
