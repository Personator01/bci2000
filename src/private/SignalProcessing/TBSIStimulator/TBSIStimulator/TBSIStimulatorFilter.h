////////////////////////////////////////////////////////////////////////////////
// Authors: 
// Description: TBSIStimulatorFilter header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_TBSISTIMULATORFILTER_H  // makes sure this header is not included more than once
#define INCLUDED_TBSISTIMULATORFILTER_H

#include "GenericFilter.h"
#include "extlib/include/TBSIapi.h"
#include "Pulse.h"
#include<string.h>

class TBSIStimulatorFilter : public GenericFilter
{
 public:
  TBSIStimulatorFilter();
  ~TBSIStimulatorFilter();
  void Publish() override;
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;



 private:
	 void OpenDevice();
	 void CloseDevice(DongleHandle);
	 void SetupStimulator(String);
	 
	 DongleHandle mhDevice;
	 bool mDeviceOpened;
	 TBSI_API_RESULT apiResult;
	 int  mStimInterval;
	 int  mCurrentBlock;
	 std::string mStimAddress;
	 bool mEnableHWTrigger;
	 bool stimAwake;

	 Pulse mPulse;
	 std::thread mSetupThread;

};

#endif // INCLUDED_TBSISTIMULATORFILTER_H
