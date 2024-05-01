////////////////////////////////////////////////////////////////////////////////
// Authors: Akshay Vyas, National Center for Adaptive Neurotechnologies
// Description: CorTecFilter implementation
////////////////////////////////////////////////////////////////////////////////

#include "CorTecFilter.h"
#include "BCIStream.h"
#include "BCIEvent.h"
#include "Environment.h"

#define gElectrode 1

using namespace std;

RegisterFilter( CorTecFilter, 2.A );
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
CorTecFilter::CorTecFilter()
	: mpExampleArray(nullptr),
	mImplantFactory(createImplantFactory(0,"../")),
mImplant(nullptr), mImplantInfo(nullptr),
mStimCommandFactory(createStimulationCommandFactory()),
mStimulating(false)
{

}

CorTecFilter::~CorTecFilter()
{
  Halt();
  // If you have allocated any memory with malloc(), call free() here.
  // If you have allocated any memory with new[], call delete[] here.
  // If you have created any object using new, call delete here.
  // Either kind of deallocation will silently ignore null pointers, so all
  // you need to do is to initialize those to zero in the constructor, and
  // deallocate them here.
  delete[] mpExampleArray;
}

void
CorTecFilter::Publish()
{
  // Define any parameters that the filter needs....

 BEGIN_PARAMETER_DEFINITIONS


	 //"Source:Signal%20Properties int SourceCh= auto "
	 //"auto 1 % // number of digitized and stored channels",

	 //"Source:Signal%20Properties int SampleBlockSize= auto "
	 //"auto 1 % // number of samples transmitted at a time",

	 //"Source:Signal%20Properties float SamplingRate= auto "
	 //"auto 0.0 % // sample rate, defined by connected device (noedit)",

	 //"Source:Signal%20Properties list SourceChGain= 1 auto "
	 //" // physical units per raw A/D unit",

	 //"Source:Signal%20Properties list SourceChOffset= 1 auto "
	 //" // raw A/D offset to subtract, typically 0",

	 //"Source:Signal%20Properties list ChannelNames= 1 auto "
	 //" // names of amplifier channels",

	 //Stimulation Parameters
	 "Stimulation:Stimulation int Enable= 1 1 0 1 "
	 " //Enable/Disable Stimulation (boolean)",

	 "Stimulation:Stimulation matrix StimulationParameters= 1 1 auto "
	 "// Define Stimulation parameters as followed: StimulatonId, StimulationType (voltage/current),DurationDuration1, amplitude1,DurationDuration2, amplitude2 ... ",

	 "Stimulation:Stimulation matrix StimulationTriggers= 1 1 auto "
	 "// Define a trigger for stimulation, convention is: StimulatonId, trigger equation,repeats, anode electrodes, kathode electrodes",

	 "Stimulation:Stimulation string StimulationExpression= % % % % "
	 " // Expression to start CorTec stimulation",

	 "Stimulation:Stimulation string AbortExpression= % % % % "
	 " // Expression to abort CorTec stimulation",
	//Filtering parameters
	 "Filtering:CorTecFilter int EnableCorTecFilter= 1 1 0 1 // enable CorTecFilter? (boolean)",                       
  
	 /*"Filtering:CorTecFilter matrix FilterStimulationParameters=  1 1 auto" 
	 "// CorTecFilter Stimulation parameters: Amplitude, Amplitude,(3 more), PulseWidth", */  
	 
	 "Filtering:CorTecFilter int CheckState= 1 1 auto // connect stimulation params", 
	  

 END_PARAMETER_DEFINITIONS


 

 BEGIN_STATE_DEFINITIONS

   "CorTecState       1 0 0 0",

   "CorTecAnode       32 0 1 0",

	//"CorTecAnode       0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1",

   "CorTecCathode     32 0 9 0",

   "CorTecAmplitude   32 0 17 0",

   "CorTecPulseWidth  32 0 21 0",

   "CorTecFunctionPeriod  32 0 29 0",

 END_STATE_DEFINITIONS
 
}

