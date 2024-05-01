////////////////////////////////////////////////////////////////////////////////
// Authors: GT
// Description: SpatialWorkingMemoryGTTask implementation
////////////////////////////////////////////////////////////////////////////////

#include "SpatialWorkingMemoryGTTask.h"
#include "RandomGenerator.h"
#include <utility>
#include <math.h>
#include <sstream>
#include <iostream>
#include "gSTIMbox.imports.h"


RegisterFilter( SpatialWorkingMemoryGTTask, 3 );
     // Change the location if appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations within SignalProcessing modules begin with "2."
     //       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
     //  - filter locations within Application modules begin with "3."


SpatialWorkingMemoryGTTask::SpatialWorkingMemoryGTTask():
mr_window( Window() )
{ 
    BEGIN_PARAMETER_DEFINITIONS
    // general parameters
    "Application:SpatialWorkingMemoryGTTask int EncodingDimension= 4 // dimension of encoding pattern",
    "Application:SpatialWorkingMemoryGTTask int GridRow= 5 // Number of rows",
    "Application:SpatialWorkingMemoryGTTask int GridColumn= 8 // Number of columns",

    "Application:SpatialWorkingMemoryGTTask float GridUpperY= 0.05 // 0 means the upper edge of the window",
    "Application:SpatialWorkingMemoryGTTask float GridLowerY= 0.95 // 1 means the lower edge of the window",
    "Application:SpatialWorkingMemoryGTTask float GridLeftX= 0.1 // 0 means the left edge of the window",
    "Application:SpatialWorkingMemoryGTTask float GridRightX= 0.9 // 1 means the right edge of the window",
    "Application:SpatialWorkingMemoryGTTask float EncodingDotWidth= 0.07f // the width of the ellipse",
    "Application:SpatialWorkingMemoryGTTask int EncodingDotColor= 0x542323 0x542323 % % // the color of encoding dots",  
    "Application:SpatialWorkingMemoryGTTask bool EnableDistratorInside= 0 0 % % // 1 means half of the blank grid will be filled with red rectangle",
    "Application:SpatialWorkingMemoryGTTask bool EnableDistratorOutside= 0 0 % % // 1 means one or more red rectangles will show up outside the encoding pattern",
    "Application:SpatialWorkingMemoryGTTask int NumberOfDistratorOutside= 1 1 % % // the number red rectangle will show up outside the encoding pattern",
        // we use circle, so only radius
      //  "Application:SpatialWorkingMemoryGTTask float EncodingDotHeight= 0.1f // the height",
    "Application:SpatialWorkingMemoryGTTask int m_num_trials= 10 // Number of trials",
    "Application:SpatialWorkingMemoryGTTask string YesResponseButton= Left // define button to answer yes, please dont change",
    "Application:SpatialWorkingMemoryGTTask string NoResponseButton= Right // define button to answer no",

    "Application:SpatialWorkingMemoryGTTask int FixationCrossColor= 0xFF0000 0xFF0000 % % ",
    "Application:SpatialWorkingMemoryGTTask string InstructionFile= ..\\tasks\\spatialWM\\instruction_4by4.bmp"
    " // path for instruction image",

    // stim parameters
    "Application:SpatialWorkingMemoryGTTask string ActiveStimSide= Left"
    " // Can only be Left or Right",
    "Application:SpatialWorkingMemoryGTTask string ActiveStimSite= Stim"
    " // Can only be Stim (Concha) or Sham (Earlobe)",
    "Application:SpatialWorkingMemoryGTTask int ActiveStimFreq= 6"
    " // initialize active_stim_freq, when stim is activated, it will deliver at this frequency",

        // parameters for baseline
        "Application:SpatialWorkingMemoryGTTask float BaselineBeginningDuration= 20000ms 20000ms 0 % // baseline phase at the begining lasts for 10s",
        "Application:SpatialWorkingMemoryGTTask float BaselineEndDuration= 20000ms 20000ms 0 % // baseline phase at the end lasts for 20s",

    // parameters for beforestart
    "Application:SpatialWorkingMemoryGTTask string TextPressToStart= Press%20to%20start // Text in phase before start",
    "Application:SpatialWorkingMemoryGTTask float BeforeStartDuration= 100000ms 100000ms 0 % // beforestart phase lasts for 100s",

    // parameters for background
    "Application:SpatialWorkingMemoryGTTask float BackgroundDurationMean= 1000ms 1000ms 0 % // background lasts for about 1s",
    "Application:SpatialWorkingMemoryGTTask float BackgroundDurationVariation= 500ms 500ms 0 % // background lasts from 0.5-1.5s",
    "Application:SpatialWorkingMemoryGTTask float BackgroundDurationRandomizationResolution= 1000 1000 0 %"
        "// randomly choose from 1000 pts ranging from -500 to 500",

    // parameters for cue
    "Application:SpatialWorkingMemoryGTTask float HorizontalRectangleOfCrossHeight= 0.02f 0.02f 0 % // The height of the horizontal bar",
    "Application:SpatialWorkingMemoryGTTask float HorizontalRectangleOfCrossWidth= 0.08f 0.08f 0 % // The width of the horizontal bar",
    "Application:SpatialWorkingMemoryGTTask float CueDuration= 1000ms 1000ms 0 % // cue phase lasts for 1",

    // parameters for encoding
    "Application:SpatialWorkingMemoryGTTask float EncodingDuration= 1000ms 1000ms 0 % // encoding phase lasts for 1s",
    "Application:SpatialWorkingMemoryGTTask float DelayDuration= 1000ms 1000ms 0 % // delay phase lasts for 1s",
    "Application:SpatialWorkingMemoryGTTask float RetrivalDuration= 2000ms 2000ms 0 % // retrival phase lasts for 2s",
    "Application:SpatialWorkingMemoryGTTask float FeedbackDuration= 1000ms 1000ms 0 % // feedback phase lasts for 1s",

    // parameters for retrival
    "Application:SpatialWorkingMemoryGTTask int NumberOfDotsInRetrival= 1 1 0 % // 1 dot will appear during retrival", 

    // parameters for waiting for start phase
    "Application:SpatialWorkingMemoryGTTask string TextWaitingToStart= Waiting%20to%20start // Text in phase before start",
    "Application:SpatialWorkingMemoryGTTask float WaitingForStartDuration= 2000ms 2000ms 0 % // waitforstart phase lasts for 2s",
    "Application:SpatialWorkingMemoryGTTask string TextCorrect= Correct // Text of correct response",

    // parameters for score phase
    "Application:SpatialWorkingMemoryGTTask float ScoreDuration= 1000ms 1000ms 0 % // score appears for 3s",

    //photodiode
    "Application:PhotoDiodePatch int PhotoDiodePatch= 1 1 0 1 // Display photo diode patch (boolean)",
    "Application:PhotoDiodePatch float PhotoDiodePatchHeight= 0.065 1 0 1 // Photo diode patch height in relative coordinates",
    "Application:PhotoDiodePatch float PhotoDiodePatchWidth= 0.05 1 0 1 // Photo diode patch width in relative coordinates",
    "Application:PhotoDiodePatch float PhotoDiodePatchLeft= 0 1 0 1 // Photo diode patch left in relative coordinates",
    "Application:PhotoDiodePatch float PhotoDiodePatchTop= 0.935 1 0 1 // Photo diode patch top in relative coordinates",
    "Application:PhotoDiodePatch int PhotoDiodePatchShape= 0 1 0 1 // Photo diode patch shape: 0 rectangle, 1 ellipse (enumeration)",
    "Application:PhotoDiodePatch int PhotoDiodePatchActiveColor= 0xffffff 0 0 0xffffffff // Photo diode patch color when active (color)",
    "Application:PhotoDiodePatch int PhotoDiodePatchInactiveColor= 0x000000 0 0 0xffffffff // Photo diode patch color when inactive (default black) "
       " use 0xff000000 for transparent (color)",
    END_PARAMETER_DEFINITIONS

    BEGIN_STATE_DEFINITIONS
    "CurrentTrial               8  0 0 0", // The Current Trial, 8 bits = 256
    "CueCoordinateX            16  0 0 0", // The x coordinate of the cue
    "CueCoordinateY            16  0 0 0", // The y coordinate of the cue
    "CueCoordinateRow             4  0 0 0", // if the grid dimension is even,
    // X coordinate row encodes the index of grid line, 
        //if the dimension is odd, this variable encodes the row
    "CueCoordinateColumn        4  0 0 0",
    "Phase                      5  0 0 0",  // 1: e_before_start, 2: e_background, 
                        // 3: e_cue, 4: e_encoding, 5: e_delay, 6: e_retrival,
                        // 7: e_feedback, 8: e_wait_to_start, 9:score

    "EncodingPatternRow1          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the 1st row of grid,
        // to decode it, we need to turn it into a binary vector (of length 16)
        // 1 at index i suggests there are dots present of ith col in the 1st row.
        // for example, (0 1 1 0 1 0 0 0) will turn into 10-based int: 2^3 + 2^5 + 2^6
    "EncodingPatternRow2          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "EncodingPatternRow3          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "EncodingPatternRow4          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "EncodingPatternRow5          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "EncodingPatternRow6          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,


    "DistratorPatternRow1          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the 1st row of grid,
    // to decode it, we need to turn it into a binary vector (of length 16)
    // 1 at index i suggests there are dots present of ith col in the 1st row.
    // for example, (0 1 1 0 1 0 0 0) will turn into 10-based int: 2^3 + 2^5 + 2^6
    "DistratorPatternRow2          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "DistratorPatternRow3          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "DistratorPatternRow4          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "DistratorPatternRow5          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "DistratorPatternRow6          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,

    "DistratorInsidePatternRow1          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the 1st row of grid,
    // to decode it, we need to turn it into a binary vector (of length 16)
    // 1 at index i suggests there are dots present of ith col in the 1st row.
    // for example, (0 1 1 0 1 0 0 0) will turn into 10-based int: 2^3 + 2^5 + 2^6
    "DistratorInsidePatternRow2          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "DistratorInsidePatternRow3          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "DistratorInsidePatternRow4          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "DistratorInsidePatternRow5          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,
    "DistratorInsidePatternRow6          16 0 0 0", // (0 1 1 0 ...) to encode the presence of dot in the grid,

    "BackgroundDuration             16 0 0 0", // (0 1 1 0 ...) encode 0.5s with 256 interval ,
    "GroundTruthResponseRetrival    1 0 0 0", // 1 means that the subject is supposed to press yes ,
    "RetrivalEvaluation             2 0 0 0",  // This state encodes if the participant's response is correct (1) or not (0), if no response =2, in baseline =3
    "RetrivalReactionTime           16 0 0 0",  // This state encodes participant's reaction time
    "ActiveStim                     1 0 0 0",  // This state encodes stimulation of the VN
    "ShamStim                       1 0 0 0",  // This state encodes stimulation of the earlobe
    "NumStopDuringTheTask              4 0 0 0 ",  // This state encodes whether or not the task is interrupted
    "ActiveStimFreq                8 0 0 0",  // This state encodes stimulation of the VN
        

    END_STATE_DEFINITIONS




}

