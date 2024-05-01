////////////////////////////////////////////////////////////////////////////////
// Authors: lingl@DESKTOP-GH0R7CA.wucon.wustl.edu
// Description: EglyDriverTask implementation
////////////////////////////////////////////////////////////////////////////////

#include "EglyDriverTask.h"
#include "BCIStream.h"
#include <random>
#include <math.h>
#include <math.h>    

#define PI 3.14159265
#define EPSILON 0.000001

RegisterFilter( EglyDriverTask, 3 );
     // Change the location if appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations within SignalProcessing modules begin with "2."
     //       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
     //  - filter locations within Application modules begin with "3."


EglyDriverTask::EglyDriverTask() :
  mrDisplay( Window() )
{
    fixation_rect = new RectangularShape(mrDisplay);
    bar1_rect = new RectangularShape(mrDisplay);
    bar2_rect = new RectangularShape(mrDisplay);
    cue_rect = new RectangularShape(mrDisplay);
    target_rect = new RectangularShape(mrDisplay);
    wv_player = new WavePlayer();
    instruction_txt = new TextStimulus(mrDisplay);
    mpProgressBar = nullptr;
}

EglyDriverTask::~EglyDriverTask()
{
  Halt();
  delete fixation_rect;
  delete bar1_rect;
  delete bar2_rect;
  delete cue_rect;
  delete target_rect;
  delete wv_player;
  delete instruction_txt;
  delete mpProgressBar;
}

void
EglyDriverTask::Publish()
{
  // Define any parameters that the filter needs....

 BEGIN_PARAMETER_DEFINITIONS
    //GUI
     //color
     "Application:Experiment string FixationColor= 0xb5b4b6 0xb5b4b6 0x00000000 0xFFFFFFFF // "
     "Color of fixation (color)",
     "Application:Experiment string BarColor= 0xb5b4b6 0xb5b4b6 0x00000000 0xFFFFFFFF // "
     "Color of bar (color)",
     "Application:Experiment string CueColor= 0x656465 0x656465 0x00000000 0xFFFFFFFF // "
     "Color of cue (color)",
     "Application:Experiment string TargetColor= 0xafaeb0 0xa5a4a6 0x00000000 0xFFFFFFFF // "
     "Color of target (color)",
     //position and size
     //"Application:Experiment float BarWidth= 0.16 0.16 0 1  //The width of fixation cross",
     "Application:Experiment float BarThick= 0.04 0.04 0 1  //The thickness of the bar",
     "Application:Experiment float FixationWidth= 0.005 0.01 0 1  //The width of the fixation cross",
     "Application:Experiment float CueThick= 0.015 0.015 0 1  //The thickness of the cue",
     "Application:Experiment float TargetWidth= 0.04 0.04 0 1  //The width of the target",
     //duration
     "Application:Experiment float FixationDurationMin= 500ms 500ms 0 % //Minimum duration of fixation",
     "Application:Experiment float FixationDurationMax= 800ms 800ms 0 %  //Maximum duration of fixation",
     "Application:Experiment float BarDurationMin= 500ms 500ms 0 % //Minimum duration of bar",
     "Application:Experiment float BarDurationMax= 1200ms 1200ms 0 %  //Maximum duration of bar",
     "Application:Experiment float CueDuration= 100ms 100ms 0 %  //Duration of cue",
     "Application:Experiment float CueToTargetDurationMin= 500ms 500ms 0 % //Minimum duration of cue to target",
     "Application:Experiment float CueToTargetDurationMax= 1700ms 1700ms 0 %  //Maximum duration of cue to target",
     "Application:Experiment float TargetDuration= 100ms 100ms 0 %  //Duration of target",
     "Application:Experiment float PostTargetResponseTime= 900ms 900ms 0 %  //The maximum time allows the subject to respond after the target disappears",
     "Application:Experiment float FeedbackDuration= 1000ms 1000ms 0 %  //Duration of feedback",
     "Application:Experiment int TotalTrial= 200 200 0 %  //Number of the total trials",
     //percentage
     "Application:Experiment float CatchTrialPerctg= 10 10 0 %  //Percentage of the catch trial(no target)(%)",
     "Application:Experiment float CueTgtMatchPerctg= 72 72 0 %  //Percentage of the cue matches the target trial(%)",
     "Application:Experiment float CueTgtSmObjctPerctg= 9 9 0 %  //Percentage of the target mismatches the cue, on the same object(%)",
     "Application:Experiment float CueTgtDifObjctPerctg= 9 9 0 %  //Percentage of the target mismatches the cue, on the different objects. Set as 0 when it is a simplified task.(%)",
     //GUI scaling
     "Application:Experiment float ScreenWidth= 59.6 59.6 0 %  //Width of the monitor screen in cm",
     "Application:Experiment float ScreenHeight= 34.1 34.1 0 %  //Height of the monitor screen in cm",
     "Application:Experiment float EyeToSreenDistance= 60 90 0 %  //Distance from the subject's eyes to the screen in cm",
     "Application:Experiment float VisualAngle= 10 20 0 360  //The degrees of visual angle",
     //accuracy
     "Application:Experiment int AdjustContrastBy= 0 2 0 2 // Adjust the contrast of target color by: 0 overall accuracy, 1 match accuracy, 2 non-match accuracy(enumeration)",
     "Application:Experiment int AdjustContrastTrialNum= 15 15 0 %  //Number of trials needed to calculate the accuracy for adjusting the contrast",
     "Application:Experiment float AccuracyUp= 0.81 0.81 0 1  //The up boundary of the accuracy",
     "Application:Experiment float AccuracyLow= 0.79 0.79 0 1  //The low boundary of the accuracy",
     //simplified
     "Application:Simplified int Simplified= 0 0 0 1 // Enable the simplified version, only has two squares rather than two bars (boolean)",
     "Application:Simplified float SquareLength= 0.06 0.06 0 1  //The lenght of square",
     //staircase
     "Application:Staircase int IsStairCase= 0 0 0 1 // Enable the Staircase (boolean)",
     "Application:Staircase int DownCounts= 3 3 0 % // Contrast decreases one step following the number of consecutive correct trials.",
     "Application:Staircase int UpCounts= 1 1 0 % // Contrast increases one step following the number of consecutive incorrect trials.",
     //photodiode
     "Application:PhotoDiodePatch int PhotoDiodePatch= 1 1 0 1 // Display photo diode patch (boolean)",
     "Application:PhotoDiodePatch float PhotoDiodePatchHeight= 0.065 1 0 1 // Photo diode patch height in relative coordinates",
     "Application:PhotoDiodePatch float PhotoDiodePatchWidth= 0.05 1 0 1 // Photo diode patch width in relative coordinates",
     "Application:PhotoDiodePatch float PhotoDiodePatchLeft= 0 1 0 1 // Photo diode patch left in relative coordinates",
     "Application:PhotoDiodePatch float PhotoDiodePatchTop= 0.935 1 0 1 // Photo diode patch top in relative coordinates",
     "Application:PhotoDiodePatch int PhotoDiodePatchShape= 0 1 0 1 // Photo diode patch shape: 0 rectangle, 1 ellipse (enumeration)",
     "Application:PhotoDiodePatch int PhotoDiodePatchActiveColor= 0xffffffff 0 0 0xffffffff // Photo diode patch color when active (color)",
     "Application:PhotoDiodePatch int PhotoDiodePatchInactiveColor= 0x807f80 0 0 0xffffffff // Photo diode patch color when inactive, use 0xff000000 for transparent (color)",

     //progress bar
     "Application:ProgressBar int ProgressBar= 1 1 0 1 // Display progress bar (boolean)",
     "Application:ProgressBar float ProgressBarHeight= 50 50 0 % // Progress bar height in pixels",
     "Application:ProgressBar float ProgressBarWidth= 250 250 0 % // Progress bar width in pixels",
     "Application:ProgressBar string ProgressBarBackgroundColor= 0x00808080 0x00808080 0x00000000 0xFFFFFFFF // Color of progress bar background (color)",
     "Application:ProgressBar string ProgressBarForegroundColor= 0x00ffff00 0x00f0f0f0 0x00000000 0xFFFFFFFF // Color of progress bar foreground (color)",

 END_PARAMETER_DEFINITIONS


  // ...and likewise any state variables:

 BEGIN_STATE_DEFINITIONS

     "PhaseNumber  8 0 0 0",
     "TrialNumber  16 0 0 0",
     //"BlockNumber 8 0 0 0",
     "BarDirection 8 0 0 0",
     "CuePosition 8 0 0 0",
     "TargetPosition 8 0 0 0",
     "TrialType 8 0 0 0",
     "FixationDuration 8 0 0 0",
     "BarDuration 8 0 0 0",
     "CueToTargetDuration 8 0 0 0",
     "TargetColor 32 0 0 0",
     "Photodiode 8 0 0 0",

 END_STATE_DEFINITIONS

}