void
CorTecFilter::AutoConfig()
{
	auto implantInfo = mImplant->getImplantInfo();
	Parameter("SamplingRate") = implantInfo->getSamplingRate();
	int channels = implantInfo->getChannelCount(); //number of channels?
	delete implantInfo;
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
		//const char* name = 0;
		//Parameter("ChannelNames")(i) << "CH" << i; // Omit "<< i" if channel names are unique.
		//Parameter("FilterChannelNames")(i) << "CH" << i; 


		//double gainFactor = 1.0;
		//Parameter("SourceChGain")(i) << gainFactor << "muV"; // Always provide correct physical unit for documentation and display purposes.
		//													 // For most amplifiers, offset removal is done within the amp hardware or driver. Otherwise, you may set this to a nonzero value.
		//Parameter("SourceChOffset")(i) = 0;

		Parameter("StimulationParameters")->SetNumColumns(4);
		Parameter("StimulationParameters")->SetNumRows(1);
		
		Parameter("StimulationTriggers")->SetNumColumns(5);
		Parameter("StimulationTriggers")->SetNumRows(1);
		
	}

	int samplesPerPacket = 1;
	Parameter("SampleBlockSize") = samplesPerPacket;
		
	if (Parameter("EnableCorTecFilter") != "auto")
		Parameter("EnableCorTecFilter") = 1;

	if (Parameter("EnableCortecFilter") != 1)
	{
		bcierr << "CorTec Filter Disabled" <<endl;
	}

	mActivateExp = (Expression) Parameter("StimulationExpression");
	mAbortExp = (Expression) Parameter("AbortExpression");
}

bool CorTecFilter::ConnectToImplant()
{
	std::vector<CExternalUnitInfo*> externalDevices = mImplantFactory->getExternalUnitInfos();
	if (externalDevices.size() == 0)
	{
		mImplant.reset();
		bcierr << "Could not find an external Cortec Device!" << std::endl;
		return false;
	}

	if (externalDevices.size() > 1)
		bciwarn << "Found more than 1 connected external Cortec Device!, first device will be used: (ID):" << externalDevices[0]->getDeviceId() << std::endl;


	std::unique_ptr<CImplantInfo> impInfo(mImplantFactory->getImplantInfo(*externalDevices[0]));
	mImplant.reset(mImplantFactory->create(*externalDevices[0], *impInfo));
	//cleanup
	for (int i = 0; i < externalDevices.size(); ++i)
		delete externalDevices[i];

	return true;
}

bool CorTecFilter::IsConnected()
{
	return (mImplant != nullptr);
}



IStimulationFunction* CorTecFilter::GetStimDefinitonFromId(int identifier)
{
	return mStimParameters.find(identifier)->second;
}

bool CorTecFilter::checkStimulationParameters(const double amplitude, const uint64_t pulseWidth, const int type)
{

	if (pulseWidth < 10)
	{
		bcierr << "Pulsewidth duration minimum is 10ms" << std::endl;
		return false;
	}
	if (pulseWidth > 2550)
	{
		bcierr << "Pulsewidth cannot be longer than 2550ms" << std::endl;
		return false;
	}

	return true;
}

void CorTecFilter::ClearStimMap()
{

#pragma message("Maybe cause memory leaks because we need to clear content of IStimulationCommand")

	for (auto it = mStimExecutionFunction.begin(); it != mStimExecutionFunction.end(); it++)
	{
		delete it->second;
	}
	mStimExecutionFunction.clear();
	for (auto it = mStimParameters.begin(); it != mStimParameters.end(); it++)
	{

		delete it->second; //free memory of stim parameter
	}
	mStimParameters.clear();

}

void CorTecFilter::AddStimDefinitionToMap(int identifier, std::vector<StimParameters> params, int type)
{
	if (type > 1)
		bcierr << "only 1 or 0 are allowed stimulation types!" << std::endl;

	IStimulationCommand* stimCmd = mStimCommandFactory->createStimulationCommand();


	IStimulationFunction* stimFunction(createStimulationFunction(100 // microampere
		, 1100 // micros
		, 12500 // micros
	));

	mStimParameters.insert(std::pair<int, IStimulationFunction*>(identifier, stimFunction));
}


