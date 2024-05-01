////////////////////////////////////////////////////////////////////////////////
// Authors: Jarod@MyPC
// Description: KastnerCueingTask implementation
////////////////////////////////////////////////////////////////////////////////

#include "KastnerCueingTask.h"
#include "BCIStream.h"


RegisterFilter( KastnerCueingTask, 3 );
     // Change the location if appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations within SignalProcessing modules begin with "2."
     //       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
     //  - filter locations within Application modules begin with "3."


KastnerCueingTask::KastnerCueingTask() :
    mrDisplay(Window()), mMessage(mrDisplay), mNextTestPhase(TestPhaseEnum::TEST_TRIAL_START)
{
    // C++ does not initialize simple types such as numbers, or pointers, by default.
    // Rather, they will have random values.
    // Take care to initialize any member variables here, so they have predictable values
    // when used for the first time.

      // initialize some settings
    mCurrentIntroScene = 0;
    mMessage.SetTextColor(RGBColor::White)
        .SetColor(RGBColor::NullColor)
        .SetTextHeight(0.2)
        .SetAspectRatioMode(GUI::AspectRatioModes::AdjustHeight)
        .SetZOrder(0.0);                                            // text is always in front

    searchObjectWidth = 0.10;
    searchObjectHeight = 0.15;
    searchObjectLeft = 0.25;
    searchObjectRight = 0.65;
    searchObjectTop = 0.3;
    SearchObjectBottom = 0.75;
    mRectSearchObjectTopLeft = { searchObjectLeft, searchObjectTop, searchObjectLeft + searchObjectWidth, searchObjectTop + searchObjectHeight };
    mRectSearchObjectTopRight = { searchObjectRight, searchObjectTop, searchObjectRight + searchObjectWidth, searchObjectTop + searchObjectHeight };
    mRectSearchObjectBottomLeft = { searchObjectLeft, SearchObjectBottom, searchObjectLeft + searchObjectWidth, SearchObjectBottom + searchObjectHeight };
    mRectSearchObjectBottomRight = { searchObjectRight, SearchObjectBottom, searchObjectRight + searchObjectWidth, SearchObjectBottom + searchObjectHeight };
    mRectSearchObjectPos[0] = mRectSearchObjectTopLeft;     // order begins top-left and proceeds clock-wise rotation
    mRectSearchObjectPos[1] = mRectSearchObjectTopRight;
    mRectSearchObjectPos[2] = mRectSearchObjectBottomRight;
    mRectSearchObjectPos[3] = mRectSearchObjectBottomLeft;

    // map the Intro image Parameter caption with an index
    mIntroImagesCaptionMap[INTRO_GALAXY] = "Galaxy";
    mIntroImagesCaptionMap[INTRO_TITLE] = "Title";
    mIntroImagesCaptionMap[INTRO_ADVENTURE] = "Adventure";
    mIntroImagesCaptionMap[INTRO_ALIEN_MAIN] = "AlienMain";
    mIntroImagesCaptionMap[INTRO_ALIEN_NO_ARMS] = "AlienWithNoArms";
    mIntroImagesCaptionMap[INTRO_ALIEN_POINT_BOTTOM_LEFT] = "AlienPointingBottomLeft";
    mIntroImagesCaptionMap[INTRO_ALIEN_POINT_TOP_RIGHT] = "AlienPointingTopRight";
    mIntroImagesCaptionMap[INTRO_FESTIVE] = "Festive";
    mIntroImagesCaptionMap[INTRO_KEYBOARD_CIRCLES] = "KeyboardKeysCircles";
    mIntroImagesCaptionMap[INTRO_KEYBOARD_KEYS_CORRECT] = "KeyboardKeysCorrect";
    mIntroImagesCaptionMap[INTRO_KEYBOARD_KEYS_INCORRECT] = "KeyboardKeysIncorrect";
    mIntroImagesCaptionMap[INTRO_KEYBOARD_FINGERS] = "KeyboardKeysFingers";
    mIntroImagesCaptionMap[INTRO_FRIEND_WAITING] = "FriendsWaiting";
    mIntroImagesCaptionMap[INTRO_FRIENDS_SUCCESS] = "FriendsSuccess";
    mIntroImagesCaptionMap[INTRO_FRIENDS_GO_HOME] = "FriendsGoHome";
    mIntroImagesCaptionMap[INTRO_PLANET] = "Planet";
    mIntroImagesCaptionMap[INTRO_RED_CIRCLE] = "RedCircle";
    mIntroImagesCaptionMap[INTRO_SHIP_TANK] = "ShipTank";

    // map the Cue image Parameter caption with an index
    mCueImagesCaptionMap[CUE_TOP_LEFT] = "TopLeft";
    mCueImagesCaptionMap[CUE_TOP_RIGHT] = "TopRight";
    mCueImagesCaptionMap[CUE_BOTTOM_RIGHT] = "BottomRight";
    mCueImagesCaptionMap[CUE_BOTTOM_LEFT] = "BottomLeft";

}

KastnerCueingTask::~KastnerCueingTask()
{
  Halt();
  // If you have allocated any memory with malloc(), call free() here.
  // If you have allocated any memory with new[], call delete[] here.
  // If you have created any object using new, call delete here.
  // Either kind of deallocation will silently ignore null pointers, so all
  // you need to do is to initialize those to zero in the constructor, and
  // deallocate them here.

    // delete all objects and free memory
    mIntroStimuliSet.DeleteObjects();
    mSearchStimuliSet.DeleteObjects();

}