void
EglyDriverTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{

  Output = Input; // this simply passes information through about SampleBlock dimensions, etc....

  Parameter("FixationColor");
  Parameter("BarColor");
  Parameter("CueColor");
  Parameter("TargetColor");
  //Parameter("BarWidth");
  Parameter("BarThick");
  Parameter("FixationWidth");
  Parameter("CueThick");
  Parameter("TargetWidth");
  Parameter("TotalTrial");
  Parameter("SampleBlockSize");
  Parameter("WindowWidth");
  Parameter("WindowHeight");
  Parameter("CatchTrialPerctg");
  Parameter("CueTgtMatchPerctg");
  Parameter("CueTgtSmObjctPerctg");
  Parameter("CueTgtDifObjctPerctg");
  Parameter("ScreenWidth");
  Parameter("ScreenHeight");
  Parameter("EyeToSreenDistance");
  Parameter("VisualAngle");
  Parameter("AccuracyUp");
  Parameter("AccuracyLow");
  Parameter("AdjustContrastBy");
  Parameter("AdjustContrastTrialNum");
  Parameter("Simplified");
  Parameter("SquareLength");
  Parameter("IsStairCase");
  Parameter("DownCounts");
  Parameter("UpCounts");

  State("PhaseNumber");
  State("TrialNumber");
  State("Running");
  State("BarDirection");
  State("CuePosition");
  State("TargetPosition");
  State("TrialType");
  State("FixationDuration");
  State("BarDuration");
  State("CueToTargetDuration");
  State("KeyUp");
  State("KeyDown");
  State("TargetColor");
  State("Photodiode");

 //check interval time
  std::map<std::string, float> check_map;
  float fixation_min = Parameter("FixationDurationMin").InSampleBlocks();
  float fixation_max = Parameter("FixationDurationMax").InSampleBlocks();
  float bar_min = Parameter("BarDurationMin").InSampleBlocks();
  float bar_max = Parameter("BarDurationMax").InSampleBlocks();
  float cue = Parameter("CueDuration").InSampleBlocks();
  float cue_to_target_min = Parameter("CueToTargetDurationMin").InSampleBlocks();
  float cue_to_target_max = Parameter("CueToTargetDurationMax").InSampleBlocks();
  float target = Parameter("TargetDuration").InSampleBlocks();
  float post_target_rsp_time = Parameter("PostTargetResponseTime").InSampleBlocks();
  float feedback = Parameter("FeedbackDuration").InSampleBlocks();
  check_map["FixationDurationMin"] = fixation_min;
  check_map["FixationDurationMax"] = fixation_max;
  check_map["BarDurationMin"] = bar_min;
  check_map["BarDurationMax"] = bar_max;
  check_map["CueDuration"] = cue;
  check_map["CueToTargetDurationMin"] = cue_to_target_min;
  check_map["CueToTargetDurationMax"] = cue_to_target_max;
  check_map["TargetDuration"] = target;
  check_map["PostTargetResponseTime"] = post_target_rsp_time;
  check_map["FeedbackDuration"] = feedback;

  float sample_rate = Parameter("SamplingRate");//Sampling rate
  unsigned int block_size = Parameter("SampleBlockSize");// Sample block size
  float block_ms = ((float)(block_size) / (float)(sample_rate)) * 1000;

  for (std::map<std::string, float>::iterator it = check_map.begin(); it != check_map.end(); it++) {
      if (it->second < 1) {
          bcierr << it->first << " must be >= 1 sampleBlock duration(" + std::to_string(block_ms) + " ms." << std::endl;
      }
      if (fmod(it->second, 1.0f) > 0) {
          bcierr << it->first << " must be integer number of blocks." << std::endl;
      }
  }

  //simplified 
  if (Parameter("Simplified")) {
      float temp = 0.0f;
      float perc = Parameter("CueTgtDifObjctPerctg");
      if (fabs(temp - perc) > EPSILON) {
          bcierr << "Parameter CueTgtDifObjctPerctg must be set as 0 when the Parameter Simplified is selected." << std::endl;
      }
  }

}


