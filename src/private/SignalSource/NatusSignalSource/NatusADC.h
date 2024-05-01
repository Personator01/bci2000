////////////////////////////////////////////////////////////////////////////////
// Authors: 
// Description: NatusADC header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_NATUSADC_H  // makes sure this header is not included more than once
#define INCLUDED_NATUSADC_H

#include "BufferedADC.h"
#include "NatusClient.h"

class NatusADC : public GenericADC
{
 public:
  NatusADC();
  ~NatusADC();
  virtual void Publish() override;
  virtual void AutoConfig(const SignalProperties&) override;
  //void OnAutoConfig() override;
  //void OnPreflight(SignalProperties& Output) const override;
  virtual void Preflight(const SignalProperties&, SignalProperties&) const override;

  virtual void Initialize(const SignalProperties&, const SignalProperties&) override;
 
  virtual void Halt() override;

  //void OnPublish() override;
  //void OnAutoConfig() override;
  //void OnPreflight( SignalProperties& Output ) const override;
  //void OnInitialize( const SignalProperties& Output ) override;
  //void OnStartAcquisition() override;
  virtual void Process(const GenericSignal&,GenericSignal&) override;
  //void DoAcquire( GenericSignal& Output ) override;
  //void OnStopAcquisition() override;


 private:
  // Use this space to declare any NatusADC-specific methods and member variables you'll need
  int mNumberOfSignalChannels;
  NatusClient client;
  float* mbuffer;
  uint32_t bufferSize;
 

};

#endif // INCLUDED_NATUSADC_H
