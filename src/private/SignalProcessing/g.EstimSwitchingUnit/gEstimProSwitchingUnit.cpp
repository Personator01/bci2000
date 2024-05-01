////////////////////////////////////////////////////////////////////////////////
// Authors: Alex Belsten belsten@neurotechcenter.org
// Description: gEstimSwitchingUnit implementation
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

#include "gEstimProSwitchingUnit.h"
#include "BCIStream.h"
#include "ProgressBarVis.h"


#define bcidbgOn
#define SELFTEST_PROGRESS_DONE 3

using namespace std;

RegisterFilter( gEstimProSwitchingUnit, 2.C3 );

enum TriggerType {
  MulitpleSoftware = 0,
  OneSoftware      = 1,
  Digital          = 2
};

// this class is a container for channel switching info (how many channels, what channels).
class StimulationSetting
{
public:
  StimulationSetting ();
  ~StimulationSetting ();

  void GetNChannels (uint8_t &_n_pos, uint8_t &_n_neg) const;
  bool GetChannels  (uint8_t **_pos,   uint8_t **_neg)   ;
  bool SetChannels (const ParamRef& _pos_vec, const ParamRef _neg_vec);
  
private:
  uint8_t  n_ch_pos,
           n_ch_neg;
  uint8_t *ch_pos,
          *ch_neg;
};

StimulationSetting::StimulationSetting () :
  ch_pos (NULL),
  ch_neg (NULL),
  n_ch_pos(0),
  n_ch_neg(0){
}

StimulationSetting::~StimulationSetting ()
{
  if (!ch_pos) delete[] ch_pos;
  if (!ch_neg) delete[] ch_neg;
  ch_pos = NULL;
  ch_neg = NULL;
}

void 
StimulationSetting::GetNChannels (uint8_t& _n_pos, uint8_t& _n_neg) const
{
  _n_pos = n_ch_pos;
  _n_neg = n_ch_neg;
}

bool 
StimulationSetting::GetChannels (uint8_t** _pos, uint8_t** _neg)
{
  if (!ch_pos || !ch_neg)
    return false;
  *_pos = ch_pos;
  *_neg = ch_neg;
  return true;
}

bool 
StimulationSetting::SetChannels (const ParamRef& _pos_param, const ParamRef _neg_param)
{
 
  if (!ch_pos) delete[] ch_pos;
  if (!ch_neg) delete[] ch_neg;
  ch_pos = NULL;
  ch_neg = NULL;

  // Note: the elements of the column can be "" (empty string)
  // this is because matlab treats a single cell as as a value
  // and the convert_bciparm script doesnt make the element of the 
  // StimConfig a embedded list. Appending an empty string in matlab
  // solves this but requires more checking for those elements here.
  n_ch_pos = 0;
  n_ch_neg = 0;
  for (int i = 0; i < _pos_param->NumRows (); i++)
    if (_pos_param (i, 0) != "")
      n_ch_pos += 1;

  for (int i = 0; i < _neg_param->NumRows (); i++)
    if (_neg_param (i, 0) != "")
      n_ch_neg += 1;

  // allocate the memory for the channel list
  ch_pos = new uint8_t[n_ch_pos];
  ch_neg = new uint8_t[n_ch_neg];

  // assign values in the arrays, but keep seperate variable for 
  // indexing them, in case empty cell is not in last row
  int ch_idx = 0;
  for (int i = 0; i < _pos_param->NumRows (); i++)
    if (_pos_param (i, 0) != "")
    {
      ch_pos[ch_idx] = (uint8_t)(_pos_param (i, 0) - 1);
      ch_idx += 1;
    }
  ch_idx = 0;
  for (int i = 0; i < _neg_param->NumRows (); i++)
    if (_neg_param (i, 0) != "")
    {
      ch_neg[ch_idx] = (uint8_t)(_neg_param (i, 0) - 1);
      ch_idx += 1;
    }
      
  return true;
}
 
gEstimProSwitchingUnit::gEstimProSwitchingUnit() :
  mEnable(false),
  mSwitched(false) {
}

gEstimProSwitchingUnit::~gEstimProSwitchingUnit()
{
  mDevice.Close ();
}

