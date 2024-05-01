////////////////////////////////////////////////////////////////////////////////
// $Id: gEstimFilter.h 7640 2023-10-03 19:53:28Z wengelhardt $
// Author: kaleb.goering@gmail.com, belsten@neurotechcenter.org
// Description:  A filter to interact with gEstim device
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
#ifndef G_ESTIM_FILTER_H
#define G_ESTIM_FILTER_H

#include "GenericFilter.h"
#include "Expression/Expression.h"
#include "Thread.h"
#include "gEstimAPI.imports.h"
#include "GenericVisualization.h"

#include <map>
#include <vector>
#include <mutex>
#include <set>

class gEstimFilter : public GenericFilter
{
public:
	// Constructor/Destructor
	gEstimFilter();
	~gEstimFilter();

	// call back function for eStim
	static void gEstimPRO_fieldsChangedCallback (PGDevice _device, uint64_t changed_flags);
	friend class gEstimThread;

 protected:
	// Virtual Interface
	void Publish() override;
	void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
	void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
	void Process( const GenericSignal& Input, GenericSignal& Output ) override;
	void StartRun() override;
	void StopRun() override;
	void Halt() override;

 private:
	// Private member methods
	void DisablePorts();
	void ChangeStimulationThread();
	void StimulationThread();
	void WaitForProcess();

	// Private member variables
	PGDevice								mEstim;
	bool										mEstimActive;
	bool										mUseEstimOn;
	Expression							mActivateExp;
	Expression							mAbortExp;
	std::vector<uint32_t>		mStimulusCodeTriggers;
	bool										mStimulating;
	static uint16_t					mlastCurrent;
	static uint16_t				  mlastVoltage;
	uint32_t mPrevStimulusCode;
	bool mNewExpression, mExpressionHasTriggered;
	bool mOldSelfTest;
	GenericVisualization mVis;
  bool mSupermodeEnabled;
  int mConfigNumber;

	//changing stim thread
	bool              mUseMultipleConfigs;
	std::thread       mChangeStimThread;
	HANDLE            mChangeStimLock;
	std::atomic_bool  mKillChangeStimThread;
	bool              mStartAfterUpload;
	int               mStimLoaded;

	//stimulation thread
	std::thread       mStimThread;
	HANDLE            mStimLock;
	std::atomic_bool  mKillStimThread;
	bool              mRunningState;

	struct StimConfig
	{
	public:
		Expression expression;
		bool       modularity;
		GAlternate polarity;
		uint16_t   pulseLength;
		uint16_t   interphaseLength;
		int16_t    pulseAmplitude;
		uint16_t   pulseFreq;
		uint16_t   numPulses;
		double     trainFreq;
		uint32_t   numTrains;
	} *mConfigToUpload;
	std::vector<StimConfig> mConfigurations;

	class gEstimThread : public Thread
	{
	public:
		gEstimThread( gEstimFilter* inFilt ) : mpFilter( inFilt ) {}
		virtual   ~gEstimThread() {}
		virtual int   OnExecute() override;
		LockableObject& GetLock() { return mDataLock; }
	private:
		Lockable<std::recursive_mutex> mDataLock;
		gEstimFilter   *mpFilter;
	} *mpgEstimThread;
};

#endif // G_ESTIM_FILTER_H
