////////////////////////////////////////////////////////////////////////////////
// $Id: CursorFeedbackTask.cpp 7464 2023-06-30 15:04:08Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The CursorFeedback Application's Task filter.
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
#include "CursorFeedbackTask.h"

#include "FeedbackScene2D.h"
#include "FeedbackScene3D.h"
#include "FileUtils.h"
#include "Localization.h"
#include "buffers.h"

#include <algorithm>

#define CURSOR_POS_BITS "12"
const int cCursorPosBits = ::atoi(CURSOR_POS_BITS);
Vector3D a = { 0 };
bool skipHoldA;
bool skipHoldB;
RGBColor CbeforeCapture, CafterCapture, CduringFailure, TbeforeCapture, TafterCapture, TduringFailure;
RegisterFilter(CursorFeedbackTask, 3);

CursorFeedbackTask::CursorFeedbackTask()
	: mpFeedbackScene(NULL), mRenderingQuality(0), mpMessage(NULL), mCursorColorFront(RGBColor::Red),
	mCursorColorBack(RGBColor::Red), mRunCount(0), mTrialCount(0), mCurFeedbackDuration(0), mMaxFeedbackDuration(0),
	mCursorSpeedX(1.0), mCursorSpeedY(1.0), mCursorSpeedZ(1.0), mrWindow(Window())
{
	BEGIN_PARAMETER_DEFINITIONS
		"Application:Window int RenderingQuality= 1 0 0 1 "
		" // rendering quality: 0: low, 1: high (enumeration)",

		"Application:3DEnvironment floatlist CameraPos= 3 50 50 150 % % "
		" // camera position vector in percent coordinates of 3D area",
		"Application:3DEnvironment floatlist CameraAim= 3 50 50 50 % % "
		" // camera aim point in percent coordinates",
		"Application:3DEnvironment int CameraProjection= 0 0 0 2 "
		" // projection type: 0: flat, 1: wide angle perspective, 2: narrow angle perspective (enumeration)",
		"Application:3DEnvironment floatlist LightSourcePos= 3 50 50 100 % % "
		" // light source position in percent coordinates",
		"Application:3DEnvironment int LightSourceColor= 0x808080 % % "
		" // light source RGB color (color)",
		"Application:3DEnvironment int WorkspaceBoundaryColor= 0xffffff 0 % % "
		" // workspace boundary color (0xff000000 for invisible) (color)",
		"Application:3DEnvironment string WorkspaceBoundaryTexture= images/grid.bmp % % % "
		" // path of workspace boundary texture (inputfile)",
		"Application:Cursor int SVAmovement= 0 0 0 3"
		" // select 0 for position based movement, 1 for velocity, and 2 for acceleration",
		"Application:Cursor float AccelerationFactor= .1 0 -3.0 3.0"
		" // multiplicative factor for acceleration rate with -3.0 to 3.0 bounds",
		"Application:Cursor float VelocityFactor= 1.0 0 -3.0 3.0"
		" // multiplicative factor for velocity with -3.0 to 3.0 bounds",
		"Application:Cursor int invertedXaxis= 0 0 0 1"
		" // invert the X axis of the joystick input",
		"Application:Cursor int invertedYaxis= 0 0 0 1"
		" // invert the Y axis of the joystick input",
		"Application:Cursor float ControlSignalXOffset= 0 0 -1 1"
		" // Offset for x axis control signal",
		"Application:Cursor float ControlSignalYOffset= 0 0 -1 1"
		" // Offset for Y axis control signal",
		"Application:Cursor float CursorWidth= 10 10 0.0 % "
		" // feedback cursor width in percent of screen width",
		"Application:Cursor int CursorColorFront= 0x808080 % % % "
		" // cursor color when it is at the front of the workspace (color)",
		"Application:Cursor int CursorColorBack= 0xffff00 % % % "
		" // cursor color when it is in the back of the workspace (color)",
		"Application:Cursor string CursorTexture= images/marble.bmp % % %"
		" // path of cursor texture (inputfile)",
		"Application:Cursor floatlist CursorPos= 3 50 50 50 % % "
		" // cursor starting position",
		"Application:Sequencing float rewardPulseLength= 100ms % 0 % "
		" // Length of reward in milliseconds",

		"Application:Targets matrix Targets= "
		" 9 "                                                       // rows
		" [pos%20x pos%20y pos%20z radius] " // columns
		"  50  50  50  5 " //this is the center target
		"  50  75  50  5 "
		"  75  50  50  5 "
		"  50  25  50  5 "
		"  25  50  50  5 "
		"  32  68  50  5 "
		"  68  68  50  5 "
		"  32  32  50  5 "
		"  68  32  50  5 "
		" // target positions and widths in percentage coordinates",
		"Application:Targets int TargetColor= 0x00FFFF % % % "
		" // target color (color)",
		"Application:Targets string TargetTexture= % % % % "
		" // path of target texture (inputfile)",
		"Application:Targets int TestAllTargets= 0 0 0 1 "
		" // test all targets for cursor collision? "
		"0: test only the visible current target, "
		"1: test all targets "
		"(enumeration)",
		END_PARAMETER_DEFINITIONS

		BEGIN_STATE_DEFINITIONS
		"CursorPosX " CURSOR_POS_BITS " 0 0 0",
		"CursorPosY " CURSOR_POS_BITS " 0 0 0",
		"CursorPosZ " CURSOR_POS_BITS " 0 0 0",
		"distanceToTarget 16 0 0 0",
		"timeOnTarget 16 0 0 0",
		"rewardPulseLength 16 0 0 0",
		"Rewarding 1 0 0 0",
		END_STATE_DEFINITIONS

		LANGUAGES "German",
		BEGIN_LOCALIZED_STRINGS
		"Timeout", "Inaktiv",
		"Be prepared ...", "Achtung ...",
		END_LOCALIZED_STRINGS

		GUI::Rect rect = { 0.5f, 0.4f, 0.5f, 0.6f };
	mpMessage = new TextField(mrWindow);
	mpMessage->SetTextColor(RGBColor::Lime)
		.SetTextHeight(0.8f)
		.SetColor(RGBColor::Gray)
		.SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
		.SetObjectRect(rect);
}