void
KastnerCueingTask::Publish()
{
  // Define any parameters that the filter needs....

 BEGIN_PARAMETER_DEFINITIONS

     //"Application:KastnerCueingTask int EnableKastnerCueingTask= 0 0 0 1 // enable KastnerCueingTask? (boolean)",              // These are just examples:
     //"Application:KastnerCueingTask float SomeParameter=  0.0 0.0 -1.0 1.0 // a useless KastnerCueingTask parameter",          //  change them, or remove them.
     "Application:KastnerCueingTask int PresentationMode= 3 3 1 3 // Mode 1 = Intro, 2 = Practice, 3 = Test",
     "Application:KastnerCueingTask int ListVsRandom= 1 1 1 2 // 1 = Pre-defined trial list, 2 = Randomly generated trials",
     "Application:KastnerCueingTask int RandomSeed= 1 1 1 65535 // Seed for random number generator",
     "Application:KastnerCueingTask int TotalTrialNum= 15 15 1 999 // Total number of trials",
     "Application:KastnerCueingTask float TimePreCueMin= 0.5 0.5 0.1 5.0",
     "Application:KastnerCueingTask float TimePreCueMax= 0.8 0.8 0.1 5.0",
     "Application:KastnerCueingTask float TimeCue= 0.3 0.3 0.1 5.0",
     "Application:KastnerCueingTask float TimePostCueMin= 0.8 0.8 0.1 5.0",
     "Application:KastnerCueingTask float TimePostCueMax= 1.2 1.2 0.1 5.0",
     "Application:KastnerCueingTask float TimeStimuli= 0.35 0.35 0.1 5.0",
     "Application:KastnerCueingTask float TimePostStimuliMin= 0.8 0.8 0.1 5.0",
     "Application:KastnerCueingTask float TimePostStimuliMax= 1.2 1.2 0.1 5.0",
     "Application:KastnerCueingTask string CueTopLeft= ../tasks/KastnerCueingTask/images/CueImages/Alien_TL_outline_red.png",
     "Application:KastnerCueingTask string CueTopRight= ../tasks/KastnerCueingTask/images/CueImages/Alien_TR_outline_red.png",
     "Application:KastnerCueingTask string CueBottomRight= ../tasks/KastnerCueingTask/images/CueImages/Alien_BR_outline_red.png",
     "Application:KastnerCueingTask string CueBottomLeft= ../tasks/KastnerCueingTask/images/CueImages/Alien_BL_outline_red.png",
     
     "Application:KastnerCueingTask string SearchPath= ../tasks/KastnerCueingTask/images/SearchImages/",
     "Application:KasnterCueingTask string TrialDefinitionFile= ../tasks/KastnerCueingTask/TrialConditions_wmSearch.csv",

     "Application:KastnerCueingTask matrix IntroGraphicsList= "
     "{ Caption Filename } "                                                // row labels
     "{ Galaxy Title Adventure AlienMain AlienWithNoArms AlienPointingBottomLeft AlienPointingTopRight Festive KeyboardKeysCircles KeyboardKeysCorrect KeyboardKeysIncorrect KeyboardKeysFingers FriendsWaiting FriendsSuccess FriendsGoHome Planet RedCircle ShipTank } " // column labels
     " Galaxy Title Adventure AlienMain AlienWithNoArms AlienPointingBottomLeft AlienPointingTopRight Festive KeyboardKeysCircles KeyboardKeysCorrect KeyboardKeysIncorrect KeyboardKeysFingers FriendsWaiting FriendsSuccess FriendsGoHome Planet RedCircle ShipTank "
     "../tasks/KastnerCueingTask/images/galaxy.png ../tasks/KastnerCueingTask/images/title.png ../tasks/KastnerCueingTask/images/adventure.png ../tasks/KastnerCueingTask/images/main_alien.png ../tasks/KastnerCueingTask/images/Alien_noArms_noAnt_outline.png ../tasks/KastnerCueingTask/images/Alien_BL_outline_red.png ../tasks/KastnerCueingTask/images/Alien_TR_outline_red.png ../tasks/KastnerCueingTask/images/olive_festive.png ../tasks/KastnerCueingTask/images/circleKeys.png ../tasks/KastnerCueingTask/images/correctKeys.png ../tasks/KastnerCueingTask/images/incorrectKeys.png ../tasks/KastnerCueingTask/images/fingersKeys.png ../tasks/KastnerCueingTask/images/friends_waiting.png ../tasks/KastnerCueingTask/images/success.png ../tasks/KastnerCueingTask/images/goHome.png ../tasks/KastnerCueingTask/images/planet.png ../tasks/KastnerCueingTask/images/redCircle.png ../tasks/KastnerCueingTask/images/ship_tank.png "
     " // captions and file paths to graphics files",

    
 END_PARAMETER_DEFINITIONS


  // ...and likewise any state variables:

 BEGIN_STATE_DEFINITIONS

     //"SomeState       8 0 0 0",    // These are just examples. Change them, or remove them.
     //"SomeOtherState 16 0 0 0",

     "TestPhase       8 0 0 0",
     "CurrentTrialNum 8 0 0 0",
     "CurrentTrialType 4 0 0 0",
     "ExpectedAnswer  8 0 0 0",
     "CurrentAnswer   8 0 0 0",
     "CurrentStimColor0    8 0 0 0",
     "CurrentStimColor1    8 0 0 0",
     "CurrentStimColor2    8 0 0 0",
     "CurrentStimColor3    8 0 0 0",
     "CurrentStimObject0    8 0 0 0",
     "CurrentStimObject1    8 0 0 0",
     "CurrentStimObject2    8 0 0 0",
     "CurrentStimObject3    8 0 0 0",
     "CurrentTarget   8 0 0 0",



 END_STATE_DEFINITIONS

}

void
KastnerCueingTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  // The user has pressed "Set Config" and we need to sanity-check everything.
  // For example, check that all necessary parameters and states are accessible:
  //
  // Parameter( "Milk" );
  // State( "Bananas" );
  //
  // Also check that the values of any parameters are sane:
  //
  // if( (float)Parameter( "Denominator" ) == 0.0f )
  //      bcierr << "Denominator cannot be zero";
  //
  // Errors issued in this way, during Preflight, still allow the user to open
  // the Config dialog box, fix bad parameters and re-try.  By contrast, errors
  // and C++ exceptions at any other stage (outside Preflight) will make the
  // system stop, such that BCI2000 will need to be relaunched entirely.

  // Note that the KastnerCueingTask instance itself, and its members, are read-only during
  // this phase, due to the "const" at the end of the Preflight prototype above.
  // Any methods called by Preflight must also be "const" in the same way.

    State("KeyDown");
    State("KeyUp");
    State("TestPhase");
    State("CurrentTrialNum");
    State("CurrentTrialType");
    State("ExpectedAnswer");
    State("CurrentAnswer");
    State("CurrentStimColor0");
    State("CurrentStimColor1");
    State("CurrentStimColor2");
    State("CurrentStimColor3");
    State("CurrentStimObject0");
    State("CurrentStimObject1");
    State("CurrentStimObject2");
    State("CurrentStimObject3");
    State("CurrentTarget");

    if ((int)Parameter("TotalTrialNum") < 1)
        bcierr << "TotalTrialNum should be atleast 1";

    if ((int)Parameter("PresentationMode") < 1 || (int)Parameter("PresentationMode") > 3)
        bcierr << "Invalid PresentationMode: must be 1 (Introduction), 2 (Practice), or 3 (Test)";
    
    if ((int)Parameter("ListVsRandom") < 1 || (int)Parameter("ListVsRandom") > 2)
        bcierr << "ListVsRandom should be 1 (pre-defined trial list) or 2 (randomly generated list)";

    if ((int)Parameter("RandomSeed") < 0)
        bcierr << "RandomSeed should be a positive integer";

    for (map<IntroImagesEnum, string>::const_iterator introImageCaptionIterator = mIntroImagesCaptionMap.begin(); introImageCaptionIterator != mIntroImagesCaptionMap.end(); ++introImageCaptionIterator)
    {
        string strFilename = Parameter("IntroGraphicsList")(PARAM_INDEX_FILENAME, introImageCaptionIterator->second).ToString();
        if (strFilename.empty()) bcierr << "Intro graphics file for '" << introImageCaptionIterator->second << "' is not found in the IntroGraphicsList parameter";
    }

    string strFilename = Parameter("TrialDefinitionFile");
    if (strFilename.empty())
        bcierr << "Trial definitions filename not found in parameters";

    if (Parameter("CueTopLeft").ToString().empty()) bcierr << "CueTopLeft graphics filename missing.";
    if (Parameter("CueTopRight").ToString().empty()) bcierr << "CueTopRight graphics filename missing.";
    if (Parameter("CueBottomRight").ToString().empty()) bcierr << "CueBottomRight graphics filename missing.";
    if (Parameter("CueBottomLeft").ToString().empty()) bcierr << "CueBottomLeft graphics filename missing.";
    if (Parameter("SearchPath").ToString().empty()) bcierr << "SearchPath graphics file not found.";

    Output = Input; // this simply passes information through about SampleBlock dimensions, etc....
}