void
gEstimProSwitchingUnit::Publish()
{
 BEGIN_PARAMETER_DEFINITIONS
   "Filtering:gEstimSwitchingUnit int ActivateSwitchingUnit= 0 0 0 1 // (boolean)",
   "Filtering:gEstimSwitchingUnit int PerformSelfTest= 0 0 0 1 // (boolean)",
   "Filtering:gEstimSwitchingUnit int DeviceID= 0 0 0 % // ID of device to connect to",
   "Filtering:gEstimSwitchingUnit int Trigger= 0 0 0 1 // "
     "Type of trigger for connected state is 0 Software, 1 Digital (enumeration)",
   "Filtering:gEstimSwitchingUnit:gEstimProSwitchingUnit matrix StimConfig= "
     "{ SwitchExpression Stim+%20ch Stim-%20ch } " // row labels
     "{ StimulusState1 StimulusState2 StimulusState3 } "          // column labels
     "StimulusCode==1  StimulusCode==2  StimulusCode==3 "         // Switch Expression
     "{ list 3 1 2 3 } { list 3 2 3 4 } { list 3 3 4 5 } "        // Stim+ ch nested lists
     "{ list 3 6 7 8 } { list 3 7 8 9 } { list 3 8 9 10 }",       // Stim- ch nested lists
 END_PARAMETER_DEFINITIONS
 
 BEGIN_STATE_DEFINITIONS
   "EstimSU 1 0 0 0",
 END_STATE_DEFINITIONS
 
}

void
gEstimProSwitchingUnit::AutoConfig (const SignalProperties& Input)
{
  if ((bool)Parameter("ActivateSwitchingUnit"))
  { 
    Parameter ("DeviceID") = 0;
    int ampID = ActualParameter ("DeviceID");
    //char initState = (char)Parameter("PerformSelfTest")  | ((char)Parameter("PerformImpedanceCheck") << 1);
    bool initState = Parameter("PerformSelfTest");
    if (!mDevice.Open (ampID, initState, Input))
      return;
  }
}

void
gEstimProSwitchingUnit::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  Output = Input;
  if ((bool)OptionalParameter ("ActivateSwitchingUnit", false))
  {
    GenericSignal preflightSignals (Input);
    State ("EstimSU");
    PreflightCondition (Parameter ("StimConfig")->NumRows () == 3);
    if (Parameter ("Trigger") == 1)
      PreflightCondition (Parameter ("StimConfig")->NumColumns () == 1);

    // for each stimuli
    for (int stim = 0; stim < Parameter ("StimConfig")->NumColumns (); stim++ )
    {
      // check that expression is valid
      Expression switchExpression = Parameter ("StimConfig")(0, stim).ToString();
      switchExpression.Compile ();
      switchExpression.Evaluate (&preflightSignals);
      // assert that the stim+ and stim- lists are not empty
      int pstim_list = 1;
      int nstim_list = 2;
      // check that there are channels (rows) in lists
      if (Parameter ("StimConfig")(pstim_list, stim)->NumRows () == 0)
        bcierr << "Switching Unit Error: Empty Stim+ ch list for stimulus " << stim + 1 << " in StimConfig. "
        << "Add channels (rows) to this list" << endl;
      if (Parameter ("StimConfig")(nstim_list, stim)->NumRows () == 0)
        bcierr << "Switching Unit Error: Empty Stim- ch list for stimulus " << stim + 1 << " in StimConfig. "
        << "Add channels (rows) to this list" << endl;
      // check that there is only one column
      if (Parameter ("StimConfig")(pstim_list, stim )->NumColumns () != 1)
        bcierr << "Switching Unit Error: Stim+ ch list for stimulus " << stim + 1 << " in StimConfig must only have one column" << endl;
      if (Parameter ("StimConfig")(nstim_list, stim)->NumColumns () != 1)
        bcierr << "Switching Unit Error: Stim- ch list for stimulus " << stim + 1 << " in StimConfig must only have one column" << endl;

      // for each val in the stim+ list assert that 0 < val < output.ch (1-based indexing)
      for (int idx = 0; idx < Parameter ("StimConfig")(pstim_list, stim)->NumRows (); idx++)
      {
        if (Parameter ("StimConfig")(pstim_list, stim)(idx, 0) != "")
          if (Parameter ("StimConfig")(pstim_list, stim)(idx, 0) > Output.Channels () ||
              Parameter ("StimConfig")(pstim_list, stim)(idx, 0) < 1)
            bcierr << " Switching Unit Error: Stim+ ch list for stimulus " << stim + 1 << " in StimConfig has an invalid value at row "
                   << idx + 1 << ". Valid entries are in range 1 to " << Output.Channels () << endl;
      }

      // for each val in the stim- list assert that 0 < val < output.ch (1-based indexing)
      for (int idx = 0; idx < Parameter ("StimConfig")(nstim_list, stim)->NumRows (); idx++)
        if (Parameter ("StimConfig")(nstim_list, stim)(idx, 0) != "")
          if (Parameter ("StimConfig")(nstim_list, stim)(idx, 0) > Output.Channels () ||
              Parameter ("StimConfig")(nstim_list, stim)(idx, 0) < 1)
            bcierr << " Switching Unit Error: Stim- ch list for stimulus " << stim + 1 << " in StimConfig has an invalid value at row "
                   << idx + 1 << ". Valid entries are in range 1 to " << Output.Channels () << endl;

      // assert that no val is in both stim- and stim+
      for (int nidx = 0; nidx < Parameter ("StimConfig")(nstim_list, stim)->NumRows (); nidx++) 
      {
        for (int pidx = 0; pidx < Parameter ("StimConfig")(pstim_list, stim)->NumRows (); pidx++)
        {
          if (Parameter ("StimConfig")(nstim_list, stim)(nidx, 0) != "" &&
            Parameter ("StimConfig")(pstim_list, stim)(pidx, 0) != "")
          {
            if (Parameter ("StimConfig")(nstim_list, stim)(nidx, 0) == Parameter ("StimConfig")(pstim_list, stim)(pidx, 0))
              bcierr << "Switching Unit Error: Duplicate value (" << Parameter ("StimConfig")(nstim_list, stim)(nidx, 0)
              << ") in Stim+ ch list and Stim- ch list for stimulus state " << stim + 1 << endl;
          }
        }
      }
    }
  }
}