void
EglyDriverTask::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
    //GUI scaling
    double tan_d = tan(Parameter("VisualAngle") * PI / 180.0);
    double range_in_cm = (double)Parameter("EyeToSreenDistance") * tan_d * 2.0;

    visual_range_horizontal = range_in_cm * 1.0 / (double)Parameter("ScreenWidth");
    visual_range_horizontal = std::round(visual_range_horizontal * 1000.0) / 1000.0;
    bciout << "horizontal range: " << visual_range_horizontal;

    visual_range_vertical = range_in_cm * 1.0 / (double)Parameter("ScreenHeight");
    visual_range_vertical = std::round(visual_range_vertical * 1000.0) / 1000.0;
    bciout << "vertical range: " << visual_range_vertical;
    
         

    //GUI
    aspectRatio = (double)Parameter("WindowWidth") / (double)Parameter("WindowHeight");
    bciout << "Ratio: " << aspectRatio;

    double fixation_height = (double)Parameter("FixationWidth") * aspectRatio;
    fixation_r.left = 0.5f - Parameter("FixationWidth") / 2;
    fixation_r.top = 0.5f - fixation_height / 2;
    fixation_r.right = 0.5f + Parameter("FixationWidth") / 2;
    fixation_r.bottom = 0.5f + fixation_height / 2;

    //simplified start
    squear_r_1.left = 0.5f - visual_range_horizontal / 2.0;
    squear_r_1.top = 0.5f - visual_range_vertical / 2.0;
    squear_r_1.right = squear_r_1.left + (double)Parameter("SquareLength");
    squear_r_1.bottom = squear_r_1.top + (double)Parameter("SquareLength") * aspectRatio;

    squear_r_2.right = 0.5f + visual_range_horizontal / 2.0;
    squear_r_2.left = squear_r_2.right - (double)Parameter("SquareLength");
    squear_r_2.top = 0.5f - visual_range_vertical / 2.0;
    squear_r_2.bottom = squear_r_2.top + (double)Parameter("SquareLength") * aspectRatio;

    squear_r_3.right = 0.5f + visual_range_horizontal / 2.0;
    squear_r_3.left = squear_r_3.right - (double)Parameter("SquareLength");
    squear_r_3.bottom = 0.5f + visual_range_vertical / 2.0;
    squear_r_3.top = squear_r_3.bottom - (double)Parameter("SquareLength") * aspectRatio;

    squear_r_4.left = 0.5f - visual_range_horizontal / 2.0;
    squear_r_4.right = squear_r_4.left + (double)Parameter("SquareLength");
    squear_r_4.bottom = 0.5f + visual_range_vertical / 2.0;
    squear_r_4.top = squear_r_4.bottom - (double)Parameter("SquareLength") * aspectRatio;

    sCue_r_1.left = squear_r_1.left - (double)Parameter("CueThick");
    sCue_r_1.right = squear_r_1.right + (double)Parameter("CueThick");
    sCue_r_1.top = squear_r_1.top - (double)Parameter("CueThick") * aspectRatio;
    sCue_r_1.bottom = squear_r_1.bottom + (double)Parameter("CueThick") * aspectRatio;

    sCue_r_2.left = squear_r_2.left - (double)Parameter("CueThick");
    sCue_r_2.right = squear_r_2.right + (double)Parameter("CueThick");
    sCue_r_2.top = squear_r_2.top - (double)Parameter("CueThick") * aspectRatio;
    sCue_r_2.bottom = squear_r_2.bottom + (double)Parameter("CueThick") * aspectRatio;

    sCue_r_3.left = squear_r_3.left - (double)Parameter("CueThick");
    sCue_r_3.right = squear_r_3.right + (double)Parameter("CueThick");
    sCue_r_3.top = squear_r_3.top - (double)Parameter("CueThick") * aspectRatio;
    sCue_r_3.bottom = squear_r_3.bottom + (double)Parameter("CueThick") * aspectRatio;

    sCue_r_4.left = squear_r_4.left - (double)Parameter("CueThick");
    sCue_r_4.right = squear_r_4.right + (double)Parameter("CueThick");
    sCue_r_4.top = squear_r_4.top - (double)Parameter("CueThick") * aspectRatio;
    sCue_r_4.bottom = squear_r_4.bottom + (double)Parameter("CueThick") * aspectRatio;

    //simplified end

    // original start
    //horizontal
    bar1_r_h.left = 0.5f - visual_range_horizontal / 2.0;
    bar1_r_h.top = 0.5f - visual_range_vertical  / 2.0;
    bar1_r_h.right = 0.5f + visual_range_horizontal / 2.0;
    bar1_r_h.bottom = bar1_r_h.top + (double)Parameter("BarThick") * aspectRatio;
    //vertical
    bar1_r_v.left = 0.5f - visual_range_horizontal / 2.0;
    bar1_r_v.top = 0.5f - visual_range_vertical / 2.0;
    bar1_r_v.right = bar1_r_v.left + (double)Parameter("BarThick");
    bar1_r_v.bottom = 0.5f + visual_range_vertical / 2.0;


    //horizontal
    bar2_r_h.bottom = 0.5f + visual_range_vertical / 2.0;
    bar2_r_h.left = 0.5f - visual_range_horizontal / 2.0;
    bar2_r_h.top = bar2_r_h.bottom - (double)Parameter("BarThick") * aspectRatio;
    bar2_r_h.right = 0.5f + visual_range_horizontal / 2.0;
    
    //vertical
    bar2_r_v.right = 0.5f + visual_range_horizontal / 2.0;
    bar2_r_v.left = bar2_r_v.right - (double)Parameter("BarThick");
    bar2_r_v.top = 0.5f - visual_range_vertical / 2.0;  
    bar2_r_v.bottom = 0.5f + visual_range_vertical / 2.0;

    //bciout << "bar1_r_h(left, top, right, bottom): " << bar1_r_h.left << ", " << bar1_r_h.top << ", " << bar1_r_h.right << ", " << bar1_r_h.bottom;
    //bciout << "bar2_r_h(left, top, right, bottom): " << bar2_r_h.left << ", " << bar2_r_h.top << ", " << bar2_r_h.right << ", " << bar2_r_h.bottom;
    //bciout << "fixation_r(left, top, right, bottom): " << fixation_r.left << ", " << fixation_r.top << ", " << fixation_r.right << ", " << fixation_r.bottom;

    //8 possible position, target
    //horizontal
    target_h_1.left = bar1_r_h.left;
    target_h_1.right = bar1_r_h.left + Parameter("TargetWidth");
    target_h_1.top = bar1_r_h.top;
    target_h_1.bottom = bar1_r_h.bottom;

    target_h_2.left = bar1_r_h.right - Parameter("TargetWidth");
    target_h_2.right = bar1_r_h.right;
    target_h_2.top = bar1_r_h.top;
    target_h_2.bottom = bar1_r_h.bottom;

    target_h_3.left = bar2_r_h.right - Parameter("TargetWidth");
    target_h_3.right = bar2_r_h.right;
    target_h_3.top = bar2_r_h.top;
    target_h_3.bottom = bar2_r_h.bottom;

    target_h_4.left = bar2_r_h.left;
    target_h_4.right = bar2_r_h.left + Parameter("TargetWidth");
    target_h_4.top = bar2_r_h.top;
    target_h_4.bottom = bar2_r_h.bottom;
    //vertical
    target_v_1.left = bar1_r_v.left;
    target_v_1.right = bar1_r_v.right;
    target_v_1.top = bar1_r_v.top;
    target_v_1.bottom = bar1_r_v.top + (double)Parameter("TargetWidth") * aspectRatio;

    target_v_2.left = bar2_r_v.left;
    target_v_2.right = bar2_r_v.right;
    target_v_2.top = bar2_r_v.top;
    target_v_2.bottom = bar2_r_v.top + (double)Parameter("TargetWidth") * aspectRatio;

    target_v_3.left = bar2_r_v.left;
    target_v_3.right = bar2_r_v.right;
    target_v_3.top = bar2_r_v.bottom - (double)Parameter("TargetWidth") * aspectRatio;
    target_v_3.bottom = bar2_r_v.bottom;

    target_v_4.left = bar1_r_v.left;
    target_v_4.right = bar1_r_v.right;
    target_v_4.top = bar1_r_v.bottom - (double)Parameter("TargetWidth") * aspectRatio;
    target_v_4.bottom = bar1_r_v.bottom;

    //8 possible position, cue
    //horizontal
    cue_h_1.left = target_h_1.left - Parameter("CueThick");
    cue_h_1.top = target_h_1.top - (double)Parameter("CueThick") * aspectRatio;
    cue_h_1.right = target_h_1.right;
    cue_h_1.bottom = target_h_1.bottom + (double)Parameter("CueThick") * aspectRatio;

    cue_h_2.left = target_h_2.left;
    cue_h_2.top = target_h_2.top - (double)Parameter("CueThick") * aspectRatio;
    cue_h_2.right = target_h_2.right + (double)Parameter("CueThick");
    cue_h_2.bottom = target_h_2.bottom + (double)Parameter("CueThick") * aspectRatio;

    cue_h_3.left = target_h_3.left;
    cue_h_3.top = target_h_3.top - (double)Parameter("CueThick") * aspectRatio;
    cue_h_3.right = target_h_3.right + Parameter("CueThick");
    cue_h_3.bottom = target_h_3.bottom + (double)Parameter("CueThick") * aspectRatio;

    cue_h_4.left = target_h_4.left - Parameter("CueThick");
    cue_h_4.top = target_h_4.top - (double)Parameter("CueThick") * aspectRatio;
    cue_h_4.right = target_h_4.right;
    cue_h_4.bottom = target_h_4.bottom + (double)Parameter("CueThick") * aspectRatio;
    //vertival
    cue_v_1.left = target_v_1.left - Parameter("CueThick");
    cue_v_1.top = target_v_1.top - (double)Parameter("CueThick") * aspectRatio;
    cue_v_1.right = target_v_1.right + Parameter("CueThick");
    cue_v_1.bottom = target_v_1.bottom;

    cue_v_2.left = target_v_2.left - Parameter("CueThick");
    cue_v_2.top = target_v_2.top - (double)Parameter("CueThick") * aspectRatio;
    cue_v_2.right = target_v_2.right + Parameter("CueThick");
    cue_v_2.bottom = target_v_2.bottom;

    cue_v_3.left = target_v_3.left - Parameter("CueThick");
    cue_v_3.top = target_v_3.top;
    cue_v_3.right = target_v_3.right + Parameter("CueThick");
    cue_v_3.bottom = target_v_3.bottom + (double)Parameter("CueThick") * aspectRatio;

    cue_v_4.left = target_v_4.left - Parameter("CueThick");
    cue_v_4.top = target_v_4.top;
    cue_v_4.right = target_v_4.right + Parameter("CueThick");
    cue_v_4.bottom = target_v_4.bottom + (double)Parameter("CueThick") * aspectRatio;

    // original end

    //target, cue rectangulor
    target_r_h.clear();
    target_r_v.clear();
    cue_r_h.clear();
    cue_r_v.clear();
    sCue_container.clear();
    sTarget_container.clear();

    target_r_h.push_back(target_h_1);
    target_r_h.push_back(target_h_2);
    target_r_h.push_back(target_h_3);
    target_r_h.push_back(target_h_4);

    target_r_v.push_back(target_v_1);
    target_r_v.push_back(target_v_2);
    target_r_v.push_back(target_v_3);
    target_r_v.push_back(target_v_4);

    cue_r_h.push_back(cue_h_1);
    cue_r_h.push_back(cue_h_2);
    cue_r_h.push_back(cue_h_3);
    cue_r_h.push_back(cue_h_4);

    cue_r_v.push_back(cue_v_1);
    cue_r_v.push_back(cue_v_2);
    cue_r_v.push_back(cue_v_3);
    cue_r_v.push_back(cue_v_4);

    sCue_container.push_back(sCue_r_1);
    sCue_container.push_back(sCue_r_2);
    sCue_container.push_back(sCue_r_3);
    sCue_container.push_back(sCue_r_4);

    sTarget_container.push_back(squear_r_1);
    sTarget_container.push_back(squear_r_2);
    sTarget_container.push_back(squear_r_3);
    sTarget_container.push_back(squear_r_4);

    fixation_rect->SetFillColor(RGBColor(Parameter("FixationColor")));
    fixation_rect->SetObjectRect(fixation_r);
    fixation_rect->SetColor(RGBColor::None);
    fixation_rect->SetVisible(false);

    bar1_rect->SetFillColor(RGBColor(Parameter("BarColor")));
    bar1_rect->SetObjectRect(squear_r_1);
    bar1_rect->SetColor(RGBColor::None);
    bar1_rect->SetZOrder(1.0);
    bar1_rect->SetVisible(false);

    bar2_rect->SetFillColor(RGBColor(Parameter("BarColor")));
    bar2_rect->SetObjectRect(squear_r_2);
    bar2_rect->SetColor(RGBColor::None);
    bar2_rect->SetZOrder(1.0);
    bar2_rect->SetVisible(false);

    target_rect->SetFillColor(RGBColor(Parameter("TargetColor")));
    target_rect->SetObjectRect(squear_r_1);
    target_rect->SetColor(RGBColor::None);
    target_rect->SetZOrder(0.0);
    target_rect->SetVisible(false);

    cue_rect->SetFillColor(RGBColor(Parameter("CueColor")));
    cue_rect->SetObjectRect(sCue_r_1);
    cue_rect->SetColor(RGBColor::None);
    cue_rect->SetZOrder(2.0);
    cue_rect->SetVisible(false);

    //sounds
    wv_player->SetVolume(100);

    //text
    instruction_txt->SetText("When you are ready, please push the ENTER.");
    instruction_txt->SetTextHeight(1.0);
    instruction_txt->SetTextColor(RGBColor::White);
    instruction_txt->SetScalingMode(GUI::ScalingMode::AdjustBoth);
    instruction_txt->SetObjectRect(GUI::Rect({ 0.48, 0.48, 0.52, 0.52 }));
    instruction_txt->Conceal();

    //interval time
    fixation_min = floor(Parameter("FixationDurationMin").InSampleBlocks());
    fixation_max= floor(Parameter("FixationDurationMax").InSampleBlocks());
    bar_min = floor(Parameter("BarDurationMin").InSampleBlocks());
    bar_max = floor(Parameter("BarDurationMax").InSampleBlocks());
    cue_dur= floor(Parameter("CueDuration").InSampleBlocks());
    cue_to_target_min = floor(Parameter("CueToTargetDurationMin").InSampleBlocks());
    cue_to_target_max = floor(Parameter("CueToTargetDurationMax").InSampleBlocks());
    target_dur = floor(Parameter("TargetDuration").InSampleBlocks());
    pst_target_rsp_dur = floor(Parameter("PostTargetResponseTime").InSampleBlocks());
    feedback_dur = floor(Parameter("FeedbackDuration").InSampleBlocks());

    //percentage
    percentage.clear();
    percentage.push_back(Parameter("CatchTrialPerctg"));
    percentage.push_back(Parameter("CueTgtMatchPerctg"));
    percentage.push_back(Parameter("CueTgtSmObjctPerctg"));
    percentage.push_back(Parameter("CueTgtDifObjctPerctg"));//if it is simplified version, always make it 0.
    
    //accuracy
    WINDOW_SIZE = Parameter("AdjustContrastTrialNum");
    acc_up = Parameter("AccuracyUp");
    acc_low = Parameter("AccuracyLow");

    //simplified
    is_simplified = Parameter("Simplified");
    bar_direct_max = is_simplified ? 4 : 2;
    //cue_position_max = is_simplified ? 2 : 4;

    //staircase
    is_staircase = Parameter("IsStairCase");
    down_counts = Parameter("DownCounts");
    up_counts = Parameter("UpCounts");
   
    target_color = Parameter("TargetColor");

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

    //progress bar
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

    //reaction time
    mOneBlockMs = ((float)Parameter("SampleBlockSize") / (float)Parameter("SamplingRate")) * 1000; //one block size in msec 
    bciout << "one block time: " << mOneBlockMs << std::endl;
}

