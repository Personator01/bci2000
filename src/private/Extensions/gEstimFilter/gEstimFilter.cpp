////////////////////////////////////////////////////////////////////////////////
// $Id: gEstimFilter.cpp 7640 2023-10-03 19:53:28Z wengelhardt $
// Author: kaleb.goering@gmail.com, belsten@neurotechcenter.org
// Description: A filter to interact with gEstim device
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2023: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "gEstimFilter.h"
#include "BCIEvent.h"
#include "Environment.h"

#include <vector>
#include <cstdio>
#include <sstream>
#include <cmath>

class gEstimFilter;

using namespace std;

RegisterFilter (gEstimFilter, 3.D);

bool gEstimPRO_GetLastAppliedCRTandVLTG (PGDevice _device, uint16_t& lastAppliedCRT, uint16_t& lastAppliedVLTG)
{
  int32_t current1_sum = gt_current1_sum(_device);
  double current1_avg = gt_current1_avg(_device);
  double voltage1_avg = gt_voltage1_avg(_device);
  double current2_avg = gt_current2_avg(_device);
  double voltage2_avg = gt_voltage2_avg(_device);

  if (gt_phase_duration1(_device) > 0)
  {
    if (current1_sum != NOT_AVAILABLE_CURRENT1_SUM_)
    {
      double c = 1e-3 * abs(current1_avg);
      double v = abs(voltage1_avg);
      if ((gt_phase_duration2(_device) > 0 || gt_alternate(_device) == GALTERNATE_YES) && current2_avg != 0)
      {
        if (c != 0)
        {
          c = 0.5 * (1e-3 * abs(current2_avg) + c);
          v = 0.5 * (abs(voltage2_avg) + v);
        }
        else
        {
          c = 1e-3 * abs(current2_avg);
          v = abs(voltage2_avg);
        }
      }
      lastAppliedCRT = (uint16_t)(1e3 * c); // muA
      lastAppliedVLTG = (uint16_t)(1e3 * v); // mV
    }
    else
    {
      lastAppliedCRT = 0;
      lastAppliedVLTG = 0;
    }
  }
  return true;
}
// define static member variables
uint16_t gEstimFilter::mlastCurrent = 0;
uint16_t gEstimFilter::mlastVoltage = 0;

gEstimFilter::gEstimFilter() : 
	mEstim(NULL), 
	mEstimActive(false), 
	mUseEstimOn(false),
	mpgEstimThread(NULL), 
	mStimulating(false),
  mKillChangeStimThread(true),
  mKillStimThread(true),
  mRunningState(false),
  mOldSelfTest(false),
  mVis("gEstim Log"),
  mConfigNumber(1)
{
  mChangeStimLock = CreateEvent(NULL, false, false, NULL);
  mStimLock = CreateEvent(NULL, false, false, NULL);
}

gEstimFilter::~gEstimFilter() 
{
  Halt();
  if (mEstimActive)
    closeDevices();
  CloseHandle(mChangeStimLock);
  CloseHandle(mStimLock);
}