SpatialWorkingMemoryGTTask::~SpatialWorkingMemoryGTTask()
{
  Halt();
  // If you have allocated any memory with malloc(), call free() here.
  // If you have allocated any memory with new[], call delete[] here.
  // If you have created any object using new, call delete here.
  // Either kind of deallocation will silently ignore null pointers, so all
  // you need to do is to initialize those to zero in the constructor, and
  // deallocate them here.
}


 

void
SpatialWorkingMemoryGTTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
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

  // Note that the SpatialWorkingMemoryGTTask instance itself, and its members, are read-only during
  // this phase, due to the "const" at the end of the Preflight prototype above.
  // Any methods called by Preflight must also be "const" in the same way.

  Output = Input; // this simply passes information through about SampleBlock dimensions, etc....
  State("Running");
  State("KeyDown");
  State("Phase");
  State("CueCoordinateX");
  State("CueCoordinateY");
  State("CueCoordinateRow");
  State("CueCoordinateColumn");
  State("EncodingPatternRow1");
  State("EncodingPatternRow2");
  State("EncodingPatternRow3");
  State("EncodingPatternRow4");
  State("EncodingPatternRow5");
  State("EncodingPatternRow6");
  State("DistratorPatternRow1");
  State("DistratorPatternRow2");
  State("DistratorPatternRow3");
  State("DistratorPatternRow4");
  State("DistratorPatternRow5");
  State("DistratorPatternRow6");
  State("DistratorInsidePatternRow1");
  State("DistratorInsidePatternRow2");
  State("DistratorInsidePatternRow3");
  State("DistratorInsidePatternRow4");
  State("DistratorInsidePatternRow5");
  State("DistratorInsidePatternRow6");
  State("GroundTruthResponseRetrival");
  State("CurrentTrial");
  State("RetrivalEvaluation");
  State("RetrivalReactionTime");

  // sampling rate and block size have been defined in the parent class
  Parameter("SamplingRate");
  Parameter("SampleBlockSize");
  Parameter("m_num_trials");
  Parameter("YesResponseButton");
  Parameter("NoResponseButton");
  Parameter("TextPressToStart");
  Parameter("TextWaitingToStart");
  Parameter("GridRow");
  Parameter("GridColumn");
  Parameter("GridLowerY");
  Parameter("GridUpperY");
  Parameter("GridLeftX");
  Parameter("GridRightX");
  Parameter("HorizontalRectangleOfCrossHeight");
  Parameter("HorizontalRectangleOfCrossWidth");
  Parameter("EncodingDimension");
  Parameter("DelayDuration");
  Parameter("ScoreDuration");
  Parameter("CueDuration");
  Parameter("EncodingDuration");
  Parameter("WindowWidth");
  Parameter("WindowHeight");
  Parameter("EncodingDotWidth");
  Parameter("NumberOfDotsInRetrival");
  Parameter("EncodingDotColor");
  Parameter("InstructionFile");
  Parameter("ActiveStimFreq");
  Parameter("EnableDistratorOutside");
  Parameter("EnableDistratorInside");
  Parameter("NumberOfDistratorOutside");
}


void
SpatialWorkingMemoryGTTask::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The signal properties can no longer be modified, but the const limitation has gone, so
  // the SpatialWorkingMemoryGTTask instance itself can be modified. Allocate any memory you need, start any
  // threads, store any information you need in private member variables.
    m_current_state = e_before_start; // participant presses a button to start
    cross_location_x = 0;
    cross_location_y = 0;
    cross_location_x_id = 0;
    cross_location_y_id = 0;
    //  window_width = Parameter("WindowWidth");
    // window_height = Parameter("WindowHeight");
    m_sample_rate = Parameter("SamplingRate");
    m_block_size = Parameter("SampleBlockSize");
    m_runtime_counter = 0;
    m_trial_counter = 0;
    number_of_blocks_passed = 0;
    m_num_trials = Parameter("m_num_trials");
    std::string button_to_respond_yes = Parameter("YesResponseButton");
    std::string button_to_respond_no = Parameter("NoResponseButton");
    m_text_press_to_start = std::string(Parameter("TextPressToStart"));
    m_text_waiting_to_start = std::string(Parameter("TextWaitingToStart"));

    // block design, convert time into blocks, m_block_size_msec means the duration of one block
    // for example, block size defaults to 32, sampling freq = 256Hz, the duration of a block is 1000/8 = 125 ms
    m_block_size_msec = (float)(m_block_size) / (float)(m_sample_rate) * 1000;
    m_beforestart_duration_blocks = Parameter("BeforeStartDuration").InMilliseconds() / m_block_size_msec;
    m_waitingforstart_duration_blocks = Parameter("WaitingForStartDuration").InMilliseconds() / m_block_size_msec;
    m_cue_duration_blocks = Parameter("CueDuration").InMilliseconds() / m_block_size_msec;
    m_encoding_duration_blocks = Parameter("EncodingDuration").InMilliseconds() / m_block_size_msec;
    m_delay_duration_blocks = Parameter("DelayDuration").InMilliseconds() / m_block_size_msec;
    m_retrival_duration_blocks = Parameter("RetrivalDuration").InMilliseconds() / m_block_size_msec;
    m_feedback_duration_blocks = Parameter("FeedbackDuration").InMilliseconds() / m_block_size_msec;
    m_score_duration_blocks = Parameter("ScoreDuration").InMilliseconds() / m_block_size_msec;
    m_baseline_begining_duration_blocks = Parameter("BaselineBeginningDuration").InMilliseconds() / m_block_size_msec;
    m_baseline_end_duration_blocks = Parameter("BaselineEndDuration").InMilliseconds() / m_block_size_msec;
    background_duration_msec = Parameter("BackgroundDurationMean").InMilliseconds();

    // check block size and sampling freq
    AppLog << "Sampling freq is:" << Parameter("SamplingRate") << std::endl;
    AppLog << "Block size is:" << Parameter("SampleBlockSize") << std::endl;
    AppLog << "if 2000Hz, 200 block size, before-starts blocks should be 1000, actual value is:" << m_beforestart_duration_blocks << std::endl;

    // check Application window size
    AppLog << "Display window width is:" << Parameter("WindowWidth") << std::endl;
    AppLog << "Display window height is:" << Parameter("WindowHeight") << std::endl;
    AppLog << "The patient cart window size is 2560*1440, 960 * 540 in test mode" << std::endl;
  
    // initialize stim freq
    active_stim_freq = Parameter("ActiveStimFreq");

    // define grid properties
    grid_not_created = true;
    n_grid_row = Parameter("GridRow") + 1;
    n_grid_col = Parameter("GridColumn") + 1;
    grid_lower_y = Parameter("GridLowerY");
    grid_upper_y = Parameter("GridUpperY");
    grid_left_x = Parameter("GridLeftX");
    grid_right_x = Parameter("GridRightX");


    grid_row_y = evenly_spaced_floats(Parameter("GridUpperY"), Parameter("GridLowerY"), n_grid_row);
    grid_col_x = evenly_spaced_floats(Parameter("GridLeftX"), Parameter("GridRightX"), n_grid_col);