void
EglyDriverTask::StartRun()
{
    my_phase = Phase::instruction;
    m_block_in_phase = 0;
    m_cur_trial = 0;
    accuracy_overall_window.clear();
    accuracy_match_window.clear();
    accuracy_non_match_window.clear();
    false_detation_rate_window.clear();
    //staircase
    overall_up_counts = 0;
    overall_down_counts = 0;
    match_up_counts = 0;
    match_down_counts = 0;
    non_match_up_counts = 0;
    non_match_down_counts = 0;
    pre_response = -1;

    Patch(0);

    //progress bar
    if (mpProgressBar)
    {
        mpProgressBar->SetTotal(Parameter("TotalTrial")).SetCurrent(0);
        mpProgressBar->SendDifferenceFrame();
    }

    State("PhaseNumber") = my_phase;
    State("TrialNumber") = 0;
    State("FixationDuration") = 0;
    State("BarDuration") = 0;
    State("CueToTargetDuration") = 0;
    State("BarDirection") = 0;
    State("CuePosition") = 0;
    State("TargetPosition") = 0;
    State("TrialType") = 0;
    State("TargetColor") = 0;
    State("Photodiode") = 0;
    
    AppLog << "=================================================" << std::endl;
    AppLog << "Trial Type                                   Percentage\n" << std::endl;
    AppLog << "Catch Trial(1)                                      " << Parameter("CatchTrialPerctg") << std::endl;
    AppLog << "Match Trial(2)                                     " << Parameter("CueTgtMatchPerctg") << std::endl;
    AppLog << "Non-match Trial Same Object(3)             " << Parameter("CueTgtSmObjctPerctg") << std::endl;
    AppLog << "Non-match Trial Different Object(4)        " << Parameter("CueTgtDifObjctPerctg") << std::endl;
   
    AppLog << "\nNumber of Trials: " << Parameter("TotalTrial") << "\n" << std::endl;
    //AppLog << "=================================================" << std::endl;
}