void
gEstimProSwitchingUnit::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  mEnable = (bool)OptionalParameter ("ActivateSwitchingUnit", false);
  if (mEnable)
  {
    if (Parameter ("Trigger") == 1)
      mTriggerType = Digital;
    else if (Parameter ("StimConfig")->NumColumns () == 1)
      mTriggerType = OneSoftware;
    else
      mTriggerType = MulitpleSoftware;

    mDevice.ClearStimConfig ();
    mDevice.LoadStimConfig (Parameter ("StimConfig"), mTriggerType != MulitpleSoftware);
  }
}

void
gEstimProSwitchingUnit::StartRun()
{
  if (mEnable)
  {
    // The user has just pressed "Start" (or "Resume")
    uint8_t state;
    mDevice.GetState (state);

    if (mTriggerType != MulitpleSoftware)
    {
      // go to prepared state
      if (state != GSU_GetConstants ()->STATE_PREPARED)
        mDevice.SetState (GSU_GetConstants ()->STATE_PREPARED);

    }
    else
    {
      // make sure the device is in the ready state
      if (state != GSU_GetConstants ()->STATE_READY)
        mDevice.SetState (GSU_GetConstants ()->STATE_READY);
    }
    mSwitched = false;
  }
}

void
gEstimProSwitchingUnit::Process( const GenericSignal& Input, GenericSignal& Output )
{
  Output = Input; 
  if (mEnable)
  {
    if (mTriggerType != Digital)
    {
      if (mSwitched)
      {
        if (mDevice.StopSwitch(Input, mTriggerType))
        {
          State ("EstimSU") = 0;
          mSwitched = false;
        }
      }
      else 
        if (mDevice.TriggerSwitch (Input, mTriggerType))
        {
          State ("EstimSU") = 1;
          mSwitched = true;
        }
    }
  }
}

void
gEstimProSwitchingUnit::StopRun()
{
  if (mEnable)
  {
    uint8_t state;
    mDevice.GetState (state);
    if (state == GSU_GetConstants ()->STATE_ACTIVE)
    {
      mDevice.SetState (GSU_GetConstants ()->STATE_READY);
      mDevice.UpdateOutput (false);
      State ("EstimSU") = 0;
      mSwitched = false;
    }
  }
}

