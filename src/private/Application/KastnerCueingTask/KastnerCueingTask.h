////////////////////////////////////////////////////////////////////////////////
// Authors: Jarod@MyPC
// Description: KastnerCueingTask header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_KASTNERCUEINGTASK_H  // makes sure this header is not included more than once
#define INCLUDED_KASTNERCUEINGTASK_H

#include "FeedbackTask.h"
#include "ImageStimulus.h"
#include "TextField.h"
#include "Association.h"
#include "windows.h"

#include <stdlib.h>

#include <map>
#include <chrono>
#include <random>
#include <algorithm>
#include <regex>

using namespace std;

class KastnerCueingTask : public FeedbackTask
{
	public:
	KastnerCueingTask();
	~KastnerCueingTask();
	void Publish() override;
	void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
	void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
	void StartRun() override;
	void Process( const GenericSignal& Input, GenericSignal& Output ) override;
	void StopRun() override;
	void Halt() override;


 private:
	ApplicationWindow& mrDisplay;

	enum ParameterColumnIndex
	{
		PARAM_INDEX_CAPTION, 
		PARAM_INDEX_FILENAME
	};			// index of the caption and filename in the Parameter list

	enum class PresentationModeEnum
	{
		MODE_INTRO = 1,
		MODE_PRACTICE,
		MODE_TEST
	};

	enum AnswerResponseEnum
	{
		ANSWER_NONE = 0,
		ANSWER_SAME,
		ANSWER_DIFFERENT,

		ANSWER_NUM_TOTAL
	};

	enum TestPhaseEnum
	{
		TEST_TRIAL_START = 0,
		TEST_CUE,
		TEST_POST_CUE,
		TEST_STIMULI,
		TEST_POST_STIMULI,
		TEST_AWAITING_ANSWER,

		TEST_NUM_TOTAL
	};

	enum IntroImagesEnum
	{
		INTRO_GALAXY = 1,
		INTRO_TITLE,
		INTRO_ADVENTURE,
		INTRO_ALIEN_MAIN,
		INTRO_ALIEN_NO_ARMS,
		INTRO_ALIEN_POINT_BOTTOM_LEFT,
		INTRO_ALIEN_POINT_TOP_RIGHT,
		INTRO_FESTIVE,
		INTRO_KEYBOARD_CIRCLES,
		INTRO_KEYBOARD_KEYS_CORRECT,
		INTRO_KEYBOARD_KEYS_INCORRECT,
		INTRO_KEYBOARD_FINGERS,
		INTRO_FRIEND_WAITING,
		INTRO_FRIENDS_SUCCESS,
		INTRO_FRIENDS_GO_HOME,
		INTRO_PLANET,
		INTRO_RED_CIRCLE,
		INTRO_SHIP_TANK
	};

	enum CueDirectionEnum
	{
		CUE_TOP_LEFT = 0,		// ordered clockwise from top-left to match Kastner CSV files
		CUE_TOP_RIGHT,
		CUE_BOTTOM_RIGHT,
		CUE_BOTTOM_LEFT,

		CUE_IMAGES_NUM_TOTAL
	};

	enum SearchColorEnum
	{
		SEARCH_COLOR_BLUE = 0,
		SEARCH_COLOR_GREEN,
		SEARCH_COLOR_ORANGE,
		SEARCH_COLOR_PINK,
		SEARCH_COLOR_PURPLE,
		SEARCH_COLOR_YELLOW,

		SEARCH_COLOR_NUM_TOTAL
	};

	enum SearchObjectEnum
	{
		SEARCH_OBJECT_ALIEN = 0,
		SEARCH_OBJECT_ASTEROID,
		SEARCH_OBJECT_ASTRONAUT,
		SEARCH_OBJECT_HELMET,
		SEARCH_OBJECT_MOON,
		SEARCH_OBJECT_PLANET,
		SEARCH_OBJECT_RADAR,
		SEARCH_OBJECT_ROCKET,
		SEARCH_OBJECT_SATURN,
		SEARCH_OBJECT_SAUCER,
		SEARCH_OBJECT_STAR,
		SEARCH_OBJECT_SUN,
		SEARCH_OBJECT_TELESCOPE,

		SEARCH_OBJECT_NUM_TOTAL
	};

	enum TrialTypeEnum
	{
		ATTENTION_VALID = 0,
		ATTENTION_INVALID,
		MEMORY_VALID,
		MEMORY_INVALID,

		TRIAL_TYPE_NUM_TOTAL
	};