CursorFeedbackTask::~CursorFeedbackTask()
{
	delete mpFeedbackScene;
}

void CursorFeedbackTask::OnPreflight(const SignalProperties& /*Input*/) const
{
	const char* vectorParams[] = {
		"CameraPos",
		"CameraAim",
		"LightSourcePos",
		"CursorPos",
	};
	for (size_t i = 0; i < sizeof(vectorParams) / sizeof(*vectorParams); ++i)
		if (Parameter(vectorParams[i])->NumValues() != 3)
			bcierr << "Parameter \"" << vectorParams[i] << "\" must have 3 entries";

	Parameter("WorkspaceBoundaryColor");
	const char* colorParams[] = {
		"CursorColorBack", "CursorColorFront", "TargetColor", "LightSourceColor",
		// WorkspaceBoundaryColor may be NullColor to indicate invisibility
	};
	for (size_t i = 0; i < sizeof(colorParams) / sizeof(*colorParams); ++i)
		if (RGBColor(Parameter(colorParams[i])) == RGBColor(RGBColor::NullColor))
			bcierr << "Invalid RGB value in " << colorParams[i];

	bool showTextures = (Parameter("RenderingQuality") > 0);
	const char* texParams[] = {
		"CursorTexture",
		"TargetTexture",
		"WorkspaceBoundaryTexture",
	};
	for (size_t i = 0; i < sizeof(texParams) / sizeof(*texParams); ++i)
	{
		std::string filename = Parameter(texParams[i]);
		if (showTextures && !filename.empty())
		{
			int w, h;
			std::vector<GLubyte> ignored;
			if (!buffers::loadWindowsBitmap(FileUtils::AbsolutePath(filename), w, h, ignored))
				bcierr << "Invalid texture file \"" << filename << "\""
				<< " given in parameter " << texParams[i];
		}
	}

	CbeforeCapture = 0xAF5519;
	CafterCapture = 0xBABABA;
	CduringFailure = 0x2D2D2D;
	TbeforeCapture = 0x05A6A6;
	TafterCapture = 0x93CE2D;
	TduringFailure = 0xCC0505;
	if (Parameter("NumberTargets") > Parameter("Targets")->NumRows())
		bcierr << "The Targets parameter must contain at least NumberTargets "
		<< "target definitions. "
		<< "Currently, Targets contains " << Parameter("Targets")->NumRows()
		<< " target definitions, and NumberTargets is " << Parameter("NumberTargets");

	if (Parameter("MaxMovementTime").InSampleBlocks() <= 0)
		bcierr << "MaxMovementTime must be greater 0";

	OptionalParameter("EnforceFixation");
}

