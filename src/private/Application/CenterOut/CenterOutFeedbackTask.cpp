////////////////////////////////////////////////////////////////////////////////
// $Id: CenterOutFeedbackTask.cpp 7464 2023-06-30 15:04:08Z mellinger $
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
#include "CenterOutFeedbackTask.h"

#include "BCIException.h"
#include "PrecisionTime.h"

int mMinHoldA;
int mMaxHoldA;
int mMinHoldB;
int mMaxHoldB;
int mMinDelayTime;
int mMaxDelayTime;
int mMinReactionTime;
int mMaxReactionTime;
//int mMaxMovementTime;

CenterOutFeedbackTask::CenterOutFeedbackTask()
	: mPhase(none), mBlocksInPhase(0), mBlocksInRun(0), mITIDuration(0), mCurrentTrial(0), mNumberOfTrials(0), mMinRunLength(0),
	mBlockRandSeq(RandomNumberGenerator), mholdA(0), mholdB(0), mDelayTime(0), mReactionTime(0), mFeedbackTime(0)
{
	BEGIN_PARAMETER_DEFINITIONS
		//"Application:Sequencing float StartTime= 1000ms % 0 % " //CENTEROUT
			//" // Minimum time to hold A",
		"Application:Sequencing int skipHoldA= 0 % 0 % " //CENTEROUT
		" // Disable Hold A requirement.",
		"Application:Sequencing int skipHoldB= 0 % 0 % " //CENTEROUT
		" // Disable Hold B requirement.",
		"Application:Sequencing float StartTime= 1000ms % 0 % " //CENTEROUT
		" // Minimum time to hit A",
		"Application:Sequencing float MinHoldATime= 318ms % 0 % " //CENTEROUT
		" // Minimum time to hold A",
		"Application:Sequencing float MaxHoldATime= 530ms % 0 % " //CENTEROUT
		" // Maximum time to hold A",
		"Application:Sequencing float MinDelayTime= 205ms % 0 % " //CENTEROUT
		" // minimum delay period before the outer target appears",
		"Application:Sequencing float MaxDelayTime= 410ms % 0 % " //CENTEROUT
		" // maximum delay period before the outer target appears",
		"Application:Sequencing float MinReactionTime= 61ms % 0 % " //CENTEROUT
		" // Minimum time to reasonably expect reaction",
		"Application:Sequencing float MaxReactionTime= 900ms % 0 % " //CENTEROUT
		" // Maximum time to reasonably expect reaction",
		"Application:Sequencing float MaxMovementTime= 415ms % 0 % " //CENTEROUT
		" // The time taken for the cursor to stop touching the central target and start touching the outer target",
		"Application:Sequencing float MinHoldBTime= 295ms % 0 % " //CENTEROUT
		" // Minimum time to hold cursor on the outertarget",
		"Application:Sequencing float MaxHoldBTime= 530ms % 0 % " //CENTEROUT
		" // Maximum time to hold cursor on the outertarget",
		"Application:Sequencing float FeedbackTime= 250ms % 0 % " //CENTEROUT
		" // The time period of trial feedback",
		"Application:Sequencing float MaxMovementTime= 415ms % 0 % " //CENTEROUT
		" // The time period of trial MovementTime",

		"Application:Sequencing float ITIDuration= 500ms 1s 0 % "
		" // duration of inter-trial interval",
		"Application:Sequencing float MinRunLength= 120s 120s 0 % "
		" // minimum duration of a run; if blank, NumberOfTrials is used",
		"Application:Sequencing int NumberOfTrials= % 0 0 % "
		" // number of trials; if blank, MinRunLength is used",
		"Application:Targets int NumberTargets= 9 9 0 255 "
		" // number of targets",
		"Application:Targets intlist TargetSequence= 0    1 % % "
		" // fixed sequence in which targets should be presented (leave empty for random)",
		END_PARAMETER_DEFINITIONS

		BEGIN_STATE_DEFINITIONS
		"TargetCode 8 0 0 0",
		"ResultCode 8 0 0 0",
		"Feedback   1 0 0 0",
		"PauseApplication 1 0 0 0",
		"mHoldATime 16 0 0 0", //CENTEROUT
		"mHoldBTime 16 0 0 0", //CENTEROUT
		"mDelayTime 16 0 0 0", //CENTEROUT
		"mReactionTime 16 0 0 0",
		"Phase 8 0 0 0",//CENTEROUT
		"Failure 4 0 0 0",
		"Success 1 0 0 0",
		//"mMovementTime 16 0 0 0", //CENTEROUT
		//"mFeedbackTime 16 0 0 0" //CENTEROUT

		END_STATE_DEFINITIONS
}

CenterOutFeedbackTask::~CenterOutFeedbackTask()
{
	Halt();
}

