////////////////////////////////////////////////////////////////////////////////
// Authors: lingl@DESKTOP-GH0R7CA.wucon.wustl.edu
// Description: EglyDriverTask header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_EGLYDRIVERTASK_H  // makes sure this header is not included more than once
#define INCLUDED_EGLYDRIVERTASK_H

#include "ApplicationBase.h"
#include "Shapes.h"
#include "WavePlayer.h"
#include "TextStimulus.h"
#include <deque>
#include "ProgressBarVis.h"

class ProgressBarVis;

class EglyDriverTask : public ApplicationBase
{
 public:
  EglyDriverTask();
  ~EglyDriverTask();
  void Publish() override;
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;
  void Halt() override;

 private:
	 enum Phase {
		 none,
		 wait_to_begin,
		 fixation,
		 bar,
		 cue,
		 cue_to_target,
		 target,
		 post_target_response,
		 feedback,
		 instruction
	 };

	 //GUI
	 ApplicationWindow& mrDisplay;
	 RectangularShape* bar1_rect;
	 RectangularShape* bar2_rect;
	 RectangularShape* fixation_rect;
	 RectangularShape* target_rect;
	 RectangularShape* cue_rect;
	 GUI::Rect fixation_r, bar1_r_h, bar2_r_h, bar1_r_v, bar2_r_v, cue_h_1, cue_h_2, cue_h_3, cue_h_4, cue_v_1, cue_v_2, cue_v_3, cue_v_4, 
		 target_h_1, target_h_2, target_h_3, target_h_4, target_v_1, target_v_2, target_v_3, target_v_4;
	 WavePlayer* wv_player;
	 TextStimulus* instruction_txt;
	 double aspectRatio;
	 std::vector<GUI::Rect> target_r_h;
	 std::vector<GUI::Rect> target_r_v;
	 std::vector<GUI::Rect> cue_r_h;
	 std::vector<GUI::Rect> cue_r_v;

	 //GUI simplified
	 GUI::Rect squear_r_1, squear_r_2, squear_r_3, squear_r_4, sCue_r_1, sCue_r_2, sCue_r_3, sCue_r_4;	 

	 //simplified variouble
	 bool is_simplified = false;
	 std::vector<GUI::Rect> sCue_container;
	 std::vector<GUI::Rect> sTarget_container;

	 //staircase variouble
	 bool is_staircase = false;
	 int down_counts = 0, up_counts = 0;
	 int overall_up_counts = 0, overall_down_counts = 0;
	 int match_up_counts = 0, match_down_counts = 0;
	 int non_match_up_counts = 0, non_match_down_counts = 0;
	 int pre_response = -1;
	 int target_color = 0;

	 //interval time
	 unsigned int fixation_min, fixation_max, bar_min, bar_max, cue_dur, target_dur, cue_to_target_min, cue_to_target_max, pst_target_rsp_dur, feedback_dur;
	 unsigned int fixation_dur, bar_dur, cue_to_target_dur;
	
	 //sequence
	 Phase my_phase = Phase::none;
	 int m_block_in_phase, m_cur_trial;

	 //trial types
	 int bar_direct_min = 1, bar_direct_max = 2; // 1: horizontal, 2: vertical | 1:top, 2:right 3: bottom, 4:left
	 int cue_position_min = 1, cue_position_max = 4; // 1: top_left, 2: top_right, 3: bottom_right, 4: bottom_left
	 unsigned int bar_direct, cue_position;
	 unsigned int trial_type;	// 1: no target, catch trial(10%)
								// 2: cue target match(72%)
								// 3: cue target don't match, on the same object(9%) 
								// 4: cue target don't match,  on the different object(9%)

	 std::vector<float> percentage;
	 int response;

	 //dynamically adjust the illuminance of the target, every 15 trials
	 std::deque<int> accuracy_overall_window, accuracy_match_window, accuracy_non_match_window, false_detation_rate_window;
	 int WINDOW_SIZE = 15;
	 float acc_up = 0.81;
	 float acc_low = 0.79;
	 const float MY_EPSILON = 0.001;
	 const double STEP = 1;

	 //path
	 const std::string BEGIN_SOUND = "../tasks/EglyDriverTask/t0500Hz.wav";
	 const std::string CORRECT_SOUND = "../tasks/EglyDriverTask/ding.wav";
	 const std::string INCORRECT_SOUND = "../tasks/EglyDriverTask/dQ0.wav";

	 //GUI scaling
	 double visual_range_vertical;
	 double visual_range_horizontal;

	 // Photo diode patch
	 struct {
		 Shape* pShape = nullptr;
		 RGBColor activeColor, inactiveColor;
	 } mPhotoDiodePatch;
	 // Access to the Display property.
	 GUI::GraphDisplay& Display()
	 {
		 return mrDisplay;
	 }
	 const GUI::GraphDisplay& Display() const
	 {
		 return mrDisplay;
	 }

	 const int BLOCK_NUM_PHOTODIODE = 5;
	 //reaction time
	 float mOneBlockMs;
	 int r_index = -1;
	 //progress bar
	 ProgressBarVis* mpProgressBar = nullptr;

	 void checkAudioPlayerErr();
	 int uniformRandomGenerator(int from, int to);
	 int distributionGenerator(std::vector<float> weights);
	 bool checkKeyPush(std::string keyName, int desire_val);
	 void playSound(std::string file);
	 float accuracyCalculator(std::deque<int> accuracy_window);
	 void adjustContrast(float accuracy);
	 void adjustContrastSC(int type);
	 void Patch(bool active);
	 bool checkKeyPushIndex(std::string keyName, int desire_val, int& index);
};

#endif // INCLUDED_EGLYDRIVERTASK_H