void
EglyDriverTask::Process( const GenericSignal& Input, GenericSignal& Output )
{
  Output = Input; // This passes the signal through unmodified.

  m_block_in_phase++;
  State("PhaseNumber") = my_phase;

  //check if the subject release the space bar
  if ( (my_phase == Phase::fixation || my_phase == Phase::bar || my_phase == Phase::cue || my_phase == Phase::cue_to_target) 
      && checkKeyPush("KeyUp", 32)) {
      my_phase = Phase::feedback;
      m_block_in_phase = 0;

  }
  
  switch (my_phase) {
  case Phase::instruction:
      if (m_block_in_phase == 1) {
          instruction_txt->Present(); 
      }

      if (checkKeyPush("KeyDown", 13)) {
          my_phase = Phase::wait_to_begin;
          m_block_in_phase = 0;
          instruction_txt->Conceal();
      }
      break;
  case Phase::wait_to_begin:
      if (m_block_in_phase == 1) {
          playSound(BEGIN_SOUND);
          Patch(1);
          State("Photodiode") = 1;
          //initalize the random durations
          fixation_dur = uniformRandomGenerator(fixation_min, fixation_max);
          bar_dur = uniformRandomGenerator(bar_min, bar_max);
          cue_to_target_dur = uniformRandomGenerator(cue_to_target_min, cue_to_target_max);
          bar_direct = uniformRandomGenerator(bar_direct_min, bar_direct_max);

          if (is_simplified) {
              switch (bar_direct)
              {
              case 1:
                  cue_position_min = 1;
                  cue_position_max = 2;
                  break;
              case 2:
                  cue_position_min = 2;
                  cue_position_max = 3;
                  break;
              case 3:
                  cue_position_min = 3;
                  cue_position_max = 4;
                  break;
              case 4:
                  cue_position_min = 1;
                  cue_position_max = 2;
                  break;
              default:
                  break;
              }
          }
          else {
              cue_position_min = 1;
              cue_position_max = 4;
          }
          cue_position = uniformRandomGenerator(cue_position_min, cue_position_max);
          if (is_simplified && bar_direct == 4) {// can only be either 1 or 4
              cue_position = cue_position == 2 ? 4 : 1;
          }
              
          trial_type = distributionGenerator(percentage);
          trial_type = trial_type + 1;
          response = 0;
          

          State("FixationDuration") = fixation_dur;
          State("BarDuration") = bar_dur;
          State("CueToTargetDuration") = cue_to_target_dur;
          State("BarDirection") = bar_direct;
          State("CuePosition") = cue_position;
          State("TrialType") = trial_type;
          State("TargetColor") = target_rect->FillColor();
          target_color = State("TargetColor");

          //trial number
          m_cur_trial++;
          State("TrialNumber") = m_cur_trial;
          AppLog << "================================================="  << std::endl;
          AppLog << "Trial #" << m_cur_trial << "/" << Parameter("TotalTrial") << std::endl;
          AppLog << "Trial Type: " << trial_type << std::endl;
          AppLog << "Fixation Duration: " << fixation_dur * mOneBlockMs << "ms." << std::endl;
          AppLog << "Bar Duration: " << bar_dur * mOneBlockMs << "ms." << std::endl;
          AppLog << "CueToTarget Duration: " << cue_to_target_dur * mOneBlockMs << "ms." << std::endl;
          AppLog << "Bar Direction: " << bar_direct << std::endl;
          AppLog << "Cue Position: " << cue_position << std::endl;
      }
      else if(m_block_in_phase == BLOCK_NUM_PHOTODIODE) {
          Patch(0);
          State("Photodiode") = 0;
      }

      if (checkKeyPush("KeyDown", 32)) {
            my_phase = Phase::fixation;
            m_block_in_phase = 0;
            //reset states
            State("FixationDuration") = 0;
            State("BarDuration") = 0;
            State("CueToTargetDuration") = 0;
            State("BarDirection") = 0;
            State("CuePosition") = 0;
            State("TrialType") = 0;
            State("TargetColor") = 0;
      }
      
      break;
  case Phase::fixation:
      if (m_block_in_phase == 1) {
          fixation_rect->SetVisible(true);
          Patch(1);
          State("Photodiode") = 1;
      }
      else if (m_block_in_phase == BLOCK_NUM_PHOTODIODE) {
          Patch(0);
          State("Photodiode") = 0;
      }

      if (m_block_in_phase >= fixation_dur) {
          my_phase = Phase::bar;
          m_block_in_phase = 0;
      }
      
      break;

  case Phase::bar:
      if (m_block_in_phase == 1) {
          if (!is_simplified) {
              if (bar_direct == 1) {
                  bar1_rect->SetObjectRect(bar1_r_h);
                  bar2_rect->SetObjectRect(bar2_r_h);
              }
              else {
                  bar1_rect->SetObjectRect(bar1_r_v);
                  bar2_rect->SetObjectRect(bar2_r_v);
              }
          }
          else {
              switch (bar_direct)
              {
              case 1:
                  bar1_rect->SetObjectRect(squear_r_1);
                  bar2_rect->SetObjectRect(squear_r_2);
                  break;
              case 2:
                  bar1_rect->SetObjectRect(squear_r_2);
                  bar2_rect->SetObjectRect(squear_r_3);
                  break;
              case 3:
                  bar1_rect->SetObjectRect(squear_r_3);
                  bar2_rect->SetObjectRect(squear_r_4);
                  break;
              case 4:
                  bar1_rect->SetObjectRect(squear_r_1);
                  bar2_rect->SetObjectRect(squear_r_4);
                  break;
              default:
                  break;
              }
          }

          
          bar1_rect->SetVisible(true);
          bar2_rect->SetVisible(true);
          Patch(1);
          State("Photodiode") = 1;
      }
      else if (m_block_in_phase == BLOCK_NUM_PHOTODIODE) {
          Patch(0);
          State("Photodiode") = 0;
      }
      else {
          if (!is_staircase) {
              if (m_block_in_phase >= bar_dur) {
                  my_phase = Phase::cue;
                  m_block_in_phase = 0;
              }
          }
          else {
              if (m_block_in_phase >= cue_to_target_dur) {
                  my_phase = Phase::target;
                  m_block_in_phase = 0;
              }
          }
      }
          

      break;

  case Phase::cue:
      if (m_block_in_phase == 1) {
          if (!is_simplified) {
              if (bar_direct == 1) cue_rect->SetObjectRect(cue_r_h[cue_position - 1]);
              else cue_rect->SetObjectRect(cue_r_v[cue_position - 1]);
          }
          else {
              cue_rect->SetObjectRect(sCue_container[cue_position - 1]);
          }

          cue_rect->SetVisible(true);
          Patch(1);
          State("Photodiode") = 1;
      }
      else if (m_block_in_phase == BLOCK_NUM_PHOTODIODE) {
          Patch(0);
          State("Photodiode") = 0;
      }

      if (m_block_in_phase >= cue_dur) {
          my_phase = Phase::cue_to_target;
          m_block_in_phase = 0;
      }
     
      break;
  case Phase::cue_to_target:
      if (m_block_in_phase == 1) {
          cue_rect->SetVisible(false);
      }
      else if (m_block_in_phase >= cue_to_target_dur) {
          my_phase = Phase::target;
          m_block_in_phase = 0;
      }
      break;
  case Phase::target:
      if (m_block_in_phase == 1) {
          switch (trial_type)
          {
          case 1://no target, catch trial

              break;
          case 2://cue target match
              if (!is_simplified) {
                  if (bar_direct == 1) target_rect->SetObjectRect(target_r_h[cue_position - 1]);
                  else target_rect->SetObjectRect(target_r_v[cue_position - 1]);
              }
              else {
                  target_rect->SetObjectRect(sTarget_container[cue_position - 1]);
              }
              State("TargetPosition") = cue_position;
              break;
          case 3: // cue target don't match, on the same object
              if (!is_simplified) {
                  if (bar_direct == 1) {
                      if (cue_position == 1 || cue_position == 3) {
                          target_rect->SetObjectRect(target_r_h[cue_position]);
                          State("TargetPosition") = cue_position + 1;
                      }
                      else if (cue_position == 2 || cue_position == 4) {
                          target_rect->SetObjectRect(target_r_h[cue_position - 2]);
                          State("TargetPosition") = cue_position - 1;
                      }
                  }
                  else {
                      if (cue_position == 1) {
                          target_rect->SetObjectRect(target_r_v[3]);
                          State("TargetPosition") = 4;
                      }
                      else if (cue_position == 2) {
                          target_rect->SetObjectRect(target_r_v[2]);
                          State("TargetPosition") = 3;
                      }
                      else if (cue_position == 3) {
                          target_rect->SetObjectRect(target_r_v[1]);
                          State("TargetPosition") = 2;
                      }
                      else if (cue_position == 4) {
                          target_rect->SetObjectRect(target_r_v[0]);
                          State("TargetPosition") = 1;
                      }
                  }
              }
              else {
                  switch (bar_direct)
                  {
                  case 1:
                      if (cue_position == 1) {
                          target_rect->SetObjectRect(sTarget_container[1]);
                          State("TargetPosition") = 2;
                      }
                      else if (cue_position == 2) {
                          target_rect->SetObjectRect(sTarget_container[0]);
                          State("TargetPosition") = 1;
                      }
                      break;
                  case 2:
                      if (cue_position == 2) {
                          target_rect->SetObjectRect(sTarget_container[2]);
                          State("TargetPosition") = 3;
                      }
                      else if (cue_position == 3) {
                          target_rect->SetObjectRect(sTarget_container[1]);
                          State("TargetPosition") = 2;
                      }
                      break;
                  case 3:
                      if (cue_position == 3) {
                          target_rect->SetObjectRect(sTarget_container[3]);
                          State("TargetPosition") = 4;
                      }
                      else if (cue_position == 4) {
                          target_rect->SetObjectRect(sTarget_container[2]);
                          State("TargetPosition") = 3;
                      }
                      break;
                  case 4:
                      if (cue_position == 1) {
                          target_rect->SetObjectRect(sTarget_container[3]);
                          State("TargetPosition") = 4;
                      }
                      else if (cue_position == 4) {
                          target_rect->SetObjectRect(sTarget_container[0]);
                          State("TargetPosition") = 1;
                      }
                      break;
                  default:
                      break;
                  }
              }

              break;
          case 4: //cue target don't match,  on the different object
              if (!is_simplified) {
                  if (bar_direct == 1) {
                      if (cue_position == 1) {
                          target_rect->SetObjectRect(target_r_h[3]);
                          State("TargetPosition") = 4;
                      }
                      else if (cue_position == 2) {
                          target_rect->SetObjectRect(target_r_h[2]);
                          State("TargetPosition") = 3;
                      }
                      else if (cue_position == 3) {
                          target_rect->SetObjectRect(target_r_h[1]);
                          State("TargetPosition") = 2;
                      }
                      else if (cue_position == 4) {
                          target_rect->SetObjectRect(target_r_h[0]);
                          State("TargetPosition") = 1;
                      }
                  }
                  else {
                      if (cue_position == 1) {
                          target_rect->SetObjectRect(target_r_v[1]);
                          State("TargetPosition") = 2;
                      }
                      else if (cue_position == 2) {
                          target_rect->SetObjectRect(target_r_v[0]);
                          State("TargetPosition") = 1;
                      }
                      else if (cue_position == 3) {
                          target_rect->SetObjectRect(target_r_v[3]);
                          State("TargetPosition") = 4;
                      }
                      else if (cue_position == 4) {
                          target_rect->SetObjectRect(target_r_v[2]);
                          State("TargetPosition") = 3;
                      }
                  }
              }
              else {
                  bcierr << "Simplified trial type should not be 4!" << std::endl;
              }

              break;
          default:
              break;
          }

          AppLog << "Target Position: " << State("TargetPosition") << std::endl;
          
          if (trial_type == 1) {
              target_rect->SetVisible(false);
          }
          else{
              target_rect->SetVisible(true);
              Patch(1);
              State("Photodiode") = 1;

              if (is_simplified && trial_type == 4) {
                  target_rect->SetVisible(false);
                  Patch(0);
                  State("Photodiode") = 0;
              }
          }
      }
      else if (m_block_in_phase == BLOCK_NUM_PHOTODIODE) {
          Patch(0);
          State("Photodiode") = 0;
          State("TargetPosition") = 0;
      }
      r_index = -1;
      if (m_block_in_phase < target_dur && checkKeyPushIndex("KeyUp", 32, r_index)) {
          //output reaction time
          float reaction_time = mOneBlockMs * (m_block_in_phase - 1) + (float)r_index / (float)Parameter("SamplingRate") * 1000;
          //bciout << "reaction time: " << reaction_time << "\n m_block_in_phase" << m_block_in_phase << "\n index: " << r_index << std::endl;
          AppLog << "Response Time: " << reaction_time << "ms." << std::endl;

        if (trial_type == 1) {
            response = 0;
        }
        else {
            response = 1;
        }

        my_phase = Phase::feedback;
        m_block_in_phase = 0;
      }
      else if (m_block_in_phase >= target_dur) {
        my_phase = Phase::post_target_response;
        m_block_in_phase = 0;
      }
      
      break;
  case Phase::post_target_response:
      r_index = -1;
      if (m_block_in_phase == 1) {
          target_rect->SetVisible(false);
      }
      else if (m_block_in_phase < pst_target_rsp_dur && checkKeyPushIndex("KeyUp", 32, r_index)) {
          //output reaction time
          float reaction_time = mOneBlockMs * (target_dur + m_block_in_phase - 1) + (float)r_index / (float)Parameter("SamplingRate") * 1000;
          //bciout << "reaction time: " << reaction_time << "\n target_dur: " << target_dur << "\n m_block_in_phase" << m_block_in_phase << "\n index: " << r_index << std::endl;
          AppLog << "Response Time: " << reaction_time << "ms." << std::endl;

          if (trial_type == 1) {
              response = 0;
          }
          else {
              response = 1;
          }
          my_phase = Phase::feedback;
          m_block_in_phase = 0;
      }
      else if (m_block_in_phase >= pst_target_rsp_dur) {
          if (trial_type == 1) {
              response = 1;
          }else{
              response = 0;
          }
          
          my_phase = Phase::feedback;
          m_block_in_phase = 0;
      }
      break;
  case Phase::feedback:
      if (m_block_in_phase == 1) {
          if (response == 1) {
              playSound(CORRECT_SOUND);
          }
          else {
              playSound(INCORRECT_SOUND);
          }
          
          fixation_rect->SetVisible(false);
          bar1_rect->SetVisible(false);
          bar2_rect->SetVisible(false);
          cue_rect->SetVisible(false);
          target_rect->SetVisible(false);

          //output the accuracy
          if (accuracy_overall_window.size() >= WINDOW_SIZE) {
              accuracy_overall_window.pop_front();
          }
          accuracy_overall_window.push_back(response);

          if (trial_type == 2) {
              if (accuracy_match_window.size() >= WINDOW_SIZE) {
                  accuracy_match_window.pop_front();
              }
              accuracy_match_window.push_back(response);
          }

          if (trial_type == 3 || trial_type == 4) {
              if (accuracy_non_match_window.size() >= WINDOW_SIZE) {
                  accuracy_non_match_window.pop_front();
              }
              accuracy_non_match_window.push_back(response);
          }

          if (trial_type == 1) {
              if (false_detation_rate_window.size() >= WINDOW_SIZE) {
                  false_detation_rate_window.pop_front();
              }
              false_detation_rate_window.push_back(!response);
          }

          float overall_acc = accuracyCalculator(accuracy_overall_window);
          overall_acc = roundf(overall_acc * 10000) / 10000;

          float match_acc = accuracyCalculator(accuracy_match_window);
          match_acc = roundf(match_acc * 10000) / 10000;

          float non_match_acc = accuracyCalculator(accuracy_non_match_window);
          non_match_acc = roundf(non_match_acc * 10000) / 10000;

          float false_dec_rate = accuracyCalculator(false_detation_rate_window);
          false_dec_rate = roundf(false_dec_rate * 10000) / 10000;
          
               
          AppLog << "Overall accuracy: " << overall_acc * 100 << "%" << std::endl;
          AppLog << "Match accuracy: " << match_acc * 100 << "%" << std::endl;
          AppLog << "Non match accuracy: " << non_match_acc * 100 << "%" << std::endl;
          AppLog << "False detection accuracy: " << false_dec_rate * 100 << "%" << std::endl;
          

          if (!is_staircase) {//adjust the illuminance of the target(main task)
              int acc_type = Parameter("AdjustContrastBy");
              switch (acc_type)
              {
              case 0:
                  if (accuracy_overall_window.size() == WINDOW_SIZE) {
                      adjustContrast(overall_acc);
                  }
                  break;
              case 1:
                  if (accuracy_match_window.size() == WINDOW_SIZE) {
                      adjustContrast(match_acc);
                  }
                  break;
              case 2:
                  if (accuracy_non_match_window.size() == WINDOW_SIZE) {
                      adjustContrast(non_match_acc);
                  }
                  break;
              default:
                  break;
              }
          }
          else {// staircase
              if (pre_response != -1 && pre_response != response) {
                  overall_up_counts = 0;
                  overall_down_counts = 0;
                  match_up_counts = 0;
                  match_down_counts = 0;
                  non_match_up_counts = 0;
                  non_match_down_counts = 0;
              }
              pre_response = response;
              int my_type = Parameter("AdjustContrastBy");
              switch (my_type)
              {
              case 0://overall
                  if (response) overall_down_counts++;
                  else overall_up_counts++;

                  if (overall_down_counts == down_counts) {
                      adjustContrastSC(2);
                      overall_down_counts = 0;
                  }
                  if (overall_up_counts == up_counts) {
                      adjustContrastSC(1);
                      overall_up_counts = 0;
                  }
                  break;
              case 1://mach
                  if (trial_type == 2) {
                      if (response) match_down_counts++;
                      else match_up_counts++;

                      if (match_down_counts == down_counts) {
                          adjustContrastSC(2);
                          match_down_counts = 0;
                      }

                      if (match_up_counts == up_counts) {
                          adjustContrastSC(1);
                          match_up_counts = 0;

                      }
                  }
              case 2://non match
                  if (trial_type == 3 || trial_type == 4) {
                      if (response) non_match_down_counts++;
                      else non_match_up_counts++;

                      if (non_match_down_counts == down_counts) {
                          adjustContrastSC(2);
                          non_match_down_counts = 0;
                      }

                      if (non_match_up_counts == up_counts) {
                          adjustContrastSC(1);
                          non_match_up_counts = 0;
                      }
                  }
                  break;
              default:
                  break;
              }           
          }

      }
      else if (m_block_in_phase >= feedback_dur) {
          if (m_cur_trial >= Parameter("TotalTrial")) {
              my_phase = Phase::none;
          }
          else {
              my_phase = Phase::wait_to_begin;              
          }
          m_block_in_phase = 0;

          //update the progress bar
          if (mpProgressBar)
          {
              Assert(mpProgressBar->Current() < mpProgressBar->Total());
              int current = mpProgressBar->Current();
              mpProgressBar->SetCurrent(current + 1);
              mpProgressBar->SendDifferenceFrame();
          }
      }
      break;
  case Phase::none:
      State("Running") = 0;
      instruction_txt->SetText("You have finished the task, thank you!");
      instruction_txt->Present();
      break;
  }


}