// initialize rectangle
    rect = { 0.0f, 0.2f, 1.0f, 0.8f};
// rect {a, b, c, d}, a, left-top x; b, let-top y, lower = higher y, righter = higher x
    for (int i = 0; i < n_grid_row; i++) {
        rect = { grid_left_x, grid_row_y[i], grid_right_x, grid_row_y[i] };
        /*
        if (i == 0) { // upper horizontal line, higher 
            rect = { 0.0f, grid_row_y[i] , 1.0f, grid_row_y[i]};
        }
        else if (i == n_grid_row - 1) {
            rect = { 0.0f, grid_row_y[i]- , 1.0f, grid_row_y[i]-0.01f };
            
        }
        
        AppLog << "check grid coordinates";
        AppLog << "the y of" << i << "th grid row : " << grid_row_y[i] << std::endl;
        */
        grid_rectangle = new RectangularShape(mr_window);
        grid_rectangle->SetFillColor(RGBColor::White);
        grid_rectangle->SetDisplayRect(rect);
        grid_rectangle->SetVisible(false);
        grid_rect_pointer_vector.push_back(grid_rectangle);
    };

    for (int i = 0; i < n_grid_col; i++) {
        rect = { grid_col_x[i], grid_lower_y, grid_col_x[i], grid_upper_y };

        /*
        if (i == 0) { // rightmost verticle line
            rect = { grid_col_x[i] , 0.0f, grid_col_x[i] , 1.0f };
        }
        else if (i == n_grid_col - 1) {
            rect = { grid_col_x[i] - 0.01f , 0.0f, grid_col_x[i] - 0.01f, 1.0f };
        }
        */

        grid_rectangle = new RectangularShape(mr_window);
        grid_rectangle->SetFillColor(RGBColor::White);
        grid_rectangle->SetDisplayRect(rect);
        grid_rectangle->SetVisible(false);
        grid_rect_pointer_vector.push_back(grid_rectangle);

    };
    /*
    bciout << "grid_rect_pointer_vector size after pushback";
    bciout << grid_rect_pointer_vector.size();
    bciout << "initialization accomplished";
    */

    // initialize the fixation cross and its potential locations
    // here we will need to define the center of the horizontal rectangle, the verticle one would be 90 degree rotation from the horizontal one
    horizontal_rect_cross_height = Parameter("HorizontalRectangleOfCrossHeight");
    horizontal_rect_cross_width = Parameter("HorizontalRectangleOfCrossWidth");

    cue_coordinate_row = 0;
    cue_coordinate_column = 0;

    // if the grid dimension is even, we will expect to see the cross center on the grid

    for (int i = 0; i < 2; i++) {
        grid_rectangle = new RectangularShape(mr_window);
        grid_rectangle->SetFillColor(RGBColor::White);
        //grid_rectangle->SetDisplayRect(rect);
        grid_rectangle->SetVisible(false);
        cross_rect_pointer_vector.push_back(grid_rectangle);
    }
    // now we find the candidate location
    width_height_ratio = Parameter("WindowWidth") / Parameter("WindowHeight"); // to adjust the width of the cross rectangle
    grid_center_1st_row = grid_upper_y + (grid_lower_y - grid_upper_y) / (n_grid_row - 1) / 2;
    grid_center_last_row = grid_lower_y - (grid_lower_y - grid_upper_y) / (n_grid_row - 1) / 2;
    grid_center_1st_col = grid_left_x + (grid_right_x - grid_left_x) / (n_grid_col - 1) / 2;
    grid_center_last_col = grid_right_x - (grid_right_x - grid_left_x) / (n_grid_col - 1) / 2;

   //  AppLog << "the center of 1st row and 1st col is (" << grid_center_1st_col << "," << grid_center_1st_row <<  ")" << std::endl;

    grid_center_y = evenly_spaced_floats(grid_center_1st_row, grid_center_last_row, Parameter("GridRow"));  // y coord for the center of each square within the grid
    grid_center_x = evenly_spaced_floats(grid_center_1st_col, grid_center_last_col, Parameter("GridColumn"));
    encoding_dim = Parameter("EncodingDimension");
    if (encoding_dim % 2 == 0) {
        // cross_candidate_location_x_ids = evenly_spaced_ints(encoding_dim / 2 + 1, n_grid_col - encoding_dim / 2, n_grid_col - encoding_dim);
        // grid row y is a vector, that has row + 1 length
        // if encoding dim = 4, cross candidate location y starts from the third row from the top
        cross_candidate_location_y =  slice(grid_row_y, encoding_dim / 2, n_grid_row - encoding_dim / 2 - 1);
        cross_candidate_location_x =  slice(grid_col_x, encoding_dim / 2, n_grid_col - encoding_dim / 2 - 1);
    }
        // odd encoding dimension
    else {
        space_from_edge = (encoding_dim - 1) / 2;
        // indices for vector grid_center_y
        // cross_candidate_location_x_ids = evenly_spaced_ints(space_from_edge, Parameter("GridColumn") - space_from_edge - 1, Parameter("GridColumn") - space_from_edge * 2);
        // cross_candidate_location_y_ids = evenly_spaced_ints(space_from_edge, Parameter("GridRow") - space_from_edge - 1, Parameter("GridRow") - space_from_edge * 2);
        cross_candidate_location_y = slice(grid_center_y, space_from_edge, Parameter("GridRow") - space_from_edge - 1);
        cross_candidate_location_x = slice(grid_center_x, space_from_edge, Parameter("GridColumn") - space_from_edge - 1);
    }

    // now we initialize variables that are necessary to show dots for encoding
    // the idea is to define two vectors coding grid center coordinates
    // then we create a boolean vector to define whether or not to present the dots
    // we will have to also declare ptr for a vector of ellipse objects
    // In cue phase, we will know the center of encoding pattern
    // Then we will define a vector containing the index of the squares
    // Then we will randomly assign ones for the selected squares
    if (encoding_dim % 2 == 0) {
        candidate_location_number = encoding_dim * encoding_dim / 2;
    }
    else {
        candidate_location_number = (encoding_dim * encoding_dim - 1) /2;
    }
    for (int i = 0; i < candidate_location_number; i++) {
        encoding_dots = new EllipticShape(mr_window);
        encoding_dots->SetColor(Parameter("EncodingDotColor").ToNumber());
        encoding_dots->SetFillColor(Parameter("EncodingDotColor").ToNumber());
        encoding_dots->SetPositionX(1); // location
        encoding_dots->SetWidth(Parameter("EncodingDotWidth"));
        encoding_dots->SetHeight(Parameter("EncodingDotWidth") * width_height_ratio);
        encoding_dots->SetVisible(false);
        encoding_circle_pointer_vector.push_back(encoding_dots);
    }

    // we initialize rectangles as distrators
    if (Parameter("EnableDistratorOutside") == 1)
    {
        n_distrator_outside = Parameter("NumberOfDistratorOutside");
        for (int i = 0; i < n_distrator_outside; i++) {
            distrator_rect = new RectangularShape(mr_window);
            distrator_rect->SetColor(Parameter("EncodingDotColor").ToNumber());
            distrator_rect->SetFillColor(Parameter("EncodingDotColor").ToNumber());
            distrator_rect->SetPositionX(1); // location
            distrator_rect->SetWidth(Parameter("EncodingDotWidth"));
            distrator_rect->SetHeight(Parameter("EncodingDotWidth") * width_height_ratio);
            distrator_rect->SetVisible(false);
            distrator_outside_pointer_vector.push_back(distrator_rect);
        }
    }
    else { n_distrator_outside = 0; }

    if (Parameter("EnableDistratorInside") == 1)
    {
        if (encoding_dim % 2 == 0) {
            n_distrator_inside = encoding_dim * encoding_dim / 2 / 2;
        }
        else {
            n_distrator_inside = (encoding_dim * encoding_dim - 1) / 2 / 2;
        }

        for (int i = 0; i < n_distrator_inside; i++) {
            distrator_rect = new RectangularShape(mr_window);
            distrator_rect->SetColor(Parameter("EncodingDotColor").ToNumber());
            distrator_rect->SetFillColor(Parameter("EncodingDotColor").ToNumber());
            distrator_rect->SetPositionX(1); // location
            distrator_rect->SetWidth(Parameter("EncodingDotWidth"));
            distrator_rect->SetHeight(Parameter("EncodingDotWidth") * width_height_ratio);
            distrator_rect->SetVisible(false);
            distrator_inside_pointer_vector.push_back(distrator_rect);
        }
    }
    else { n_distrator_inside = 0; }

    // AppLog << "n_distrator_inside is" << n_distrator_inside << std::endl;
   
        

    // initialize the grid center
  // we expand grid_center_x into a vector of length Parameter("GridRow") * Parameter("GridColumn")
    grid_center_coordinate_x = std::vector<float>(Parameter("GridRow") * Parameter("GridColumn"));
    grid_center_coordinate_y = std::vector<float>(Parameter("GridRow") * Parameter("GridColumn"));

    // grid_center_coordinate_x[ i * column + j - 1] represent the coordinate of ith row and jth col.
    for (int row = 0; row < Parameter("GridRow"); ++row) {
        for (int col = 0; col < Parameter("GridColumn"); ++col) {

            //float x = grid_center_1st_col + col * 2 * 1 / Parameter("GridColumn") / 2;
            //float y = grid_center_1st_row + row * 2 * 1 / Parameter("GridRow") / 2;
            // Store the coordinates in the respective vectors
            grid_center_coordinate_x[row * (n_grid_col - 1) + col] = grid_center_x[col];
            grid_center_coordinate_y[row * (n_grid_col - 1) + col] = grid_center_y[row];
            // AppLog << "check grid center coordinates" << std::endl;
            // AppLog << "the center of" << row << "th row, " << col <<"th col is: ("<< x << ", " << y << ")" << std::endl;
        }
    }

    // predefine bool_v_of_encoding_pattern_row_ten_based
    row_encoding_pattern_vector = std::vector<int>(n_grid_row - 1);
    row_distrator_pattern_vector = std::vector<int>(n_grid_row - 1);
    row_distrator_inside_pattern_vector = std::vector<int>(n_grid_row - 1);


    n_dots_in_retrival = Parameter("NumberOfDotsInRetrival");
    // we define the potential locations of the circle
    // we later take a subset of inputs include the center of the grid and the dimension

    // initialize for the feedback phase
    response_ground_truth = false;
    reaction_time = 0;
    retrival_evaluation_code = 3;
    n_correct_response = 0;

    // initialize two variables to code whether stimulate VN
    active_stim_this_trial = false;
    sham_stim_this_trial = false;

    // show instruction at the first trial
    m_is_first_trial = true;

    // initialize the stop state
    num_stop_during_the_task = 0;

    // set text stimulus
    GUI::Rect     m_full_rectangle;
    m_full_rectangle.left = 0.0f;
    m_full_rectangle.right = 1.0f;
    m_full_rectangle.top = 0.1f;
    m_full_rectangle.bottom = 0.5f;

    mp_text_stimulus = new TextStimulus(mr_window);
    mp_text_stimulus->SetText("");
    mp_text_stimulus->SetTextHeight(0.2f);
    mp_text_stimulus->SetDisplayRect(m_full_rectangle);
    mp_text_stimulus->SetColor(RGBColor::Black);
    mp_text_stimulus->SetTextColor(RGBColor::Green);
    mp_text_stimulus->Present();
    mp_text_stimulus->SetVisible(false);

    m_full_rectangle.left = 0.0f;
    m_full_rectangle.right = 1.0f;
    m_full_rectangle.top = 0.4f;
    m_full_rectangle.bottom = 0.8f;

    rt_feedback_text_stimulus = new TextStimulus(mr_window);
    rt_feedback_text_stimulus->SetText("");
    rt_feedback_text_stimulus->SetTextHeight(0.2f);
    rt_feedback_text_stimulus->SetDisplayRect(m_full_rectangle);
    rt_feedback_text_stimulus->SetColor(RGBColor::Black);
    rt_feedback_text_stimulus->SetTextColor(RGBColor::Green);
    rt_feedback_text_stimulus->Present();
    rt_feedback_text_stimulus->SetVisible(false);

    // set image stimulus (instruction)
    instruction_image_rectangle.left = 0.49f;
    instruction_image_rectangle.right = 0.51f;
    instruction_image_rectangle.top = 0.49f;
    instruction_image_rectangle.bottom = 0.51f;

    instruction_image = new ImageStimulus(mr_window);
    instruction_image->SetDisplayRect(instruction_image_rectangle);
    instruction_image->SetPresentationMode(VisualStimulus::ShowHide);
    instruction_image->SetAspectRatioMode(GUI::AspectRatioModes::AdjustBoth);
    instruction_image->SetFile(Parameter("InstructionFile"));
    instruction_image->Conceal();

    //photodiode
    delete mPhotoDiodePatch.pShape;
    mPhotoDiodePatch.pShape = nullptr;
    if (Parameter("PhotoDiodePatch") != 0)
    {
        mPhotoDiodePatch.activeColor = Parameter("PhotoDiodePatchActiveColor").ToNumber();
        mPhotoDiodePatch.inactiveColor = Parameter("PhotoDiodePatchinActiveColor").ToNumber();
        enum { rectangle = 0, ellipse = 1 };
        switch (int(Parameter("PhotoDiodePatchShape")))
        {
        case rectangle:
            mPhotoDiodePatch.pShape = new RectangularShape(Display(), -1);
            break;
        case ellipse:
        default:
            mPhotoDiodePatch.pShape = new EllipticShape(Display(), -1);
            break;
        }
        mPhotoDiodePatch.pShape->SetHeight(Parameter("PhotoDiodePatchHeight"))
            .SetWidth(Parameter("PhotoDiodePatchWidth"))
            .SetPositionX(Parameter("PhotoDiodePatchLeft") + Parameter("PhotoDiodePatchWidth") / 2.0)
            .SetPositionY(Parameter("PhotoDiodePatchTop") + Parameter("PhotoDiodePatchHeight") / 2.0);
        Patch(0);
    }

    AppLog << "initialization done" << std::endl;

    

}