void
KastnerCueingTask::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The signal properties can no longer be modified, but the const limitation has gone, so
  // the KastnerCueingTask instance itself can be modified. Allocate any memory you need, start any
  // threads, store any information you need in private member variables.
    
    //static std::random_device rd;
    std::default_random_engine randomEngineGenerator;
    std::uniform_int_distribution<int> randomCueDistribution(0, CUE_IMAGES_NUM_TOTAL - 1);
    std::discrete_distribution<int> randomTrialTypeDistribution({ mPercentAttention * mPercentValid, mPercentAttention * (1 - mPercentValid), (1 - mPercentAttention) * mPercentValid, (1 - mPercentAttention) * (1 - mPercentValid) });
    std::mt19937 randomizeEngine = std::default_random_engine((int)Parameter("RandomSeed"));
    std::string strFilename;            // reused variable to hold the filename obtained from the Parameter list
    ImageStimulus* newStimulus;         // reused variable for loading each stimuli before collecting in mInstroStimuliSet

    // determine Intro, Training, or Testing mode
    mCurrentMode = static_cast<PresentationModeEnum>( int(Parameter("PresentationMode")) );

    // load timing parameters
    mTimePreCueMin = float(Parameter("TimePreCueMin"));
    mTimePreCueMax = float(Parameter("TimePreCueMax"));
    mTimeCue = float(Parameter("TimeCue"));
    mTimePostCueMin = float(Parameter("TimePostCueMin"));
    mTimePostCueMax = float(Parameter("TimePostCueMax"));
    mTimeStimuli = float(Parameter("TimeStimuli"));
    mTimePostStimuliMin = float(Parameter("TimePostStimuliMin"));
    mTimePostStimuliMax = float(Parameter("TimePostStimuliMax"));

    // reset states and counters
    mCurrentIntroScene = 1;
    mSceneAnimationFrame = 0;
    mCurrentTrialNum = 0;
    mNumCorrectTrials = 0;
    mNextTestPhase = TestPhaseEnum::TEST_TRIAL_START;
    mIsSwitchedScene = true;
    mTrialDefList.clear();

    // determine if we're using a pre-defined list of trials vs randomly generated
    mListVsRandom = static_cast<ListVsRandomTrialsEnum>( int(Parameter("ListVsRandom")) );
    if (mListVsRandom == ListVsRandomTrialsEnum::TRIALS_LIST)
    {
        // Load CSV trial definition file
        LoadTrialDefinitionFile();
        mTotalTrialNum = mTrialDefList.size();
    }
    else
    {
        mTotalTrialNum = Parameter("TotalTrialNum");

        // generate a random trial definition list
        for (int i = 0; i < mTotalTrialNum; ++i)
        {
            TrialDefinition trialDef{};     // struct of elements defining an individual trial

            // pick a random trial type
            trialDef.trialType = static_cast<TrialTypeEnum>(randomTrialTypeDistribution(randomEngineGenerator));

            // pick a random target index and cue direction
            trialDef.targetIndex = randomCueDistribution(randomEngineGenerator);
            trialDef.cueDirection = static_cast<CueDirectionEnum>(randomCueDistribution(randomEngineGenerator));

            // randomize colors/objects without replacement (i.e. shuffle the colors/objects)
            std::shuffle(mArrayColorIndices.begin(), mArrayColorIndices.end(), randomizeEngine);
            std::shuffle(mArrayObjectIndices.begin(), mArrayObjectIndices.end(), randomizeEngine);
            for (int i = 0; i < NUM_OBJECTS_PER_TRIAL; ++i)
            {
                trialDef.searchColor[i] = static_cast<SearchColorEnum>(mArrayColorIndices[i]);
                trialDef.searchObject[i] = static_cast<SearchObjectEnum>(mArrayObjectIndices[i]);
            }

            mTrialDefList.push_back(trialDef);
        }
    }

    // setup a text field for on-screen instructions
    GUI::Rect rectTextInstructions = { 0.5, 0.80, 0.5, 1.0 };
    mMessage.SetTextColor(RGBColor::White)
        .SetTextHeight(0.3)
        .SetColor(RGBColor::NullColor)
        .SetAspectRatioMode(GUI::AspectRatioModes::AdjustHeight)
        .SetObjectRect(rectTextInstructions);

    // clear all the image simulus sets
    mIntroStimuliSet.DeleteObjects();
    mIntroImagesStimuliMap.clear();
    mSearchStimuliSet.DeleteObjects();

    // load all Intro graphics files
    map<IntroImagesEnum, string>::const_iterator introImagesCaptionIterator;
    for (introImagesCaptionIterator = mIntroImagesCaptionMap.begin(); introImagesCaptionIterator != mIntroImagesCaptionMap.end(); ++introImagesCaptionIterator)
    {
        strFilename = Parameter("IntroGraphicsList")(PARAM_INDEX_FILENAME, introImagesCaptionIterator->second).ToString();     // get filename from Parameters

        // load the image into an ImageStimulus object
        newStimulus = new ImageStimulus(mrDisplay);
        newStimulus->SetFile(strFilename);
        newStimulus->SetZOrder(0.5);                     // most Stimuli will be middle ground (allow text in front), exceptions editied below

        // collect all images in a StimuliSet and a mapping of tte index to the ImageStimulus pointer
        mIntroStimuliSet.Add(newStimulus);
        mIntroImagesStimuliMap[introImagesCaptionIterator->first] = newStimulus;
    }

    // exceptions to the default settings for Intro graphics
    mIntroImagesStimuliMap[INTRO_GALAXY]->SetZOrder(1.0);     // Galaxy will always be background
    
    // load the 4 Cue images
    strFilename = Parameter("CueTopLeft").ToString();
    newStimulus = new ImageStimulus(mrDisplay);
    newStimulus->SetFile(strFilename);
    newStimulus->SetZOrder(0.5);
    mSearchStimuliSet.Add(newStimulus);
    mCueStimuliArray[CueDirectionEnum::CUE_TOP_LEFT] = newStimulus;

    strFilename = Parameter("CueTopRight").ToString();
    newStimulus = new ImageStimulus(mrDisplay);
    newStimulus->SetFile(strFilename);
    newStimulus->SetZOrder(0.5);
    mSearchStimuliSet.Add(newStimulus);
    mCueStimuliArray[CueDirectionEnum::CUE_TOP_RIGHT] = newStimulus;

    strFilename = Parameter("CueBottomRight").ToString();
    newStimulus = new ImageStimulus(mrDisplay);
    newStimulus->SetFile(strFilename);
    newStimulus->SetZOrder(0.5);
    mSearchStimuliSet.Add(newStimulus);
    mCueStimuliArray[CueDirectionEnum::CUE_BOTTOM_RIGHT] = newStimulus;

    strFilename = Parameter("CueBottomLeft").ToString();
    newStimulus = new ImageStimulus(mrDisplay);
    newStimulus->SetFile(strFilename);
    newStimulus->SetZOrder(0.5);
    mSearchStimuliSet.Add(newStimulus);
    mCueStimuliArray[CueDirectionEnum::CUE_BOTTOM_LEFT] = newStimulus;

    // load search object graphics
    string searchPathBase = Parameter("SearchPath").ToString();

    for (int i = 0; i < SEARCH_COLOR_NUM_TOTAL; ++i)
    {
        for (int j = 0; j < SEARCH_OBJECT_NUM_TOTAL; ++j)
        {
            string filename = searchPathBase + strCueColorList[i] + strCueObjectList[j] + ".png";   // build relative file path to search object graphics

            mSearchStimuliArray[i][j] = new ImageStimulus(mrDisplay);
            mSearchStimuliArray[i][j]->SetFile(filename);
            mSearchStimuliArray[i][j]->SetZOrder(0.5);
            mSearchStimuliSet.Add(mSearchStimuliArray[i][j]);
        }
    }
}

void
KastnerCueingTask::StartRun()
{
    // The user has just pressed "Start" (or "Resume")
    mIsSwitchedScene = true;    // triggers a redraw of the current scene
}

