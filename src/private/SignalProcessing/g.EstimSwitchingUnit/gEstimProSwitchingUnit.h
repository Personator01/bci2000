////////////////////////////////////////////////////////////////////////////////
// Authors: Alex Belsten belsten@neurotechcenter.org
// Description: gEstimSwitchingUnit header
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

#ifndef INCLUDED_GESTIMSWITCHINGUNIT_H  // makes sure this header is not included more than once
#define INCLUDED_GESTIMSWITCHINGUNIT_H

#include <map>

#include "GenericFilter.h"
#include "Expression/Expression.h"
//#include "gEstimProSwitchingUnitAPI.h"
#include "SwitchingUnitAPI.imports.h"


class   StimulationSetting;
enum    TriggerType;
typedef uint8_t GSU_ERR_TYPE;
typedef std::pair<Expression, StimulationSetting> Setting;
typedef std::list<Setting> SwitchSettings;
class ProgressBarVis;

class gEstimProSwitchingUnit : public GenericFilter
{
 public:
  gEstimProSwitchingUnit();
  ~gEstimProSwitchingUnit();
  void Publish    (                                                              )       override;
  void AutoConfig (const SignalProperties& Input                                 )       override;
  void Preflight  (const SignalProperties& Input,       SignalProperties& Output ) const override;
  void Initialize (const SignalProperties& Input, const SignalProperties& Output )       override;
  void StartRun   (                                                              )       override;
  void Process    (const GenericSignal&    Input,          GenericSignal& Output )       override;
  void StopRun    (                                                              )       override;

 private:
  bool mEnable;
  bool mSwitched;
  TriggerType mTriggerType;

  class gEstimPSUDevice
  {
  public:
    gEstimPSUDevice ();
    bool Open     (uint8_t  ampID, char initState, const SignalProperties& Input);
    bool GetState (uint8_t& state) const;
    bool SetState (uint8_t  state);
    bool LoadStimConfig (const ParamRef& StimConfig, bool OneConfig=false);
    bool TriggerSwitch (const GenericSignal& Input, const TriggerType& trigType);
    bool StopSwitch (const GenericSignal& Input, const TriggerType& trigType);
    void ClearStimConfig ();
    bool Close ();

    void UpdateOutput (bool triggerSwitch = true);
  private:
    bool PerformSelfTest ();
    bool ParseError (GSU_ERR_TYPE err, std::string cmd="") const;
    void SetUpProgressBar(std::string title);
    void CloseProgressBar();
    
    GenericVisualization     mVis;  
    uint8_t                  mAmpID;
    SwitchSettings           mSwitchSettings;
    SwitchSettings::iterator mCurrentSwitch;
    ProgressBarVis*          mpProgressBar;
  } mDevice;
};

#endif // INCLUDED_GESTIMSWITCHINGUNIT_H