void gEstimFilter::Publish() 
{
	if (!OptionalParameter("ActivateEstim", 0)) return;


	// Define the gEstimFilter Parameters
	BEGIN_PARAMETER_DEFINITIONS

		"gEstim:gEstim int ActivateEstim= 1 0 0 1"
			" // allow use of g.Estim device (boolean)",

		"gEstim:gEstim int UseStimulusPresentation= 1 0 0 1"
			" // Use EstimOn row in Stimuli parameter(boolean)",

    "gEstim:gEstim int UseMultipleConfigurations= 1 0 0 1"
    " // Use StimulationConfigurations table to vary stimulation(boolean)",

    "gEstim:gEstim int DoSelfTest= 1 1 0 1"
    " // Perform Self test - recommended (boolean)",

    "gEstim:gEstim int QuietStimulation= 0 0 0 1"
    " // Make stimulation noiseless (boolean)",

    "gEstim:Stimulation matrix StimulationConfigurations= "
    "{  Expression      Modularity(1=Biphasic)  Polarity(1=Alternating)   PhaseDuration(us) InterphaseDuration(us) "
    "   Magnitude(mA) PulseFrequency(Hz)  NumberOfPulses  FrequencyOfTrains(Hz) NumberOfTrains } { Configuration1 }"
    "   StimulusCode==1 1                       0                         200               100                     "
    "   1             20                  10              0.5                   5                "
      " // Configurations of stimulation",  

		"gEstim:Stimulation string StimulationExpression= % % % % "
			" // Expression to start g.Estim stimulation (To stimulate after upload, leave blank and don't UseStimulusPresentation)",

		"gEstim:Stimulation string AbortExpression= % % % % "
			" // Expression to abort g.Estim stimulation",

		"gEstim:Stimulation int Modularity= 1 1 0 1 "
			" // Modularity of pulses: 1 Biphasic, 0 Monophasic (enumeration)",

		"gEstim:Stimulation int Polarity= 1 1 0 1 "
			" // Polarity of pulses: 1 Alternating, 0 Steady (enumeration)",

		"gEstim:Stimulation float PhaseDuration= 200mus % % %"
			" // Duration of phase",

		"gEstim:Stimulation float InterphaseDuration= 100mus % % % "
			" // Duration between phases",

		"gEstim:Stimulation float Magnitude= 1 % % % "
			" // Magnitude of pulses (in milliamps)",

		"gEstim:Stimulation float PulseFrequency= 2Hz % % % "
			" // Frequency of pulses",

		"gEstim:Stimulation int NumberOfPulses= 1 1 1 % "
			" // Number of pulses",

		"gEstim:Stimulation float FrequencyOfTrains= 1Hz % % % "
			" // Frequency of trains ",

		"gEstim:Stimulation int NumberOfTrains= 1 1 1 % "
			" // Number of trains",

		"gEstim:Stimulation int Jitter= 0 0 0 100 "
			" // % jitter of between trains",

		"gEstim:Electrode int ElectrodeType= 1 1 1 3 "
			" // Type of electrode being used: 1 Circle, 2 Depth, 3 Other (enumeration)",
		
		"gEstim:CircularElectrode int CircleDiameter= 2000 2000 % % "
			" // Exposed diameter of the electrode in micrometers [1um]",

		"gEstim:DepthElectrode int ContactDiameter= 2000 2000 % % "
			" // Contact diameter of the electrode in micrometers [1um]",

		"gEstim:DepthElectrode int ContactLength= 1000 1000 % % "
			" // Contact length of the electrode in micrometers [1um]",

		"gEstim:OtherElectrode int ExposedSurfaceArea= 1750000000 1750000000 % % "
			" // Exposed surface area of electrode in square micrometers [1um^2]",

	END_PARAMETER_DEFINITIONS

	// Define the gEstimFilter States
	BEGIN_STATE_DEFINITIONS

		"EstimStimulus 8 0 0 0",

		"EstimCurrent 16 0 0 0", // ------ muA --------

		"EstimVoltage 16 0 0 0", // ------ mV ---------

		"EstimImpedance 16 0 0", // ------ Ohms -------

	END_STATE_DEFINITIONS
}

void gEstimFilter::Preflight( const SignalProperties &Input, SignalProperties &Output ) const 
{

	Output = Input;
	
	State("Running");

	if ((bool)OptionalParameter("ActivateEstim",false)) 
	{
    //only restart if device isn't already open
    if (count_openDevices() < 1)
    {
      // Test for devices
      openDevices(true);
      PGDevice* pd = gt_openDevices();
      int availableDevices = count_openDevices();
      closeDevices();
      if (availableDevices < 1)
        bcierr << "No g.Estim devices found" << endl;
      else if (availableDevices > 1)
        bcierr << "Multiple g.Estim devices found" << endl;
    }
    //store all expressions that will be used
    multiset<string> allUsedExpressions;

		// Test expressions
		GenericSignal preflightSignals(Input);
    for (int c = 0; c < Parameter("StimulationConfigurations")->NumColumns(); c++)
    {
      Expression exp = (Expression)Parameter("StimulationConfigurations")(0, c);
      exp.Compile();
      exp.Evaluate(&preflightSignals);
      allUsedExpressions.insert(Parameter("StimulationConfigurations")(0, c).ToString());
    }
    //can start right after stim is uploaded
    if (Parameter("StimulationExpression").IsNull())
    {
      if (!(bool)Parameter("UseMultipleConfigurations"))
        bcierr << "Estim error: Must specify StimulationExpression if not using multiple stimulation configurations" << endl;
    }
    else
    {
      Expression stimExpression = Expression(Parameter("StimulationExpression"));
      stimExpression.Compile();
      stimExpression.Evaluate(&preflightSignals);
      allUsedExpressions.insert(Parameter("StimulationExpression").ToString());
    }
		Expression abortExpression = Expression(Parameter("AbortExpression"));
		abortExpression.Compile();
		abortExpression.Evaluate(&preflightSignals);
    allUsedExpressions.insert(Parameter("AbortExpression").ToString());

    Parameter("DoSelfTest");
		Parameter ("PhaseDuration");			 
		Parameter ("InterphaseDuration");
		Parameter ("Magnitude");								
		Parameter ("PulseFrequency");							
		Parameter ("FrequencyOfTrains");
		Parameter("QuietStimulation");

		// Warn if Monophasic and steady
		if ((!Parameter("Modularity") && !Parameter("Polarity")))
			bciwarn << "Stimulation is set to steady monophasic pulses which will cause charge to accumulate" << endl;
    Parameter ("ElectrodeType");
    
    Parameter ("CircleDiameter");

    Parameter ("ContactDiameter");
    Parameter ("ContactLength");

    Parameter ("ExposedSurfaceArea");

		// check if the "EstimOn" Row exists in the "Stimuli" matrix. 
		if ((bool)Parameter ("UseStimulusPresentation"))
		{
			State("StimulusCode");
			if (!Parameter ("Stimuli")->RowLabels ().Exists ("EstimOn"))
			{
				bcierr << "gEstim error: EstimOn row in Stimuli parameter not present";
			}
			// check that the elements of the EstimOn row are only 1 or 0
			for (int i = 0; i < Parameter ("Stimuli")->NumColumns (); i++)
			{
				if (Parameter ("Stimuli")("EstimOn", i).ToString() != "0" && Parameter ("Stimuli")("EstimOn", i).ToString () != "1")
				{
					bcierr << "gEstim error: EstimOn row in Stimuli parameter can only contain zeros or ones.";   
				}
        if (Parameter("Stimuli")("EstimOn", i).ToString() == "1")
        {
          allUsedExpressions.insert(("StimulusCode==" + to_string(i+1)));
        }
			}
		}

    //validate the same expression isn't used to trigger multiple things
    //first take out empty strings
    while (*allUsedExpressions.begin() == "")
      allUsedExpressions.erase(allUsedExpressions.begin());
    auto duplicate = adjacent_find(allUsedExpressions.begin(), allUsedExpressions.end());
    if (duplicate != allUsedExpressions.end())
    {
      bcierr << "gEstim Error: The same expression (" << *duplicate
        << ") is used for multiple triggers. Please use unique triggers." << endl;
    }
	}
}

