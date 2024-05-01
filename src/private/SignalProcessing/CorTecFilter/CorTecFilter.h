////////////////////////////////////////////////////////////////////////////////
// Authors: Akshay Vyas, National Center for Adaptive Neurotechnologies
// Description: CorTecFilter header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_CORTECFILTER_H  // makes sure this header is not included more than once
#define INCLUDED_CORTECFILTER_H

#include "GenericFilter.h"
#include "bicapid.imports.h"
#include "Thread.h"
#include "Mutex.h"
#include <deque>
#include "bic3232constants.h"
#include "IStimulationFunction.h"
#include "Expression/Expression.h"

using namespace cortec::implantapi;

class CorTecFilter : public GenericFilter
{
 public:
  CorTecFilter();
  ~CorTecFilter();
  void Publish() override;
  void AutoConfig();
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;
  void Halt() override;

 private:
   // Use this space to declare any CorTecFilter-specific methods and member variables you'll need
   float* mpExampleArray;

   typedef struct _stim
   {
	   double amplitude;
	   double duration;
   } StimParameters;
   void ClearStimMap();
   void AddStimDefinitionToMap(int identifier, std::vector<StimParameters> params, int type);

   IStimulationFunction* GetStimDefinitonFromId(int identifier);

   IStimulationFunction* createStimulationFunction(const double amplitude, const uint64_t pulseWidth, const uint64_t functionPeriod);

   bool checkStimulationParameters(const double amplitude, const uint64_t pulseWidth, const int type);

   bool ConnectToImplant();
   bool IsConnected();
   //void StartStimulation(int stimId);
   typedef enum { Voltage = 0, Current = 1 } StimType;

   int mNumberOfSignalChannels;
   std::map<int, IStimulationFunction*> mStimParameters;
   std::map<int, IStimulationCommand*> mStimExecutionFunction;
   std::unique_ptr<IImplantFactory> mImplantFactory;
   std::unique_ptr<IImplant> mImplant;
   std::unique_ptr<CImplantInfo> mImplantInfo;
   std::unique_ptr<IStimulationCommandFactory> mStimCommandFactory;
   int mBlockSize;
   int mSamplingRate;
   bool mStimulating;
   Expression mActivateExp;
   Expression mAbortExp;

   Mutex queueMutex;
   std::deque<float> mData;


   int mCorTecAnode;
   int mCorTecCathode;
   double mCorTecAmplitude;
   uint64_t mCorTecPulseWidth;
   int mCorTecFunctionPeriod;
};

#endif // INCLUDED_CORTECFILTER_H