void CursorFeedbackTask::OnInitialize(const SignalProperties& /*Input*/)
{
	// Cursor speed in pixels per signal block duration:
	float feedbackDuration = Parameter("MaxMovementTime").InSampleBlocks();
	// On average, we need to cross half the workspace during a trial.
	mCursorSpeedX = 100.0 / feedbackDuration / 2;
	mCursorSpeedY = 100.0 / feedbackDuration / 2;
	mCursorSpeedZ = 100.0 / feedbackDuration / 2;
	mCursorWidth = Parameter("CursorWidth");
	/*if (Parameter("CursorWidth") <= (Parameter("Targets")->Value(5, 3)))
		mCursorWidth = Parameter("Targets")->Value(5, 3);*/
		// TODO: if the cursor width is smaller than the target width then you will need to hold the cursor in the target based on target size

	mAccelerationFactor = Parameter("AccelerationFactor");
	mVelocityFactor = Parameter("VelocityFactor");
	mMinDelayTime = static_cast<int>(Parameter("MinDelayTime").InSampleBlocks()); //CENTEROUT
	mMaxDelayTime = static_cast<int>(Parameter("MaxDelayTime").InSampleBlocks()); //CENTEROUT
	CSXOffset = Parameter("ControlSignalXOffset");
	CSYOffset = Parameter("ControlSignalYOffset");
	mMinReactionTime = static_cast<int>(Parameter("MinReactionTime").InSampleBlocks()); //CENTEROUT
	mMaxReactionTime = static_cast<int>(Parameter("MaxReactionTime").InSampleBlocks()); //CENTEROUT
	mMaxMovementTime = static_cast<int>(Parameter("MaxMovementTime").InSampleBlocks());
	mStartTime = static_cast<int>(Parameter("StartTime").InSampleBlocks());

	mFeedbackTime = static_cast<int>(Parameter("FeedbackTime").InSampleBlocks()); //CENTEROUT

	skipHoldA = Parameter("skipHoldA");
	skipHoldB = Parameter("skipHoldB");
	//float rewardPulseWidth = (Parameter("rewardPulseLength").InMilliseconds());
	//State("rewardPulseLength") = rewardPulseWidth;

	mCursorColorFront = RGBColor(Parameter("CursorColorFront"));
	mCursorColorBack = RGBColor(Parameter("CursorColorBack"));

	int renderingQuality = Parameter("RenderingQuality");
	if (renderingQuality != mRenderingQuality)
	{
		mrWindow.Hide();
		mRenderingQuality = renderingQuality;
	}
	delete mpFeedbackScene;
	if (renderingQuality == 0)
		mpFeedbackScene = new FeedbackScene2D(mrWindow);
	else
		mpFeedbackScene = new FeedbackScene3D(mrWindow);
	mpFeedbackScene->Initialize();
	mpFeedbackScene->SetCursorColor(mCursorColorFront);
	mpFeedbackScene->SetCursorVisible(true); // CENTER OUT: Set so that cursor is always visible on the screen

	mrWindow.Show();
	DisplayMessage("");
}