void
KastnerCueingTask::Process(const GenericSignal& Input, GenericSignal& Output)
{
    // And now we're processing a single SampleBlock of data.
    // Remember not to take too much CPU time here, or you will break the real-time constraint.

    // check for key press at any sample timepoint within this block
    for (int i = 0; i < Statevector->Samples(); i++)
    {
        switch (State("KeyDown")(i))
        {
        case VK_SPACE:
            if (mIsAwaitingContinue)
            {
                mIsAwaitingContinue = false;
                mIsSwitchedScene = true;
                mSceneAnimationFrame = 0;
                mCurrentIntroScene = mCurrentIntroScene + 1;
            }

            break;

        case 'L':
            if (mIsAwaitingAnswer)
                Answer(AnswerResponseEnum::ANSWER_SAME);

            break;

        case 'A':
            if (mIsAwaitingAnswer)
                Answer(AnswerResponseEnum::ANSWER_DIFFERENT);

            break;

        default:
            break;

        }
    }

    // check scene animation timer
    if (mIsTimerSet)
    {
        chrono::duration<double> timeDelta;
        timeDelta = chrono::steady_clock::now() - mTimerStart;

        // when timer expires then switch the scene
        if (timeDelta.count() > mTimeOnTimer)
        {
            mIsSwitchedScene = true;
            mIsTimerSet = false;
            mTimeOnTimer = 0.0;
        }
    }

    // if we just switched scene, then change the graphics
    if (mIsSwitchedScene)
    {
        mIsSwitchedScene = false;

        // clear the scene
        mIntroStimuliSet.Conceal();
        mSearchStimuliSet.Conceal();
        mMessage.Hide();

        // manage scene for Introduction, Practice, and Testing modes
        switch (mCurrentMode)
        {
        case PresentationModeEnum::MODE_INTRO:
            PresentIntroMode();
            break;

        case PresentationModeEnum::MODE_PRACTICE:
            PresentTrainMode();
            break;

        case PresentationModeEnum::MODE_TEST:
            PresentTestMode();
            break;

        default:
            mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
            mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

            bcierr << "Invalid Presentation Mode";

            mMessage.SetText("Invalid Presentation Mode")
                .SetTextColor(RGBColor::White)
                .SetTextHeight(0.2)
                .SetColor(RGBColor::NullColor)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustHeight)
                .SetObjectRect(mRectInstructions)
                .Show();

            break;

        }
    }

  Output = Input; // This passes the signal through unmodified.
}

void
KastnerCueingTask::StopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // or because the run has reached its natural end.
  //bciwarn << "Goodbye World.";
  // You know, you can delete methods if you're not using them.
  // Remove the corresponding declaration from KastnerCueingTask.h too, if so.
}

void
KastnerCueingTask::Halt()
{
  // Stop any threads or other asynchronous activity.
  // Good practice is to write the Halt() method such that it is safe to call it even *before*
  // Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
}

void KastnerCueingTask::StartTimer(float timeInSeconds)
{
    mTimeOnTimer = timeInSeconds;
    mTimerStart = chrono::steady_clock::now();
    mIsTimerSet = true;
}

void KastnerCueingTask::StartTimer(float timeInSecondsMin, float timeInSecondsMax)
{
    //srand(1); // TODO: seed the rng with a seed parameter in Initialize

    float rndTimeInRange = ((float)rand() / RAND_MAX) * (timeInSecondsMax - timeInSecondsMin) + timeInSecondsMin;
    StartTimer(rndTimeInRange);
}

int KastnerCueingTask::LoadTrialDefinitionFile()
{
    const vector<string> expectedColumnNames = { "trial_type", "cue_quad", "TL", "TR", "BR", "BL", "probe" };
    const int numColumns = expectedColumnNames.size();

    string strFilename = Parameter("TrialDefinitionFile");
    ifstream fileStream;
    string strRow, strElement;
    vector<string> rowVector;
    stringstream ssRow;
    int rowNum = 0;

    fileStream.open(strFilename, ios_base::in);
    if (!fileStream)
    {
        bcierr << "Could not open CSV file: " << strFilename;
        return -1;
    }

    // read the column headers from the first row and confirm expected order
    getline(fileStream, strRow);
    ssRow << strRow;
    while (getline(ssRow, strElement, ','))
        rowVector.push_back(strElement);

    // confrim column names match our expectations
    if (rowVector != expectedColumnNames)
    {
        bcierr << "Error reading CSV with unexpected column headers.";
        return 0;
    }

    // read each row that defines a single trial
    while (getline(fileStream, strRow))
    {
        TrialDefinition trialDef{};     // struct of elements defining an individual trial
        smatch regstrMatch;             // regex string match object
        regex regexPattern;             // regex search pater object
        SearchColorEnum targetColor;
        SearchObjectEnum targetObject;


        rowNum++;                       // keep track of row number (N.B. row 0 holds the columns names)

        // clear temporary variables that are reused for each row's data
        rowVector.clear();
        ssRow.clear();

        ssRow << strRow;
        while (getline(ssRow, strElement, ','))
        {
            rowVector.push_back(strElement);
        }
        if (rowVector.size() != numColumns)
        {
            bcierr << "Invalid number of elements in CSV file on row: " << rowNum;
            return 0;
        }

        // parse the trial type: "attention valid", "attention invalid", "memory valid", or "memory invalid"
        strElement = rowVector[0];      // MAGICNUMBER: first column is the trial type
        regexPattern.assign("(attention valid)|(attention invalid)|(memory valid)|(memory invalid)");
        //regex_match(strElement, sm, regexPattern, regex_constants::match_default);
        if (! regex_match(strElement, regstrMatch, regexPattern, regex_constants::match_default))
        {
            bcierr << "Failed CSV parsing to identify trial type at row " << rowNum << " - '" << strElement;
            return 0;
        }
        trialDef.trialType = mMapTrialType[strElement];

        // parse the cue direction: "TL", "TR", "BR", "BL"
        strElement = rowVector[1];      // MAGICNUMBER: second column is the cue direction
        //regex regexPattern("(TL)|(TR)|(BR)|(BL)");
        regexPattern.assign("TL|TR|BR|BL");
        if (!regex_match(strElement, regstrMatch, regexPattern, regex_constants::match_default))
        {
            bcierr << "Failed CSV parsing to identify cue direction at row " << rowNum << " - '" << strElement;
            return 0;
        }
        trialDef.cueDirection = mMapCueDirection[strElement];

        // we're going to parse the target (aka probe) before the list of cues so we can idenify the index of the corresponding cue next
        strElement = rowVector[6];      // MAGICNUMBER: seventh column is the target (aka probe)
        regexPattern.assign("(blue|green|orange|pink|purple|yellow)(Alien|Asteroid|Astronaut|Helmet|Moon|Planet|Radar|Rocket|Saturn|Saucer|Star|Sun|Telescope).png");
        if (!regex_match(strElement, regstrMatch, regexPattern, regex_constants::match_default))
        {
            bcierr << "Failed CSV parsing to identify target color/object at row " << rowNum << " - '" << strElement;
            return 0;
        }
        targetColor = mMapCueColor[regstrMatch[1]];
        targetObject = mMapCueObject[regstrMatch[2]];

        // parse the four cue color/objects
        trialDef.targetIndex = -1;      // initialize to an invalid index to confirm it gets set correctly after this loop
        for (int i = 0; i < NUM_OBJECTS_PER_TRIAL; ++i)
        {
            strElement = rowVector[i + 2];  // MAGICNUMBER: third through sixth columns are the four cue color/objects
            regexPattern.assign("(blue|green|orange|pink|purple|yellow)(Alien|Asteroid|Astronaut|Helmet|Moon|Planet|Radar|Rocket|Saturn|Saucer|Star|Sun|Telescope).png");
            if (!regex_match(strElement, regstrMatch, regexPattern, regex_constants::match_default))
            {
                bcierr << "Failed CSV parsing to identify cue color/object at row " << rowNum << " - '" << strElement;
                return 0;
            }
            trialDef.searchColor[i] = mMapCueColor[regstrMatch[1]];
            trialDef.searchObject[i] = mMapCueObject[regstrMatch[2]];
            
            // if this cue matches the target then record its index
            if (trialDef.searchColor[i] == targetColor && trialDef.searchObject[i] == targetObject)
                trialDef.targetIndex = i;
        }

        // confirm we found a target that matched one of the cues
        if (trialDef.targetIndex < 0 || trialDef.targetIndex >= NUM_OBJECTS_PER_TRIAL)
        {
            bcierr << "Failed CSV parsing. Cue target did not match a cue object at row " << rowNum;
            return 0;
        }
        
        // add the individual trial definition to the list
        mTrialDefList.push_back(trialDef);
    }

    return 1;
}