void
SpatialWorkingMemoryGTTask::StartRun()
{
  // The user has just pressed "Start" (or "Resume")

}


void
SpatialWorkingMemoryGTTask::Process(const GenericSignal& Input, GenericSignal& Output)
{

    // And now we're processing a single SampleBlock of data.
    // Remember not to take too much CPU time here, or you will break the real-time constraint.

    Output = Input; // This passes the signal through unmodified.
   // bciout << "enter process";
   // bciout << m_current_state;
    // after m_num_trials, we stop the process
    if (m_trial_counter >= m_num_trials) // 
    {
        return;
    }

    if (m_is_first_trial)
    {   
        Patch(1);
        instruction_image->Present();
        m_current_state = e_instruction;
        m_is_first_trial = false;
    }


    // keep track of the states
    State("Phase") = m_current_state;
    State("CueCoordinateX") = cross_location_x * 1000;
    State("CueCoordinateY") = cross_location_y * 1000;
    State("CueCoordinateRow") = cue_coordinate_row;
    // (encoding_dim % 2 == 0) ? (cross_location_y_id + encoding_dim / 2 + 1) : (cross_location_y_id + (encoding_dim + 1) / 2);
    State("CueCoordinateColumn") = cue_coordinate_column;
    // (encoding_dim % 2 == 0) ? (cross_location_x_id + encoding_dim / 2 + 1) : (cross_location_x_id + (encoding_dim + 1) / 2);
    State("EncodingPatternRow1") = row_encoding_pattern_vector[0];
    State("EncodingPatternRow2") = row_encoding_pattern_vector[1];
    State("EncodingPatternRow3") = row_encoding_pattern_vector[2];
    State("EncodingPatternRow4") = row_encoding_pattern_vector[3];
    State("EncodingPatternRow5") = row_encoding_pattern_vector[4];
    State("EncodingPatternRow6") = row_encoding_pattern_vector[5];
    State("DistratorPatternRow1") = row_distrator_pattern_vector[0];
    State("DistratorPatternRow2") = row_distrator_pattern_vector[1];
    State("DistratorPatternRow3") = row_distrator_pattern_vector[2];
    State("DistratorPatternRow4") = row_distrator_pattern_vector[3];
    State("DistratorPatternRow5") = row_distrator_pattern_vector[4];
    State("DistratorPatternRow6") = row_distrator_pattern_vector[5];
    State("DistratorInsidePatternRow1") = row_distrator_inside_pattern_vector[0];
    State("DistratorInsidePatternRow2") = row_distrator_inside_pattern_vector[1];
    State("DistratorInsidePatternRow3") = row_distrator_inside_pattern_vector[2];
    State("DistratorInsidePatternRow4") = row_distrator_inside_pattern_vector[3];
    State("DistratorInsidePatternRow5") = row_distrator_inside_pattern_vector[4];
    State("DistratorInsidePatternRow6") = row_distrator_inside_pattern_vector[5];
    State("GroundTruthResponseRetrival") = response_ground_truth;
    State("CurrentTrial") = m_trial_counter;
    State("BackgroundDuration") = background_duration_msec;
    State("RetrivalReactionTime") = reaction_time;
    State("RetrivalEvaluation") = retrival_evaluation_code;
    State("ActiveStim") = active_stim_this_trial;
    State("ShamStim") = sham_stim_this_trial;
    State("NumStopDuringTheTask") = num_stop_during_the_task;
    State("ActiveStimFreq") = active_stim_freq;

    // state machine to switch states
    switch (m_current_state)
    {
        //////////////////////////////////////////////////////////////////////////

    case e_instruction:
    {

        // present a text stimulus "press to start"
        number_of_blocks_passed++;
        if (IsButtonPressed()) {
            // show a cross
            mp_text_stimulus->SetText("+");
            mp_text_stimulus->SetVisible(true);
            mp_text_stimulus->Present();
            m_current_state = e_baseline_beginning;
            number_of_blocks_passed = 0;
            instruction_image->Conceal();
            AppLog << "instruction read" << std::endl;
            Patch(1);
        }
    }
    break;

    case e_baseline_beginning:
    {

        // present a text stimulus "press to start"
        number_of_blocks_passed++;
        if (number_of_blocks_passed >= m_baseline_begining_duration_blocks) {
            m_current_state = e_before_start;
            mp_text_stimulus->SetVisible(false);
            mp_text_stimulus->Conceal();
            number_of_blocks_passed = 0;
            AppLog << "enter before start" << std::endl;
            Patch(0);
        }
    }
    break;
    //////////////////////////////////////////////////////////////////////////

    case e_before_start:
    {

        mp_text_stimulus->SetText(m_text_press_to_start);
        mp_text_stimulus->SetVisible(true);
        mp_text_stimulus->Present();
        // present a text stimulus "press to start"
        number_of_blocks_passed++;

        // check if the participant press the button first trial period has expired
        if (IsButtonPressed())
            //if (IsButtonPressed() || (number_of_blocks_passed >= m_beforestart_duration_blocks)) // if maximal waiting time is passed
        {
            // define the duration for next state
            random_number = RandomNumber(0, Parameter("BackgroundDurationRandomizationResolution"));
            background_duration_msec = Parameter("BackgroundDurationMean").InMilliseconds() +
                Parameter("BackgroundDurationVariation").InMilliseconds() *
                (random_number - (Parameter("BackgroundDurationRandomizationResolution") / 2)) / Parameter("BackgroundDurationRandomizationResolution") * 2;

            m_background_duration_blocks = background_duration_msec / m_block_size_msec;

            /*
            bciout << "background duration";
            bciout << background_duration_msec;
            bciout << "after n blocks we will switch to the next state";
            bciout << m_background_duration_blocks;
            */

            // define stim configuration
            active_stim_this_trial = RandomNumber(0, 1);
            sham_stim_this_trial = RandomNumber(0, 1);

            // go to next state
            mp_text_stimulus->Conceal();
            // mp_text_stimulus->SetVisible(false);
            m_current_state = e_background;
            number_of_blocks_passed = 0;
            Patch(1);
            AppLog << "enter background phase" << std::endl;
        }
    }
    break;

    case e_background:
    {

        // make the grid visible

        for (int i = 0; i < n_grid_row + n_grid_col; i++) {
            grid_rect_pointer_vector[i]->SetVisible(true);
        }

        number_of_blocks_passed++;


        if (number_of_blocks_passed >= m_background_duration_blocks) // if maximal waiting time is passed
        {
            /*
            bciout << "grid_rect_pointer_vector size before wait to start";
            bciout << grid_rect_pointer_vector.size();
            */

            // go to next state
            m_current_state = e_cue;
            AppLog << "enter cue phase" << std::endl;
            number_of_blocks_passed = 0;
            // randomly pick an x and y coordinates
            // in dim=4, when id = 0, means the cross will appear at the third row
            cross_location_y_id = RandomNumber(0, cross_candidate_location_y.size() - 1);
            cross_location_y = cross_candidate_location_y[cross_location_y_id];
            cross_location_x_id = RandomNumber(0, cross_candidate_location_x.size() - 1);
            cross_location_x = cross_candidate_location_x[cross_location_x_id];

            // AppLog << "cross_location_x" << cross_location_x << std::endl;

            // calculate the top-left corner and button-right corner coordinates
            // cross_rect_pointer_vector[0] is the horizontal rectangle
            rect = { cross_location_x - horizontal_rect_cross_width / 2,
                cross_location_y - horizontal_rect_cross_height / 2,
                cross_location_x + horizontal_rect_cross_width / 2,
                cross_location_y + horizontal_rect_cross_height / 2 };
            cross_rect_pointer_vector[0]->SetDisplayRect(rect);

            // cross_rect_pointer_vector[1]->SetVisible(true);
            // set the verticle rect
            // we also adjust the width based on the width/height ratio

            rect = { cross_location_x - horizontal_rect_cross_height / 2 / width_height_ratio ,
                cross_location_y - width_height_ratio * horizontal_rect_cross_width / 2,
                cross_location_x + horizontal_rect_cross_height / 2 / width_height_ratio,
                cross_location_y + width_height_ratio * horizontal_rect_cross_width / 2 };
            cross_rect_pointer_vector[1]->SetDisplayRect(rect);

            // update cue coordinate states
            cue_coordinate_row = (encoding_dim % 2 == 0) ? (cross_location_y_id + encoding_dim / 2 + 1) : (cross_location_y_id + (encoding_dim + 1) / 2);
            cue_coordinate_column = (encoding_dim % 2 == 0) ? (cross_location_x_id + encoding_dim / 2 + 1) : (cross_location_x_id + (encoding_dim + 1) / 2);

            // show cue
            for (int i = 0; i < 2; i++) {
                cross_rect_pointer_vector[i]->SetVisible(true);
            }


            // patch disappear during cue
            Patch(0);
        }

    }
    break;



    case e_cue:
    {
        number_of_blocks_passed++;

        if (number_of_blocks_passed >= m_cue_duration_blocks) {

            for (int i = 0; i < 2; i++) {
                cross_rect_pointer_vector[i]->SetVisible(false);
            }

            m_current_state = e_encoding;

            AppLog << "enter encoding phase" << std::endl;
            number_of_blocks_passed = 0;
            // set cue coordinate state to 0


            // set the center of each dots
            cross_location_row = (encoding_dim % 2 == 0) ? (cross_location_y_id + encoding_dim / 2 + 1) : (cross_location_y_id + (encoding_dim + 1) / 2);
            cross_location_col = (encoding_dim % 2 == 0) ? (cross_location_x_id + encoding_dim / 2 + 1) : (cross_location_x_id + (encoding_dim + 1) / 2);

            // initialize 2 vectors to store the coordinate of the dot


            // define the subset of the grid
            unsigned int left_col; // 0 means the leftmost row in the grid
            unsigned int right_col;
            unsigned int top_row;
            unsigned int bottom_row;
            randomSequence = std::vector<bool>(1, false);
            randomSequence_distrator = std::vector<bool>(1, false);
            // if left col = 0, means the left most column
            left_col = cross_location_x_id;
            right_col = cross_location_x_id + encoding_dim - 1;
            top_row = cross_location_y_id;
            bottom_row = cross_location_y_id + encoding_dim - 1;
            /*
            bciout << "cross_location_x_id";
            bciout << cross_location_x_id;
            bciout << "left_col";
            bciout << left_col;
            bciout << "right_col";
            bciout << right_col;
            */


            // randomly assign 1 to the bool vector of length candidate_location_number
            if (encoding_dim % 2 == 0) {
                randomSequence = generateRandomSequence(encoding_dim * encoding_dim, candidate_location_number);
            }
            else {
                randomSequence = generateRandomSequence(encoding_dim * encoding_dim, candidate_location_number);
                while (randomSequence[(encoding_dim * encoding_dim - 1) / 2] == true) {
                    randomSequence = generateRandomSequence(encoding_dim * encoding_dim, candidate_location_number);
                }
            }

            // randomly assign 1 to the bool vector of length candidate_location_number

            if (Parameter("EnableDistratorOutside") == 1)
            {
                randomSequence_distrator = generateRandomSequence((n_grid_col - 1) * (n_grid_row - 1) - encoding_dim * encoding_dim, n_distrator_outside);
            }
            /*
            for (int i = 0; i < randomSequence_distrator.size(); i++) {
                AppLog << randomSequence_distrator[i] << std::endl;
            }

            // the sequence is well generated
            bciout << "random sequence is";
            for (bool value : randomSequence) {
                bciout << std::boolalpha << value << " ";
            }
            */

            // encoding_pattern = new std::vector<bool>
            int count = 0;
            int count_random_sequence = 0;

            // test the encoding accuracy
            // AppLog << "size of bool v of encoding col" << bool_v_of_encoding_pattern_col.size() << std::endl;
            for (int row = top_row; row <= bottom_row; ++row) {
                // define a boolean vector (binary) to encode the presence of dot for this row
                std::vector<bool> bool_v_of_encoding_pattern_row(n_grid_col - 1, false);
                for (int col = left_col; col <= right_col; ++col) {
                    if (randomSequence[count_random_sequence] == true) {

                        float grid_center_coordinate_x_encoding_dot = grid_center_coordinate_x[row * (n_grid_col - 1) + col];
                        float grid_center_coordinate_y_encoding_dot = grid_center_coordinate_y[row * (n_grid_col - 1) + col];
                        /*
                        bciout << "grid_center_coordinate_x_encoding_dot";
                        bciout << grid_center_coordinate_x_encoding_dot;
                        bciout << "grid_center_coordinate_y_encoding_dot";
                        bciout << grid_center_coordinate_y_encoding_dot;
                        */
                        encoding_circle_pointer_vector[count]->SetCenterX(grid_center_coordinate_x_encoding_dot);
                        encoding_circle_pointer_vector[count]->SetCenterY(grid_center_coordinate_y_encoding_dot);
                        // AppLog << "row =" << row << "col = " << col << std::endl;
                        bool_v_of_encoding_pattern_row[col] = true;

                        count++;
                        // we need to set encoding_pattern value so that the state can track
                        //////////////////
                    }
                    count_random_sequence++;
                }

                // after iteration of one row finished
                int encoding_pattern_row_ten_based = convertToBase10(bool_v_of_encoding_pattern_row);
                /* AppLog << "encoding value = " << encoding_pattern_row_ten_based << " , row boolean vector i:" << std::endl;
                for (int i = 0; i < bool_v_of_encoding_pattern_row.size(); i++) {
                    AppLog << bool_v_of_encoding_pattern_row[i] << std::endl;
                }
                */
                row_encoding_pattern_vector[row] = encoding_pattern_row_ten_based;
            }

            // keep random sequence for evaluation

            /*
            for (int i = 0; i < bool_v_of_encoding_pattern_col.size(); i++) {
                AppLog << bool_v_of_encoding_pattern_col[i] << std::endl;
            }
            AppLog << "value of bool v of encoding col" << bool_v_of_encoding_pattern_col_ten_based << std::endl;
            */
            // bciout << "count_random_sequence";
            // bciout << count_random_sequence;

            
            presented_dots_encoding = randomSequence;

            if (Parameter("EnableDistratorOutside") == 1)
            {
                int count_distrator = 0;
                int count_random_sequence_distrator = 0;


                /* AppLog << "encoding_dim * encoding_dim - candidate_location_number: " << encoding_dim * encoding_dim - candidate_location_number << std::endl;
                AppLog << "randomSequence_distrator size" << randomSequence_distrator.size() << std::endl;
                AppLog << "top row " << top_row << std::endl;
                AppLog << "bottom row " << bottom_row << std::endl;
                AppLog << "left col " << left_col << std::endl;
                AppLog << "right col " << right_col << std::endl;
                AppLog << "distrator_outside_pointer_vector size " << distrator_outside_pointer_vector.size() << std::endl;
                AppLog << "row_distrator_pattern_vector size " << row_distrator_pattern_vector.size() << std::endl;
                */

                // calculate distrator locations

                for (int row = 0; row < n_grid_row - 1; ++row) {

                    // define a boolean vector (binary) to encode the presence of dot for this row
                    std::vector<bool> bool_v_of_distrator_pattern_row(n_grid_col - 1, false);
                    for (int col = 0; col < n_grid_col - 1; ++col) {
                        if ((row >= top_row) && (row <= bottom_row) && (col <= right_col) && (col >= left_col)) {
                           // AppLog << "inside encoding pattern" << std::endl;
                            continue;
                        }
                        // AppLog << "row get distractor location " << row << std::endl;
                        // AppLog << "col get distractor location " << col << std::endl;

                        if (randomSequence_distrator[count_random_sequence_distrator] == true) {

                            //  AppLog << "distrator here " << std::endl;
                            float grid_center_coordinate_x_encoding_dot = grid_center_coordinate_x[row * (n_grid_col - 1) + col];
                            float grid_center_coordinate_y_encoding_dot = grid_center_coordinate_y[row * (n_grid_col - 1) + col];
                            /*
                            bciout << "grid_center_coordinate_x_encoding_dot";
                            bciout << grid_center_coordinate_x_encoding_dot;
                            bciout << "grid_center_coordinate_y_encoding_dot";
                            bciout << grid_center_coordinate_y_encoding_dot;
                            */
                            distrator_outside_pointer_vector[count_distrator]->SetCenterX(grid_center_coordinate_x_encoding_dot);
                            distrator_outside_pointer_vector[count_distrator]->SetCenterY(grid_center_coordinate_y_encoding_dot);
                            // AppLog << "row =" << row << "col = " << col << std::endl;
                            bool_v_of_distrator_pattern_row[col] = true;

                            count_distrator++;
                            // we need to set encoding_pattern value so that the state can track
                            //////////////////
                        }
                        count_random_sequence_distrator++;
                    }

                    // after iteration of one row finished
                    int distrator_pattern_row_ten_based = convertToBase10(bool_v_of_distrator_pattern_row);
                    /* AppLog << "encoding value = " << encoding_pattern_row_ten_based << " , row boolean vector i:" << std::endl;
                    for (int i = 0; i < bool_v_of_encoding_pattern_row.size(); i++) {
                        AppLog << bool_v_of_encoding_pattern_row[i] << std::endl;
                    }
                    */
                    row_distrator_pattern_vector[row] = distrator_pattern_row_ten_based;
                }
            }

            if (Parameter("EnableDistratorInside") == 1)
            {
                // randomly assign 1 to the bool vector of length candidate_location_number / 2
                if (encoding_dim % 2 == 0) {
                    randomSequence_distrator_inside = generateRandomSequence(encoding_dim * encoding_dim / 2, candidate_location_number / 2);
                }
                else {
                    randomSequence_distrator_inside = generateRandomSequence((encoding_dim * encoding_dim - 1) / 2, candidate_location_number / 2);
                    /*
                    while (randomSequence_distrator_inside[(encoding_dim * encoding_dim - 1) / 2 / 2] == true) {
                        randomSequence_distrator_inside = generateRandomSequence((encoding_dim * encoding_dim - 1) / 2 , candidate_location_number / 2);
                    }
                    */
                }
                /*
                AppLog << "randomSequence_distrator_inside is " << std::endl;
                for (int i = 0; i < randomSequence_distrator_inside.size(); i++) {
                    AppLog << randomSequence_distrator_inside[i] << std::endl;
                }
                AppLog << "distrator_inside_pointer_vector size " << distrator_inside_pointer_vector.size() << std::endl;
                */



                int count_distrator_inside = 0;
                int count_random_sequence_distrator_inside = 0;
                int count_random_sequence_encoding = 0;
                // test the encoding accuracy
                // AppLog << "size of bool v of encoding col" << bool_v_of_encoding_pattern_col.size() << std::endl;
                for (int row = top_row; row <= bottom_row; ++row) {
                    // define a boolean vector (binary) to encode the presence of dot for this row
                    std::vector<bool> bool_v_of_distrator_inside_pattern_row(n_grid_col - 1, false);
                    for (int col = left_col; col <= right_col; ++col) {
                        // AppLog << "row " << row << std::endl;
                        // AppLog << "col " << col << std::endl;
                        if (randomSequence[count_random_sequence_encoding] == true) {
                            // if there is encoding dots, continue
                            count_random_sequence_encoding++;
                           // AppLog << "continue " << std::endl;
                            continue;
                            
                        }
                        // skip the middle element
                        if ((encoding_dim % 2 != 0) && (row == top_row + (encoding_dim - 1 ) / 2) && (col == left_col + (encoding_dim - 1) / 2 )) {
                            // if there is encoding dots, continue
                            count_random_sequence_encoding++;
                            // AppLog << "continue " << std::endl;
                            continue;

                        }

                        else if (randomSequence_distrator_inside[count_random_sequence_distrator_inside] == true) {

                            // AppLog << "randomSequence_distrator_inside is true" << std::endl; 

                            float grid_center_coordinate_x_encoding_dot = grid_center_coordinate_x[row * (n_grid_col - 1) + col];
                            float grid_center_coordinate_y_encoding_dot = grid_center_coordinate_y[row * (n_grid_col - 1) + col];
                            
                            // bciout << "grid_center_coordinate_x_encoding_dot";
                            // bciout << grid_center_coordinate_x_encoding_dot;
                            // bciout << "grid_center_coordinate_y_encoding_dot";
                            // bciout << grid_center_coordinate_y_encoding_dot;
                            
                            distrator_inside_pointer_vector[count_distrator_inside]->SetCenterX(grid_center_coordinate_x_encoding_dot);
                            distrator_inside_pointer_vector[count_distrator_inside]->SetCenterY(grid_center_coordinate_y_encoding_dot);
                            // AppLog << "row =" << row << "col = " << col << std::endl;
                            bool_v_of_distrator_inside_pattern_row[col] = true;

                            count_distrator_inside++;
                            // we need to set encoding_pattern value so that the state can track
                            //////////////////
                        }
                        count_random_sequence_distrator_inside++;
                        count_random_sequence_encoding++;
                    }

                    // after iteration of one row finished
                    int distrator_pattern_row_ten_based = convertToBase10(bool_v_of_distrator_inside_pattern_row);
                    /* AppLog << "encoding value = " << encoding_pattern_row_ten_based << " , row boolean vector i:" << std::endl;
                    for (int i = 0; i < bool_v_of_encoding_pattern_row.size(); i++) {
                        AppLog << bool_v_of_encoding_pattern_row[i] << std::endl;
                    }
                    */
                    row_distrator_inside_pattern_vector[row] = distrator_pattern_row_ten_based;
                }
            }


            // turn on the dots
            for (int i = 0; i < candidate_location_number; i++) {
                // set visible
                encoding_circle_pointer_vector[i]->SetVisible(true);
            }

            if (Parameter("EnableDistratorOutside") == 1)
            {
                // turn on the distrators
                for (int i = 0; i < n_distrator_outside; i++) {
                    // set visible
                    distrator_outside_pointer_vector[i]->SetVisible(true);
                }
            }

            if (Parameter("EnableDistratorInside") == 1)
            {
                // turn on the distrators
                for (int i = 0; i < n_distrator_inside; i++) {
                    // set visible
                    distrator_inside_pointer_vector[i]->SetVisible(true);
                }
            }

            /*
            cross_location_x = 0;
            cross_location_x_id = 0;
            cross_location_y = 0;
            cross_location_y_id = 0;
            */


            // reset cue coordinates state
            cue_coordinate_row = 0;
            cue_coordinate_column = 0;

            // turn on patch
            Patch(1);
        }
    }
    
    break;

    case e_encoding:
    {
        number_of_blocks_passed++;


        if (number_of_blocks_passed >= m_encoding_duration_blocks) {
            m_current_state = e_delay;

            AppLog << "enter delay phase" << std::endl;
            number_of_blocks_passed = 0;

            // turn off the dots
            for (int i = 0; i < candidate_location_number; i++) {
                encoding_circle_pointer_vector[i]->SetVisible(false);
            }

                // reset the encoding pattern state
            for (int row = 0; row <= n_grid_row; ++row) {
                row_encoding_pattern_vector[row] = 0;
                row_distrator_inside_pattern_vector[row] = 0;
                row_distrator_pattern_vector[row] = 0;
            }

            // turn off the distrators
            for (int i = 0; i < n_distrator_outside; i++) {
                // set visible
                distrator_outside_pointer_vector[i]->SetVisible(false);
            }

            // turn on the distrators
            for (int i = 0; i < n_distrator_inside; i++) {
                // set visible
                distrator_inside_pointer_vector[i]->SetVisible(false);
            }

            // turn off patch during delay
            Patch(0);

            
        }
    }
    break;

    case e_delay:
    {
        number_of_blocks_passed++;
        // here we present only one dot.

        if (number_of_blocks_passed >= m_delay_duration_blocks) {

            m_current_state = e_retrival;
            // randomly make a dot appear
            unsigned int left_col; // 0 means the leftmost row in the grid
            unsigned int right_col;
            unsigned int top_row;
            unsigned int bottom_row;
            randomSequence = std::vector<bool>(1, false);
            // if left col = 0, means the left most column
            left_col = cross_location_x_id;
            right_col = cross_location_x_id + encoding_dim - 1;
            top_row = cross_location_y_id;
            bottom_row = cross_location_y_id + encoding_dim - 1;

                

            // randomly assign 1 to the bool vector of length candidate_location_number
            if (encoding_dim % 2 == 0) {
                randomSequence = generateRandomSequence(encoding_dim * encoding_dim, n_dots_in_retrival);
            }
            else {
                randomSequence = generateRandomSequence(encoding_dim * encoding_dim, n_dots_in_retrival);
                while (randomSequence[(encoding_dim * encoding_dim - 1) / 2] == true) {
                    randomSequence = generateRandomSequence(encoding_dim * encoding_dim, n_dots_in_retrival);
                }
            }

            response_ground_truth = evaluateRetrival(presented_dots_encoding, randomSequence);
            /*
            bciout << "random seq";
            for (bool value : randomSequence) {
                bciout << std::boolalpha << value << " ";
            }
            bciout << "presented_dots";
            for (bool value : presented_dots_encoding) {
                bciout << std::boolalpha << value << " ";
            }
            bciout << "ground truth:";
                bciout << response_ground_truth;

            bciout << "n_dots_in_retrival";
            bciout << n_dots_in_retrival;
            bciout << "random sequence is";
            for (bool value : randomSequence) {
                bciout << std::boolalpha << value << " ";
            }
            */

            int count = 0;
            int count_random_sequence = 0;
            for (int row = top_row; row <= bottom_row; ++row) {
                // define a boolean vector (binary) to encode the presence of dot for this row
                std::vector<bool> bool_v_of_encoding_pattern_row(n_grid_col - 1, false);
                for (int col = left_col; col <= right_col; ++col) {
                    if (randomSequence[count_random_sequence] == true) {

                        float grid_center_coordinate_x_encoding_dot = grid_center_coordinate_x[row * (n_grid_col - 1) + col];
                        float grid_center_coordinate_y_encoding_dot = grid_center_coordinate_y[row * (n_grid_col - 1) + col];

                        encoding_circle_pointer_vector[count]->SetCenterX(grid_center_coordinate_x_encoding_dot);
                        encoding_circle_pointer_vector[count]->SetCenterY(grid_center_coordinate_y_encoding_dot);
                        encoding_circle_pointer_vector[count]->SetVisible(true);
                        bool_v_of_encoding_pattern_row[col] = true;

                        count++;
                    }
                    count_random_sequence++;
                }
                // after iterate over one row
                int encoding_pattern_row_ten_based = convertToBase10(bool_v_of_encoding_pattern_row);
                row_encoding_pattern_vector[row] = encoding_pattern_row_ten_based;
            }

            // reset cross location
            cross_location_x = 0;
            cross_location_x_id = 0;
            cross_location_y = 0;
            cross_location_y_id = 0;

            // turn on patch during retrival
            Patch(1);

            AppLog << "enter retrival phase" << std::endl;
            number_of_blocks_passed = 0;
        }
    }
    break;

    case e_retrival:
    {
        number_of_blocks_passed++;
            
        if (IsButtonPressed()) {
            // turn off the grid
            for (int i = 0; i < n_grid_row + n_grid_col; i++) {

                grid_rect_pointer_vector[i]->SetVisible(false);
            }
            // turn off the dots
            for (int i = 0; i < candidate_location_number; i++) {
                encoding_circle_pointer_vector[i]->SetVisible(false);
            }
            m_current_state = e_feedback;

            // reset the encoding pattern state
            for (int row = 0; row <= n_grid_row; ++row) {
                row_encoding_pattern_vector[row] = 0;
            }


            // 
            if ((IsRightArrowButtonPressed()) && response_ground_truth) {
                // give feedback, correct or incorrect
                mp_text_stimulus->SetText("Correct");
                retrival_evaluation_code = 1;
                n_correct_response++;
            }
            else if ((IsLeftArrowButtonPressed()) && !response_ground_truth) {
                mp_text_stimulus->SetText("Correct");
                retrival_evaluation_code = 1;
                n_correct_response++;
            }
            else {
                mp_text_stimulus->SetText("Incorrect");
                retrival_evaluation_code = 0;
            }
            mp_text_stimulus->SetVisible(true);
            mp_text_stimulus->Present();

            // show reaction time
            // AppLog << "before reaction time calculation" << std::endl;
            reaction_time = ReactionTime();
            // AppLog << "after reaction time calculation" << std::endl;
            reaction_time_vector_score.push_back(reaction_time);

            std::string strReactionTime = std::to_string(reaction_time);
            rt_feedback_text_stimulus->SetText("Reaction time:" + strReactionTime + "ms");
            rt_feedback_text_stimulus->SetVisible(true);
            rt_feedback_text_stimulus->Present();

            AppLog << "enter feedback phase" << std::endl;
            number_of_blocks_passed = 0;

            // patch off during feedback
            Patch(0);
        }
        else if (number_of_blocks_passed >= m_retrival_duration_blocks) {
            // turn off the grid
            for (int i = 0; i < n_grid_row + n_grid_col; i++) {

                grid_rect_pointer_vector[i]->SetVisible(false);
            }
            // turn off the dots
            for (int i = 0; i < candidate_location_number; i++) {
                encoding_circle_pointer_vector[i]->SetVisible(false);
            }

            // reset the encoding pattern state
            for (int row = 0; row <= n_grid_row; ++row) {
                row_encoding_pattern_vector[row] = 0;
            }

            m_current_state = e_feedback;
            // no response
            mp_text_stimulus->SetText("No response");
            retrival_evaluation_code = 2;
        mp_text_stimulus->SetVisible(true);
        mp_text_stimulus->Present();
        AppLog << "enter feedback phase" << std::endl;
        number_of_blocks_passed = 0;

        // patch off during feedback
        Patch(0);
        }
    }
    break;

    case e_feedback:
    {
        number_of_blocks_passed++;


        if (number_of_blocks_passed >= m_feedback_duration_blocks) {
                
            m_current_state = e_wait_to_start;
            mp_text_stimulus->Conceal();
            rt_feedback_text_stimulus->Conceal();
            AppLog << "enter waiting for start phase" << std::endl;
            number_of_blocks_passed = 0;

            // reset response ground truth
            response_ground_truth = false;
            // reset retrival_evaluation_code
            retrival_evaluation_code = 3;
            // reset reaction time
            reaction_time = 0;
            // reset stim variables
            active_stim_this_trial = false;
            sham_stim_this_trial = false;
            Patch(1);

            // give score if reached the last trial
            if (m_trial_counter == m_num_trials - 1) {
                m_current_state = e_baseline_end;
                AppLog << "enter baseline end phase" << std::endl;
                number_of_blocks_passed = 0;
                mp_text_stimulus->SetText("+");
                mp_text_stimulus->SetVisible(true);
                mp_text_stimulus->Present();
                /*
                m_current_state = e_score;
                // calculate number of correct answer
                mp_text_stimulus->SetText("Accuracy: " + std::to_string(n_correct_response) + "/" + std::to_string(m_num_trials));
                // mp_text_stimulus->SetText("Accuracy: " + std::to_string(n_correct_response) + "/" + std::to_string(m_num_trials));
                mp_text_stimulus->SetVisible(true);
                mp_text_stimulus->Present();
                bciout << "enter score phase";
                int reaction_time_sum = 0;
                for (int reaction_time : reaction_time_vector_score) {
                    reaction_time_sum += reaction_time;
                }

                double reaction_time_mean = static_cast<double>(reaction_time_sum) / reaction_time_vector_score.size();
                // show average reaction time
                rt_feedback_text_stimulus->SetText("Average reaction time:" + std::to_string(static_cast<int>(reaction_time_mean)) + "ms");
                rt_feedback_text_stimulus->SetVisible(true);
                rt_feedback_text_stimulus->Present();
                // no patch during score phase
                Patch(0);
                */
            }
                
        }
    }
    break;

    case e_wait_to_start:
    {
        number_of_blocks_passed++;
        mp_text_stimulus->SetText(m_text_waiting_to_start);
        mp_text_stimulus->Present();

        if (number_of_blocks_passed >= m_waitingforstart_duration_blocks) {
            m_current_state = e_before_start;
            mp_text_stimulus->Conceal();
            AppLog << "enter before start phase" << std::endl;
            number_of_blocks_passed = 0;

            // update trial number
            m_trial_counter++;

            Patch(0);

        }
    }
    break;

    case e_baseline_end:
    {
        number_of_blocks_passed++;
        // end of the task
        if (number_of_blocks_passed >= m_baseline_end_duration_blocks) {
            mp_text_stimulus->SetVisible(false);
            mp_text_stimulus->Conceal();
            number_of_blocks_passed = 0;
            // update trial number
            m_current_state = e_score;
            mp_text_stimulus->SetText("Accuracy: " + std::to_string(n_correct_response) + "/" + std::to_string(m_num_trials));
            mp_text_stimulus->SetVisible(true);
            mp_text_stimulus->Present();
            AppLog << "enter score phase" << std::endl;
            int reaction_time_sum = 0;
            for (int reaction_time : reaction_time_vector_score) {
                reaction_time_sum += reaction_time;
            }

            double reaction_time_mean = static_cast<double>(reaction_time_sum) / reaction_time_vector_score.size();
            // show average reaction time
            rt_feedback_text_stimulus->SetText("Average reaction time:" + std::to_string(static_cast<int>(reaction_time_mean)) + "ms");
            rt_feedback_text_stimulus->SetVisible(true);
            rt_feedback_text_stimulus->Present();
            // no patch during score phase
            Patch(0);

        }
    }
    break;

    case e_score:
    {
        number_of_blocks_passed++;
        // end of the task
        if (number_of_blocks_passed >= m_score_duration_blocks) {
            number_of_blocks_passed = 0;
            // update trial number
            m_trial_counter++;
            AppLog << "==== Finished !!! =====" << std::endl;
            State("Running") = 0;
                
        }
    }
    break;
    }
    
}

