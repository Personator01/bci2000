//DeviceConfiguration.cpp: Helper class to abstract the configuration for a specific device.
//If a new device is desired to be used, you only have to change the first constructor

#include "CCEPFilter.h"
#include "DeviceConfiguration.h"
using namespace std;


StimulatorConfiguration::StimulatorConfiguration(ParamRef deviceName)
: stimulator(nullptr)
{
  if (deviceName.ToNumber() == deviceNames::NeuroOmega)
  {
    //device specific things for Neuro Omega
    stimulator = new NeuroOmegaStimulator();
  }
  /*DECLARE ANY OTHER STIMULATORS HERE*/
  else
  {
    stimulator = new DefaultStimulator();
  }
}

StimulatorConfiguration::~StimulatorConfiguration()
{
  if (stimulator != nullptr)
  {
    delete stimulator;
  }
}

set<int>
StimulatorConfiguration::getChannels(const GenericSignal* input)
{
  return stimulator->getChannels(input);
}

bool
StimulatorConfiguration::validStimulator() const
{
  return stimulator->validStimulator();
}

string
StimulatorConfiguration::getEpochLength() const
{
  return stimulator->getEpochLength();
}

void
StimulatorConfiguration::checkParameters(const SignalProperties& Input) const
{
  return stimulator->checkParameters(Input);
}

void
StimulatorConfiguration::initializeStimulator()
{
  return stimulator->initializeStimulator();
}

//same check for every device
void
AbstractStimulator::checkParameters(const SignalProperties& Input) const
{
  Expression onsetExpression = Parameter("OnsetExpression").ToString();
  GenericSignal input(Input);
  onsetExpression.Evaluate(&input, 0);
}

/**************DEFAULT STIMULATOR DEVICE**************************/
//For any device that isn't implemented, do the bare minimum needed

bool
DefaultStimulator::validStimulator() const
{
  return true;
}

// length if parameter is auto
string
DefaultStimulator::getEpochLength() const
{
  bcidbg << "Default stimulator" << endl;
  return "500ms";
}

void
DefaultStimulator::initializeStimulator()
{
}

set<int>
DefaultStimulator::getChannels(const GenericSignal* input)
{
  set<int> v;
  return v;
}