void
EglyDriverTask::StopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // or because the run has reached its natural end.
  bciwarn << "Goodbye World.";
  if (is_staircase) {
      AppLog << "\nThe target color is: " << "0x" << std::hex << target_color << std::endl;
  }
  my_phase = Phase::instruction;
  m_block_in_phase = 0;
  m_cur_trial = 0;
  accuracy_overall_window.clear();
  accuracy_match_window.clear();
  accuracy_non_match_window.clear();
  false_detation_rate_window.clear();

  State("PhaseNumber") = my_phase;
  State("TrialNumber") = 0;
  State("FixationDuration") = 0;
  State("BarDuration") = 0;
  State("CueToTargetDuration") = 0;
  State("BarDirection") = 0;
  State("CuePosition") = 0;
  State("TargetPosition") = 0;
  State("TrialType") = 0;
  State("TargetColor") = 0;
  // You know, you can delete methods if you're not using them.
  // Remove the corresponding declaration from EglyDriverTask.h too, if so.
}

void
EglyDriverTask::Halt()
{
  // Stop any threads or other asynchronous activity.
  // Good practice is to write the Halt() method such that it is safe to call it even *before*
  // Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
}

void EglyDriverTask::checkAudioPlayerErr() {
    switch (EglyDriverTask::wv_player->ErrorState())
    {
    case WavePlayer::noError:
        //bciout << "wavepalyer no error";
        break;
    case WavePlayer::fileOpeningError:
        bcierr << "waveplayer:the File property was set to a non-existent file, or a file that could not be loaded as an audio file.";
        break;
    case WavePlayer::featureNotSupported:
        bcierr << "waveplayer:a feature provided by one of the properties (such as panning) is not supported by the current implementation.";
        break;
    case WavePlayer::invalidParams:
        bcierr << "waveplayer:a property is out of range.";
        break;
    case WavePlayer::initError:
        bcierr << "waveplayer:there was an error when initializing an underlying library (such as DirectX Audio under Windows).";
        break;
    case WavePlayer::genError:
        bcierr << "waveplayer:an error occurred that does not fit into any of the remaining categories.";
        break;
    default:
        bcierr << "waveplayer:undefined error state";
        break;
    }
}