void
SpatialWorkingMemoryGTTask::StopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // or because the run has reached its natural end.

    AllStimulusOff();

    num_stop_during_the_task++;
    m_current_state = e_wait_to_start;
  bciwarn << "END";
  // You know, you can delete methods if you're not using them.
  // Remove the corresponding declaration from SpatialWorkingMemoryGTTask.h too, if so.
}

void
SpatialWorkingMemoryGTTask::Halt()
{
  // Stop any threads or other asynchronous activity.
  // Good practice is to write the Halt() method such that it is safe to call it even *before*
  // Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
}

// a function that detects if any button is pressed
bool
SpatialWorkingMemoryGTTask::IsButtonPressed()
{
    //Check for keypresses
    bool press = false;
        for (unsigned int i = 0; i < m_block_size; i++)
        {
           // bciout << State("KeyDown")(i);
            if ((State("KeyDown")(i) == VK_LEFT) || (State("KeyDown")(i) == VK_RIGHT))
            {
                press = true;
                //				bciout << "key pressed" << endl;
            }
        }
        return press;
}

// a function that detects if left arrow button is pressed
bool
SpatialWorkingMemoryGTTask::IsLeftArrowButtonPressed()
{

    //Check for keypresses
    bool press = false;

    for (unsigned int i = 0; i < m_block_size; i++)
    {
        if (State("KeyDown")(i) == VK_LEFT)
        {
            press = true;
            AppLog << "left arrow pressed" << std::endl;
        }
    }
    return press;
}