// steps through frames of a task tutorial as an introduction for the user
void KastnerCueingTask::PresentIntroMode()
{
    // switch for which scene of the Introduction we are on
    switch (mCurrentIntroScene)
    {
    case 1: // title:
        // set the Galaxy to the background full screen
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        // set the Title to foreground and middle screen
        mIntroImagesStimuliMap[INTRO_TITLE]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_TITLE]->Present();

        mMessage.SetText("Press a key to begin.")
            .SetTextHeight(0.2)
            .SetObjectRect({ 0.5, 0.80, 0.5, 1.0 })
            .Show();

        mIsAwaitingContinue = true;
        break;

    case 2: // this is Olive:
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mIntroImagesStimuliMap[INTRO_ALIEN_MAIN]->SetObjectRect({ 0.0, 0.4, 0.2, 0.6 });
        mIntroImagesStimuliMap[INTRO_ALIEN_MAIN]->Present();

        mIntroImagesStimuliMap[INTRO_PLANET]->SetObjectRect({ 0.85, 0.4, 1.0, 0.6 });
        mIntroImagesStimuliMap[INTRO_PLANET]->Present();

        mMessage.SetText("This is Olive.\nShe is an adventurous young alien\nfrom a plant far far away.")
            .SetTextHeight(0.2)
            .SetObjectRect({ 0.2, 0.4, 0.8, 0.6 })
            .Show();

        mIsAwaitingContinue = true;
        break;

    case 3: // she likes to explore
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mIntroImagesStimuliMap[INTRO_ADVENTURE]->SetObjectRect({ 0.1, 0.0, 0.9, 0.65 });
        mIntroImagesStimuliMap[INTRO_ADVENTURE]->Present();

        mMessage.SetText("She likes to hop in her lightning fast\nspaceship and explore different\nplanets all over the universe!")
            .SetTextHeight(0.2)
            .SetObjectRect({ 0.1, 0.6, 0.9, 0.9 })
            .Show();

        mIsAwaitingContinue = true;
        break;

    case 4: // she's stuck far away
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mIntroImagesStimuliMap[INTRO_FRIENDS_GO_HOME]->SetObjectRect({ 0.1, 0.1, 0.9, 0.6 });
        mIntroImagesStimuliMap[INTRO_FRIENDS_GO_HOME]->Present();

        mMessage.SetText("On her latest adventure she ran out\nof fuel and got stuck far away from home!\n\nLuckily YOU can help her get back\nhome to her friends!")
            .SetTextHeight(0.2)
            //.SetAspectRatioMode(GUI::AspectRatioModes::AdjustBoth)
            .SetObjectRect({ 0.1, 0.65, 0.9, 0.9 })
            .Show();

        mIsAwaitingContinue = true;
        break;

    case 5: // you can help solve puzzles
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("You can help Olive get home by\nsolving puzzles that will power\nher spacship!")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

        mIsAwaitingContinue = true;
        break;

    case 6: // each puzzle secret piece
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("Each puzzle has a secret piece that\nis correct!")
            .SetTextHeight(0.2)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

        mIsAwaitingContinue = true;
        break;

    case 7: // Olive will give hints
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("You'll have to solve a lot of puzzles\nto fuel he ship, but Olive will give\nyou hints along the way!")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

        mIsAwaitingContinue = true;
        break;

    case 8: // red arm to point
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("Olive will user her red arm to point to\nthe corner of the screen that\nmatches with the correct piece!\nPick the right piece using Olive's hint!")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->SetObjectRect(mRectFocusCenter);
        mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->Present();

        mIsAwaitingContinue = true;
        break;

    case 9: // animation of pointing prior to search objects
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("For some of the puzzles, she will\npoint to the corner with the correct piece\nBEFORE you see all the puzzle pieces...")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        // these graphics vary depending on the frame of the animation
        switch (mSceneAnimationFrame)
        {
        case 0:
            //mTimeOnTimer = 5.0;          // 5 seconds for this frame of the animation
            //mTimerStart = chrono::steady_clock::now();
            //mIsTimerSet = true;         // start the timer
            StartTimer(5.0);

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;    // prepare for next animation step

            break;

        case 1:
            //mTimeOnTimer = mTimeCue;
            //mTimerStart = chrono::steady_clock::now();
            //mIsTimerSet = true;
            StartTimer(mTimeCue);

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Conceal();
            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->Present();

            mSceneAnimationFrame++;

            break;

        case 2:
            //mTimeOnTimer = mTimePreTarget;
            //mTimerStart = chrono::steady_clock::now();
            //mIsTimerSet = true;
            StartTimer(mTimePostCueMax);

            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->Conceal();
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;

            break;

        case 3:
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

            mIsAwaitingContinue = true;
            mSceneAnimationFrame = 0;
            break;

        default:
            mIsAwaitingContinue = true;
            mSceneAnimationFrame = 0;

            break;
        }

        break;

    case 10: // remember location
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("Remember where she pointed and\nthe piece that shows up in that corner!\nIn this puzzle, Olive pointed\nto the top right puzzle piece.")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

        mIntroImagesStimuliMap[INTRO_RED_CIRCLE]->SetObjectRect(mRectSearchObjectTopRight);
        mIntroImagesStimuliMap[INTRO_RED_CIRCLE]->Present();

        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

        mIsAwaitingContinue = true;
        break;

    case 11: // remember puzzle piece to answer
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("Remember this puzzles piece\nbecause you will need it to answer\nthe next part correctly.")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

        mIntroImagesStimuliMap[INTRO_RED_CIRCLE]->SetObjectRect(mRectSearchObjectTopRight);
        mIntroImagesStimuliMap[INTRO_RED_CIRCLE]->Present();

        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
        mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
        mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

        mIsAwaitingContinue = true;
        break;

    case 12: // next puzzle piece
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("Next you will be shown one puzzle piece\nand will need to use your keyboard to\nrespond whether or not it was the same piece\nthat Olive pointed to.")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectFocusCenter);
        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

        mIsAwaitingContinue = true;
        break;

    case 13: // respond with keyboard
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("To respond you'll be using the ['L']\nand ['A'] keys on your keyboard.")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mIntroImagesStimuliMap[INTRO_KEYBOARD_CIRCLES]->SetObjectRect({ 0.1, 0.3 , 0.9, 0.9 });
        mIntroImagesStimuliMap[INTRO_KEYBOARD_CIRCLES]->Present();

        mIsAwaitingContinue = true;
        break;

    case 14: // be ready to respond
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("To be ready to respond quickly,\nkeep your index fingers gently over\nthe ['L'] and ['A'] keys.")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mIntroImagesStimuliMap[INTRO_KEYBOARD_FINGERS]->SetObjectRect({ 0.1, 0.3 , 0.9, 0.9 });
        mIntroImagesStimuliMap[INTRO_KEYBOARD_FINGERS]->Present();

        mIsAwaitingContinue = true;
        break;

    case 15: // press L for same
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("If you are shown the SAME puzzle\npiece as the one Olive pointed to,\nrespond 'correct' by pressing ['L'].")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect({ 0.425, 0.375, 0.575, 0.575 });
        mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

        mIntroImagesStimuliMap[INTRO_KEYBOARD_KEYS_CORRECT]->SetObjectRect({ 0.2, 0.55 , 0.8, 0.95 });
        mIntroImagesStimuliMap[INTRO_KEYBOARD_KEYS_CORRECT]->Present();

        mIsAwaitingContinue = true;
        break;

    case 16: // press A for different
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("If you are shown a DIFFERENT\npuzzle piece as the one Olive\npointed to respond 'wrong' by\npressing ['A'].")
            .SetTextHeight(0.2)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
            .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
            .Show();

        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect({ 0.425, 0.375, 0.575, 0.575 });
        mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

        mIntroImagesStimuliMap[INTRO_KEYBOARD_KEYS_INCORRECT]->SetObjectRect({ 0.2, 0.55 , 0.8, 0.95 });
        mIntroImagesStimuliMap[INTRO_KEYBOARD_KEYS_INCORRECT]->Present();

        mIsAwaitingContinue = true;
        break;

    case 17: // animate all that together
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        // these graphics vary depending on the frame of the animation
        switch (mSceneAnimationFrame)
        {
        case 0:
            //mTimeOnTimer = 1.5;
            //mTimerStart = chrono::steady_clock::now();
            //mIsTimerSet = true;
            StartTimer(1.5);

            mMessage.SetText("Let's see what that will look like all together...")
                .SetTextHeight(0.2)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
                .Show();

            mSceneAnimationFrame++;

            break;

        case 1:
            //mTimeOnTimer = 1.5;
            //mTimerStart = chrono::steady_clock::now();
            //mIsTimerSet = true;
            StartTimer(1.5);

            mMessage.SetText("First, you will see Olive.")
                .SetTextHeight(0.2)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;

            break;

        case 2:
            //mTimeOnTimer = 1.75;
            //mTimerStart = chrono::steady_clock::now();
            //mIsTimerSet = true;
            StartTimer(1.75);

            mMessage.SetText("Then, Olive will give you a hint.")
                .SetTextHeight(0.2)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->Present();

            mSceneAnimationFrame++;

            break;

        case 3:
            //mTimeOnTimer = 1.57;
            //mTimerStart = chrono::steady_clock::now();
            //mIsTimerSet = true;
            StartTimer(1.75);

            mMessage.SetText("After a short period of time...")
                .SetTextHeight(0.2)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;

            break;

        case 4:
            //mTimeOnTimer = 3;
            //mTimerStart = chrono::steady_clock::now();
            //mIsTimerSet = true;
            StartTimer(3.0);

            mMessage.SetText("...the puzzle pieces will show up.\nTry to remember the piece\nthat matches with Olive's hint!")
                .SetTextHeight(0.15)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.0, 0.0, 1.0, 0.35 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

            mSceneAnimationFrame++;

            break;

        case 5:
            //mTimeOnTimer = 1.75;
            //mTimerStart = chrono::steady_clock::now();
            //mIsTimerSet = true;
            StartTimer(1.75);

            mMessage.SetText("After another short period of time...")
                .SetTextHeight(0.2)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.3 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;

            break;

        case 6:
            mMessage.SetText("... You'll have to respond whether or not\nthe piece you see the same piece as the hint.\nRemember press ['L'] if it matches\nand ['A'] if it does not.")
                .SetTextHeight(0.15)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.0, 0.0, 1.0, 0.35 })
                .Show();

            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectFocusCenter);
            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

            mSceneAnimationFrame = 0;

            // wait for an answer
            mExpectedAnswer = AnswerResponseEnum::ANSWER_SAME;
            mIsAwaitingAnswer = true;
            break;

        default:
            mIsAwaitingAnswer = false;
            mIsAwaitingContinue = true;
            mSceneAnimationFrame = 0;

            break;
        }

        break;

    case 18: // animate once more without directions
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        // these graphics vary depending on the frame of the animation
        switch (mSceneAnimationFrame)
        {
        case 0:
            mMessage.SetText("Let's do that one more time, but this\ntime without directions.\n\nDon't forget to respond at the end!\n\n Press ['L'] if the peices match and\n['A'] if they do not.")
                .SetTextHeight(0.1)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.05, 0.9, 0.5 })
                .Show();

            mIntroImagesStimuliMap[INTRO_KEYBOARD_FINGERS]->SetObjectRect({ 0.1, 0.4, 0.9, 1.0 });
            mIntroImagesStimuliMap[INTRO_KEYBOARD_FINGERS]->Present();

            mSceneAnimationFrame++;
            StartTimer(4.0);

            break;

        case 1:
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;
            StartTimer(1.5);
            break;

        case 2:
            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_BOTTOM_LEFT]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_BOTTOM_LEFT]->Present();

            mSceneAnimationFrame++;
            StartTimer(1.75);

            break;

        case 3:
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;
            StartTimer(1.5);
            break;

        case 4:
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

            mSceneAnimationFrame++;
            StartTimer(3.0);

            break;

        case 5:
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;
            StartTimer(1.75);

            break;

        case 6:
            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectFocusCenter);
            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

            mSceneAnimationFrame = 0;

            // wait for an answer
            mExpectedAnswer = AnswerResponseEnum::ANSWER_SAME;
            mIsAwaitingAnswer = true;

            break;

        default:
            mSceneAnimationFrame = 0;

            break;
        }

        break;

    case 19: // more challenging hint AFTER
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("To make the game more fun, there\nare some trickier puzzles where\nOlive will give you the hint AFTER\nyou have seen the puzzle pieces.")
            .SetTextColor(RGBColor::White)
            .SetTextHeight(0.1)
            .SetColor(RGBColor::NullColor)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
            .SetObjectRect({ 0.05, 0.05, 0.95, 0.75 })
            .Show();

        mIsAwaitingContinue = true;
        break;

    case 20: // animate hint AFTER puzzle pieces
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        // these graphics vary depending on the frame of the animation
        switch (mSceneAnimationFrame)
        {
        case 0:
            StartTimer(2.0);

            mMessage.SetText("That will look like this.")
                .SetTextHeight(0.2)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mSceneAnimationFrame++;

            break;

        case 1:
            StartTimer(1.0);

            mMessage.SetText("That will look like this.")
                .SetTextHeight(0.2)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

            mSceneAnimationFrame++;

            break;

        case 2:
            StartTimer(0.75);

            mMessage.SetText("That will look like this.")
                .SetTextHeight(0.2)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;

            break;

        case 3:
            mMessage.SetText("That will look like this.")
                .SetTextHeight(0.2)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->Present();

            mSceneAnimationFrame = 0;

            mIsAwaitingContinue = true;

            break;

        default:
            mSceneAnimationFrame = 0;
            mIsAwaitingContinue = true;

            break;
        }

        break;

    case 21: // practice that slowly
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("You'll respond the same way\nas you did before.\n\nLet's go through an example\nslowly to practice.")
            .SetTextColor(RGBColor::White)
            .SetTextHeight(0.1)
            .SetColor(RGBColor::NullColor)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
            .SetObjectRect({ 0.05, 0.05, 0.95, 0.75 })
            .Show();

        mIsAwaitingContinue = true;

        break;

    case 22: // animate hint AFTER puzzle pieces with instructions

        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        // these graphics vary depending on the frame of the animation
        switch (mSceneAnimationFrame)
        {
        case 0:
            mMessage.SetText("First you will see Olive.")
                .SetTextHeight(0.1)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;
            StartTimer(1.5);

            break;

        case 1:
            mMessage.SetText("Then the puzzle pieces will show up.\nTry to remember the pieces\nso you can use the hint!")
                .SetTextHeight(0.1)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->SetObjectRect(mRectSearchObjectTopLeft);
            mSearchStimuliArray[SEARCH_COLOR_BLUE][SEARCH_OBJECT_ASTRONAUT]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectSearchObjectTopRight);
            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->SetObjectRect(mRectSearchObjectBottomLeft);
            mSearchStimuliArray[SEARCH_COLOR_PINK][SEARCH_OBJECT_SATURN]->Present();

            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->SetObjectRect(mRectSearchObjectBottomRight);
            mSearchStimuliArray[SEARCH_COLOR_GREEN][SEARCH_OBJECT_ROCKET]->Present();

            mSceneAnimationFrame++;
            StartTimer(4.0);

            break;

        case 2:
            mMessage.SetText("After a short period of time....")
                .SetTextHeight(0.1)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;
            StartTimer(1.75);

            break;

        case 3:
            mMessage.SetText("Then, Olive will give you a hint.")
                .SetTextHeight(0.1)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_POINT_TOP_RIGHT]->Present();

            mSceneAnimationFrame++;
            StartTimer(1.75);

            break;

        case 4:
            mMessage.SetText("After another short period of time...")
                .SetTextHeight(0.1)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            mSceneAnimationFrame++;
            StartTimer(1.75);

            break;

        case 5:
            mMessage.SetText("...You'll have to respond whether or not\nthe piece you see the same piece as the hint.\n\nRemember press ['L'] if it matches and ['A'] if it does not.")
                .SetTextHeight(0.1)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.1, 0.0, 0.9, 0.4 })
                .Show();

            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->SetObjectRect(mRectFocusCenter);
            mSearchStimuliArray[SEARCH_COLOR_PURPLE][SEARCH_OBJECT_SAUCER]->Present();

            mSceneAnimationFrame = 0;
            mExpectedAnswer = AnswerResponseEnum::ANSWER_SAME;
            mIsAwaitingAnswer = true;

            break;


        default:
            mSceneAnimationFrame = 0;
            mIsAwaitingContinue = true;

            break;
        }

        break;

    case 23: // keep focus on Olive
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("One more reminder... it is important that you\nkeep your eyes on Olive in the center of the screen\nthe whole time you are playing the game!\n\nIf you look around you might miss her hints!")
            .SetTextColor(RGBColor::White)
            .SetTextHeight(0.07)
            .SetColor(RGBColor::NullColor)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
            .SetObjectRect({ 0.05, 0.05, 0.95, 0.75 })
            .Show();

        mIsAwaitingContinue = true;

        break;

    case 24: // Got it?
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("Got it?")
            .SetTextColor(RGBColor::White)
            .SetTextHeight(0.175)
            .SetColor(RGBColor::NullColor)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
            .SetObjectRect({ 0.05, 0.05, 0.95, 0.75 })
            .Show();

        mIsAwaitingContinue = true;

        break;

    default:
        mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
        mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

        mMessage.SetText("End of Introduction")
            .SetTextColor(RGBColor::White)
            .SetTextHeight(0.2)
            .SetColor(RGBColor::NullColor)
            .SetObjectRect(mRectInstructions)
            .SetAspectRatioMode(GUI::AspectRatioModes::AdjustHeight)
            .Show();

        break;
    }
}