int EglyDriverTask::uniformRandomGenerator(int from, int to) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>  distr(from, to);

    return distr(generator);
}

int EglyDriverTask::distributionGenerator(std::vector<float> weights) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> distribt(weights.begin(), weights.end());
    return distribt(gen);
}

bool EglyDriverTask::checkKeyPush(std::string keyName, int desire_val) {
    for (unsigned int i = 0; i < Parameter("SampleBlockSize"); i++)
    {
        if (State(keyName)(i) == desire_val) {
            return true;
        }
    }
    return false;
}

bool EglyDriverTask::checkKeyPushIndex(std::string keyName, int desire_val, int& index) {
    for (unsigned int i = 0; i < Parameter("SampleBlockSize"); i++)
    {
        if (State(keyName)(i) == desire_val) {
            index = i;
            return true;
        }
    }
    index = -1;
    return false;
}

void EglyDriverTask::playSound(std::string file) {
    wv_player->SetFile(file);
    if (wv_player->IsPlaying())
        wv_player->Stop();
    wv_player->Play();
    checkAudioPlayerErr();
}

float EglyDriverTask::accuracyCalculator(std::deque<int> accuracy_window) {
    int window_sum = 0;
    float accuracy = 0.0;
 
    for (std::deque<int>::iterator it = accuracy_window.begin(); it != accuracy_window.end(); it++) {
        window_sum += *it;
    }
    accuracy = (float)window_sum / (float)accuracy_window.size();

    return accuracy;
}