void CursorFeedbackTask::OnStartRun()
{
	++mRunCount;
	mTrialCount = 0;
	mTrialStatistics.Reset();
	AppLog << "Run #" << mRunCount << " started";
	State("rewardPulseLength") = (Parameter("rewardPulseLength").InMilliseconds());
	DisplayMessage("");
}

void CursorFeedbackTask::OnStopRun()
{
	AppLog << "Run " << mRunCount << " finished: " << mTrialStatistics.Total() << " trials, " << mTrialStatistics.Hits()
		<< " hits, " << mTrialStatistics.Invalid() << " invalid.\n";
	int validTrials = mTrialStatistics.Total() - mTrialStatistics.Invalid();
	if (validTrials > 0)
		AppLog << (200 * mTrialStatistics.Hits() + 1) / validTrials / 2 << "% correct, " << mTrialStatistics.Bits()
		<< " bits transferred.\n";
	AppLog << "=====================" << std::endl;

	DisplayMessage("");
}

void CursorFeedbackTask::OnTrialBegin()
{
	a = { 0 };
	++mTrialCount;
	AppLog.Screen << "Trial #" << mTrialCount << ", target: " << State("TargetCode") << std::endl;

	DisplayMessage("");
	//RGBColor targetColor = RGBColor(Parameter("TargetColor"));
	for (int i = 0; i < mpFeedbackScene->NumTargets(); ++i)
	{
		mpFeedbackScene->SetTargetColor(CbeforeCapture, i);
		//mpFeedbackScene->SetTargetVisible(State("TargetCode") == i + 1, i);

	}

	mpFeedbackScene->SetTargetVisible(true, 0);
}

void CursorFeedbackTask::OnTrialEnd()
{
	DisplayMessage("");
	for (int i = 0; i < mpFeedbackScene->NumTargets(); ++i)
		mpFeedbackScene->SetTargetVisible(false, i);

}

void CursorFeedbackTask::OnFeedbackBegin()
{
	mCurFeedbackDuration = 0;

}

void CursorFeedbackTask::OnFeedbackEnd()
{
	if (State("Failure") == 0)
	{
		//mTrialStatistics.Update(State("TargetCode"), State("ResultCode"));
		AppLog.Screen << "-> trial success" << std::endl;
		State("Success") = 1;
	}
	else if (State("Failure") == 1)
	{
		AppLog.Screen << "-> Failure to capture" << std::endl;
		mTrialStatistics.UpdateInvalid();
	}
	else if (State("Failure") == 2)
	{
		AppLog.Screen << "-> Hold A failure" << std::endl;
		mTrialStatistics.UpdateInvalid();
	}
	else if (State("Failure") == 3)
	{
		AppLog.Screen << "-> Delay failure" << std::endl;
		mTrialStatistics.UpdateInvalid();
	}
	else if (State("Failure") == 4)
	{
		AppLog.Screen << "-> Min reaction time failure" << std::endl;
		mTrialStatistics.UpdateInvalid();
	}
	else if (State("Failure") == 5)
	{
		AppLog.Screen << "-> Max reaction time failure" << std::endl;
		mTrialStatistics.UpdateInvalid();
	}
	else if (State("Failure") == 6)
	{
		AppLog.Screen << "-> Movement failure" << std::endl;
		mTrialStatistics.UpdateInvalid();
	}
	else if (State("Failure") == 7)
	{
		AppLog.Screen << "-> Hold B failure" << std::endl;
		mTrialStatistics.UpdateInvalid();
	}
	else {
		AppLog.Screen << "-> Unkown failure" << std::endl;
		mTrialStatistics.UpdateInvalid();
	}
}