void gEstimFilter::Initialize( const SignalProperties &Input, const SignalProperties &Output ) 
{
	mPrevStimulusCode = 0;
	mEstimActive = (bool)OptionalParameter("ActivateEstim",false);
  if (mEstimActive)
  {
    bcidbg(0) << "This filter will NOT work in debug mode" << endl;

    //only restart if device isn't already open
    //also conduct self test if that parameter was changed
    if (count_openDevices() < 1 || ((bool)Parameter("DoSelfTest") && !mOldSelfTest))
    {
      closeDevices();
      mEstim = NULL;
      if (!openDevice(&mEstim, NULL, true))
      {
        bcierr << "Unable to connect to g.Estim device" << endl;
      }
      bciwarn << "Estim: Verifying input parameters..." << endl;
      // Start box and self-diagnostics
      if ((bool)Parameter("DoSelfTest"))
      {
        if (!selftest(mEstim)) // Must be run
          bcierr << "Estim: unable to pass self test. Make "
          << "sure batteries are charged and unplug "
          << "leads from cathode and anode ports on "
          << "front of device." << endl;
      }
      else
        //put in stop mode
        stop(mEstim);
    }
    else
    {
      stop(mEstim);
    }
    //print device info      
    stringstream ss;
    ss << "Device Info:";
    ss << "\n\tIdentifier:            \t" << mEstim->identifier_;
    ss << "\n\tSerial Number:    \t" << mEstim->serial_;
    ss << "\n\tHardware Version: \t" << mEstim->HW_version_;
    ss << "\n\tFirmware Version: \t" << mEstim->FW_version_;
    bciout << ss.str();

    auto fw = mEstim->FW_version_;
    string minorV = "";
    mSupermodeEnabled = false;
    if (FWVERSION_SIZE > 3)
    {
      minorV = minorV + fw[3];
      minorV = minorV + fw[4];
      int majorV = int(fw[1] - '0');
      //major > 1 or minor > 23
      if (majorV > 1 || stoi(minorV) > 23)
        mSupermodeEnabled = true;
    }

    mOldSelfTest = (bool)Parameter("DoSelfTest");

    st_operationmode(mEstim, GESTIM_MODE_BASIC);
    wait_for_end_of_action(mEstim);
    register_with_polling(mEstim, gEstimPRO_fieldsChangedCallback);

    // Get parameters
    mActivateExp = (Expression)Parameter("StimulationExpression");
    mUseMultipleConfigs = (bool)Parameter("UseMultipleConfigurations");
    mAbortExp = (Expression)Parameter("AbortExpression");
    mUseEstimOn = (bool)Parameter("UseStimulusPresentation");

    //start stim immediately after upload if separate expression is not provided
    string sExp = Parameter("StimulationExpression").ToString();
    mStartAfterUpload = sExp.empty() && !mUseEstimOn;
    if (mUseEstimOn)
    {
      mStimulusCodeTriggers.clear();
      // get the index of the column's with EstimOn == 1
      // index, i, corresponds to StimulusCode i+1
      for (int i = 0; i < Parameter("Stimuli")->NumColumns(); i++)
      {
        if (Parameter("Stimuli")("EstimOn", i).ToString() == "1")
        {
          // this list is sorted, so now we can binary search it in Process()
          mStimulusCodeTriggers.push_back(i + 1);
        }
      }
    }

    mConfigurations.clear();
    mConfigToUpload = nullptr;
    for (int i = 0; i < Parameter("StimulationConfigurations")->NumColumns(); i++)
    {
      auto config = Parameter("StimulationConfigurations")(i);

      Expression exp = Parameter("StimulationConfigurations")(0, i).ToString();
      int modul = Parameter("StimulationConfigurations")(1, i);
      int polar = Parameter("StimulationConfigurations")(2, i);
      int pulseLength = Parameter("StimulationConfigurations")(3, i) / 10; // multiple of  10us
      int interphaseLength = Parameter("StimulationConfigurations")(4, i) / 10; // multiple of  10us
      int pulseAmplitude = Parameter("StimulationConfigurations")(5, i) * 100;									// multiple of  10uA
      int frequency = Parameter("StimulationConfigurations")(6, i) * 10;					// multiple of 0.1Hz
      int numberPulses = Parameter("StimulationConfigurations")(7, i);
      double freqOfTrains = Parameter("StimulationConfigurations")(8, i);							// multiple of   1Hz
      int numberTrains = Parameter("StimulationConfigurations")(9, i);

      if (pulseLength > gt_phase_duration1_max(mEstim) || pulseLength < gt_phase_duration1_min(mEstim))
        bcierr << "gEstim: PhaseDuration must be between " << (gt_phase_duration1_min(mEstim) * 10) << "us and " << (gt_phase_duration1_max(mEstim) * 10) << "us" << endl;

      if (interphaseLength > gt_interphase_max(mEstim) || interphaseLength < gt_interphase_min(mEstim))
        bcierr << "gEstim: InterphaseDuration must be between " << (gt_interphase_min(mEstim) * 10) << "us and " << (gt_interphase_max(mEstim) * 10) << "us" << endl;

      if (pulseAmplitude > gt_phase_current1_max(mEstim) || pulseAmplitude < gt_phase_current1_min(mEstim))
        bcierr << "gEstim: Magnitude must be between " << (gt_phase_current1_min(mEstim) * 10) << "uA and " << (gt_phase_current1_max(mEstim) * 10) << "uA" << endl;

      if (frequency > gt_pulse_rate_max(mEstim) || frequency < gt_pulse_rate_min(mEstim))
        bcierr << "gEstim: PulseFrequency must be between " << (gt_pulse_rate_min(mEstim) * 0.1) << "Hz and " << (gt_pulse_rate_max(mEstim) * 0.1) << "Hz" << endl;

      if (numberPulses > gt_pulses_max(mEstim) || numberPulses < gt_pulses_min(mEstim))
        bcierr << "gEstim: NumberOfPulses must be between " << gt_pulses_min(mEstim) << " and " << gt_pulses_max(mEstim) << endl;

      if (freqOfTrains > gt_train_rate_max(mEstim) || freqOfTrains < gt_train_rate_min(mEstim))
        bcierr << "gEstim: FrequencyOfTrains must be between " << gt_train_rate_min(mEstim) << " Hz and " << gt_train_rate_max(mEstim) << endl;

      if (numberTrains > gt_n_trains_max(mEstim) || numberTrains < gt_n_trains_min(mEstim))
        bcierr << "gEstim: NumberOfTrains must be between " << gt_n_trains_min(mEstim) << " and " << gt_n_trains_max(mEstim) << endl;

      StimConfig c{ exp, modul, (GAlternate)polar, pulseLength, interphaseLength, pulseAmplitude, frequency, numberPulses, freqOfTrains, numberTrains };
      mConfigurations.push_back(c);
    }

    int modul, polar, numberPulses, numberTrains, pulseLength, interphaseLength, pulseAmplitude, frequency;
    double freqOfTrains;
    if (mUseMultipleConfigs)
    {
      //default: use the first column
      mConfigToUpload = &mConfigurations[0];
      modul = mConfigToUpload->modularity;
      polar = mConfigToUpload->polarity;
      numberPulses = mConfigToUpload->numPulses;
      numberTrains = mConfigToUpload->numTrains;
      pulseLength = mConfigToUpload->pulseLength;
      interphaseLength = mConfigToUpload->interphaseLength;
      pulseAmplitude = mConfigToUpload->pulseAmplitude;
      frequency = mConfigToUpload->pulseFreq;
      freqOfTrains = mConfigToUpload->trainFreq;

      //let user know if they can't use supermode
      if (!mSupermodeEnabled)
        bciwarn << "gEstim: Device's firmware version cannot enable SUPERMODE (changing stimulation settings with active device). "
                << "Switching stimulation configurations will take longer than 200ms. "
                << "Update firmware version to above V1.23 to enable SUPERMODE" << endl;
    }
    else
    {
      modul = Parameter("Modularity");
      polar = Parameter("Polarity");
      numberPulses = Parameter("NumberOfPulses");
      numberTrains = Parameter("NumberOfTrains");
      //int jitter           = Parameter ("Jitter");
      pulseLength = Parameter("PhaseDuration").InMilliseconds() * 100;			  // multiple of  10us
      interphaseLength = Parameter("InterphaseDuration").InMilliseconds() * 100;   // multiple of  10us
      pulseAmplitude = Parameter("Magnitude") * 100;												  	  // multiple of  10uA
      frequency = Parameter("PulseFrequency").InHertz() * 10;						   	// multiple of 0.1Hz
      freqOfTrains = Parameter("FrequencyOfTrains").InHertz();							  // multiple of   1Hz

      // Check parameters with device specific max/mins
      // device max/mins have consistant values with BCI2000 parameters
      if (pulseLength > gt_phase_duration1_max(mEstim) || pulseLength < gt_phase_duration1_min(mEstim))
        bcierr << "gEstim: PhaseDuration must be between " << (gt_phase_duration1_min(mEstim) * 10) << "us and " << (gt_phase_duration1_max(mEstim) * 10) << "us" << endl;

      if (interphaseLength > gt_interphase_max(mEstim) || interphaseLength < gt_interphase_min(mEstim))
        bcierr << "gEstim: InterphaseDuration must be between " << (gt_interphase_min(mEstim) * 10) << "us and " << (gt_interphase_max(mEstim) * 10) << "us" << endl;

      if (pulseAmplitude > gt_phase_current1_max(mEstim) || pulseAmplitude < gt_phase_current1_min(mEstim))
        bcierr << "gEstim: Magnitude must be between " << (gt_phase_current1_min(mEstim) * 10) << "uA and " << (gt_phase_current1_max(mEstim) * 10) << "uA" << endl;

      if (frequency > gt_pulse_rate_max(mEstim) || frequency < gt_pulse_rate_min(mEstim))
        bcierr << "gEstim: PulseFrequency must be between " << (gt_pulse_rate_min(mEstim) * 0.1) << "Hz and " << (gt_pulse_rate_max(mEstim) * 0.1) << "Hz" << endl;

      if (numberPulses > gt_pulses_max(mEstim) || numberPulses < gt_pulses_min(mEstim))
        bcierr << "gEstim: NumberOfPulses must be between " << gt_pulses_min(mEstim) << " and " << gt_pulses_max(mEstim) << endl;

      if (freqOfTrains > gt_train_rate_max(mEstim) || freqOfTrains < gt_train_rate_min(mEstim))
        bcierr << "gEstim: FrequencyOfTrains must be between " << gt_train_rate_min(mEstim) << " Hz and " << gt_train_rate_max(mEstim) << endl;

      if (numberTrains > gt_n_trains_max(mEstim) || numberTrains < gt_n_trains_min(mEstim))
        bcierr << "gEstim: NumberOfTrains must be between " << gt_n_trains_min(mEstim) << " and " << gt_n_trains_max(mEstim) << endl;
    }
    int jitter = Parameter("Jitter");
    if (jitter > gt_jitter_max(mEstim) || jitter < gt_jitter_min(mEstim))
      bcierr << "gEstim: Jitter must be between " << gt_jitter_min(mEstim) << "% and " << gt_jitter_max(mEstim) << "%" << endl;


    switch ((int)Parameter("ElectrodeType"))
    {
    case 1:
      if ((int)Parameter("CircleDiameter") > gt_el_circ_diameter_max(mEstim) ||
        (int)Parameter("CircleDiameter") < gt_el_circ_diameter_min(mEstim))
        bcierr << "gEstim: CircleDiameter must be between " << gt_el_circ_diameter_min(mEstim)
        << " and " << gt_el_circ_diameter_max(mEstim) << endl;

      st_el_type(mEstim, ELECTRODE_CIRCULAR);
      st_el_circ_diameter(mEstim, (int)Parameter("CircleDiameter"));
      break;
    case 2:
      if ((int)Parameter("ContactDiameter") > gt_el_depth_diameter_max(mEstim) ||
        (int)Parameter("ContactDiameter") < gt_el_depth_diameter_min(mEstim))
        bcierr << "gEstim: ContactDiameter must be between " << gt_el_depth_diameter_min(mEstim)
        << " and " << gt_el_depth_diameter_max(mEstim) << endl;

      if ((int)Parameter("ContactLength") > gt_el_depth_length_max(mEstim) ||
        (int)Parameter("ContactLength") < gt_el_depth_length_min(mEstim))
        bcierr << "gEstim: ContactLength must be between " << gt_el_depth_length_min(mEstim)
        << " and " << gt_el_depth_length_max(mEstim) << endl;

      st_el_type(mEstim, ELECTRODE_DEPTH);
      st_el_depth_diameter(mEstim, (int)Parameter("ContactDiameter"));
      st_el_depth_length(mEstim, (int)Parameter("ContactLength"));
      break;
    case 3:
      if ((int)Parameter("ExposedSurfaceArea") > gt_el_other_surface_max(mEstim) ||
        (int)Parameter("ExposedSurfaceArea") < gt_el_other_surface_min(mEstim))
        bcierr << "gEstim: ExposedSurfaceArea must be between " << gt_el_other_surface_min(mEstim)
        << " and " << gt_el_other_surface_max(mEstim) << endl;

      st_el_type(mEstim, ELECTRODE_OTHER);
      st_el_other_surface(mEstim, (int)Parameter("ExposedSurfaceArea"));
      break;
    }

    // Set parameters
    st_defaults(mEstim);
    if (polar)
      st_alternate(mEstim, GALTERNATE_YES);
    else
      st_alternate(mEstim, GALTERNATE_NO);

    st_phase_duration1(mEstim, pulseLength);
    st_phase_duration2(mEstim, (pulseLength * modul));
    st_interphase(mEstim, interphaseLength);
    st_phase_current1(mEstim, pulseAmplitude);
    st_phase_current2(mEstim, (-1 * pulseAmplitude));
    st_pulse_rate(mEstim, frequency);
    st_pulses(mEstim, numberPulses);
    st_train_rate(mEstim, freqOfTrains);
    st_n_trains(mEstim, numberTrains);
    st_jitter(mEstim, jitter);
    auto pastDoFunction = gt_do1_function(mEstim);
    st_do1_function(mEstim, pastDoFunction & GDO_PULSE);
    pastDoFunction = gt_do2_function(mEstim);
    st_do2_function(mEstim, pastDoFunction & GDO_PULSE);

    //allow for changing stimulation
    uint16_t dioState = gt_dio_enabled(mEstim);
    if (mSupermodeEnabled)
    {
      dioState = dioState | (1 << GMASK_NOAUTOSTOP);
      dioState = dioState | (1 << GMASK_SUPERMODE);
    }
    //make quiet
    if (Parameter("QuietStimulation"))
    {
      dioState = dioState & ~(1 << GMASK_BUZZER);
    }
    st_dio_enabled(mEstim, dioState);
    //snd_dio_enabled(mEstim);
    
    // Send config
    activate(mEstim);
    WaitForProcess();
    rcv_stim(mEstim);
    stringstream memostream;
    memostream << "\t Phase duration:         \t" << (gt_phase_duration1(mEstim) * 10) << "mus\n";
    memostream << "\t Interphase duration: \t" << (gt_interphase(mEstim) * 10) << "mus\n";
    memostream << "\t Magnitude:               \t" << (gt_phase_current1(mEstim) / 100.0) << "mA\n";
    memostream << "\t Pulse frequency:         \t" << (gt_pulse_rate(mEstim) * 0.1) << "Hz\n";
    memostream << "\t Number of pulses:        \t" << (gt_pulses(mEstim)) << "\n";
    memostream << "\t Train frequency:         \t" << (gt_train_rate(mEstim)) << "Hz\n";
    memostream << "\t Number of trains:        \t" << (gt_n_trains(mEstim)) << "\n";
    memostream << "\t Jitter:                         \t" << (gt_jitter(mEstim)) << "%\n";
    string memostring = memostream.str();
    bciout << memostring.c_str();
    bciwarn << "Estim: The charge of a sequence will be " << (gt_train_charge(mEstim)/*10pC*/ * numberTrains / 100) << "nC" << endl;	// EstimPRO	
    if ((gt_phase_current1(mEstim) / 100.0) >= 10)
      bciwarn << "Estim Warning: Stimulation magnitude is greater than 10mA - proceed with caution." << endl;


    //bciwarn << "The charge of a sequence will be " << (gt_train_charge(mEstim) * numberTrains / 25000) << "uC" << endl;	 // FES * 40pC

    /*
    bciwarn << "The resistance is " << gt_resistance(mEstim) << "O" << endl;
    bciwarn << "The resistance is " << measure_resistance(mEstim) << "O" << endl;
    bciwarn << "The resistance is " << gt_test_resistance(mEstim) << "O" << endl;
    */

    // Create the communication thread
    mpgEstimThread = new gEstimThread(this);
    if (mpgEstimThread)
    {
      mpgEstimThread->Start();
    }
    else
    {
      bcierr << "Could not start g.Estim thread" << endl;
    }
    //Create changing stimulation thread
    if (mUseMultipleConfigs)
    {
      if (mKillChangeStimThread)
      {
        mKillChangeStimThread = false;
        mChangeStimThread = thread(&gEstimFilter::ChangeStimulationThread, this);
      }
    }
    //Create stimulation thread
    if (mKillStimThread)
    {
      mKillStimThread = false;
      mStimThread = thread(&gEstimFilter::StimulationThread, this);
    }

    //initialize logger window for stimulation output
    mVis.Send(CfgID::WindowTitle, "gEstim Stimulation Logger");
  }
}