// a function that detects if right arrow button is pressed
bool
SpatialWorkingMemoryGTTask::IsRightArrowButtonPressed()
{

    //Check for keypresses
    bool press = false;

    for (unsigned int i = 0; i < m_block_size; i++)
    {
        if (State("KeyDown")(i) == VK_RIGHT)
        {
            press = true;
            AppLog << "right arrow pressed" << std::endl;
        }
    }
    return press;
}

// generate a random number. This is used in the transition from background to cue phase
int
SpatialWorkingMemoryGTTask::RandomNumber(int min, int max)
{
    int random_number = min + m_random_generator.Random() % (max - min + 1);
    return random_number;
}
 
Point rotatePoint90Degrees(double x, double y, double centerX, double centerY) {
    Point rotatedPoint;
    double translatedX = x - centerX;
    double translatedY = y - centerY;
    rotatedPoint.x = -translatedY + centerX;
    rotatedPoint.y = translatedX + centerY;
    return rotatedPoint;
}

// a function to determine the encoding pattern


std::vector<bool> SpatialWorkingMemoryGTTask::generateRandomSequence(int sequence_length, int n_true) {
    std::vector<bool> sequence(sequence_length, false); // Initialize a sequence of length 16 with all false values

    // Set up random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sequence_length - 1);

    int count = 0;
    while (count < n_true) {
        int index = dis(gen); // Generate a random index between 0 and 15

        if (!sequence[index]) {
            sequence[index] = true; // Set the value at the random index to true
            count++;
        }
    }
    return sequence;
}