void
CorTecFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  // The user has pressed "Set Config" and we need to sanity-check everything.
  // For example, check that all necessary parameters and states are accessible:
  //
  // Parameter( "Milk" );
  // State( "Bananas" );
  //
  // Also check that the values of any parameters are sane:
  //
  // if( (float)Parameter( "Denominator" ) == 0.0f )
  //      bcierr << "Denominator cannot be zero" << endl;
  //
  // Errors issued in this way, during Preflight, still allow the user to open
  // the Config dialog box, fix bad parameters and re-try.  By contrast, errors
  // and C++ exceptions at any other stage (outside Preflight) will make the
  // system stop, such that BCI2000 will need to be relaunched entirely.

  Output = Input; // this simply passes information through about SampleBlock dimensions, etc....

  // ... or alternatively, we could modify that info here:

  // Let's imagine this filter has only one output, namely the amount of stuff detected in the signal:
  // Output.SetChannels( 1 );
  // Output.ChannelLabels()[0] = "Stuff";

  // Imagine we want to output twice as many samples (or bins) as we receive from the input:
  // Output.SetElements( Input.Elements() * 2 );
       
  // Note that the CorTecFilter instance itself, and its members, are read-only during
  // this phase, due to the "const" at the end of the Preflight prototype above.
  // Any methods called by Preflight must also be "const" in the same way.

  SignalType sigType = SignalType::float32;

  int samplesPerBlock = Parameter("SampleBlockSize");
  int numberOfSignalChannels = Parameter("SourceCh");
  Output = SignalProperties(numberOfSignalChannels, samplesPerBlock, sigType);
  // Note that the CortecADC instance itself, and its members, are read-only during the
  // Preflight phase---note the "const" at the end of the OnPreflight prototype above.
  // Any methods called by OnPreflight must also be declared as "const" in the same way.

  auto stimParams = Parameter("StimulationParameters");
  auto stimtriggers = Parameter("StimulationTriggers");
  if ((stimParams->NumRows() != 0) && (stimParams->NumColumns() < 4))
	  bcierr << "Stim Parameters must have at least 4 Columns: \n Identifier, type of stimulation, Stimulus Duration1, amplitude1" << std::endl;

  if ((stimParams->NumColumns() % 2) != 0)
	  bcierr << "The Parameter: StimulationParameters must be an even number, check for incomplete duration/amplitude pairs" << std::endl;
  if (stimtriggers->NumColumns() != 5)
	  bcierr << "The Paramter: StimulationTriggers must have 5 Columns: \n Identifier, stimulus equation, repeats, anode electrode, Cathode electrode" << std::endl;


  std::set<int> identifierList;
  //check if each stimulus is unique
  for (int i = 0; i < stimParams->NumRows(); i++)
  {
	  if (!identifierList.insert(stimParams(i, 0)).second)
		  bcierr << "Identifier values must be unique!" << std::endl;
  }

  //check if each trigger is matched to an existing stimulus
  for (int i = 0; i < stimtriggers->NumRows(); i++)
  {
	  if (identifierList.find(stimtriggers(i, 0)) == identifierList.end())
		  bcierr << "Identifier must be defined!" << std::endl;
  }

  State("CorTecState");
  State("CorTecAnode");
  State("CorTecCathode");
  State("CorTecAmplitude");
  State("CorTecPulseWidth");
  State("CorTecFunctionPeriod");
}