void gEstimFilter::StartRun() 
{
  mRunningState = true;
  mStimLoaded = 0;
  mExpressionHasTriggered = false;
  if (mEstimActive)
  {
    mVis.Send("\r----New Run----");
  }
}

void gEstimFilter::StopRun() 
{
	if (mEstimActive)
	{
		abort_sequence(mEstim);
		mStimulating = false;
    mVis.Send("----Run Stopped----");
	}
  mRunningState = false;
}

void gEstimFilter::Halt() 
{
  //release changing stim thread
  mRunningState = false;
  if (!mKillChangeStimThread)
  {
    mKillChangeStimThread = true;
    SetEvent(mChangeStimLock);
    mChangeStimThread.join();
  }
  //release stim thread
  if (!mKillStimThread)
  {
    mKillStimThread = true;
    SetEvent(mStimLock);
    mStimThread.join();
  }

	// Kill thread
	if(mEstimActive) 
	{
		if (mpgEstimThread)
		{
			mpgEstimThread->Terminate();
		}
		delete mpgEstimThread;
		mpgEstimThread = NULL;
		unregister_with_polling(mEstim,gEstimPRO_fieldsChangedCallback);
	}
}

void gEstimFilter::Process( const GenericSignal &Input, GenericSignal &Output ) 
{
	Output = Input;
	if (mEstimActive) 
	{
		if (mpgEstimThread) 
		{
			GState state;
			state = gt_state(mEstim);
			
      if (state == GSTATE_ACTIVE || state == GSTATE_STIMULATION)
      {
        //if stimulation is done
        if (gt_remaining(mEstim) == 0)
        {
          // check if we need to upload any new configurations
          if (mUseMultipleConfigs)
          {
            for (auto it = mConfigurations.begin(); it != mConfigurations.end(); it++)
            {
              int col = it - mConfigurations.begin() + 1;
              if (it->expression.Evaluate(&Input) && (mStimLoaded != col))
              {
                mStimLoaded = col; //column of which config is loaded
                mConfigNumber = col;
                mConfigToUpload = &(*it);
                SetEvent(mChangeStimLock);
                mVis.Send("\rConfiguration " + to_string(col) + " uploaded from StimulationConfigurations table.");
              }
            }
          }
          //if start expression is true and we didn't just start stimulating, stimulate
          if (!mStartAfterUpload && mActivateExp.Evaluate(&Input) && !mExpressionHasTriggered)
          {
            mExpressionHasTriggered = true;
            mStimulating = true;
            SetEvent(mStimLock);
            mVis.Send("Stimulation started. ActivateExpression evaluated true");
          }
          //check Stimi parameter for if expressions are true
          else if (mUseEstimOn
                && (std::binary_search(mStimulusCodeTriggers.begin(), mStimulusCodeTriggers.end(), State("StimulusCode")) 
                && mPrevStimulusCode != State("StimulusCode")))
            {
              mStimulating = true;
              SetEvent(mStimLock);
              mVis.Send("Stimulation started. Triggered by Stimulus Code " + to_string(State("StimulusCode")));
            }
        }
        //stimulation is going on
        else if (mStimulating)
        {
          //check if we need to abort
          if (mAbortExp.Evaluate(&Input))
          {
            mStimulating = false;
            SetEvent(mStimLock);
            mVis.Send("Stimulation Aborted");
          }
          //abort if none of stimulus codes are true in Stimuli table
          else if (mUseEstimOn)
          {
            if (!std::binary_search(mStimulusCodeTriggers.begin(), mStimulusCodeTriggers.end(), State("StimulusCode")) && !mActivateExp.Evaluate(&Input))
            {
              mStimulating = false;
              SetEvent(mStimLock);
              mVis.Send("Stimulation stopped. Triggered by Stimulus Code " + to_string(State("StimulusCode")));
            }
          }
        }
      }
      //update states
			State("EstimStimulus") = (gt_remaining(mEstim) != 0) * mConfigNumber;
			State ("EstimCurrent") = mlastCurrent; // muA
			State ("EstimVoltage") = mlastVoltage; // mV
	
			if (mlastCurrent != 0)
			{
				// convert to double to preserve numerical accuracy
				// do the division when both numbers have ~ same exponent
				State ("EstimImpedance") = (uint16_t)(( (double)mlastVoltage/(double)mlastCurrent )*(1e3/*ohm*/));
			}
			else 
				State ("EstimImpedance") = 0;
		}
		else 
			bcierr << "Lost connection to gEstimThread" << endl;
	}

  //store past state to only trigger once per true expression
	if (mUseEstimOn)
    mPrevStimulusCode = State("StimulusCode");
  if (!mStartAfterUpload)
  {
    if (mExpressionHasTriggered)
      mExpressionHasTriggered = mActivateExp.Evaluate(&Input);
  }
}