gEstimProSwitchingUnit::gEstimPSUDevice::gEstimPSUDevice () :
  mAmpID (0),
  mVis ("SUFILTER"),
  mpProgressBar(nullptr) {
  mSwitchSettings = SwitchSettings ();
}

bool
gEstimProSwitchingUnit::gEstimPSUDevice::Open (uint8_t ampID, char initState, const SignalProperties& Input)
{
  // Does all error handling and notifications
  bool res;
  mAmpID = ampID;
  uint8_t nDevices = 1;
  bool open = false;
  res = ParseError(GSU_GetNumberOfAvailableDevices(&nDevices), "GetNumberOfAvailableDevices");
  if (nDevices < 1)
  {
    uint8_t state;
    //no error??? device already connected :)
    if (GSU_GetState(mAmpID, &state) == GSU_GetConstants() -> ERR_NONE)
      open = true;
    else
    {
      bcierr << "No switching unit devices connected to system. Check that device is on and connected." << endl;
      return false;
    }
  }
  if (!open)
  {
    if (!ParseError(GSU_Open(mAmpID), "Open"))
    {
      bcierr << "Switching Unit Error: Unable to open device." << endl;
      return false;
    }
    mVis.Send(CfgID::WindowTitle, "Switching Unit Information");
    mVis.Send("========= This window will populate with information from the switching unit =========\n");
  }
  else
  {
    mVis.Send("========= Switching unit information =========\n");
  }
  //self-test
  if (initState & 1)
  {
    bciwarn << "Switching Unit: Attempting self-test." << endl;
    if (!PerformSelfTest ())
    {
      bcierr << "Switching Unit Error: Unable to pass self test. Unplug all channels"
             << " and the stimulator from front of device, and try again." << endl;
      return false;
    }
    bciwarn << "Switching Unit: self-test passed." << endl;
  }
  else
    SetState(GSU_GetConstants()->STATE_READY);

  return true;
}

bool
gEstimProSwitchingUnit::gEstimPSUDevice::Close ()
{
  return ParseError(GSU_Close (mAmpID));
}

bool 
gEstimProSwitchingUnit::gEstimPSUDevice::GetState (uint8_t &state) const
{
  if (!ParseError (GSU_GetState (mAmpID, &state), "GetState"))
  {
    bcierr << "Switching Unit Error: unable to get state" << endl;
    return false;
  }
  return true;
}

bool
gEstimProSwitchingUnit::gEstimPSUDevice::SetState (uint8_t state)
{
  if (!ParseError (GSU_SetState (mAmpID, state), "SetState"))
  {
    bcierr << "Switching Unit Error: unable to set state" << endl;
    return false;
  }
  return true;
}

bool
gEstimProSwitchingUnit::gEstimPSUDevice::LoadStimConfig (const ParamRef& StimConfig, bool OneConfig)
{
  // does error handling and notification
  for (int stim = 0; stim < StimConfig->NumColumns (); stim++)
  {
    int pstim_list = 1;
    int nstim_list = 2;

    Expression stimCodeKey = (Expression)StimConfig (0, stim);
    StimulationSetting stimSettingValue = StimulationSetting ();
    if (!stimSettingValue.SetChannels (StimConfig (pstim_list, stim), StimConfig (nstim_list, stim)))
    {
      bcierr << "Switching Unit Error: Cannot load channel switching parameters to device." << endl;
      return false;
    }
    // add to map
    Setting switchSetting = Setting (stimCodeKey, stimSettingValue);
    mSwitchSettings.push_back (switchSetting);
  }
  // if there is only one stim, upload the config to the device
  if (OneConfig)
  {
    uint8_t n_pos;
    uint8_t n_neg;
    uint8_t* ch_pos = NULL;
    uint8_t* ch_neg = NULL;
    SwitchSettings::iterator itr = mSwitchSettings.begin ();
    itr->second.GetNChannels (n_pos, n_neg);
    itr->second.GetChannels (&ch_pos, &ch_neg);
    if (!ParseError (GSU_SetStimulationSetting (mAmpID, ch_pos, n_pos, ch_neg, n_neg), "SetStimulationSetting"))
    {
      bcierr << "Switching Unit Error: Cannot load channel switching parameters to device." << endl;
      return false;
    }
  }
  return true;
}

