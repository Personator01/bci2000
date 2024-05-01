////////////////////////////////////////////////////////////////////////////////
// Authors: lingl@DESKTOP-GH0R7CA.wucon.wustl.edu
// Description: Communication_task_dualTask header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_COMMUNICATION_TASK_DUALTASK_H  // makes sure this header is not included more than once
#define INCLUDED_COMMUNICATION_TASK_DUALTASK_H

#include "ApplicationBase.h"
#include "Slider.h"
#include <map>
#include <vector>
#include <string>
#include "TextStimulus.h"
#include "ImageStimulus.h"
#include "Shapes.h"
#include "HyperscanningApplicationBase.h"

class Communication_task_dualTask : public HyperscanningApplicationBase
{
 public:
  Communication_task_dualTask();
  ~Communication_task_dualTask();
  void SharedPublish() override;
  void SharedPreflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void SharedInitialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void SharedAutoConfig( const SignalProperties& Input ) override;
  void SharedStartRun() override;
  void SharedProcess( const GenericSignal& Input, GenericSignal& Output ) override;
  void SharedStopRun() override;
  void SharedHalt() override;

 private:
   //enum
	 enum Phase{
		 instruction,
		 wait_to_begin,
		 role_text,
		 image_present,
		 //sender_adjust,
		 //sender_lock_answer,
		 //receiver_adjust,
		 //receiver_lock_answer,
		 feedback,
		 block_break,
		 end,
		 none
   };
   //GUI
   ApplicationWindow& mrDisplay;
   ImageStimulus* m_img_container = nullptr, *m_instruction_container = nullptr;
   TextStimulus *m_dim_1a_txt = nullptr, * m_dim_1b_txt = nullptr, * m_dim_2a_txt = nullptr, * m_dim_2b_txt = nullptr;
   TextStimulus *m_opt_1_txt = nullptr, *m_opt_2_txt = nullptr;
   ImageStimulus* m_feedback_container = nullptr;
   TextStimulus* m_role_txt = nullptr;
   Slider *my_slider_1 = nullptr, *my_slider_2 = nullptr;

   // Photo diode patch
   struct {
	   Shape* pShape = nullptr;
	   RGBColor activeColor, inactiveColor;
   } mPhotoDiodePatch;

   //random sequece
   std::vector<int> random_sequence;

   //others
   std::vector<std::string> img_recv_v, img_send_v, dimt_1a_v, dimt_1b_v, dimt_2a_v, dimt_2b_v,
	   opt_1_v, opt_2_v;
   std::vector<int> sender_v, jitter_v, cor_rsps_v;
   std::vector<std::string> instructs_list;
   std::vector<int> break_trial_list;
   unsigned int  m_sample_rate, m_block_size;
   unsigned int feedback_dur_in_block, role_dur_in_block;
   unsigned int m_block_in_phase, m_current_trial_num;
   Phase my_phase = none;
   int instructs_idx;
   int client_number; // 1: needed to reverse; 0: keep the sender list
   const float slider_step = 0.05;
   const float multi_step = 100.0;
   int m_pre_button_state_joybt1 = 0, m_pre_button_state_joybt2 = 0, m_pre_button_state_joybt3 = 0,
	   m_pre_button_state_joybt4 = 0, m_pre_button_state_joybt9 = 0, m_pre_button_state_joybt10 = 0;
   float m_pre_axe_v_1 = 0.0, m_pre_axe_v_2 = 0.0;
   int trial_num;
   int pre_opt = 0;
   //test
   bool is_reverse_sender;


   //method
   void MoveSliderAxes(float step);//sender
   void UpdateSliderAxes();//receiver
   void MoveOptions();//sender
   void UpdateOptions();//receiver
   bool CheckKeyBoardPress(std::string s, int value);
   bool CheckLongEvent(std::string s, int value, int& m_pre_button_state);
   void Patch(bool active);//photodiode
};

#endif // INCLUDED_COMMUNICATION_TASK_DUALTASK_H