std::vector<bool> SpatialWorkingMemoryGTTask::generateRandomSequenceDistrator(std::vector<bool> base_sequence, int sequence_length, int n_true) {
    std::vector<bool> sequence(sequence_length, false); // Initialize a sequence of length 16 with all false values

    // Set up random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sequence_length - 1);

    int count = 0;
    while (count < n_true) {
        int index = dis(gen); // 

        if ((!sequence[index]) && (!sequence[index])) {
            sequence[index] = true; // Set the value at the random index to true
            count++;
        }
    }
    return sequence;
}

int SpatialWorkingMemoryGTTask::convertToBase10(const std::vector<bool>& sequence) {
    int base10Value = 0;
    int power = sequence.size() - 1;

    for (bool bit : sequence) {
        if (bit) {
            base10Value += std::pow(2, power);
        }
        power--;
    }

    return base10Value;
}

bool SpatialWorkingMemoryGTTask::evaluateRetrival(const std::vector<bool>& vector1, const std::vector<bool>& vector2) {
    // Ensure vector sizes match
    if (vector1.size() != vector2.size()) {
        std::cout << "Vector sizes do not match." << std::endl;
        return false;
    }

    // Get indices of true values in vector2
    std::vector<size_t> trueIndices;
    for (size_t i = 0; i < vector2.size(); ++i) {
        if (vector2[i]) {
            trueIndices.push_back(i);
        }
    }
   // bciout << "true indices size";
   // bciout << trueIndices.size();
    // Check if elements at true indices in vector1 are all true
    for (size_t index : trueIndices) {
        if (!vector1[index]) {
            return false;
        }
    }

    return true;
}