	enum ListVsRandomTrialsEnum
	{
		TRIALS_LIST = 1,
		TRIALS_RANDOM = 2
	};

	int objectIdx = SEARCH_OBJECT_TELESCOPE;	// temp variable used in Train mode for debugging

	// trial variables
	const static int NUM_OBJECTS_PER_TRIAL = 4;
	std::vector<int> mArrayColorIndices = { 0, 1, 2, 3, 4, 5 };
	std::vector<int> mArrayObjectIndices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
	double mPercentValid = 0.80;				// 80% valid trials
	double mPercentAttention = 0.50;			// 50% attention trials

	struct TrialDefinition
	{
		TrialTypeEnum trialType;
		CueDirectionEnum cueDirection;
		SearchColorEnum searchColor[NUM_OBJECTS_PER_TRIAL];
		SearchObjectEnum searchObject[NUM_OBJECTS_PER_TRIAL];
		int targetIndex;
	};

	// trial definition list
	vector<TrialDefinition> mTrialDefList;
	TrialDefinition mTrialDefCurrent;
	ListVsRandomTrialsEnum mListVsRandom;		// TODO probably don't actually need this
	
	// field for instruction text
	TextField mMessage;
	
	// intro images
	SetOfStimuli mIntroStimuliSet;								// collects all the simulus images to easily garbage collect
	map<IntroImagesEnum, string> mIntroImagesCaptionMap;		// maps an image index to the corresponding caption string
	map<int, ImageStimulus *> mIntroImagesStimuliMap;			// maps an image index to the corresponding ImageStimulus object

	// cue images
	map<CueDirectionEnum, string> mCueImagesCaptionMap;			// maps an image index to the corresponding caption string
	map<int, ImageStimulus *> mCueImagesStimuliMap;				// maps an image index to the corresponding ImageStimulus object
	ImageStimulus* mCueStimuliArray[CUE_IMAGES_NUM_TOTAL];

	// search object variables
	SetOfStimuli mSearchStimuliSet;
	ImageStimulus* mSearchStimuliArray[SEARCH_COLOR_NUM_TOTAL][SEARCH_OBJECT_NUM_TOTAL];

	// state management
	int mCurrentIntroScene = 0;
	TestPhaseEnum mNextTestPhase = TestPhaseEnum::TEST_TRIAL_START;
	int mCurrentTrialNum = 0;
	int mTotalTrialNum = 0;

	int mNumCorrectTrials = 0;

	// state management
	PresentationModeEnum mCurrentMode = PresentationModeEnum::MODE_INTRO;
	int mSceneAnimationFrame = 0;
	float mTimeOnTimer = 0;
	bool mIsSwitchedScene = true;
	bool mIsTimerSet = false;
	bool mIsAwaitingAnswer = false;
	bool mIsAwaitingContinue = true;
	AnswerResponseEnum mExpectedAnswer = AnswerResponseEnum::ANSWER_NONE;

	// stimulus timing
	chrono::time_point<chrono::steady_clock> mTimerStart;
	float mTimePreCueMin = 0.5;
	float mTimePreCueMax = 0.8;
	float mTimeCue = 0.3;
	float mTimePostCueMin = 0.8;
	float mTimePostCueMax = 1.2;
	float mTimeStimuli = 0.35;
	float mTimePostStimuliMin = 0.8;
	float mTimePostStimuliMax = 1.2;

	// common screen object positions
	const GUI::Rect mRectFullScreen = { 0.0, 0.0, 1.0, 1.0 };				// normalized coordinates to top-left (0,0) and full width-height (1,1)
	const GUI::Rect mRectInstructions = { 0.2, 0.2, 0.8, 0.6 };				// instructions text box
	const GUI::Rect mRectFocusCenter = { 0.425, 0.475, 0.575, 0.675 };		// focus object in the center of the screen

