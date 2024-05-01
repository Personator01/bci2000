////////////////////////////////////////////////////////////////////////////////
// $Id: CenterOutFeedbackTask.h 7464 2023-06-30 15:04:08Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for application modules that provide feedback in a
//   trial-based paradigm.
//   Inheriting from ApplicationBase, descendants of FeedbackTask have access
//   to the AppLog, AppLog.File and AppLog.Screen streams declared in
//   ApplicationBase.
//
//   This class performs sequencing, and dispatches GenericFilter::Process()
//   calls to its virtual member functions, depending on the current trial
//   phase. Child classes (descendants) of FeedbackTask implement event
//   handlers by overriding its virtual functions.
//
//   Sequence of events         Typical application behavior
//
//   OnPreflight
//   OnInitialize
//   OnStartRun                 display initial message
//   DoPreRun*
//   Loop {
//    OnTrialBegin              display target
//    DoPreFeedback*
//    OnFeedbackBegin           show cursor
//    DoFeedback*               update cursor position
//    OnFeedbackEnd             hide cursor, mark target as hit
//    DoPostFeedback*
//    OnTrialEnd                hide targets
//    DoITI*
//   }
//   OnStopRun                  display final message
//   OnHalt
//
//   Events marked with * will occur multiple times in a row.
//   Progress from one state to the next will occur according to the sequencing
//   parameters, or if requested by a handler via its doProgress output argument.
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
#ifndef FEEDBACK_TASK_H
#define FEEDBACK_TASK_H

#include "ApplicationBase.h"
#include "BlockRandSeq.h"

#include <vector>

class CenterOutFeedbackTask : public ApplicationBase
{
private:
	enum TaskPhases
	{
		none,
		preRun,
		holdA,
		delay,
		reaction,
		movement,
		holdB,
		ITI,
		postRun,
		failState,
	};
	// Events to be handled by FeedbackTask descendants.
	//  Events triggered by the GenericFilter interface
	virtual void OnPreflight(const SignalProperties& Input) const
	{
	}
	virtual void OnInitialize(const SignalProperties& Input)
	{
	}
	virtual void OnStartRun()
	{
	}
	virtual void OnStopRun()
	{
	}
	virtual void OnHalt()
	{
	}
	//  Events triggered during the course of a trial
	virtual void OnTrialBegin()
	{
	}
	virtual void OnTrialEnd()
	{
	}
	virtual void OnFeedbackBegin()
	{
	}
	virtual void OnFeedbackEnd()
	{
	}
	//  Dispatching of the input signal.
	//  Each call to GenericSignal::Process() is dispatched to one of these
	//  events, depending on the phase in the sequence.
	//  There, each handler function corresponds to a phase.
	//  If a handler sets the "progress" argument to true, the application's
	//  state will switch to the next phase independently of the phases' pre-set
	//  durations.
	virtual void DoPreRun(const GenericSignal&, bool& doProgress)
	{
	}
	virtual void DoHoldA(const GenericSignal&, bool& doProgress)
	{
	}
	virtual void DoDelay(const GenericSignal&, bool& doProgress)
	{
	}
	virtual void DoReaction(const GenericSignal&, bool& doProgress)
	{
	}
	virtual void DoMovement(const GenericSignal&, bool& doProgress)
	{
	}
	virtual void DoHoldB(const GenericSignal&, bool& doProgress)
	{
	}
	virtual void DoITI(const GenericSignal&, bool& doProgress)
	{
	}

protected:
	CenterOutFeedbackTask();

public:
	virtual ~CenterOutFeedbackTask();

	// Implementation of the GenericFilter interface.
	void Preflight(const SignalProperties&, SignalProperties&) const override;
	void Initialize(const SignalProperties&, const SignalProperties&) override;
	void Process(const GenericSignal&, GenericSignal&) override;
	void Resting(const GenericSignal&, GenericSignal&) override;
	void StartRun() override;
	void StopRun() override;
	void Halt() override;

private:
	int mPhase, mBlocksInPhase;
	long long mBlocksInRun;

	int mStartTime, mITIDuration, mCurrentTrial,
		mNumberOfTrials, mholdA, mholdB, mDelayTime, mReactionTime, mFeedbackTime;
	long long mMinRunLength;

	BlockRandSeq mBlockRandSeq;
	std::vector<int> mFixedTargetSequence;
};

#endif // FEEDBACK_TASK_H
