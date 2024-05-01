////////////////////////////////////////////////////////////////////////////////
// Authors: BCI2000@DESKTOP-3BOJQC5.wucon.wustl.edu
// Description: GT VibrationHabituationTask implementation
////////////////////////////////////////////////////////////////////////////////

#include "VibrationHabituationTask.h"
#include "RandomGenerator.h"
#include <utility>
#include <math.h>
#include <sstream>
#include <iostream>
#include "gSTIMbox.imports.h"
#include "ProgressBarVis.h"

#include "BCIStream.h"

RegisterFilter( VibrationHabituationTask, 3 );
     // Change the location if appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations within SignalProcessing modules begin with "2."
     //       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
     //  - filter locations within Application modules begin with "3."


VibrationHabituationTask::VibrationHabituationTask() :
    mr_window(Window()), mpProgressBar(nullptr)
{
    BEGIN_PARAMETER_DEFINITIONS
        // general parameters
        "Application:VibrationHabituationTask string OutputOneSide= Left // The side of Output1 from the stimbox",
        "Application:VibrationHabituationTask string OutputTwoSide= Right // The side of Output2 from the stimbox",
        "Application:VibrationHabituationTask string OutputOneSite= Stim // The site of Output1 from the stimbox, stim represents concha",
        "Application:VibrationHabituationTask string OutputTwoSite= Sham // The site of Output2 from the stimbox, sham represents earlobe",
        "Application:VibrationHabituationTask int FreqOne= 6 // We specify two freqs, this is the first freq ",
        "Application:VibrationHabituationTask int FreqTwo= 20 // We specify two freqs, this is the second freq ",

        "Application:VibrationHabituationTask int m_num_trials= 20 // Number of trials",

        "Application:VibrationHabituationTask int FixationCrossColor= 0xFF0000 0xFF0000 % % ",        
        "Application:VibrationHabituationTask string InstructionFile= ..\\tasks\\vibrationHabituation_task\\instruction_vibration_habituation.bmp"
        " // path for instruction image",

        "Application:VibrationHabituationTask string StrongerFile= ..\\tasks\\vibrationHabituation_task\\stronger_vibration_habituation.bmp",
        "Application:VibrationHabituationTask string LouderFile= ..\\tasks\\vibrationHabituation_task\\louder_vibration_habituation.bmp",
        

        // parameters for baseline
        "Application:VibrationHabituationTask float BaselineBeginningDuration= 10000ms 10000ms 0 % // baseline phase at the begining lasts for 10s",
        "Application:VibrationHabituationTask float BaselineEndDuration= 10000ms 10000ms 0 % // baseline phase at the end lasts for 20s",

        // parameters for beforestart
        "Application:VibrationHabituationTask float BeforeStartDuration= 2000ms 2000ms 0 % // beforestart phase lasts for 100s",

        // parameters for vibration one
        "Application:VibrationHabituationTask float VibrationOneDuration= 2000ms 2000ms 0 % // vibration 1 lasts for about 2s",

        // parameters for vibration two
        "Application:VibrationHabituationTask float VibrationTwoDuration= 2000ms 2000ms 0 % // vibration 2 lasts for about 2s",

        // parameters for post vibration one
        "Application:VibrationHabituationTask float PostVibrationOneDuration= 4000ms 4000ms 0 % // post vibration one lasts for about 4s",

        // parameters for post vibration two
        "Application:VibrationHabituationTask float PostVibrationTwoDuration= 4000ms 4000ms 0 % // post vibration one lasts for about 4s",

        // parameters for stronger
        "Application:VibrationHabituationTask float StrongerDuration= 10000ms 10000ms 0 % // stronger  actually when the participant presses a button",

        // parameters for louder
        "Application:VibrationHabituationTask float LouderDuration= 10000ms 10000ms 0 % // louder actually when the participant presses a button",


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

        // progress bar
        "Application:ProgressBar int ProgressBar= 0 1 0 1 // "
        "Display progress bar (boolean)",
        "Application:ProgressBar float ProgressBarHeight= 50 50 0 % //"
        "Progress bar height in pixels",
        "Application:ProgressBar float ProgressBarWidth= 250 250 0 % //"
        "Progress bar width in pixels",
        "Application:ProgressBar string ProgressBarBackgroundColor= 0x00808080 0x00808080 0x00000000 0xFFFFFFFF // "
        "Color of progress bar background (color)",
        "Application:ProgressBar string ProgressBarForegroundColor= 0x00ffff00 0x00f0f0f0 0x00000000 0xFFFFFFFF // "
        "Color of progress bar foreground (color)",

        END_PARAMETER_DEFINITIONS

        BEGIN_STATE_DEFINITIONS
        "CurrentTrial               8  0 0 0", // The Current Trial, 8 bits = 256

        "OutputOneBool             2  0 0 0", // state controlling on/off the output1
        // 1 means output1 of the stimbox is turned on
        "OutputOneFreq        5  0 0 0", // state encoding the frequency of output1, for example 1 means 6Hz
        "OutputTwoBool             2  0 0 0", // state controlling on/off the output1
        // 1 means output1 of the stimbox is turned on
        "OutputTwoFreq        5  0 0 0", // state encoding the frequency of output1, for example 1 means 6Hz
        "Phase                      5  0 0 0",  
        //     e_before_start = 2, e_vibration_one = 3,
        // e_post_vibration_one = 4, e_vibration_two = 5, e_post_vibration_two = 6, e_stronger = 7, e_louder = 8,
        // e_instruction = 9, e_baseline_beginning = 10,
        // e_baseline_end = 11

        "StrongerOutputOne        2  0 0 0", // state encoding the participant's answer. 1 means that output 1 is thought to be stronger
        "LouderOutputOne        2  0 0 0", // state encoding the participant's answer. 1 means that output 1 is thought to be louder
        "StrongerOutputTwo        2  0 0 0", // state encoding the participant's answer. 1 means that output 2 is thought to be stronger
        "LouderOutputTwo        2  0 0 0", // state encoding the participant's answer. 1 means that output 2 is thought to be louder
        "OutputOneOrTwoIsFirst        2  0 0 0", // state encoding the the order of vibration.
        "LouderSame        2  0 0 0", //  1 means that both outputs are thought to have same volume
        "StrongerSame        2  0 0 0", // 1 means that both outputs are thought to have same displacement
        // 1 means that output 1 is first presented (when the "vibration 1 is showing on the screen")

        "NumStopDuringTheTask              4 0 0 0 ",  // This state encodes whether or not the task is interrupted


        END_STATE_DEFINITIONS




}