// calculate reaction time if button pressed
float SpatialWorkingMemoryGTTask::ReactionTime()
{
    unsigned int offset = 0;
    for (offset = 0; offset < m_block_size; offset++)
    {
        if ((State("KeyDown")(offset) == VK_LEFT) || (State("KeyDown")(offset) == VK_RIGHT))
        {
            break;
        }
    }
return (float)(number_of_blocks_passed - 1) * m_block_size_msec + (float)(offset + 1) / (float)(m_block_size)*m_block_size_msec;

}

void SpatialWorkingMemoryGTTask::Patch(bool active)
{
    if (mPhotoDiodePatch.pShape) {
        if (active) {
            mPhotoDiodePatch.pShape->SetColor(mPhotoDiodePatch.activeColor).SetFillColor(mPhotoDiodePatch.activeColor);
           // AppLog << "active photodiode patch" << std::endl;
        }
        else
            mPhotoDiodePatch.pShape->SetColor(mPhotoDiodePatch.inactiveColor).SetFillColor(mPhotoDiodePatch.inactiveColor);
    }
}

void
SpatialWorkingMemoryGTTask::AllStimulusOff()
{
    instruction_image->Conceal();
    rt_feedback_text_stimulus->Conceal();
    mp_text_stimulus->Conceal();
    for (int i = 0; i < candidate_location_number; i++) {
        encoding_circle_pointer_vector[i]->SetVisible(false);
    }
    for (int i = 0; i < 2; i++) {
        cross_rect_pointer_vector[i]->SetVisible(false);
    }

    for (int i = 0; i < n_grid_row + n_grid_col; i++) {
        grid_rect_pointer_vector[i]->SetVisible(false);
    }

}