void CursorFeedbackTask::DoPreRun(const GenericSignal& ControlSignal, bool& doProgress) // Phase 0
{
	CursorMove(ControlSignal);


	State("distanceToTarget") = mpFeedbackScene->CursorTargetDistance(0); //central target
	if (mpFeedbackScene->TargetHit(0)) {
		State("Failure") = 0;
		mpFeedbackScene->SetCursorColor(CafterCapture);
		mpFeedbackScene->SetTargetColor(TafterCapture, 0);
		doProgress = true;
	}
	else
		if (mCurFeedbackDuration > mStartTime) {
			State("Failure") = 1; //PreRun Failure to capture
			mpFeedbackScene->SetCursorColor(CduringFailure);
			mpFeedbackScene->SetTargetColor(TduringFailure, 0);
		}


	doProgress = doProgress || (++mCurFeedbackDuration > mStartTime) || (mpFeedbackScene->TargetHit(0));
}

void CursorFeedbackTask::DoHoldA(const GenericSignal& ControlSignal, bool& doProgress) //Phase 1
{

	CursorMove(ControlSignal);


	State("distanceToTarget") = mpFeedbackScene->CursorTargetDistance(0); //hold on central target
	if (!skipHoldA) {
		if (State("distanceToTarget") > mCursorWidth) {
			State("Failure") = 2; //HoldA "HOLD A FAILURE"
			mpFeedbackScene->SetCursorColor(CduringFailure);
			mpFeedbackScene->SetTargetColor(TduringFailure, 0);
		}
		else {
			mpFeedbackScene->SetCursorColor(CafterCapture);
			//mpFeedbackScene->SetTargetColor(RGBColor::Green, 0);
		}
	}
	else {
		doProgress = true;
	}
	
	doProgress = doProgress ||(++mCurFeedbackDuration > State("mHoldATime")) || (State("Failure") != 0);
}

void CursorFeedbackTask::DoDelay(const GenericSignal& ControlSignal, bool& doProgress) //phase 2
{
	if (mCurFeedbackDuration <= 2) {
		mpFeedbackScene->SetTargetVisible(true, State("TargetCode") - 1);
	}

	CursorMove(ControlSignal);

	State("distanceToTarget") = mpFeedbackScene->CursorTargetDistance(0); //hold on central target

	if (State("distanceToTarget") > mCursorWidth) {
		State("Failure") = 3; //Delay "DELAY FAILURE"
		mpFeedbackScene->SetCursorColor(CduringFailure);
		mpFeedbackScene->SetTargetColor(TduringFailure, 0);
	}
	doProgress = (++mCurFeedbackDuration > State("mDelayTime"));
	doProgress = doProgress || (State("Failure") != 0);
}