VibrationHabituationTask::~VibrationHabituationTask()
{
  Halt();
  // If you have allocated any memory with malloc(), call free() here.
  // If you have allocated any memory with new[], call delete[] here.
  // If you have created any object using new, call delete here.
  // Either kind of deallocation will silently ignore null pointers, so all
  // you need to do is to initialize those to zero in the constructor, and
  // deallocate them here.
  delete mpProgressBar;
}



void
VibrationHabituationTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
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

  // Note that the VibrationHabituationTask instance itself, and its members, are read-only during
  // this phase, due to the "const" at the end of the Preflight prototype above.
  // Any methods called by Preflight must also be "const" in the same way.

    Output = Input; // this simply passes information through about SampleBlock dimensions, etc....
    State("Running");
    State("KeyDown");
    State("Phase");
    State("CurrentTrial");
    State("OutputOneBool");
    State("OutputOneFreq");
    State("OutputTwoBool");
    State("OutputTwoFreq");
    State("StrongerOutputOne");
    State("LouderOutputOne");
    State("StrongerOutputTwo");
    State("LouderOutputTwo");
    State("LouderSame");
    State("StrongerSame");
    State("OutputOneOrTwoIsFirst");

    // sampling rate and block size have been defined in the parent class
    Parameter("SamplingRate");
    Parameter("SampleBlockSize");
    Parameter("m_num_trials");

   
    Parameter("WindowWidth");
    Parameter("WindowHeight");
    Parameter("InstructionFile");
    Parameter("LouderFile");
    Parameter("StrongerFile");
    Parameter("FreqOne");
    Parameter("FreqTwo");

}