void EglyDriverTask::adjustContrast(float accuracy) {
    RGBColor cur_color = target_rect->FillColor();
    int r = cur_color.R();
    int g = cur_color.G();
    int b = cur_color.B();

    if (accuracy - acc_low < MY_EPSILON) { // make target darker 
        r = r - STEP < 0 ? 0 : r - STEP;
        g = g - STEP < 0 ? 0 : g - STEP;
        b = b - STEP < 0 ? 0 : b - STEP;
        target_rect->SetFillColor(RGBColor(r, g, b));
    }
    else if (accuracy - acc_up > MY_EPSILON) {
        r = r + STEP > 255 ? 255 : r + STEP;
        g = g + STEP > 255 ? 255 : g + STEP;
        b = b + STEP > 255 ? 255 : b + STEP;

        int color_after = RGBColor(r, g, b);
        int color_bar = bar1_rect->FillColor();
        //AppLog << "color_bar" << color_bar << std::endl;

        if (color_after > color_bar) {
            target_rect->SetFillColor(bar1_rect->FillColor());
        }
        else {
            target_rect->SetFillColor(RGBColor(r, g, b));
        }
    }
}

/*
type == 1, be dark by 1 STEP
type == 2, be bright by 1 STEP

*/
void EglyDriverTask::adjustContrastSC(int type) {
    RGBColor cur_color = target_rect->FillColor();
    int r = cur_color.R();
    int g = cur_color.G();
    int b = cur_color.B();

    if (type == 1) { // make target darker 
        r = r - STEP < 0 ? 0 : r - STEP;
        g = g - STEP < 0 ? 0 : g - STEP;
        b = b - STEP < 0 ? 0 : b - STEP;
        target_rect->SetFillColor(RGBColor(r, g, b));
        //AppLog << "target darker" << std::endl;
    }
    else if (type == 2) {
        r = r + STEP > 255 ? 255 : r + STEP;
        g = g + STEP > 255 ? 255 : g + STEP;
        b = b + STEP > 255 ? 255 : b + STEP;

        int color_after = RGBColor(r, g, b);
        int color_bar = bar1_rect->FillColor();
        if (color_after > color_bar) {
            target_rect->SetFillColor(bar1_rect->FillColor());
        }
        else {
            target_rect->SetFillColor(RGBColor(r, g, b));
        }
    }
}

void EglyDriverTask::Patch(bool active)
{
    if (mPhotoDiodePatch.pShape) {
        if (active)
            mPhotoDiodePatch.pShape->SetColor(mPhotoDiodePatch.activeColor).SetFillColor(mPhotoDiodePatch.activeColor);
        else
            mPhotoDiodePatch.pShape->SetColor(mPhotoDiodePatch.inactiveColor).SetFillColor(mPhotoDiodePatch.inactiveColor);
    }
}