void
gEstimFilter::ChangeStimulationThread()
{
  while (!mKillChangeStimThread)
  {
    DWORD result = WaitForSingleObject(mChangeStimLock, INFINITE);
    if (result == WAIT_OBJECT_0)
    {
      if (mRunningState)
      {
        if (!mSupermodeEnabled)
        {
          //stop gEstim before we can modify
          stop(mEstim);
          WaitForProcess();
        }

        //change stimulation parameters
        st_alternate(mEstim,        mConfigToUpload->polarity);
        st_phase_duration1(mEstim,  mConfigToUpload->pulseLength);
        st_phase_duration2(mEstim,  mConfigToUpload->pulseLength * mConfigToUpload->modularity);
        st_interphase(mEstim,       mConfigToUpload->interphaseLength);
        st_phase_current1(mEstim,   mConfigToUpload->pulseAmplitude);
        st_phase_current2(mEstim,   mConfigToUpload->pulseAmplitude * -1);
        st_pulse_rate(mEstim,       mConfigToUpload->pulseFreq);
        st_pulses(mEstim,           mConfigToUpload->numPulses);
        st_train_rate(mEstim,       mConfigToUpload->trainFreq);
        st_n_trains(mEstim,         mConfigToUpload->numTrains);
        //update and activate again
        activate(mEstim);
        //check for errors
        WaitForProcess();

        //start stim if we didn't specify a separate expression
        if (mStartAfterUpload)
        {
          mVis.Send("Stimulation started after upload");
          mStimulating = true;
          SetEvent(mStimLock);
          Sleep(1); //allows for process to actually wait before resetting
          WaitForProcess();
          mStimLoaded = 0; //reset
        }
      }
    }
  }
}