	// relative coordinates for the 4 search objects
	float searchObjectWidth = 0.10;
	float searchObjectHeight = 0.15;
	float searchObjectLeft = 0.25;
	float searchObjectRight = 0.65;
	float searchObjectTop = 0.3;
	float SearchObjectBottom = 0.75;
	GUI::Rect mRectSearchObjectTopLeft = { searchObjectLeft, searchObjectTop, searchObjectLeft + searchObjectWidth, searchObjectTop + searchObjectHeight };
	GUI::Rect mRectSearchObjectTopRight = { searchObjectRight, searchObjectTop, searchObjectRight + searchObjectWidth, searchObjectTop + searchObjectHeight };
	GUI::Rect mRectSearchObjectBottomLeft = { searchObjectLeft, SearchObjectBottom, searchObjectLeft + searchObjectWidth, SearchObjectBottom + searchObjectHeight };
	GUI::Rect mRectSearchObjectBottomRight = { searchObjectRight, SearchObjectBottom, searchObjectRight + searchObjectWidth, SearchObjectBottom + searchObjectHeight };
	GUI::Rect mRectSearchObjectPos[NUM_OBJECTS_PER_TRIAL] = { mRectSearchObjectTopLeft, mRectSearchObjectTopRight, mRectSearchObjectBottomLeft, mRectSearchObjectBottomRight };

	// maps text descriptions to corresponding Enum
	map<string, TrialTypeEnum> mMapTrialType = { { "attention valid", TrialTypeEnum::ATTENTION_VALID }, { "attention invalid", TrialTypeEnum::ATTENTION_INVALID }, { "memory valid", TrialTypeEnum::MEMORY_VALID }, { "memory invalid", TrialTypeEnum::MEMORY_INVALID } };
	map<string, CueDirectionEnum> mMapCueDirection = { { "TL", CueDirectionEnum::CUE_TOP_LEFT }, { "TR", CueDirectionEnum::CUE_TOP_RIGHT },{ "BR", CueDirectionEnum::CUE_BOTTOM_RIGHT },{ "BL", CueDirectionEnum::CUE_BOTTOM_LEFT } };
	map<string, SearchColorEnum> mMapCueColor = { { "blue", SearchColorEnum::SEARCH_COLOR_BLUE }, { "green", SearchColorEnum::SEARCH_COLOR_GREEN }, { "orange", SearchColorEnum::SEARCH_COLOR_ORANGE }, { "pink", SearchColorEnum::SEARCH_COLOR_PINK }, { "purple", SearchColorEnum::SEARCH_COLOR_PURPLE }, { "yellow", SearchColorEnum::SEARCH_COLOR_YELLOW } };
	map<string, SearchObjectEnum> mMapCueObject = { { "Alien", SearchObjectEnum::SEARCH_OBJECT_ALIEN }, { "Asteroid", SearchObjectEnum::SEARCH_OBJECT_ASTEROID }, { "Astronaut", SearchObjectEnum::SEARCH_OBJECT_ASTRONAUT }, { "Helmet", SearchObjectEnum::SEARCH_OBJECT_HELMET }, { "Moon", SearchObjectEnum::SEARCH_OBJECT_MOON }, { "Planet", SearchObjectEnum::SEARCH_OBJECT_PLANET }, { "Radar", SearchObjectEnum::SEARCH_OBJECT_RADAR }, { "Rocket", SearchObjectEnum::SEARCH_OBJECT_ROCKET }, { "Saturn", SearchObjectEnum::SEARCH_OBJECT_SATURN }, { "Saucer", SearchObjectEnum::SEARCH_OBJECT_SAUCER }, { "Star", SearchObjectEnum::SEARCH_OBJECT_STAR }, { "Sun", SearchObjectEnum::SEARCH_OBJECT_SUN }, { "Telescope", SearchObjectEnum::SEARCH_OBJECT_TELESCOPE } };
	string strTrialTypeList[TRIAL_TYPE_NUM_TOTAL] = { "Attention Valid", "Attention Invalid", "Memory Valid", "Memory Invalid" };
	string strCueDirectionList[CUE_IMAGES_NUM_TOTAL] = { "TL", "TR", "BR", "BL" };
	string strCueColorList[SEARCH_COLOR_NUM_TOTAL] = { "blue", "green", "orange", "pink", "purple", "yellow" };
	string strCueObjectList[SEARCH_OBJECT_NUM_TOTAL] = { "Alien", "Asteroid", "Astronaut", "Helmet", "Moon", "Planet", "Radar", "Rocket", "Saturn", "Saucer", "Star", "Sun", "Telescope" };
	string strAnswerList[ANSWER_NUM_TOTAL] = { "None", "Same", "Different" };

	// functions
	void StartTimer(float timeInSeconds);
	void StartTimer(float timeInSecondsMin, float timeInSecondsMax);
	void PresentIntroMode();
	void PresentTrainMode();
	void PresentTestMode();
	void Answer(AnswerResponseEnum answer);

	int LoadTrialDefinitionFile();
};

#endif // INCLUDED_KASTNERCUEINGTASK_H
