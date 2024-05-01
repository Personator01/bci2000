////////////////////////////////////////////////////////////////////////////////
// Authors: Brunnerlab@DESKTOP-F8KRI7F.wucon.wustl.edu
// Description: DeviceConfiguration header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_DEVICECONFIGURATION_H  // makes sure this header is not included more than once
#define INCLUDED_DEVICECONFIGURATION_H


#include "GenericFilter.h"
#include "Environment.h"
#include "Expression.h"
#include <unordered_map>
#include <unordered_set>

//Abstract class for abstract interface
class AbstractStimulator : public Environment {
public:
  virtual std::set<int> getChannels(const GenericSignal*) = 0;
  virtual bool validStimulator() const = 0;
  virtual std::string getEpochLength() const = 0;
  virtual void initializeStimulator() = 0;

  void checkParameters(const SignalProperties&) const;
};

//Default implementation
class DefaultStimulator : public AbstractStimulator {
public:
  std::set<int> getChannels(const GenericSignal*) override;
  bool validStimulator() const override;
  std::string getEpochLength() const override;
  void initializeStimulator() override;

};

//interface that is used with CCEPFilter
class StimulatorConfiguration
{
public:
  //StimulatorConfiguration();
  StimulatorConfiguration(ParamRef);
  ~StimulatorConfiguration();
  std::set<int> getChannels(const GenericSignal*);
  bool validStimulator() const;
  std::string getEpochLength() const;
  void checkParameters(const SignalProperties&) const;
  void initializeStimulator();

private:
  AbstractStimulator* stimulator;
  enum deviceNames { basic, NeuroOmega };
};

//Neuro Omega implementation
class NeuroOmegaStimulator : public AbstractStimulator {
public:
  ~NeuroOmegaStimulator();
  std::set<int> getChannels(const GenericSignal*) override;
  bool validStimulator() const override;
  std::string getEpochLength() const override;
  void initializeStimulator() override;

private:
  std::unordered_map<Expression*, std::set<int>> triggeredChannels;
};

//Declare any future implemenations here...


#endif // INCLUDED_DEVICECONFIGURATION_H