// a training version of the task used for debugging
void KastnerCueingTask::PresentTrainMode()
{
    //TODO: this is a debug mode at present
    mSearchStimuliArray[0][objectIdx]->SetObjectRect({ 0.0, 0.0, 0.2, 0.2 });
    mSearchStimuliArray[0][objectIdx]->Present();

    mSearchStimuliArray[1][objectIdx]->SetObjectRect({ 0.4, 0.0, 0.6, 0.2 });
    mSearchStimuliArray[1][objectIdx]->Present();

    mSearchStimuliArray[2][objectIdx]->SetObjectRect({ 0.8, 0.0, 1.0, 0.2 });
    mSearchStimuliArray[2][objectIdx]->Present();

    mSearchStimuliArray[3][objectIdx]->SetObjectRect({ 0.0, 0.5, 0.2, 0.7 });
    mSearchStimuliArray[3][objectIdx]->Present();

    mSearchStimuliArray[4][objectIdx]->SetObjectRect({ 0.4, 0.5, 0.6, 0.7 });
    mSearchStimuliArray[4][objectIdx]->Present();

    mSearchStimuliArray[5][objectIdx]->SetObjectRect({ 0.8, 0.5, 1.0, 0.7 });
    mSearchStimuliArray[5][objectIdx]->Present();

    objectIdx++;
    if (objectIdx >= SEARCH_OBJECT_NUM_TOTAL)
        objectIdx = 0;
    mIsAwaitingContinue = true;
}

