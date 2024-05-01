////////////////////////////////////////////////////////////////////////////////
// Authors: 
// Description: ReactionTimeVisualPerceptionTask header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_REACTIONTIMEVISUALPERCEPTIONTASK_H  // makes sure this header is not included more than once
#define INCLUDED_REACTIONTIMEVISUALPERCEPTIONTASK_H

#include "ApplicationBase.h"
#include "TextStimulus.h"
#include "ImageStimulus.h"
#include "WavePlayer.h"
#include "BlockRandSeq.h"

enum e_state {e_first_trial, e_cue, e_pre_stimuli, e_stimuli_strong,e_isi_salient,e_stimuli_threshold,e_stimuli_catch, e_stimuli_catch_salient, e_feedback, e_isi, e_false_start, e_fixation_violated,e_force_calibration};
enum e_stimuli {salient_stim,threshold_stim,salient_no_threshold,no_threshold,stimulated_trial,stimulated_catch_strong,stimulated_catch_no_stim};
enum e_calibration {cal_black,cal_white,cal_finished};

class ReactionTimeVisualPerceptionTask : public ApplicationBase
{
public:
	ReactionTimeVisualPerceptionTask();
	~ReactionTimeVisualPerceptionTask();
	void Publish() override;
	void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
	void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
	void StartRun() override;
	void Process( const GenericSignal& Input, GenericSignal& Output ) override;
	void StopRun() override;
	void Halt() override;

private:
	bool DoForceCalibration();
	const GenericSignal *mp_input;
	bool VisualSensorCorrect(bool expected);
	void SetStimulationState (bool state);
	void ThresholdStimulusOn();
	void ThresholdStimulusOff();
	void FreeMemory();
	bool IsButtonPressed();
	void PrintSummary();
	int RandomNumber(int min, int max);
	float         m_push_button_threshold; 
	unsigned int m_cal_steps;
	bool wait_for_buttonpress;
	RandomGenerator m_random_generator;
	BlockRandSeq m_random_generator_orientation;
	BlockRandSeq m_random_block;
	ApplicationWindow& mrDisplay;
	GUI::Rect     m_image_rectangle;
	GUI::Rect     m_stim_rectangle;
	ImageStimulus* p_salientstimulus;
	ImageStimulus m_fixation;
	ImageStimulus m_IsIMask;
	ImageStimulus m_IsINoise;
	ImageStimulus m_SalientCatch;
	TextStimulus m_text_stimulus;
	std::vector<std::string> m_list_isi_picture;

	std::vector<ImageStimulus*> v_threshold_stimulus;
	std::vector<e_stimuli> v_stimulis;

	WavePlayer   *p_cue_sound;
	WavePlayer   *p_feedback_yes;
	WavePlayer   *p_feedback_no;
	WavePlayer   *p_feedback_estim_start;
	WavePlayer   *p_feedback_estim_stop;

	e_calibration m_calibration;
	e_state _currentState;
	e_stimuli _currStimuli;
	float         m_block_size_msec;
	unsigned int m_threshold_trials;
	unsigned int m_stim_threshold_trials;
	unsigned int m_salient_trials;
	unsigned int m_threshold_catch_trials;
	unsigned int m_salient_catch_trials;
	unsigned int m_stim_threshold_catch_trials;
	unsigned int m_stim_salient_catch_trials;
	unsigned int m_detected_trials;
	unsigned int m_stim_detected_trials;
	unsigned int m_stim_undetected_trials;
	unsigned int m_undetected_trials;
	unsigned int m_wrong_detect_trials;
	unsigned int m_wrong_undetect_trials;
	unsigned int m_stim_wrong_detect_trials;
	unsigned int m_stim_wrong_undetect_trials;
	unsigned int m_stim_visible_blocks;
	unsigned int m_SalientStimuliISIMax_blocks;
	unsigned int m_SalientStimuliISIMin_blocks;
	unsigned int m_SalientStimuliISI_blocks;
	unsigned int m_SalientStimuliDuration_blocks;

	unsigned int presented_nostim_catch;
	unsigned int presented_salient_catch;

	unsigned int m_visual_sensor;
	float m_visual_sensor_thresh;

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
	bool          m_use_space_button;
	bool          m_use_Digital_input;
	uint8_t		  m_orientation;

	// Use this space to declare any ReactionTimeVisualPerceptionTask-specific methods and member variables you'll need
};

#endif // INCLUDED_REACTIONTIMEVISUALPERCEPTIONTASK_H