void
gEstimFilter::StimulationThread()
{
  while (!mKillStimThread)
  {
    DWORD result = WaitForSingleObject(mStimLock, 60000); //reset every minute
    if (result == WAIT_OBJECT_0)
    {
      if (mRunningState)
      {
        if (mStimulating)
        {
          start_sequence(mEstim);
        }
        else
          abort_sequence(mEstim);
      }
    }
		else
		{
			//wake up device if it is at risk of sleeping
			st_state(mEstim, GSTATE_ACTIVE);
			snd_state(mEstim);
		}
  }
}

void
gEstimFilter::WaitForProcess()
{
  if (mRunningState)
  {
    wait_for_end_of_action(mEstim);
    GError err = gt_error(mEstim);
    if (err != GERROR_SUCCESS)
    {
      //most likely due to stopping run during stimulation
      if (err == GERROR_OPERATION_ABORTED)
        bciwarn << "Estim error - " << gt_error_text(mEstim, gt_error(mEstim)) << endl;
      else
        bcierr << "Estim error - " << gt_error_text(mEstim, gt_error(mEstim)) << endl;
    }
  }
}

int gEstimFilter::gEstimThread::OnExecute() 
{
	if (mpFilter -> mEstimActive) 
	{
			while (!Terminating())
			{
				rcv_state(mpFilter->mEstim);
				ThreadUtils::SleepForMs(100);
			}
	}
	return 0;
}

void gEstimFilter::gEstimPRO_fieldsChangedCallback (PGDevice _device, uint64_t changed_flags)
{
	uint16_t lastCurrent;
	uint16_t  lastVoltage;
	uint16_t  lastImpedance;
	gEstimPRO_GetLastAppliedCRTandVLTG (_device, lastCurrent, lastVoltage);
	/*
	bciwarn << "\nCurrent: "   << (int)lastCurrent
					<< "\nVoltage: "   << (int)lastVoltage
					<< "\nImpedance: " << (int)lastImpedance << endl;
	*/
	mlastCurrent = lastCurrent;
	mlastVoltage = lastVoltage;
}