// the actual task sequence to test their spatial attention by evaluating answers to valid/invalid trials of stimuli with directional cueing
void KastnerCueingTask::PresentTestMode()
{
    // update State as we start the next Test Phase
    State("TestPhase") = static_cast<int>(mNextTestPhase);

    // logic for each Test Phase
    switch (mNextTestPhase)
    {
    case TestPhaseEnum::TEST_TRIAL_START:
        // this is the start of a new trial
        mCurrentTrialNum++;
        if (mCurrentTrialNum > mTotalTrialNum)
        {
            bciout << "Run completed the total number of trials.";
            StopRun();
        }
        else
        {
            // update the State trial number
            State("CurrentTrialNum") = mCurrentTrialNum;

            // update the current trial definition
            mTrialDefCurrent = mTrialDefList[mCurrentTrialNum - 1];

            // expect "Same" answer for Valid trials and "Different" answer for Invalid trials
            if (mTrialDefCurrent.trialType == TrialTypeEnum::ATTENTION_VALID || mTrialDefCurrent.trialType == TrialTypeEnum::MEMORY_VALID)
                mExpectedAnswer = AnswerResponseEnum::ANSWER_SAME;
            else
                mExpectedAnswer = AnswerResponseEnum::ANSWER_DIFFERENT;

            // update State variables
            State("CurrentTrialType") = mTrialDefCurrent.trialType;
            State("CurrentTarget") = mTrialDefCurrent.targetIndex;
            State("ExpectedAnswer") = static_cast<int>(mExpectedAnswer);
            State("CurrentAnswer") = static_cast<int>(AnswerResponseEnum::ANSWER_NONE);     // reset the current answer to None at the beginning of a trial

            // print trial info to output log
            bciout << "Trial " << mCurrentTrialNum << " - type: " << strTrialTypeList[int(mTrialDefCurrent.trialType)] << ", dir: " << strCueDirectionList[int(mTrialDefCurrent.cueDirection)] << "\n"
                << "  0: " << strCueColorList[mTrialDefCurrent.searchColor[0]] << " " << strCueObjectList[mTrialDefCurrent.searchObject[0]] << "\n"
                << "  1: " << strCueColorList[mTrialDefCurrent.searchColor[1]] << " " << strCueObjectList[mTrialDefCurrent.searchObject[1]] << "\n"
                << "  2: " << strCueColorList[mTrialDefCurrent.searchColor[2]] << " " << strCueObjectList[mTrialDefCurrent.searchObject[2]] << "\n"
                << "  3: " << strCueColorList[mTrialDefCurrent.searchColor[3]] << " " << strCueObjectList[mTrialDefCurrent.searchObject[3]] << "\n"
                << "  Cue: " << mTrialDefCurrent.targetIndex << "\n";

            // get the search targets
            for (int i = 0; i < NUM_OBJECTS_PER_TRIAL; ++i)
            {
                string strCurrentTargetColor = "CurrentStimColor";
                string strCurrentTargetObject = "CurrentStimObject";

                State(strCurrentTargetColor + std::to_string(i)) = mTrialDefCurrent.searchColor[i];
                State(strCurrentTargetObject + std::to_string(i)) = mTrialDefCurrent.searchObject[i];
            }

            // display the fixation object (e.g. Olive with no arms)
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
            mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

            // set the next phase then start a timer
            if (mTrialDefCurrent.trialType == TrialTypeEnum::ATTENTION_VALID || mTrialDefCurrent.trialType == TrialTypeEnum::ATTENTION_INVALID)
                mNextTestPhase = TestPhaseEnum::TEST_CUE;       // for an Attention trial the Cue comes before the Stimulis
            else 
                mNextTestPhase = TestPhaseEnum::TEST_STIMULI;   // for a Memory trial the Stimuli comes before the Cue

            StartTimer(mTimePreCueMin, mTimePreCueMax);           // TODO: make time interval a parameter for TestPhase timing
        }

        break;

    case TestPhaseEnum::TEST_CUE:
        // Olive points in the direction of the target (i.e. the hint)
        mCueStimuliArray[mTrialDefCurrent.cueDirection]->SetObjectRect(mRectFocusCenter);
        mCueStimuliArray[mTrialDefCurrent.cueDirection]->Present();

        mNextTestPhase = TestPhaseEnum::TEST_POST_CUE;
        StartTimer(mTimeCue);

        break;

    case TestPhaseEnum::TEST_POST_CUE:
        // Olive with no arms is the fixation object
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

        // for an Attention trial the Stimuli come *after* the Cue, so now we proceed to the Stimuli
        if (mTrialDefCurrent.trialType == TrialTypeEnum::ATTENTION_VALID || mTrialDefCurrent.trialType == TrialTypeEnum::ATTENTION_INVALID)
        {
            mNextTestPhase = TestPhaseEnum::TEST_STIMULI;
        }
        else // for a Memory trial the Stimuli come *before* the Cue, so now we wait for an answer 
        {
            mNextTestPhase = TestPhaseEnum::TEST_AWAITING_ANSWER;
        }

        StartTimer(mTimePostCueMin, mTimePostCueMax);

        break;

    case TestPhaseEnum::TEST_STIMULI:
        // show the 4 stimuli around Olive
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

        for (int i = 0; i < NUM_OBJECTS_PER_TRIAL; ++i)
        {
            //bciout << "color: " << strColorList[ mTrialDefCurrent.searchColor[i] ] << " and object: " << strObjectList[ mTrialDefCurrent.searchObject[i] ];
            mSearchStimuliArray[mTrialDefCurrent.searchColor[i]][mTrialDefCurrent.searchObject[i]]->SetObjectRect(mRectSearchObjectPos[i]);
            mSearchStimuliArray[mTrialDefCurrent.searchColor[i]][mTrialDefCurrent.searchObject[i]]->Present();
        }

        mNextTestPhase = TestPhaseEnum::TEST_POST_STIMULI;
        StartTimer(mTimeStimuli);

        break;

    case TestPhaseEnum::TEST_POST_STIMULI:
        // Olive with no arms is the fixation object
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->SetObjectRect(mRectFocusCenter);
        mIntroImagesStimuliMap[INTRO_ALIEN_NO_ARMS]->Present();

        // for an Attention trial the Stimuli come *after* the Cue, so now we wait for an answer
        if (mTrialDefCurrent.trialType == TrialTypeEnum::ATTENTION_VALID || mTrialDefCurrent.trialType == TrialTypeEnum::ATTENTION_INVALID)
        {
            mNextTestPhase = TestPhaseEnum::TEST_AWAITING_ANSWER;
        }
        else // for a Memory trial the Stimuli come *before* the Cue, so now we proceed to the Cue 
        {
            mNextTestPhase = TestPhaseEnum::TEST_CUE;
        }

        StartTimer(mTimePostStimuliMin, mTimePostStimuliMax);

        break;

    case TestPhaseEnum::TEST_AWAITING_ANSWER:
        mSearchStimuliArray[ mTrialDefCurrent.searchColor[mTrialDefCurrent.targetIndex] ][mTrialDefCurrent.searchObject[mTrialDefCurrent.targetIndex] ]->SetObjectRect(mRectFocusCenter);
        mSearchStimuliArray[ mTrialDefCurrent.searchColor[mTrialDefCurrent.targetIndex] ][mTrialDefCurrent.searchObject[mTrialDefCurrent.targetIndex] ]->Present();

        mNextTestPhase = TestPhaseEnum::TEST_TRIAL_START;   // return to first stage of the next trial
        mIsAwaitingAnswer = true;

        break;

    default:
        mNextTestPhase = TestPhaseEnum::TEST_TRIAL_START;
        StartTimer(0.0);

        break;
    }// end switch

}