void
VibrationHabituationTask::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The signal properties can no longer be modified, but the const limitation has gone, so
  // the VibrationHabituationTask instance itself can be modified. Allocate any memory you need, start any
  // threads, store any information you need in private member variables.
    m_current_state = e_before_start; // participant presses a button to start

    m_sample_rate = Parameter("SamplingRate");
    m_block_size = Parameter("SampleBlockSize");
    m_runtime_counter = 0;
    m_trial_counter = 0;
    number_of_blocks_passed = 0;
    m_num_trials = Parameter("m_num_trials");

    m_text_vibration_one = "Vibration 1";
    m_text_vibration_two = "Vibration 1";

    // block design, convert time into blocks, m_block_size_msec means the duration of one block
    // for example, block size defaults to 32, sampling freq = 256Hz, the duration of a block is 1000/8 = 125 ms
    m_block_size_msec = (float)(m_block_size) / (float)(m_sample_rate) * 1000;
    m_beforestart_duration_blocks = Parameter("BeforeStartDuration").InMilliseconds() / m_block_size_msec;

    m_vibration_one_duration_blocks = Parameter("VibrationOneDuration").InMilliseconds() / m_block_size_msec;
    m_vibration_two_duration_blocks = Parameter("VibrationTwoDuration").InMilliseconds() / m_block_size_msec;
    m_post_vibration_one_duration_blocks = Parameter("PostVibrationOneDuration").InMilliseconds() / m_block_size_msec;
    m_post_vibration_two_duration_blocks = Parameter("PostVibrationTwoDuration").InMilliseconds() / m_block_size_msec;
    m_stronger_duration_blocks = Parameter("StrongerDuration").InMilliseconds() / m_block_size_msec;
    m_louder_duration_blocks = Parameter("LouderDuration").InMilliseconds() / m_block_size_msec;

    m_baseline_begining_duration_blocks = Parameter("BaselineBeginningDuration").InMilliseconds() / m_block_size_msec;
    m_baseline_end_duration_blocks = Parameter("BaselineEndDuration").InMilliseconds() / m_block_size_msec;

    // check block size and sampling freq
    AppLog << "Sampling freq is:" << Parameter("SamplingRate") << std::endl;
    AppLog << "Block size is:" << Parameter("SampleBlockSize") << std::endl;
    AppLog << "if 2000Hz, 200 block size, before-start (2s) block should be 20" << std::endl;
    AppLog << "actual value is : " << m_beforestart_duration_blocks << std::endl;

    // check Application window size
    AppLog << "Display window width is:" << Parameter("WindowWidth") << std::endl;
    AppLog << "Display window height is:" << Parameter("WindowHeight") << std::endl;
    AppLog << "The patient cart window size is 2560*1440, 960 * 540 in test mode" << std::endl;

    // define vibration order, 1 means output 1 is first presented
    int i_upper_bound;
    if (m_num_trials % 2 != 0) {
        i_upper_bound = (m_num_trials + 1) / 2;
    }
    else {
        i_upper_bound = m_num_trials / 2;
    }

    // Add 10 ones and 10 twos
    for (int i = 0; i < i_upper_bound; i++) {
        vibration_order_vector.push_back(1);
        vibration_order_vector.push_back(2);
        output_one_freq_vector.push_back(1);      // define vibration freq for output 1, 1 means freq1 is used
        output_one_freq_vector.push_back(2);
        output_two_freq_vector.push_back(1);
        output_two_freq_vector.push_back(2);
    }

  

    // Randomize the order of the elements
    std::random_device rd;
    std::mt19937 g1(rd());
    std::shuffle(vibration_order_vector.begin(), vibration_order_vector.end(), g1);
    std::mt19937 g2(rd());
    std::shuffle(output_one_freq_vector.begin(), output_one_freq_vector.end(), g2);
    std::mt19937 g3(rd());
    std::shuffle(output_two_freq_vector.begin(), output_two_freq_vector.end(), g3);

    /*
    AppLog << "we can create output_two_freq_vector: " << std::endl;
    for (size_t i = 0; i < output_two_freq_vector.size(); i++) {
        AppLog << output_two_freq_vector[i] << " " << std::endl;
    }
    */

    // initialize freq one and two -> pass parameter to global variable
    freq_one = Parameter("FreqOne");
    freq_two = Parameter("FreqTwo");
 
  
    if (m_num_trials % 2 != 0) {
        vibration_order_vector.pop_back();
        output_one_freq_vector.pop_back();
        output_two_freq_vector.pop_back();
    }

    // initiate states encoding the freq and order of output 1 and 2
    output_one_or_two_is_first = 0;
    output_one_bool = false;
    output_two_bool = false;
    output_one_freq = 0;
    output_two_freq = 0;
    stronger_output_one = false;
    stronger_output_two = false;
    stronger_same = false;
    louder_output_one = false;
    louder_output_two = false;
    louder_same = false;

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
    m_full_rectangle.bottom = 0.8f;

    mp_text_stimulus = new TextStimulus(mr_window);
    mp_text_stimulus->SetText("");
    mp_text_stimulus->SetTextHeight(0.2f);
    mp_text_stimulus->SetDisplayRect(m_full_rectangle);
    mp_text_stimulus->SetColor(RGBColor::Black);
    mp_text_stimulus->SetTextColor(RGBColor::Green);
    mp_text_stimulus->Present();
    mp_text_stimulus->SetVisible(false);


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

    // set image stimulus (stronger)
    rect.left = 0.1f;
    rect.right = 0.9f;
    rect.top = 0.1f;
    rect.bottom = 0.9f;

    stronger_image = new ImageStimulus(mr_window);
    stronger_image->SetDisplayRect(rect);
    stronger_image->SetPresentationMode(VisualStimulus::ShowHide);
    stronger_image->SetAspectRatioMode(GUI::AspectRatioModes::AdjustBoth);
    stronger_image->SetFile(Parameter("StrongerFile"));
    stronger_image->Conceal();

    // set image stimulus (louder)
    louder_image = new ImageStimulus(mr_window);
    louder_image->SetDisplayRect(rect);
    louder_image->SetPresentationMode(VisualStimulus::ShowHide);
    louder_image->SetAspectRatioMode(GUI::AspectRatioModes::AdjustBoth);
    louder_image->SetFile(Parameter("LouderFile"));
    louder_image->Conceal();

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
    if (mpProgressBar)
    {
        mpProgressBar->Send(CfgID::Visible, false);
    }
    delete mpProgressBar;
    mpProgressBar = nullptr;
    if (Parameter("ProgressBar") != 0)
    {
        mpProgressBar = new ProgressBarVis();
        mpProgressBar->SetBackgroundColor(RGBColor(Parameter("ProgressBarBackgroundColor")));
        mpProgressBar->SetForegroundColor(RGBColor(Parameter("ProgressBarForegroundColor")));
        mpProgressBar->SetHeight(Parameter("ProgressBarHeight"));
        mpProgressBar->SetWidth(Parameter("ProgressBarWidth"));

        mpProgressBar->Send(CfgID::WindowTitle, "Progress");
        mpProgressBar->Send(CfgID::Visible, true);
        mpProgressBar->SendReferenceFrame();
    }

    AppLog << "initialization done" << std::endl;



}

