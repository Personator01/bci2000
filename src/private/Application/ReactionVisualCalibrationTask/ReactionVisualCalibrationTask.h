////////////////////////////////////////////////////////////////////////////////
// Authors: 
// Description: ReactionVisualCalibrationTask header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_REACTIONVISUALCALIBRATIONTASK_H  // makes sure this header is not included more than once
#define INCLUDED_REACTIONVISUALCALIBRATIONTASK_H

#include "ApplicationBase.h"
#include <string>
#include "TextStimulus.h"
#include "ImageStimulus.h"
#include "WavePlayer.h"
#include "Shapes.h"
#include "BlockRandSeq.h"

enum e_state {e_first_trial,e_initial_staircase,e_wait_button, e_cue, e_pre_stimuli,  e_stimuli, e_feedback, e_isi, e_prePEST, e_PEST, e_false_start, e_fixation_violated, e_staircase,e_force_calibration,e_wait_paradigm};
enum e_calibration {cal_black,cal_white,cal_finished};
class ReactionVisualCalibrationTask : public ApplicationBase
{
 public:
  ReactionVisualCalibrationTask();
  ~ReactionVisualCalibrationTask();
  void Publish() override;
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;
  void Halt() override;


 private:
	 bool VisualSensorCorrect(bool expected);
	 bool DoForceCalibration();
	void clearVisualStimuliMap();
	unsigned int CalculateStimulusIndex(float);
	void PrepareStimulus(float attenuation);
    void StimulusOn();
    void StimulusOff();
    //void CueImageOn();
	//void CueImageOff();
    void SummarizeExperiment();
    float ReactionTime();
    bool IsButtonPressed();
    unsigned int m_pushed_botton_flag;
    int RandomNumber(int min, int max);
	
    RandomGenerator m_random_generator;
	BlockRandSeq m_random_generator_orientation;
   ApplicationWindow& mrDisplay;
   bool wait_for_buttonpress;
   e_state m_current_state;
   // Use this space to declare any APPWINDOW-specific methods and member variables you'll need
    int m_visual_sensor;
	int m_visual_sensor_thresh;
    int m_num_stimuli;
	unsigned int m_cal_steps;
	e_calibration m_calibration;
    float         m_attenuation_min;
    float         m_attenuation_max;
    float         m_attenuation_stepping;
    float         m_PestStartLevel;           // starting value for intensity                 
	float         m_PestFinalStepSize;        // smallest value for intensity change     
	float         m_PestWaldFactor;           // number of missed trials x target likelihood without intensity change     
	float         m_PestTargetPerformance;    // target performance ratio
    float         m_PestStepsize;
    float         m_PestStartStepSize;
    float		  m_visual_intensity;
    float         m_PestMaxstepsize;
    float         m_PestMinlevel;
    float         m_min_rt; 
    float         m_max_rt;  
    float         m_block_size_msec;
    float         m_push_button_threshold; 
    float         m_copy_stimulus_level;

	bool m_overrideCalib;
	std::vector<int> m_staircase_steps;
	unsigned int m_stim_visible_blocks;
	unsigned int  m_runtime_counter;
    unsigned int  m_num_trials;
	unsigned int  m_trial_counter;
    unsigned int  m_block_delta;
    unsigned int  m_block_size;
    unsigned int  m_cue_duration_blocks;
    unsigned int  m_isi_duration_blocks;
    unsigned int  m_stimuli_duration_blocks;
    unsigned int  m_feedback_blocks;
    unsigned int  m_sample_rate;
    unsigned int  m_push_button_channel; 
    unsigned int  m_rand_blocks;
    unsigned int  m_wait_min_blocks;
	unsigned int  m_wait_max_blocks;
    unsigned int  m_Pest_num_correct;
    unsigned int  m_historylength;
    unsigned int  m_Pest_doubled;
    unsigned int m_Pest_samelevelcounter;
    unsigned int m_Pesttrialcounter;
    unsigned int m_max_num_trials;
    bool          m_is_first_trial;
    bool          m_do_pre_cue;
    bool          m_use_space_button;
    bool		  m_present_marker;
    bool          m_use_Digital_input;
	bool		  m_use_pest;
	uint8_t		  m_orientation;
       
    GUI::Rect     m_cue_image_rectangle;
    GUI::Rect     m_marker_rectangle;
    GUI::Rect     m_full_rectangle;


	BlockRandSeq m_random_block;

	std::vector<int> m_stimuliIdx;
    TextStimulus  mp_text_stimulus;
	ImageStimulus mp_fixation;
	ImageStimulus* mp_stim;
	ImageStimulus* m_FeedbackStimulus;
	ImageStimulus m_IsINoise;
	WavePlayer   *cue_sound;
	WavePlayer   *feedback_yes;
	WavePlayer   *feedback_no;

	const GenericSignal *mp_input;
	std::vector<std::vector<ImageStimulus*> > m_list_visual_stimuli;
	std::vector<ImageStimulus*> m_list_isi_picture;
    std::vector<float> m_reaction_time;
    std::vector<float> m_stimulus_Level;
    std::vector<float> m_stimulus_Level_copy;
    std::vector<float> m_Pest_percentage_check;
    std::vector<float> m_PEST_performance;
    std::vector<float> m_list_attenuation; 
    std::string         m_Pest_history_correct;
    std::string         a;
    std::string         b;
    std::string         m_text_feedback_pre_cue;
    std::stringstream   m_text_stimulus_string;
    std::string         m_text_feedback_false_start;



   // Use this space to declare any ReactionVisualCalibrationTask-specific methods and member variables you'll need
};

#endif // INCLUDED_REACTIONVISUALCALIBRATIONTASK_H