void
CorTecFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{

	mCorTecAnode = State("CorTecAnode");
	mCorTecCathode = State("CorTecCathode");
	mCorTecAmplitude = State("CorTecAmplitude");
	mCorTecPulseWidth = State("CorTecPulseWidth");
	mCorTecFunctionPeriod = State("CorTecFunctionPeriod");

  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The signal properties can no longer be modified, but the const limitation has gone, so
  // the CorTecFilter instance itself can be modified. Allocate any memory you need, start any
  // threads, store any information you need in private member variables.

  // For memory allocation, a reasonable approach is to first deallocate and set the
  // pointer to zero:
  delete[] mpExampleArray;
  mpExampleArray = nullptr;
  // Then, depending on parameter configuration, you may choose to allocate memory,
  // or leave the pointer at zero.

  mStimulating = (bool) Parameter("CheckState");

  if (!IsConnected())
  {
	  bcierr << "No device connected!" << endl;
	  return;
  }
  else
	  bciout << "Device Connected" << endl;

  mNumberOfSignalChannels = Parameter("SourceCh");
  auto stimParams = Parameter("StimulationParameters");
  auto stimtriggers = Parameter("StimulationTriggers");
  ClearStimMap();
  std::vector<StimParameters> stimvec;

  for (int i = 0; i < stimParams->NumRows(); i++)
  {   //(int identifier, float amplitude,float duration, int type)
	  //AddStimDefinitionToMap(stimParams(i, 0), stimParams(i, 1), stimParams(i, 1), stimParams(i, 2));
	  //col id=0 -> identifier for stimulus
	  //col id=1 -> stimulus type
	  //col id=3+2*i ->  Duration of stimulus
	  //col id=2+2*(i-1) -> amplitude of stim
	  for (int stid = 0; stid < (stimParams->NumRows() - 2) / 2; ++stid)
	  {
		  float dur = stimParams(i, 3 + stid * 2).InMilliseconds();
		  float ampl = stimParams(i, 2 + (stid + 1) * 2);
		  StimParameters param;
		  param.amplitude = ampl;
		  param.duration = dur;
		  stimvec.push_back(param);

	  }
	  AddStimDefinitionToMap(stimParams(i, 0), stimvec, stimParams(i, 1));

  }

  for (int i = 0; i < stimtriggers->NumRows(); i++)
  {
	  //StimulatonId, trigger equation, repeats, anode electrodes, kathode electrodes
	  //lets assume for now that there is only one anode and one kathode!
	  IStimulationFunction* stimFunction(GetStimDefinitonFromId(stimtriggers(i, 0)));
	  stimFunction->setRepetitions(stimtriggers(i, 2));
	  std::set<uint32_t> sourceChannels;
	  std::set<uint32_t> destChannels;
	  sourceChannels.insert(stimtriggers(i, 3));
	  destChannels.insert(stimtriggers(i, 4));
	  stimFunction->setVirtualStimulationElectrodes(sourceChannels, destChannels);

	  IStimulationCommand* stimCmd = mStimCommandFactory->createStimulationCommand();
	  stimCmd->append(stimFunction);
	  mStimExecutionFunction.insert(std::pair<int, IStimulationCommand*>(i, stimCmd));
	  //mStimExecutionFunction.insert(std::pair<int, IStimulationFunction*>(i, ));
  }


 
  auto impInfo = mImplant->getImplantInfo();
  mSamplingRate = impInfo->getSamplingRate();

  bciout << "Device Type:" << impInfo->getDeviceType() << std::endl;
  bciout << "Device ID:" << impInfo->getDeviceId() << std::endl;
  bciout << "Firmware Version:" << impInfo->getFirmwareVersion() << std::endl;

  delete impInfo;


  if (!(bool)OptionalParameter("CheckState", false))
	  bcierr << "CorTec Stimulation State: Disabled" << endl;
}

void
CorTecFilter::StartRun()
{
  // The user has just pressed "Start" (or "Resume")
  bciout << "Hello World!" << endl;
}

// helper function
IStimulationFunction* CorTecFilter::createStimulationFunction(const double amplitude, const uint64_t pulseWidth, const uint64_t functionPeriod)
{
	const uint64_t dz0 = 10; // in microseconds 10micros is minimum
	int64_t dz1 = static_cast<int64_t>(functionPeriod) - 5 * static_cast<int64_t>(pulseWidth) - 2 * static_cast<int64_t>(dz0);
	if (dz1 < 10)
	{
		throw std::runtime_error("Function duration and pulseWidth do not match.");
	}
	if (pulseWidth > 2550 || dz1 > 20400)
	{
		//throw cortec::base::CInvalidArgumentException("Parameters out of range.");
		bcierr << "Parameters out of range" <<endl;
	}
	static unique_ptr<IStimulationCommandFactory> factory(createStimulationCommandFactory());
	// Defining StimulationFunction with fixed duration of 12500micros 
	IStimulationFunction* pulseStimFunction = factory->createStimulationFunction();
	IStimulationAtom* pwAtom = factory->createRect4AmplitudeStimulationAtom(amplitude, amplitude, amplitude, amplitude,
		pulseWidth); // main pulse
	pulseStimFunction->append(pwAtom);
	pulseStimFunction->append(factory->createRect4AmplitudeStimulationAtom(0.0, 0.0, 0.0, 0.0, dz0));     // dz0 pause
	pulseStimFunction->append(factory->createRect4AmplitudeStimulationAtom(                // compensation pulse
		amplitude / static_cast<double>(-4.0)
		, amplitude / static_cast<double>(-4.0)
		, amplitude / static_cast<double>(-4.0)
		, amplitude / static_cast<double>(-4.0)
		, static_cast<uint64_t>(4.0) * pwAtom->getDuration()));
	pulseStimFunction->append(factory->createRect4AmplitudeStimulationAtom(0.0, 0.0, 0.0, 0.0, dz0));     // dz0 pause
	pulseStimFunction->append(factory->createRect4AmplitudeStimulationAtom(0.0, 0.0, 0.0, 0.0, dz1));     // dz1 pause
	return pulseStimFunction;
}