void
VibrationHabituationTask::StartRun()
{
  // The user has just pressed "Start" (or "Resume")
    if (mpProgressBar)
    {
        mpProgressBar->SetTotal(m_num_trials - m_trial_counter).SetCurrent(0);
        mpProgressBar->SendDifferenceFrame();
    }

}


void
VibrationHabituationTask::Process( const GenericSignal& Input, GenericSignal& Output )
{

  // And now we're processing a single SampleBlock of data.
  // Remember not to take too much CPU time here, or you will break the real-time constraint.

  Output = Input; // This passes the signal through unmodified.
  if (m_trial_counter > m_num_trials) // from 0 to m_num_trials - 1
  {
      return;
  }

  if (m_is_first_trial)
  {
      Patch(0);
      instruction_image->Present();
      m_current_state = e_instruction;
      m_is_first_trial = false;
  }


  // keep track of the states
  State("Phase") = m_current_state;
  State("CurrentTrial") = m_trial_counter;
  State("OutputOneBool") = output_one_bool;
  State("OutputTwoBool") = output_two_bool;
  State("OutputTwoFreq") = output_two_freq;
  State("OutputOneFreq") = output_one_freq;
  State("StrongerOutputOne") = stronger_output_one;
  State("LouderOutputOne") = louder_output_one;
  State("StrongerOutputTwo") = stronger_output_two;
  State("LouderOutputTwo") = louder_output_two;
  State("StrongerSame") = stronger_same;
  State("LouderSame") = louder_same;
  State("OutputOneOrTwoIsFirst") = output_one_or_two_is_first; // int, 1 means output 1 is first, 2 means output 2 is first


  State("NumStopDuringTheTask") = num_stop_during_the_task;

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
      // AppLog << "number_of_blocks_passed:" << number_of_blocks_passed  << std::endl;
      number_of_blocks_passed++;


      if (number_of_blocks_passed >= m_beforestart_duration_blocks){
         // AppLog << "m_beforestart_duration_blocks:" << m_beforestart_duration_blocks << std::endl;
          AppLog << "m_trial_counter" << m_trial_counter << std::endl;

          // reset stronger and lounder result
          stronger_output_one = false;
          stronger_output_two = false;
          stronger_same = false;
          louder_output_one = false;
          louder_output_two = false;
          louder_same = false;

         // check if this is the last trial
          if (m_trial_counter == m_num_trials) {
              m_current_state = e_baseline_end;
              // AppLog << "enter baseline end phase" << std::endl;
              mp_text_stimulus->SetText("+");
              mp_text_stimulus->SetVisible(true);
              mp_text_stimulus->Present();
              number_of_blocks_passed = 0;
          }
          else {
              //if (IsButtonPressed() || (number_of_blocks_passed >= m_beforestart_duration_blocks)) // if maximal waiting time is passed
         // define the order and freq for output1 and 2
              if (vibration_order_vector[m_trial_counter] == 1) {
                  output_one_or_two_is_first = 1;
              }
              else {
                  output_one_or_two_is_first = 2;
              }

              if (output_one_freq_vector[m_trial_counter] == 1) {
                  output_one_freq = freq_one;
              }
              else
              {
                  output_one_freq = freq_two;
              }

              if (output_two_freq_vector[m_trial_counter] == 1) {
                  output_two_freq = freq_one;
              }
              else
              {
                  output_two_freq = freq_two;
              }

              // check if output 1 should be turned on in vibration one phase
              if (output_one_or_two_is_first == 1) {
                  // this is used to turn on output one 
                  output_one_bool = true;
              }
              else if (output_one_or_two_is_first == 2) {
                  output_two_bool = true;
              }
              // show text
              mp_text_stimulus->SetText("Vibration 1");
              mp_text_stimulus->SetVisible(true);
              mp_text_stimulus->Present();
              m_current_state = e_vibration_one;
              number_of_blocks_passed = 0;
              Patch(1);
              AppLog << "enter vibration 1 phase" << std::endl;

              m_trial_counter++;
              // AppLog << "m_trial_counter after update in before start" << m_trial_counter << std::endl;

              if (mpProgressBar != 0)
              {
                  Assert(mpProgressBar->Current() < mpProgressBar->Total());
                  int current = mpProgressBar->Current();
                  mpProgressBar->SetCurrent(current + 1);
                  mpProgressBar->SendDifferenceFrame();
              }
          }

          
      }
  }
  break;

  case e_vibration_one:
  {


      number_of_blocks_passed++;


      if (number_of_blocks_passed >= m_vibration_one_duration_blocks) // if maximal waiting time is passed
      {

          // turn off output 1 and 2
          output_one_bool = false;
          output_two_bool = false;
          mp_text_stimulus->Conceal();
          mp_text_stimulus->SetVisible(false);
          m_current_state = e_post_vibration_one;
          AppLog << "enter post vibration 1 phase" << std::endl;
          number_of_blocks_passed = 0;

          // patch disappear during post vibration one
          Patch(0);
      }

  }
  break;



  case e_post_vibration_one:
  {
      number_of_blocks_passed++;

      if (number_of_blocks_passed >= m_post_vibration_one_duration_blocks) {

          // check if output 1 should be turned on in vibration one phase
          if (output_one_or_two_is_first == 1) {
              // this is used to turn on output one 
              output_two_bool = true;
          }
          else if (output_one_or_two_is_first == 2) {
              output_one_bool = true;
          }
          mp_text_stimulus->SetText("Vibration 2");
          mp_text_stimulus->SetVisible(true);
          mp_text_stimulus->Present();
          m_current_state = e_vibration_two;

          AppLog << "enter vibration 2 phase" << std::endl;
          number_of_blocks_passed = 0;

          // turn on patch
          Patch(1);
      }
  }
  break;

  case e_vibration_two:
  {
      number_of_blocks_passed++;


      if (number_of_blocks_passed >= m_vibration_two_duration_blocks) {
          m_current_state = e_post_vibration_two;

          AppLog << "enter post vibration 2 phase" << std::endl;
          number_of_blocks_passed = 0;

          // turn off the text
          mp_text_stimulus->Conceal();
          mp_text_stimulus->SetVisible(false);
          // turn off output 1 and 2
          output_one_bool = false;
          output_two_bool = false;

          // turn off patch during delay
          Patch(0);

      }
  }
  break;

  case e_post_vibration_two:
  {
      number_of_blocks_passed++;
      // here we present only one dot.

      if (number_of_blocks_passed >= m_post_vibration_two_duration_blocks) {

          // reset the states
          output_two_freq = 0;
          output_one_freq = 0;
          m_current_state = e_stronger;

          // turn on patch during retrival
          Patch(1);

          // show image
          stronger_image->Present();

          AppLog << "enter stronger phase" << std::endl;
          number_of_blocks_passed = 0;
      }
  }
  break;

  case e_stronger:
  {
      number_of_blocks_passed++;
      // know which button is pressed
// if right arrow is pressed (vibration 2 is stronger)
// and output one is first
// then the participant thinks output2 is stronger
      if (IsRightArrowButtonPressed()) {
          if (output_one_or_two_is_first == 1) {
              stronger_output_two = true;
          }
          else if (output_one_or_two_is_first == 2) {
              stronger_output_one = true;
          }
          m_current_state = e_louder;
          AppLog << "enter louder phase" << std::endl;
          number_of_blocks_passed = 0;

          // conceal stronger image
          stronger_image->Conceal();
          louder_image->Present();
          // patch on during louder
          Patch(1);
      }
      else if (IsLeftArrowButtonPressed()) {
          if (output_one_or_two_is_first == 1) {
              stronger_output_one = true;
          }
          else if (output_one_or_two_is_first == 2) {
              stronger_output_two = true;
          }
          m_current_state = e_louder;
          AppLog << "enter louder phase" << std::endl;
          number_of_blocks_passed = 0;

          // conceal stronger image
          stronger_image->Conceal();
          louder_image->Present();
          // patch on during louder
          Patch(1);
      }
      else if (IsDownArrowButtonPressed()) {
          stronger_same = true;
          m_current_state = e_louder;
          AppLog << "enter louder phase" << std::endl;
          number_of_blocks_passed = 0;

          // conceal stronger image
          stronger_image->Conceal();
          louder_image->Present();
          // patch on during louder
          Patch(1);
      }
  }
  break;

  case e_louder:
  {
      number_of_blocks_passed++;

      if (IsRightArrowButtonPressed()) {
          if (output_one_or_two_is_first == 1) {
              louder_output_two = true;
          }
          else if (output_one_or_two_is_first == 2) {
              louder_output_one = true;
          }
          //reset output_one_or_two_is_first
          output_one_or_two_is_first = 0;

        m_current_state = e_before_start;
        AppLog << "enter before start phase" << std::endl;
        number_of_blocks_passed = 0;


          // conceal stronger image
          louder_image->Conceal();
          // patch on during louder
          Patch(0);
      }
      else if (IsLeftArrowButtonPressed()) {
          if (output_one_or_two_is_first == 1) {
              louder_output_one = true;
          }
          else if (output_one_or_two_is_first == 2) {
              louder_output_two = true;
          }
          //reset output_one_or_two_is_first
          output_one_or_two_is_first = 0;

            m_current_state = e_before_start;
            AppLog << "enter before start phase" << std::endl;
            number_of_blocks_passed = 0;

          // conceal stronger image
          louder_image->Conceal();
          // patch on during louder
          Patch(0);
      }
      else if (IsDownArrowButtonPressed()) {
          louder_same = true;

          //reset output_one_or_two_is_first
          output_one_or_two_is_first = 0;

          m_current_state = e_before_start;
          AppLog << "enter before start phase" << std::endl;
          number_of_blocks_passed = 0;
 



          // conceal stronger image
          louder_image->Conceal();
          // patch on during louder
          Patch(0);
      }

  }
  break;


  case e_baseline_end:
  {
      number_of_blocks_passed++;
      // end of the task
      if (number_of_blocks_passed >= m_baseline_end_duration_blocks) {
          m_trial_counter++;
          // reset stronger and lounder result
          stronger_output_one = false;
          stronger_output_two = false;
          stronger_same = false;
          louder_output_one = false;
          louder_output_two = false;
          louder_same = false;
          // AppLog << "m trial counter in baseline end" << m_trial_counter << std::endl;
          mp_text_stimulus->SetVisible(false);
          mp_text_stimulus->Conceal();
          number_of_blocks_passed = 0;
          Patch(0);
          AppLog << "==== Finished !!! =====" << std::endl;
          State("Running") = 0;
      }
  }
  break;
  }

}