void CursorFeedbackTask::DoReaction(const GenericSignal& ControlSignal, bool& doProgress) //Phase 3
{
	if (mCurFeedbackDuration <= 2) {
		mpFeedbackScene->SetTargetVisible(false, 0);
	}

	CursorMove(ControlSignal);

	State("distanceToTarget") = mpFeedbackScene->CursorTargetDistance(0); //hold on central target
	if ((mCurFeedbackDuration < mMinDelayTime) && State("distanceToTarget") > mCursorWidth) {
		mpFeedbackScene->SetCursorColor(CduringFailure);
		mpFeedbackScene->SetTargetColor(TduringFailure, State("TargetCode") - 1);
		bciout << "Cursor moved too quickly";
		State("Failure") = 4;//Reaction "Min Reaction Time Failure" //CHECKME
	}
	if ((mCurFeedbackDuration >= (mMaxReactionTime /*this needs to be changed*/ - 3)) && State("distancetotarget") < mCursorWidth) { //if curtime is over max delay time and if on target 


		mpFeedbackScene->SetCursorColor(CduringFailure);
		mpFeedbackScene->SetTargetColor(TduringFailure, State("TargetCode") - 1);
		bciout << "cursor moved too slowly";
		State("Failure") = 5; //reaction "max reaction time failure"
	}
	if (mCurFeedbackDuration < mMaxReactionTime) {
		State("Failure") = 6;
	}
	if (mpFeedbackScene->TargetHit(State("TargetCode") - 1)) {
		State("Failure") = 0;
		mpFeedbackScene->SetCursorColor(CafterCapture);
		mpFeedbackScene->SetTargetColor(TafterCapture, State("TargetCode") - 1);
		doProgress = true;
	}




	doProgress = doProgress || (++mCurFeedbackDuration > mMaxReactionTime);

}
void CursorFeedbackTask::DoMovement(const GenericSignal& ControlSignal, bool& doProgress) //Phase 4
{
	CursorMove(ControlSignal);

	State("distanceToTarget") = mpFeedbackScene->CursorTargetDistance(State("TargetCode") - 1); //outer target
	if (skipHoldB == 0) {
		if (State("distanceToTarget") <= mCursorWidth) {
			State("Failure") = 0;
			mpFeedbackScene->SetCursorColor(CafterCapture);
			mpFeedbackScene->SetTargetColor(TafterCapture, State("TargetCode") - 1);
			doProgress = true;
		}
		else {
			mpFeedbackScene->SetCursorColor(CduringFailure);
			mpFeedbackScene->SetTargetColor(TduringFailure, State("TargetCode") - 1);
			if (State("Failure") == 0)
				State("Failure") = 6; //Movement "Movement Failure"

		}
	}
	else {
		doProgress = true;
	}

	doProgress = doProgress || (++mCurFeedbackDuration > mStartTime);
	doProgress = doProgress || (State("Failure") != 0);

}
void CursorFeedbackTask::DoHoldB(const GenericSignal& ControlSignal, bool& doProgress) //Phase 5
{
	CursorMove(ControlSignal);


	State("distanceToTarget") = mpFeedbackScene->CursorTargetDistance(State("TargetCode") - 1);//hold on outer target
	if (!skipHoldB) {
		if (State("distanceToTarget") > mCursorWidth) {
			State("Failure") = 7; //HoldB "Hold B failure"
			mpFeedbackScene->SetCursorColor(CduringFailure);
			mpFeedbackScene->SetTargetColor(TduringFailure, State("TargetCode") - 1);

		}
		else {
			mpFeedbackScene->SetCursorColor(CafterCapture);
			mpFeedbackScene->SetTargetColor(TafterCapture, State("TargetCode") - 1);
		}
	}
	else {
		doProgress = true;
	}
	doProgress = doProgress || (++mCurFeedbackDuration > State("mHoldBTime"));
	doProgress = doProgress || (State("Failure") != 0);

}
void CursorFeedbackTask::DoITI(const GenericSignal& ControlSignal, bool& doProgress) //Phase 6
{
	/*for (int i = 0; i < mpFeedbackScene->NumTargets(); ++i)
		mpFeedbackScene->SetTargetVisible(false, i);*/
	if (State("Success") == 1 && ++mCurFeedbackDuration < Parameter("rewardPulseLength").InSampleBlocks()) State("Rewarding") = 1; else State("Rewarding") = 0;
}
// Access to graphic objects
void CursorFeedbackTask::MoveCursorTo(float inX, float inY, float inZ)
{
	// Adjust the cursor's color according to its z position:
	float z = inZ / 100;
	RGBColor color = z * mCursorColorFront + (1 - z) * mCursorColorBack;
	mpFeedbackScene->SetCursorColor(color);
	Vector3D pos = { inX, inY, inZ };
	mpFeedbackScene->SetCursorPosition(pos);
}

