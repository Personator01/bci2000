#include "DeviceConfiguration.h"
using namespace std;


/**************NEURO OMEGA SPECIFIC DEFINITIONS**************************/
//everything below needs to be copied if you are implementing a new device
//set-up will vary depending on the set-up of the stimulator

bool
NeuroOmegaStimulator::validStimulator() const
{
  if (!OptionalParameter("EnableNeuroOmegaStim", false))
  {
    bcierr << "CCEPFilter error: You have chosen the NeuroOmega as your stimulator, yet "
      << "NeuroOmega stimulation is not enabled. Change \"Stimulator\" to \"auto\" or enable the device." << endl;
    return false;
  }
  Parameter("SampleBlockSize");
  Parameter("SamplingRate");
  Parameter("StimulationConfigurations");
  Parameter("StimulationTriggers");
  Parameter("RecordingChIDs");
  return true;
}

// get max stimulation length
string
NeuroOmegaStimulator::getEpochLength() const
{
  double maxLength = 0;
  for (int i = 0; i < Parameter("StimulationConfigurations")->NumColumns(); i++)
  {
    double l = Parameter("StimulationConfigurations")(7, i);
    if (l > maxLength)
      maxLength = l;
  }
  bcidbg << "NeuroOmega: max time " << maxLength << endl;
  return to_string(maxLength) + "s";
}

void
NeuroOmegaStimulator::initializeStimulator()
{
  //go thru RecordingChIDs
  std::map<int, int> idsToChannels; // {id, channel index}
  auto ids = Parameter("RecordingChIDs");
  for (int i = 0; i < ids->NumRows(); i++)
  {
    idsToChannels.insert(std::make_pair(static_cast<int>(ids(i, 0).ToNumber()), i));
  }

  //go thru StimulationTriggers
  auto trigs = Parameter("StimulationTriggers");
  for (int c = 0; c < trigs->NumColumns(); c++)
  {
    Expression* exp = new Expression(trigs(0, c));
    set<int> s;
    //make sure it is not in map
    auto it = triggeredChannels.find(exp);
    if (it == triggeredChannels.end())
    {
      //insert, then find pointer to it
      triggeredChannels.insert({ exp, s });
      it = triggeredChannels.find(exp);
    }
    //channel(s)
    ParamRef v = trigs(2, c);

    //insert all values
    for (int n = 0; n < v->NumValues(); n++)
    {
      //convert chs to between 0 - numChannels
      int ch = idsToChannels[v(n, 0)];
      //add to list of channels for trigger
      (it->second).insert(ch);
    }
  }
}

//input: Generic Signal input
//output: channel number that is being trigged (0 - numChannels), or could be multiple channels
set<int>
NeuroOmegaStimulator::getChannels(const GenericSignal* input)
{
  for (auto it = triggeredChannels.begin(); it != triggeredChannels.end(); it++)
  {
    Expression* e = it->first;
    if (e->Evaluate(input))
    {
      //only one expression should be true
      bcidbg << "Stim " << e->AsString() << " is true" << endl;
      return it->second;
    }
  }

  set<int> v;
  return v;
}

NeuroOmegaStimulator::~NeuroOmegaStimulator()
{
  //clean-up
  for (auto it = triggeredChannels.begin(); it != triggeredChannels.end(); it++)
  {
    delete it->first;
  }
}