void CenterOutFeedbackTask::Preflight(const SignalProperties& Input, SignalProperties& Output) const
{
	OptionalParameter("RandomSeed");
	State("Running");

	if (!std::string(Parameter("MinRunLength")).empty() && !std::string(Parameter("NumberOfTrials")).empty())
		bcierr << "Either MinRunLength or NumberOfTrials must be blank";

	int nTargets = Parameter("NumberTargets");
	for (int i = 0; i < Parameter("TargetSequence")->NumValues(); i++)
	{
		int val = Parameter("TargetSequence")(i);
		if (val < 1 || val > nTargets)
			bcierr << "TargetSequence contains illegal value " << val << ": values must be integers from 1 to "
			<< nTargets << " (because NumberTargets=" << nTargets << ")";
	}

	OptionalState("FixationViolated");

	bcidbg(2) << "Event: Preflight";
	OnPreflight(Input);
	Output = Input;
}

void CenterOutFeedbackTask::Initialize(const SignalProperties& Input, const SignalProperties& Output)
{
	ApplicationBase::Initialize(Input, Output);

	mBlockRandSeq.SetBlockSize(Parameter("NumberTargets"));

	mFixedTargetSequence.clear();
	for (int i = 0; i < Parameter("TargetSequence")->NumValues(); i++)
		mFixedTargetSequence.push_back(Parameter("TargetSequence")(i));

	mNumberOfTrials = 0;
	if (!std::string(Parameter("NumberOfTrials")).empty())
		mNumberOfTrials = Parameter("NumberOfTrials");
	mStartTime = static_cast<int>(Parameter("StartTime").InSampleBlocks());
	mMinHoldA = static_cast<int>(Parameter("MinHoldATime").InSampleBlocks()); //CENTEROUT
	mMaxHoldA = static_cast<int>(Parameter("MaxHoldATime").InSampleBlocks()); //CENTEROUT

	mMinHoldB = static_cast<int>(Parameter("MinHoldBTime").InSampleBlocks()); //CENTEROUT
	mMaxHoldB = static_cast<int>(Parameter("MaxHoldBTime").InSampleBlocks()); //CENTEROUT

	mMinDelayTime = static_cast<int>(Parameter("MinDelayTime").InSampleBlocks()); //CENTEROUT
	mMaxDelayTime = static_cast<int>(Parameter("MaxDelayTime").InSampleBlocks()); //CENTEROUT

	mMinReactionTime = static_cast<int>(Parameter("MinReactionTime").InSampleBlocks()); //CENTEROUT
	mMaxReactionTime = static_cast<int>(Parameter("MaxReactionTime").InSampleBlocks()); //CENTEROUT

	//mMaxMovementTime = static_cast<int>(Parameter("MaxMovementTime").InSampleBlocks()); //CENTEROUT

	mFeedbackTime = static_cast<int>(Parameter("FeedbackTime").InSampleBlocks()); //CENTEROUT

	mITIDuration = static_cast<int>(Parameter("ITIDuration").InSampleBlocks());
	mMinRunLength = static_cast<long long>(Parameter("MinRunLength").InSampleBlocks());
	bcidbg(2) << "Event: Initialize";
	OnInitialize(Input);
}

void CenterOutFeedbackTask::StartRun()
{
	mBlocksInRun = 0;
	mBlocksInPhase = 0;
	mCurrentTrial = 0;
	mPhase = ITI;
	bcidbg(2) << "Event: StartRun";
	State("Failure") = 0;
	OnStartRun();
}

void CenterOutFeedbackTask::StopRun()
{
	if (State("Feedback"))
	{
		bcidbg(2) << "Event: FeedbackEnd";
		OnFeedbackEnd();
		State("Feedback") = false;
	}
	if (State("TargetCode") != 0)
	{
		bcidbg(2) << "Event: TrialEnd";
		OnTrialEnd();
		State("TargetCode") = 0;
	}
	State("ResultCode") = 0;
	mPhase = none;

	bcidbg(2) << "Event: StopRun";
	OnStopRun();
}

void CenterOutFeedbackTask::Halt()
{
	bcidbg(2) << "Event: Halt";
	OnHalt();
}

void CenterOutFeedbackTask::Resting(const GenericSignal& Input, GenericSignal& Output)
{
	Output = Input;
}