bool
gEstimProSwitchingUnit::gEstimPSUDevice::TriggerSwitch (const GenericSignal& Input, const TriggerType& trigType)
{
  SwitchSettings::iterator itr = mSwitchSettings.begin ();
  if (trigType == OneSoftware)
  {
    if (itr->first.Evaluate(&Input))
    {
      this->SetState (GSU_GetConstants ()->STATE_ACTIVE);
      mCurrentSwitch = itr;

      // output to console what channels are switched
      this->UpdateOutput ();

      return true;
    }
  }
  else
  {
    while (itr != mSwitchSettings.end ())
    {
      if (itr->first.Evaluate (&Input))
      {
        mCurrentSwitch = itr;

        uint8_t state;
        this->GetState (state);
        if (state != GSU_GetConstants ()->STATE_READY)
          this->SetState (GSU_GetConstants ()->STATE_READY);
        uint8_t n_pos;
        uint8_t n_neg;
        uint8_t* ch_pos = NULL;
        uint8_t* ch_neg = NULL;
        itr->second.GetNChannels (n_pos, n_neg);
        itr->second.GetChannels (&ch_pos, &ch_neg);
        if (!ParseError (GSU_SetStimulationSetting (mAmpID, ch_pos, n_pos, ch_neg, n_neg), "SetStimulationSetting"))
        {
          bcierr << "Switching Unit Error: Cannot load channel switching parameters to device." << endl;
          return false;
        }
        this->SetState (GSU_GetConstants ()->STATE_PREPARED);
        this->SetState (GSU_GetConstants ()->STATE_ACTIVE);

        // output to console what channels are switched 
        this->UpdateOutput ();

        return true;
      }
      itr++;
    }
  }
  return false;
}


bool
gEstimProSwitchingUnit::gEstimPSUDevice::StopSwitch (const GenericSignal& Input, const TriggerType& trigType)
{
  if (!(mCurrentSwitch->first.Evaluate (&Input)))
  {
    this->SetState (GSU_GetConstants ()->STATE_PREPARED);
    // update window
    UpdateOutput (false);
    return true;
  }
  return false;
}

void 
gEstimProSwitchingUnit::gEstimPSUDevice::ClearStimConfig ()
{
  mSwitchSettings.clear ();
  ParseError (GSU_ClearStimulationSettingList (mAmpID), "ClearStimulationSettingList");
}

bool
gEstimProSwitchingUnit::gEstimPSUDevice::PerformSelfTest ()
{
  bool res = true;
  
  if (!this->SetState(GSU_GetConstants ()->STATE_SELFTEST))
  {
    bcierr << "Switching Unit Error: Unable to start self test" << endl;
    return false;
  }
  //start progress bar
  SetUpProgressBar("Self-test Progress");
  
  
  double progress = 0; // 0 - 1, indicating progress
  uint8_t  step = 0;
  uint16_t substep = 0;
  int count = 0;
  while (progress != 1 && count < 150)
  {
    ThreadUtils::SleepForMs (200);
    res = ParseError (GSU_GetSelftestResult (mAmpID, &progress, &step, &substep), "GetSelftestResult");
    count += 1;
    
    //update the progress bar
    mpProgressBar->SetCurrent(progress*100);
    mpProgressBar->SendDifferenceFrame();
  }
  bcidbg << "Result of self-test: " << (int)step << endl;
  return res;
}

void
gEstimProSwitchingUnit::gEstimPSUDevice::SetUpProgressBar(string title)
{
  mpProgressBar = new ProgressBarVis();
  mpProgressBar->SetBackgroundColor(RGBColor::DkGray);
  mpProgressBar->SetForegroundColor(RGBColor::Aqua);
  mpProgressBar->SetHeight(100);
  mpProgressBar->SetWidth(400);
  mpProgressBar->Send(CfgID::WindowTitle, title);
  mpProgressBar->Send(CfgID::Visible, true);
  mpProgressBar->SetTotal(100).SetCurrent(0);
  mpProgressBar->SendReferenceFrame();
}