void CursorFeedbackTask::DisplayMessage(const std::string& inMessage)
{
	if (inMessage.empty())
	{
		mpMessage->Hide();
	}
	else
	{
		mpMessage->SetText(" " + inMessage + " ");
		mpMessage->Show();
	}
}
// Access to control of cursor
void CursorFeedbackTask::CursorMove(const GenericSignal& ControlSignal)
{
	Vector3D pos = mpFeedbackScene->CursorPosition(), v = { 0 };
	if (Parameter("SVAmovement") == 0) { //position
		if (ControlSignal.Channels() > 0 && !IsNaN(ControlSignal(0, 0))) {
			if (Parameter("invertedXaxis") == 0) {
				pos.x = (ControlSignal(0, 0) + 1.5) * (100 / 3);
			}
			else {
				pos.x = -(ControlSignal(0, 0) - 1.5) * (100 / 3);
			}
		}
		if (ControlSignal.Channels() > 1 && !IsNaN(ControlSignal(0, 0))) {
			if (Parameter("invertedYaxis") == 0) {
				pos.y = (ControlSignal(1, 0) + 1.5) * (100 / 3);
			}
			else {
				pos.y = -(ControlSignal(1, 0) - 1.5) * (100 / 3);
			}
		}
		if (ControlSignal.Channels() > 2 && !IsNaN(ControlSignal(0, 0)))
			pos.z = (ControlSignal(2, 0) + 1.5) * (100 / 3);
	}
	else if (Parameter("SVAmovement") == 1) { //Velocity
		if (ControlSignal.Channels() > 0 && !IsNaN(ControlSignal(0, 0))) {
			if (Parameter("invertedXaxis") == 0) {
				v.x = (mCursorSpeedX * ControlSignal(0, 0)) + CSXOffset;
			}
			else {
				v.x = -mCursorSpeedX * ControlSignal(0, 0) + CSXOffset;
			}
		}
		if (ControlSignal.Channels() > 1 && !IsNaN(ControlSignal(0, 0))) {
			if (Parameter("invertedYaxis") == 0) {
				v.y = mCursorSpeedY * ControlSignal(1, 0) + CSYOffset;
			}
			else {
				v.y = -mCursorSpeedY * ControlSignal(1, 0) + CSYOffset;
			}
		}
		if (ControlSignal.Channels() > 2 && !IsNaN(ControlSignal(0, 0)))
			v.z = mCursorSpeedZ * ControlSignal(2, 0);
		pos = pos + v * mVelocityFactor;
		v = pos - mpFeedbackScene->CursorPosition();
	}
	else { //Acceleration
		if (ControlSignal.Channels() > 0 && !IsNaN(ControlSignal(0, 0))) {
			if (Parameter("invertedXaxis") == 0) {
				v.x = (mCursorSpeedX * ControlSignal(0, 0)) + CSXOffset;
			}
			else {
				v.x = -mCursorSpeedX * ControlSignal(0, 0) + CSXOffset;
			}
		}
		if (ControlSignal.Channels() > 1 && !IsNaN(ControlSignal(0, 0))) {
			if (Parameter("invertedYaxis") == 0) {
				v.y = mCursorSpeedY * ControlSignal(1, 0) + CSYOffset;
			}
			else {
				v.y = -mCursorSpeedY * ControlSignal(1, 0) + CSYOffset;
			}
		}
		if (ControlSignal.Channels() > 2 && !IsNaN(ControlSignal(0, 0)))
			v.z = mCursorSpeedZ * ControlSignal(2, 0);
		a = v + a;
		pos = pos + mAccelerationFactor * a;
		v = pos - mpFeedbackScene->CursorPosition();
	}
	// Restrict cursor movement to the inside of the bounding box:
	float r = mpFeedbackScene->CursorRadius();
	pos.x = std::max(r, std::min(100 - r, pos.x)), pos.y = std::max(r, std::min(100 - r, pos.y)),
		pos.z = std::max(r, std::min(100 - r, pos.z));

	if (Parameter("SVAmovement") == 0) {
		mpFeedbackScene->SetCursorPosition(pos);
	}
	else {
		mpFeedbackScene->SetCursorPosition(pos);
		mpFeedbackScene->SetCursorVelocity(v);
	}
	const float coordToState = ((1 << cCursorPosBits) - 1) / 100.0;
	State("CursorPosX") = static_cast<int>(pos.x * coordToState);
	State("CursorPosY") = static_cast<int>(pos.y * coordToState);
	State("CursorPosZ") = static_cast<int>(pos.z * coordToState);

}