// Called when the user presses an answer key (e.g. A for Different or L for Same)
void KastnerCueingTask::Answer(AnswerResponseEnum userAnswer)
{
    // update the state variables
    State("CurrentAnswer") = static_cast<int>(userAnswer);

    // keep a count of number correct answers
    if (mExpectedAnswer == userAnswer)
    {
        mNumCorrectTrials++;
    }

    // print response to output log
    float percentCorrect = (mCurrentTrialNum <= 0) ? 0 : float(mNumCorrectTrials) / float(mCurrentTrialNum); // just to make sure we don't devide by zero, even though mCurrentTrialNum should never be 0 at this stage because it is incremented at the beginning of every new run
    bciout << "ExpectedAnswer: " << strAnswerList[static_cast<int>(mExpectedAnswer) ] << "\n"
        << "CurrentAnswer: " << strAnswerList[ static_cast<int>(userAnswer) ] << "\n"
        << "% Correct: " << percentCorrect;

    // change the waiting status
    mIsAwaitingAnswer = false;

    // show visual feedback if we're in the Intro Mode
    if (mCurrentMode == PresentationModeEnum::MODE_INTRO)
    {
        if (userAnswer == mExpectedAnswer)
        {
            mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
            mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

            mMessage.SetText("Great Job!\nThat was correct!")
                .SetTextColor(RGBColor::White)
                .SetTextHeight(0.1)
                .SetColor(RGBColor::NullColor)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.05, 0.05, 0.95, 0.75 })
                .Show();
        }
        else
        {
            string responseMsg;
            if (userAnswer == AnswerResponseEnum::ANSWER_DIFFERENT)
                responseMsg = "Sorry, those pieces matched\nso you should have responded by pressing['L']";
            else
                responseMsg = "Sorry, those pieces were different\nso you should have responded by pressing ['A']";

            mIntroImagesStimuliMap[INTRO_GALAXY]->SetObjectRect(mRectFullScreen);
            mIntroImagesStimuliMap[INTRO_GALAXY]->Present();

            mMessage.SetText(responseMsg + "\n\nDon't worry, we will try again!")
                .SetTextColor(RGBColor::White)
                .SetTextHeight(0.075)
                .SetColor(RGBColor::NullColor)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
                .SetObjectRect({ 0.05, 0.05, 0.95, 0.75 })
                .Show();
        }

        mIsAwaitingContinue = true;
    }
    else if (mCurrentMode == PresentationModeEnum::MODE_TEST)
    {
        mIsSwitchedScene = true;
    }
}