void
gEstimProSwitchingUnit::gEstimPSUDevice::CloseProgressBar()
{
  mpProgressBar->Send(CfgID::Visible, false);
  delete mpProgressBar;
  mpProgressBar = nullptr;
}

void 
gEstimProSwitchingUnit::gEstimPSUDevice::UpdateOutput (bool triggerSwitch)
{
  if (!triggerSwitch) 
  {
    // Just stopped a switch
    mVis.Send ("====== Switch terminated ======\n\n");
    return;
  }
  // populate a stream with info about channels that were switched
  stringstream memostream;
  uint8_t  n_positive,
           n_negative;
  uint8_t* ch_pos = NULL;
  uint8_t* ch_neg = NULL;

  memostream << "====== Switch triggered ======\n";
  // list the positive and negative channels
  mCurrentSwitch->second.GetNChannels(n_positive, n_negative);
  mCurrentSwitch->second.GetChannels (   &ch_pos,    &ch_neg);

  memostream << "Positive Channel(s):   ";
  for (int i = 0; i < n_positive-1; i++) 
  {
    memostream << (int)ch_pos[i]+1 << ", ";
  }
  memostream << (int)ch_pos[n_positive-1]+1 << "\n";
  memostream << "Negative Channel(s):  ";
  for (int i = 0; i < n_negative-1; i++)
  {
    memostream << (int)ch_neg[i]+1 << ", ";
  }
  memostream << (int)ch_neg[n_negative - 1]+1 << "\n";
  string memostring = memostream.str ();
  mVis.Send (memostring.c_str ());
}

bool 
gEstimProSwitchingUnit::gEstimPSUDevice::ParseError (GSU_ERR_TYPE err, std::string cmd) const
{
  
  if (err == GSU_GetConstants ()->ERR_NONE)  // GSU_ERR_NONE
  {
    return true;
  }
  else if (err == GSU_GetConstants ()->ERR_SEMANTIC)
  {
    bcidbg << "GSU " << cmd << " Error: error protocol" << endl;
    return false;
  }
  else if (err == GSU_GetConstants ()->ERR_SELF_TEST_FAILED)
  {
    bcidbg << "GSU " << cmd << " Error: self test failed" << endl;
    return false;
  }
  else if (err == GSU_GetConstants ()->ERR_CMD_DENIED)
  {
    bcidbg << "GSU " << cmd << " Error: cmd denied" << endl;
    return false;
  }
  else if (err == GSU_GetConstants ()->ERR_INVALID_TRANSITION)
  {
    bcidbg << "GSU " << cmd << " Error: invalid transition" << endl;
    return false;
  }
  else if (err == GSU_GetConstants ()->ERR_INVALID_DEVICE_INDEX)
  {
    bcidbg << "GSU " << cmd << " Error: invalid device index" << endl;
    return false;
  }
  else if (err == GSU_GetConstants()->ERR_INVALID_CHANNEL_INDEX)
  {
    bcidbg << "GSU " << cmd << " Error: invalid channel index, or stimulation setting is empty" << endl;
    return false;
  }
  else if (err == GSU_GetConstants ()->ERR_STATE)
  {
    bcidbg << "GSU " << cmd << " Error: error state" << endl;
    return false;
  }
  else if (err == GSU_GetConstants ()->ERR_GENERAL)
  {
    bcidbg << "GSU " << cmd << " Error: general error, no insight available" << endl;
    return false;
  }
  else if (err == GSU_GetConstants()->ERR_DONGLE_MISSING)
  {
    bcidbg << "GSU " << cmd << " Error: dongle is missing" << endl;
    return false;
  }
  else if (err == GSU_GetConstants()->ERR_DEVICE_STOPPED)
  {
    bcidbg << "GSU " << cmd << " Error: device is in stopped state" << endl;
    return false;
  }
  else if (err == GSU_GetConstants()->ERR_BUSY)
  {
    bcidbg << "GSU " << cmd << " Error: device is busy with other operations" << endl;
    return false;
  }
  else
  {
    bciwarn << "GSU " << cmd << " Unknown return value. Unable to parse." << endl;
    return true;
  }
}