void
CorTecFilter::Process(const GenericSignal& Input, GenericSignal& Output)
{

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
	

	if ((bool)OptionalParameter("CheckState", false))
	{
		
		if (mCorTecAnode != State("CorTecAnode") || mCorTecCathode != State("CorTecCathode") || mCorTecAmplitude != State("CorTecAmplitude") || mCorTecPulseWidth != State("CorTecPulseWidth") || mCorTecFunctionPeriod != State("CorTecFunctionPeriod"))
		{
			mImplant.reset();

			std::unique_ptr<IStimulationFunction> stimFunction(createStimulationFunction(State("CorTecAmplitude") // microampere
				, State("CorTecPulseWidth") // micros
				, State("CorTecFunctionPeriod") // micros
			));

			// saving the current state as previous state
			mCorTecAnode = State("CorTecAnode");
			mCorTecCathode = State("CorTecCathode");
			mCorTecAmplitude = State("CorTecAmplitude");
			mCorTecPulseWidth = State("CorTecPulseWidth");
			mCorTecFunctionPeriod = State("CorTecFunctionPeriod");
		}

		std::unique_ptr<IStimulationCommandFactory> factory(createStimulationCommandFactory());
		IStimulationCommand* stimCmd = factory->createStimulationCommand();

		std::unique_ptr<IStimulationFunction> stimFunction(createStimulationFunction(100 // microampere
			, 1100 // micros
			, 12500 // micros
		));
		stimFunction->setRepetitions(1);

		// set stimulation channels: channel 1 as source, ground electrode as destination.
		std::set<uint32_t> sourceChannels;
		sourceChannels.insert(0);
		std::set<uint32_t> destinationChannels;
		//destinationChannels.insert(BIC3232Constants::c_groundElectrode);
		destinationChannels.insert(gElectrode);
		stimFunction->setVirtualStimulationElectrodes(sourceChannels, destinationChannels);

		std::unique_ptr<IStimulationFunction> pauseFunction(createStimulationFunction(0.0, 2500, 20780));
		pauseFunction->setRepetitions(2);

		// beta 17-20Hz: 18,5185185Hz ==> one pulse per 54,0 (~1110 repetitions for 60s)
		for (int i = 0; i < 1110; ++i)
		{
			//  12500micros + 20780micros * 2 = 54060 micros
			stimCmd->append(stimFunction->clone());
			stimCmd->append(pauseFunction->clone());
		}

		// start the stimulation...
		mImplant->startStimulation(stimCmd);
		// wait for 2 seconds. During this time, the implant executes the stimulation command.
		std::this_thread::sleep_for(2s);


		if (mActivateExp.Evaluate(&Input)) {
			mStimulating = true;
		}
		else if (mAbortExp.Evaluate(&Input))
		{
			mStimulating = false;
		}



		if (mStimulating) {
			State("CorTecState") = 1;
		}
		else
		{
			State("CorTecState") = 0;
		}
	}
	else
	{
		bcierr << "CorTec Stimulation State: Disabled" << endl;
	}

}

void
CorTecFilter::StopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // or because the run has reached its natural end.
  bciwarn << "Goodbye World." << endl;
  // You know, you can delete methods if you're not using them.
  // Remove the corresponding declaration from CorTecFilter.h too, if so.
}

void
CorTecFilter::Halt()
{
  // Stop any threads or other asynchronous activity.
  // Good practice is to write the Halt() method such that it is safe to call it even *before*
  // Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
}

