////////////////////////////////////////////////////////////////////////////////
// Authors: Markus Adamek, Alexander Belsten (adamek/belsten @neurotechcenter.org)
// Description: CortecADC header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_CORTECADC_H  // makes sure this header is not included more than once
#define INCLUDED_CORTECADC_H

static_assert(BICAPI_VERSION == 200 || BICAPI_VERSION == 230,
    "Supported BIC API versions are 200 and 230.");

#include "BufferedADC.h"
#include "bicapi.imports.h"

#include "Thread.h"
#include "Expression/Expression.h"
#include <deque>
#include <chrono>
#include <concurrent_queue.h>
#include <fstream>

typedef int PulseID;

using namespace cortec::implantapi;

class CortecADC : public BufferedADC
{
public:
  CortecADC();
  ~CortecADC();
  //BufferedADC
  void Publish();
  void OnAutoConfig() override;
  void OnPreflight(SignalProperties&) const override;
  void OnInitialize(const SignalProperties&) override;
  void OnStartAcquisition() override;
  void DoAcquire(GenericSignal&) override;
  void OnStopAcquisition() override;
  //GenericADC
  void Preflight(const SignalProperties&, SignalProperties&) const override;
  void Initialize(const SignalProperties&, const SignalProperties&) override;
  void Process(const GenericSignal&, GenericSignal&) override;
  void StartRun() override;
  void StopRun() override;

private:
  void GetImpedances();
  std::ofstream FindFile(std::string filePath, int fileNum);

  bool ConstructStimulationPulse(
      int    pulseID,
      double amplitude, 
      double pulse_duration, 
      double dead_zone_0, 
      double dead_zone_1
  );
  bool InitializeStimulationFunction(IStimulationCommand* cmd, 
    int pulseID,
      std::set<uint32_t> source_loc, 
      std::set<uint32_t> destination_loc, 
      int repetitions);
  bool ConstructStimulationCommand(
      Expression  trigger,
      int         pulseID,
      std::set<uint32_t> source_loc,
      std::set<uint32_t> destination_loc,
      int         repetitions,
      double      train_freq,
      int         train_reps
  );
  std::set<uint32_t> BCI2000ListToSet(ParamRef ch_parm, bool subtract_one) const;
  void ClearPulseMap();
  void ClearCommandList();
  bool ConnectToImplant();
  void DisconnectFromImplant();
  bool IsConnected();

  //BufferedADC additions
  HANDLE mDataLock;
  int    mElementSize;
  int    mBufferSizeMs;

  void StartStimulation(IStimulationCommand* cmd, uint8_t functionID);

  std::unique_ptr<IStimulationCommandFactory> mStimCommandFactory;
  std::map<PulseID, IStimulationFunction*>    mStimPulses;
  struct StimCommands {
    Expression exp;
    IStimulationCommand* cmd;
    bool triggered;
    uint8_t functionId; //only used in Persistant Function mode
  };
  std::list<StimCommands> mStimCommands;
  std::unique_ptr<IImplantFactory>            mImplantFactory;
  std::unique_ptr<IImplant>                   mImplant;
  std::unique_ptr<CImplantInfo>               mImplantInfo;
  std::set<uint32_t> mReferenceLocations;
  bool mIsMeasuring,
      mTimeOutOccurred,
      mStimulationEnabled,
      mStimulationTriggered,
      mRecording;
  int mStimId;
  StimulationMode mStimMode;

  int mNumberOfSignalChannels;
  int mBlockSize_ms;
  int mSamplingRate;

  //impedance
  GenericVisualization mVis;

  // debug vars
  int mprintcounterpush,
      mprintcounterpop;

  // data vars
  double* mPrevSample;
  concurrency::concurrent_queue<double*> mDataQueue;

  // static state vars
  uint16_t mImplantVoltage;
  uint16_t mImplantHumidity;
  uint16_t mImplantControlValue;
  uint16_t mImplantPrimaryCoilCurrent;
  uint16_t mImplantTemperature;
  bool     mImplantStimulation;
  bool     mImplantStimulationBursts;


  class CortecDataListener : public cortec::implantapi::IImplantListener
  {
  public:
      CortecDataListener(CortecADC* parent);
      void ResetCounter() { this->mResetCounter = true; }
  private:
    CortecADC* parent;
    bool       mResetCounter;
    uint32_t   mSampleCounter;
    void onData(                      const std::vector<CSample>* samples ) override;
    void onStimulationStateChanged(   const bool isStimulating            ) override;
    void onMeasurementStateChanged(   const bool isMeasuring              ) override;
    void onConnectionStateChanged(    const connection_info_t& info       ) override;
    void onImplantVoltageChanged(     const double voltage                ) override;
    void onPrimaryCoilCurrentChanged( const double currentMilliA          ) override;
    void onImplantControlValueChanged(const double controlValue           ) override;
    void onTemperatureChanged(        const double temperature            ) override;
    void onHumidityChanged(           const double humidity               ) override;
    void onError(                     const std::exception& err           ) override;
    void onDataProcessingTooSlow(                                         ) override;
    void onStimulationFunctionFinished(const uint64_t numFinishedFunctions) override;

#if(BICAPI_VERSION==230)
    void onLastStimulationFunctionId(const uint16_t id) override {}
#endif

    void onRfQualityUpdate(const int8_t antennaQualitydBm,
        const uint16_t validFramesReceived, const uint16_t invalidHandshake,
        const uint16_t radioCrcErrors, const uint16_t otherRxErrors,
        const uint32_t rxQueueOverflows, const uint32_t txQueueOverflows) override;
    void onChannelUpdate(const uint8_t rfChannel) override;

  };
  friend class CortecDataListener;
  CortecDataListener mListener;
};

#endif // INCLUDED_CORTECADC_H