void CenterOutFeedbackTask::Process(const GenericSignal& Input, GenericSignal& Output)
{

	if (State("PauseApplication"))
	{
		Resting(Input, Output);
		return;
	}

	bool doProgress = true;
	while (doProgress)
	{
		switch (mPhase)
		{
		case preRun: // phase 0

			doProgress = (mBlocksInPhase >= mStartTime);
			State("Phase") = 0;
			DoPreRun(Input, doProgress);
			break;

		case holdA: //phase 1

			doProgress = (mBlocksInPhase >= State("mHoldATime"));
			State("Phase") = 1;

			DoHoldA(Input, doProgress);

			break;

		case delay: //phase 2

			doProgress = (mBlocksInPhase >= State("mDelayTime"));
			State("Phase") = 2;
			DoDelay(Input, doProgress);
			break;

		case reaction: //phase 3
			State("Phase") = 3;
			doProgress = (mBlocksInPhase >= mMaxReactionTime);
			DoReaction(Input, doProgress);
			break;

		case movement: //phase 4

			State("Phase") = 4;
			doProgress = (mBlocksInPhase >= static_cast<int>(Parameter("MaxMovementTime").InSampleBlocks()));
			DoMovement(Input, doProgress);
			break;
		case holdB: //phase 5

			State("Phase") = 5;
			doProgress = (mBlocksInPhase >= State("mHoldBTime"));
			DoHoldB(Input, doProgress);
			break;

		case ITI: //phase 6
			State("mHoldATime") = rand() % (mMaxHoldA - mMinHoldA + 1) + mMinHoldA;
			State("mHoldBTime") = rand() % (mMaxHoldB - mMinHoldB + 1) + mMinHoldB;
			State("mDelayTime") = rand() % (mMaxDelayTime - mMinDelayTime + 1) + mMinDelayTime;
			State("Phase") = 6;
			doProgress = (mBlocksInPhase >= mITIDuration);
			DoITI(Input, doProgress);
			break;

		case failState: //Only when failed
			State("Phase") = 7;
			State("Success") = 0;

			break;
		case postRun: //End of experiment
			//State("timeOnTarget") = 0;
			State("Phase") = 8;
			doProgress = false;
			break;

		default:
			throw bcierr << "Unknown phase value: " << mPhase;
		}
		if (doProgress)
		{
			mBlocksInPhase = 0;
			switch (mPhase)
			{

			case ITI:
				OnTrialEnd();
				if (mFixedTargetSequence.size())
					State("TargetCode") = mFixedTargetSequence[mCurrentTrial % mFixedTargetSequence.size()];
				else
					State("TargetCode") = mBlockRandSeq.NextElement();
				++mCurrentTrial;
				bcidbg << "Event: ITI";
				State("Success") = 0;
				OnTrialBegin();
				OnFeedbackBegin();
				mPhase = preRun;
				break;
			case preRun: {
				State("Feedback") = true;
				OnFeedbackBegin();
				//State("TargetCode") = 0;
				//State("ResultCode") = 0;
				bcidbg << "Event: preRun";
				State("timeOnTarget") = 0;

				if (State("Failure") == 0) {
					mPhase = holdA;
				}
				else {
					mPhase = failState;
				}
				break;
			}


			case holdA: {
				//State("Feedback") = true;
				OnFeedbackBegin();
				bcidbg << "Event: holdA";
				State("timeOnTarget") = 0;
				//State("ResultCode") = 0;
				if (State("Failure") == 0) {
					mPhase = delay;
				}
				else {
					mPhase = failState;
				}
				break;
			}

			case delay: {
				OnFeedbackBegin();
				bcidbg << "Event: delay";
				State("timeOnTarget") = 0;
				State("ResultCode") = 0;
				if (State("Failure") == 0) {
					mPhase = reaction;
				}
				else {
					mPhase = failState;
				}
				break;
			}
			case reaction: {
				OnFeedbackBegin();
				bcidbg << "Event: reaction";
				State("timeOnTarget") = 0;
				if (State("Failure") == 0) {
					mPhase = movement;
				}
				else {
					mPhase = failState;
				}
				break;
			}
			case movement: {
				OnFeedbackBegin();
				bcidbg << "Event: movement";
				State("timeOnTarget") = 0;
				if (State("Failure") == 0) {
					mPhase = holdB;
				}
				else {
					mPhase = failState;
				}
				break;
			}
			case holdB: {
				OnFeedbackBegin();
				bcidbg << "Event: holdB";
				State("timeOnTarget") = 0;
				State("TargetCode") = 0;
				State("ResultCode") = 0;

				bcidbg(3) << "Blocks in Run: " << mBlocksInRun << "/" << mMinRunLength;
				bool runFinished = false;
				if (mNumberOfTrials > 0)
					runFinished = (mCurrentTrial >= mNumberOfTrials);
				else
					runFinished = (mBlocksInRun >= mMinRunLength);
				if (runFinished)
				{
					doProgress = false;
					State("Running") = false;
					mPhase = postRun;
				}
				else
				{
					if (State("Failure") == 0) {
						OnFeedbackEnd();
						
						State("Success") = 1;
						State("ResultCode") = State("TargetCode");
						mPhase = ITI;
					}
					else {
						mPhase = failState;
					}
				}
				break;
			}

			case failState: {
				OnFeedbackBegin();
				//State("Feedback") = false;
				OnFeedbackEnd();
				State("Failure") = 0;
				//bcidbg << "Event: failState";
				mPhase = ITI;
				break;
			}



			default:
				throw bcierr << "Unknown phase value: " << mPhase;
			}
		}
	}
	++mBlocksInRun;
	++mBlocksInPhase;
	Output = Input;
}