void
VibrationHabituationTask::StopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // or because the run has reached its natural end.
    AllStimulusOff();

    num_stop_during_the_task++;
    m_current_state = e_before_start;
    bciwarn << "END";
  // You know, you can delete methods if you're not using them.
  // Remove the corresponding declaration from VibrationHabituationTask.h too, if so.
}

void
VibrationHabituationTask::Halt()
{
  // Stop any threads or other asynchronous activity.
  // Good practice is to write the Halt() method such that it is safe to call it even *before*
  // Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
}

// a function that detects if any button is pressed
bool
VibrationHabituationTask::IsButtonPressed()
{
    //Check for keypresses
    bool press = false;
    for (unsigned int i = 0; i < m_block_size; i++)
    {
        // bciout << State("KeyDown")(i);
        if ((State("KeyDown")(i) == VK_LEFT) || (State("KeyDown")(i) == VK_RIGHT) || (State("KeyDown")(i) == VK_DOWN))
        {
            press = true;
            //				bciout << "key pressed" << endl;
        }
    }
    return press;
}

// a function that detects if left arrow button is pressed
bool
VibrationHabituationTask::IsLeftArrowButtonPressed()
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
VibrationHabituationTask::IsRightArrowButtonPressed()
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

// a function that detects if the down arrow button is pressed
bool
VibrationHabituationTask::IsDownArrowButtonPressed()
{

    //Check for keypresses
    bool press = false;

    for (unsigned int i = 0; i < m_block_size; i++)
    {
        if (State("KeyDown")(i) == VK_DOWN)
        {
            press = true;
            AppLog << "down arrow pressed" << std::endl;
        }
    }
    return press;
}

// generate a random number. This is used in the transition from background to cue phase
int
VibrationHabituationTask::RandomNumber(int min, int max)
{
    int random_number = min + m_random_generator.Random() % (max - min + 1);
    return random_number;
}



// a function to determine the encoding pattern


std::vector<bool> VibrationHabituationTask::generateRandomSequence(int sequence_length, int n_true) {
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

int VibrationHabituationTask::convertToBase10(const std::vector<bool>& sequence) {
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


void VibrationHabituationTask::Patch(bool active)
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
VibrationHabituationTask::AllStimulusOff()
{
    mp_text_stimulus->Conceal();
    stronger_image->Conceal();
    instruction_image->Conceal();
    louder_image->Conceal();


}
