////////////////////////////////////////////////////////////////////////////////
// Authors:
// Description: ReactionTimeVisualPerceptionTask implementation
////////////////////////////////////////////////////////////////////////////////

#include "ReactionTimeVisualPerceptionTask.h"
#include "BCIStream.h"
#include <Windows.h>
#define SENS_CALIB_DUR 10

#define __nulldelete(p)\
	if(p != NULL) { delete(p);p = NULL;}

#define __initsound(p,x)\
	{\
	if(!string(Parameter(x)).empty())\
		{\
		p=new WavePlayer();\
		p->SetFile(Parameter(x));\
		}\
	else {p = NULL;} }\

using namespace std;


RegisterFilter (ReactionTimeVisualPerceptionTask, 3);
// Change the location if appropriate, to determine where your filter gets
// sorted into the chain. By convention:
//  - filter locations within SignalSource modules begin with "1."
//  - filter locations within SignalProcessing modules begin with "2."
//       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
//  - filter locations within Application modules begin with "3."


ReactionTimeVisualPerceptionTask::ReactionTimeVisualPerceptionTask () :
	mrDisplay (Window ()), m_fixation (mrDisplay), m_text_stimulus (mrDisplay), m_random_generator (), m_random_block (m_random_generator),
	m_IsIMask (mrDisplay), m_IsINoise (mrDisplay), m_SalientCatch(mrDisplay), m_random_generator_orientation (m_random_generator), p_feedback_estim_start (NULL), p_feedback_estim_stop (NULL)//init m_random_generator before block
{
	BEGIN_PARAMETER_DEFINITIONS
		"Application:ReactionTimeTask matrix PestStimuli= "
		"{1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 }"
		"{ intensity filename_0 filename_45 filename_90 filename_135 filename_225 filename_270 filename_315 }"
		"1 ..\\tasks\\VisGrating\\1_0.bmp  ..\\tasks\\VisGrating\\1_45.bmp  ..\\tasks\\VisGrating\\1_90.bmp  ..\\tasks\\VisGrating\\1_135.bmp  ..\\tasks\\VisGrating\\1_225.bmp  ..\\tasks\\VisGrating\\1_270.bmp  ..\\tasks\\VisGrating\\1_315.bmp "
		"2 ..\\tasks\\VisGrating\\2_0.bmp  ..\\tasks\\VisGrating\\2_45.bmp  ..\\tasks\\VisGrating\\2_90.bmp  ..\\tasks\\VisGrating\\2_135.bmp  ..\\tasks\\VisGrating\\2_225.bmp  ..\\tasks\\VisGrating\\2_270.bmp  ..\\tasks\\VisGrating\\2_315.bmp "
		"3 ..\\tasks\\VisGrating\\3_0.bmp  ..\\tasks\\VisGrating\\3_45.bmp  ..\\tasks\\VisGrating\\3_90.bmp  ..\\tasks\\VisGrating\\3_135.bmp  ..\\tasks\\VisGrating\\3_225.bmp  ..\\tasks\\VisGrating\\3_270.bmp  ..\\tasks\\VisGrating\\3_315.bmp "
		"4 ..\\tasks\\VisGrating\\4_0.bmp  ..\\tasks\\VisGrating\\4_45.bmp  ..\\tasks\\VisGrating\\4_90.bmp  ..\\tasks\\VisGrating\\4_135.bmp  ..\\tasks\\VisGrating\\4_225.bmp  ..\\tasks\\VisGrating\\4_270.bmp  ..\\tasks\\VisGrating\\4_315.bmp "
		"5 ..\\tasks\\VisGrating\\5_0.bmp  ..\\tasks\\VisGrating\\5_45.bmp  ..\\tasks\\VisGrating\\5_90.bmp  ..\\tasks\\VisGrating\\5_135.bmp  ..\\tasks\\VisGrating\\5_225.bmp  ..\\tasks\\VisGrating\\5_270.bmp  ..\\tasks\\VisGrating\\5_315.bmp "
		"6 ..\\tasks\\VisGrating\\6_0.bmp  ..\\tasks\\VisGrating\\6_45.bmp  ..\\tasks\\VisGrating\\6_90.bmp  ..\\tasks\\VisGrating\\6_135.bmp  ..\\tasks\\VisGrating\\6_225.bmp  ..\\tasks\\VisGrating\\6_270.bmp  ..\\tasks\\VisGrating\\6_315.bmp "
		"7 ..\\tasks\\VisGrating\\7_0.bmp  ..\\tasks\\VisGrating\\7_45.bmp  ..\\tasks\\VisGrating\\7_90.bmp  ..\\tasks\\VisGrating\\7_135.bmp  ..\\tasks\\VisGrating\\7_225.bmp  ..\\tasks\\VisGrating\\7_270.bmp  ..\\tasks\\VisGrating\\7_315.bmp "
		"8 ..\\tasks\\VisGrating\\8_0.bmp  ..\\tasks\\VisGrating\\8_45.bmp  ..\\tasks\\VisGrating\\8_90.bmp  ..\\tasks\\VisGrating\\8_135.bmp  ..\\tasks\\VisGrating\\8_225.bmp  ..\\tasks\\VisGrating\\8_270.bmp  ..\\tasks\\VisGrating\\8_315.bmp "
		"9 ..\\tasks\\VisGrating\\9_0.bmp  ..\\tasks\\VisGrating\\9_45.bmp  ..\\tasks\\VisGrating\\9_90.bmp  ..\\tasks\\VisGrating\\9_135.bmp  ..\\tasks\\VisGrating\\9_225.bmp  ..\\tasks\\VisGrating\\9_270.bmp  ..\\tasks\\VisGrating\\9_315.bmp "
		"10 ..\\tasks\\VisGrating\\10_0.bmp  ..\\tasks\\VisGrating\\10_45.bmp  ..\\tasks\\VisGrating\\10_90.bmp  ..\\tasks\\VisGrating\\10_135.bmp  ..\\tasks\\VisGrating\\10_225.bmp  ..\\tasks\\VisGrating\\10_270.bmp  ..\\tasks\\VisGrating\\10_315.bmp "
		"11 ..\\tasks\\VisGrating\\11_0.bmp  ..\\tasks\\VisGrating\\11_45.bmp  ..\\tasks\\VisGrating\\11_90.bmp  ..\\tasks\\VisGrating\\11_135.bmp  ..\\tasks\\VisGrating\\11_225.bmp  ..\\tasks\\VisGrating\\11_270.bmp  ..\\tasks\\VisGrating\\11_315.bmp "
		"12 ..\\tasks\\VisGrating\\12_0.bmp  ..\\tasks\\VisGrating\\12_45.bmp  ..\\tasks\\VisGrating\\12_90.bmp  ..\\tasks\\VisGrating\\12_135.bmp  ..\\tasks\\VisGrating\\12_225.bmp  ..\\tasks\\VisGrating\\12_270.bmp  ..\\tasks\\VisGrating\\12_315.bmp "
		"13 ..\\tasks\\VisGrating\\13_0.bmp  ..\\tasks\\VisGrating\\13_45.bmp  ..\\tasks\\VisGrating\\13_90.bmp  ..\\tasks\\VisGrating\\13_135.bmp  ..\\tasks\\VisGrating\\13_225.bmp  ..\\tasks\\VisGrating\\13_270.bmp  ..\\tasks\\VisGrating\\13_315.bmp "
		"14 ..\\tasks\\VisGrating\\14_0.bmp  ..\\tasks\\VisGrating\\14_45.bmp  ..\\tasks\\VisGrating\\14_90.bmp  ..\\tasks\\VisGrating\\14_135.bmp  ..\\tasks\\VisGrating\\14_225.bmp  ..\\tasks\\VisGrating\\14_270.bmp  ..\\tasks\\VisGrating\\14_315.bmp "
		"15 ..\\tasks\\VisGrating\\15_0.bmp  ..\\tasks\\VisGrating\\15_45.bmp  ..\\tasks\\VisGrating\\15_90.bmp  ..\\tasks\\VisGrating\\15_135.bmp  ..\\tasks\\VisGrating\\15_225.bmp  ..\\tasks\\VisGrating\\15_270.bmp  ..\\tasks\\VisGrating\\15_315.bmp "
		"16 ..\\tasks\\VisGrating\\16_0.bmp  ..\\tasks\\VisGrating\\16_45.bmp  ..\\tasks\\VisGrating\\16_90.bmp  ..\\tasks\\VisGrating\\16_135.bmp  ..\\tasks\\VisGrating\\16_225.bmp  ..\\tasks\\VisGrating\\16_270.bmp  ..\\tasks\\VisGrating\\16_315.bmp "
		"17 ..\\tasks\\VisGrating\\17_0.bmp  ..\\tasks\\VisGrating\\17_45.bmp  ..\\tasks\\VisGrating\\17_90.bmp  ..\\tasks\\VisGrating\\17_135.bmp  ..\\tasks\\VisGrating\\17_225.bmp  ..\\tasks\\VisGrating\\17_270.bmp  ..\\tasks\\VisGrating\\17_315.bmp "
		"18 ..\\tasks\\VisGrating\\18_0.bmp  ..\\tasks\\VisGrating\\18_45.bmp  ..\\tasks\\VisGrating\\18_90.bmp  ..\\tasks\\VisGrating\\18_135.bmp  ..\\tasks\\VisGrating\\18_225.bmp  ..\\tasks\\VisGrating\\18_270.bmp  ..\\tasks\\VisGrating\\18_315.bmp "
		"19 ..\\tasks\\VisGrating\\19_0.bmp  ..\\tasks\\VisGrating\\19_45.bmp  ..\\tasks\\VisGrating\\19_90.bmp  ..\\tasks\\VisGrating\\19_135.bmp  ..\\tasks\\VisGrating\\19_225.bmp  ..\\tasks\\VisGrating\\19_270.bmp  ..\\tasks\\VisGrating\\19_315.bmp "
		"20 ..\\tasks\\VisGrating\\20_0.bmp  ..\\tasks\\VisGrating\\20_45.bmp  ..\\tasks\\VisGrating\\20_90.bmp  ..\\tasks\\VisGrating\\20_135.bmp  ..\\tasks\\VisGrating\\20_225.bmp  ..\\tasks\\VisGrating\\20_270.bmp  ..\\tasks\\VisGrating\\20_315.bmp "
		" // experiment definition ",
		"Application:ReactionTimeTask matrix IsIPicture= "
		"{1}"
		"{filename_fixation filename_0 filename_45 filename_90 filename_135 filename_225 filename_270 filename_315}"
		" ..\\tasks\\VisGratingNegative\\fixation.png ..\\tasks\\VisGratingNegative\\0.png  ..\\tasks\\VisGratingNegative\\45.png  ..\\tasks\\VisGratingNegative\\90.png  ..\\tasks\\VisGratingNegative\\135.png  ..\\tasks\\VisGratingNegative\\225.png  ..\\tasks\\VisGratingNegative\\270.png  ..\\tasks\\VisGratingNegative\\315.png ",

		"Application:ReactionTimeTask string NoisePicture= ..\\tasks\\VisGrating\\noise.bmp",
		"Application:ReactionTimeTask string FixationPicture= ..\\tasks\\VisGrating\\fixation.bmp ",
		"Application:ReactionTimeTask string AuditoryCue= .\\sounds\\tick.wav ",
		"Application:ReactionTimeTask string AuditoryFeedbackYes= .\\sounds\\yes.wav",
		"Application:ReactionTimeTask string AuditoryFeedbackNo= .\\sounds\\no.wav",
		"Application:ReactionTimeTask int UseSpaceButton= 1 1 0 1 // enable keyboard space button ? (boolean)",
		"Application:ReactionTimeTask int UseDigitalInputButton= 0 1 0 1 // enable Digital input button ? (boolean)",
		"Application:ReactionTimeTask int PushButtonChannel= 2 2 1 % // channel number of push button",
		"Application:ReactionTimeTask int VisualSensorChannel= 1 2 1 % // channel number of Visual Sensor",
		"Application:ReactionTimeTask int PushButtonThreshold= 1e5 1000 0 % // channel number of push button",
		"Application:ReactionTimeTask int VisualSensorThreshold= 1e5 1000 0 % // channel number of push button",
		"Application:ReactionTimeTask int OverrideAutocalibration= 0 0 0 1 // ignore recalibration  (boolean)",
		END_PARAMETER_DEFINITIONS

		BEGIN_PARAMETER_DEFINITIONS
		"Application:ReactionTimeTask string SoundOnStimulationStart= .\\sounds\\yes.wav // Sound to be played when electrical stimulation starts ",
		"Application:ReactionTimeTask string SoundOnStimulationEnd= .\\sounds\\no.wav // Sound to be played when electrical stimulation ends ",
		END_PARAMETER_DEFINITIONS

		BEGIN_PARAMETER_DEFINITIONS
		"Application:ReactionTimeTask int PerceptionThreshold= 5 5  0 10000 % // final performance ratio",
		"Application:ReactionTimeTask int ThresholdTrials= 100 100  0 1000 % // trials with threshold stimuli",
		"Application:ReactionTimeTask int StimulatedThresholdTrials= 100 100  0 1000 % // trials with threshold stimuli",
		"Application:ReactionTimeTask int SalientTrials= 0 0  0 1000 % // trials with salient stimlui",
		"Application:ReactionTimeTask int ThresholdCatchFraction= 0.05 0.2  0 1 % // final performance ratio",
		"Application:ReactionTimeTask int StimulatedThresholdCatchFraction= 0.05 0.2  0 1 % // final performance ratio",
		"Application:ReactionTimeTask string SalientStimulus= ",
		END_PARAMETER_DEFINITIONS

		BEGIN_PARAMETER_DEFINITIONS
		// Timing   
		"Application:ReactionTimeTask float StimulusVisible= 2000ms 1000ms 0 % // amout of time the threshold stim is visible",
		"Application:ReactionTimeTask float CueDuration= 375ms 1000ms 0 % // cue duration",
		"Application:ReactionTimeTask float ISIDuration= 1000ms 1000ms 0 % // IsI duration",
		"Application:ReactionTimeTask float SalientStimuliDuration= 375ms 300ms 0 % // salient stimuli duration",
		"Application:ReactionTimeTask float SalientStimuliISIMin= 375ms 300ms 0 % // min duration between salient and thresh stim",
		"Application:ReactionTimeTask float SalientStimuliISIMax= 500ms 300ms 0 % // max duration between salient and thresh stim",
		"Application:ReactionTimeTask float StimuliDuration= 2000ms 4000ms 0 % // stimuli duration",
		"Application:ReactionTimeTask float FeedbackDuration= 500ms 1000ms 0 % // feedback duration",
		"Application:ReactionTimeTask float WaitMin= 1s 1s 0 % // min time to present stimulus",
		"Application:ReactionTimeTask float WaitMax= 3s 3s 0 % // max time to present stimulus",
		END_PARAMETER_DEFINITIONS

		BEGIN_STATE_DEFINITIONS
		"CurrentTrial		8  0 0 0", // The Current Trial
		"ExperimentState	4  0 0 0", //
		"CueVisual			1  0 0 0", //
		"StimVisual			1  0 0 0", //
		"StimType			3  0 0 0",
		"StimOrientation	3  0 0 0",
		"EStimulation		1  0 0 0",
		END_STATE_DEFINITIONS


		p_salientstimulus = NULL;
	p_cue_sound = NULL;
	p_feedback_no = NULL;
	p_feedback_yes = NULL;
	GUI::Rect m_full_rectangle;
	m_full_rectangle.left = 0.0f;
	m_full_rectangle.right = 1.0f;
	m_full_rectangle.top = 0.2f;
	m_full_rectangle.bottom = 0.8f;

	m_text_stimulus.SetText ("");
	m_text_stimulus.SetTextHeight (0.1f);
	m_text_stimulus.SetZOrder (-1.0);
	m_text_stimulus.SetDisplayRect (m_full_rectangle);
	m_text_stimulus.SetColor (RGBColor::Black);
	m_text_stimulus.SetTextColor (RGBColor::Green);
	m_text_stimulus.Present ();
	m_text_stimulus.Conceal ();


}

ReactionTimeVisualPerceptionTask::~ReactionTimeVisualPerceptionTask ()
{
	Halt ();
	FreeMemory ();
}

void
ReactionTimeVisualPerceptionTask::Publish ()
{

}

void
ReactionTimeVisualPerceptionTask::Preflight (const SignalProperties& Input, SignalProperties& Output) const
{
	Output = Input;

	if (Parameter ("UseDigitalInputButton") == 1)
	{
		State ("DigitalInput1");
		State ("DigitalInput2");
		State ("DigitalInput3");
		State ("DigitalInput4");
		State ("EStimulation");
	}
	else
	{
		OptionalState ("DigitalInput1");
		OptionalState ("DigitalInput2");
		OptionalState ("DigitalInput3");
		OptionalState ("DigitalInput4");
	}

	if (Parameter ("UseSpaceButton"))
		State ("KeyDown");
	else
		OptionalState ("KeyDown");
	// create pointer to parameters
	ParamRef AuditoryCue = Parameter ("AuditoryCue");
	ParamRef PestStimuli = Parameter ("PestStimuli");
	OptionalState ("GazeCorrectionMode");
	// touching states

	State ("Running");
	State ("CurrentTrial");
	State ("ExperimentState");
	State ("CueVisual");
	State ("StimVisual");
	State ("StimType");
	State ("StimOrientation");
	// touching parameters

	Parameter ("SoundOnStimulationStart");
	Parameter ("SoundOnStimulationEnd");

	Parameter ("OverrideAutocalibration");
	Parameter ("VisualSensorChannel");
	Parameter ("VisualSensorThreshold");
	Parameter ("PushButtonThreshold");
	Parameter ("StimulusVisible");
	Parameter ("CueDuration");
	Parameter ("StimuliDuration");
	Parameter ("SampleBlockSize");
	Parameter ("SamplingRate");
	Parameter ("PushButtonChannel");
	Parameter ("WaitMin");
	Parameter ("WaitMax");
	Parameter ("ISIDuration");
	Parameter ("PerceptionThreshold");
	//Parameter("TextFeedbackPreCue");
	Parameter ("ThresholdTrials");
	Parameter ("SalientTrials");
	Parameter ("ThresholdCatchFraction");
	Parameter ("SalientStimulus");

	OptionalState ("FixationViolated");
	Parameter ("IsIPicture");



	if (Parameter ("SalientTrials") != 0 && string (Parameter ("SalientStimulus")).empty ())
		bcierr << "No Salient stimuli defined!" << endl;

	if (string (Parameter ("NoisePicture")).empty ())
	{
		bciwarn << "No noise picture assigned!" << endl;
	}

	if (string (Parameter ("FixationPicture")).empty ())
	{
		bcierr << "No fixation picture assigned!" << endl;
	}

	if (string (Parameter ("AuditoryFeedbackYes")).empty ())
	{
		bciwarn << "No auditory Yes Feedback assigned!" << endl;
	}

	if (string (Parameter ("AuditoryFeedbackNo")).empty ())
	{
		bciwarn << "No auditory No Feedback assigned!" << endl;
	}

	if (string (AuditoryCue).empty ())
	{
		bciwarn << "No auditory Cue assigned!" << endl;
	}

	if (Parameter ("SalientStimuliISIMin").InMilliseconds () >= Parameter ("SalientStimuliISIMax").InMilliseconds ())
		bcierr << "Parameter SalientStimuliISIMin (" << Parameter ("SalientStimuliISIMin") << ") must be smaller than SalientStimuliISIMax (" << Parameter ("SalientStimuliISIMax") << ")." << endl;
	// check min reaction time is smaller than max reaction time


	// check that WaitMin is smaller than WaitMax
	if (Parameter ("WaitMin").InMilliseconds () > Parameter ("WaitMax").InMilliseconds ())
		bcierr << "Parameter WaitMin (" << Parameter ("WaitMax") << ") must be smaller or equal to WaitMax (" << Parameter ("WaitMax") << ")." << endl;

	double block_size_msec = (double)(Parameter ("SampleBlockSize")) / (double)(Parameter ("SamplingRate").InHertz ()) * 1000;

	// check that WaitMin is a multiple of the sample block time
	if (fmod ((double)Parameter ("WaitMin").InMilliseconds (), block_size_msec) > 1e-6)
		bcierr << "Parameter WaitMin (" << Parameter ("WaitMin") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	// check that ISIDuration is a multiple of the sample block time
	if (fmod ((double)Parameter ("ISIDuration").InMilliseconds (), block_size_msec) > 1e-6)
		bcierr << "Parameter ISIDuration (" << Parameter ("ISIDuration") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	if (fmod ((double)Parameter ("SalientStimuliISIMax").InMilliseconds (), block_size_msec) > 1e-6)
		bcierr << "Parameter SalientStimuliISIMax (" << Parameter ("SalientStimuliISIMax") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	// check that ISIDuration is a multiple of the sample block time
	if (fmod ((double)Parameter ("SalientStimuliISIMin").InMilliseconds (), block_size_msec) > 1e-6)
		bcierr << "Parameter SalientStimuliISIMin (" << Parameter ("SalientStimuliISIMin") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	// check that ISIDuration is a multiple of the sample block time
	if (fmod ((double)Parameter ("SalientStimuliDuration").InMilliseconds (), block_size_msec) > 1e-6)
		bcierr << "Parameter SalientStimuliDuration (" << Parameter ("SalientStimuliDuration") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;


	// Check that the PestStimuli matrix has at least 2 columns
	if (Parameter ("PestStimuli")->NumColumns () < 2)
		bcierr << "Parameter PestStimuli has " << Parameter ("PestStimuli")->NumColumns () << " columns, but must have at least 2 columns." << endl;

	if (fmod ((double)Parameter ("StimulusVisible").InMilliseconds (), block_size_msec) > 1e-6)
		bcierr << "Parameter StimulusVisible (" << Parameter ("StimulusVisible") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	// check that cueDuration is a multiple of the sample block time
	if (fmod ((double)Parameter ("CueDuration").InMilliseconds (), block_size_msec) > 1e-6)
		bcierr << "Parameter CueDuration (" << Parameter ("CueDuration") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	// check that FeedbackDuration is a multiple of the sample block time
	if (fmod ((double)Parameter ("FeedbackDuration").InMilliseconds (), block_size_msec) > 1e-6)
		bcierr << "Parameter FeedbackDuration (" << Parameter ("FeedbackDuration") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	int num_stimuli = PestStimuli->NumRows (); // get the total number of different intensities

	// vector of all the visual stimulus intensity values
	std::vector<int> list_attenuation;
	list_attenuation.clear ();

	for (unsigned int row = 0; row < num_stimuli; row++)
		list_attenuation.push_back (PestStimuli (row, 0));
	int threshold = Parameter ("PerceptionThreshold");
	if (std::find (list_attenuation.begin (), list_attenuation.end (), threshold) == list_attenuation.end ())
		bcierr << "Chosen threshold stimuli is not available in provided stimuli!" << endl;
	if (Parameter ("FeedbackDuration") != 0)
		if (Parameter ("IsIPicture")->NumColumns () != (PestStimuli->NumColumns ()))
		{
			bcierr << "Number of Interstim Pictures must match Number of possible stimuli pictures +1 for a certain stimuli" << endl;
		}

}


void
ReactionTimeVisualPerceptionTask::Initialize (const SignalProperties& Input, const SignalProperties& Output)
{
	ParamRef PestStimuli = Parameter ("PestStimuli");
	ParamRef IsIPictures = Parameter ("IsIPicture");
	_currentState = e_first_trial;
	FreeMemory ();
	m_image_rectangle.left = 0.35f;
	m_image_rectangle.right = 0.65f;
	m_image_rectangle.top = 0.35f;
	m_image_rectangle.bottom = 0.65f;
	m_calibration = cal_black; m_cal_steps = 0;

	if (States->Exists ("GazeCorrectionMode"))
	{
		State ("GazeCorrectionMode") = 1;
		// Disable drawing of your stimuli in here
	}
	wait_for_buttonpress = false;
	if (Parameter ("SalientTrials") != 0)
	{
		p_salientstimulus = new ImageStimulus (mrDisplay);
		p_salientstimulus->SetZOrder (0.0);
		p_salientstimulus->SetFile (Parameter ("SalientStimulus"));
		p_salientstimulus->SetDisplayRect (m_image_rectangle);
		p_salientstimulus->SetAspectRatioMode (GUI::AspectRatioModes::AdjustBoth);
		p_salientstimulus->Conceal ();
	}
	m_stim_rectangle.left = 0.0f;
	m_stim_rectangle.right = 1.0f;
	m_stim_rectangle.top = 0.0f;
	m_stim_rectangle.bottom = 1.0f;

	m_fixation.SetFile (Parameter ("FixationPicture"));
	m_fixation.SetZOrder (1.0);
	m_fixation.SetDisplayRect (m_stim_rectangle);
	m_fixation.SetAspectRatioMode (GUI::AspectRatioModes::AdjustBoth);
	m_fixation.Conceal ();

	m_IsIMask.SetZOrder (0.0);
	m_IsIMask.SetDisplayRect (m_stim_rectangle);
	m_IsIMask.SetAspectRatioMode (GUI::AspectRatioModes::AdjustBoth);
	m_IsIMask.Conceal ();

	

	m_SalientCatch.SetZOrder (0.0);
	m_SalientCatch.SetDisplayRect (m_stim_rectangle);
	m_SalientCatch.SetAspectRatioMode (GUI::AspectRatioModes::AdjustBoth);
	m_SalientCatch.Conceal ();
	

	m_IsINoise.SetFile (Parameter ("NoisePicture"));
	m_IsINoise.SetZOrder (0.0);
	m_IsINoise.SetDisplayRect (m_stim_rectangle);
	m_IsINoise.SetAspectRatioMode (GUI::AspectRatioModes::AdjustBoth);
	m_IsINoise.Conceal ();

	State ("EStimulation") = 0;
	__initsound (p_feedback_estim_stop, "SoundOnStimulationEnd");
	__initsound (p_feedback_estim_start, "SoundOnStimulationStart");
	__initsound (p_cue_sound, "AuditoryCue");
	__initsound (p_feedback_yes, "AuditoryFeedbackYes");
	__initsound (p_feedback_no, "AuditoryFeedbackNo");

	//find and set Threshold stimuli

	int num_stimuli = PestStimuli->NumRows (); // get the total number of different intensities

	// vector of all the visual stimulus intensity values
	std::vector<int> list_attenuation;
	list_attenuation.clear ();

	for (unsigned int row = 0; row < num_stimuli; row++)
		list_attenuation.push_back (PestStimuli (row, 0));

	
	int stim_idx = *std::find (list_attenuation.begin (), list_attenuation.end (), Parameter ("PerceptionThreshold"));
	num_stimuli = PestStimuli->NumColumns ();
	for (unsigned int col = 1; col < num_stimuli; col++)
	{
		ImageStimulus *p_visual_stimulus = new ImageStimulus (mrDisplay);

		p_visual_stimulus->SetPresentationMode (VisualStimulus::ShowHide);
		p_visual_stimulus->SetDisplayRect (m_stim_rectangle);
		p_visual_stimulus->SetAspectRatioMode (GUI::AspectRatioModes::AdjustBoth);
		p_visual_stimulus->SetFile (PestStimuli (stim_idx, col));
		p_visual_stimulus->SetZOrder (0.0); //always on top
		//p_visual_stimulus->Present();
		p_visual_stimulus->Conceal ();
		v_threshold_stimulus.push_back (p_visual_stimulus);
	}
	m_SalientCatch.SetFile (PestStimuli (PestStimuli->NumRows()-1, num_stimuli-1)); //the salient catch trial is the last available picture
	m_list_isi_picture.clear ();
	if (m_feedback_blocks != 0)
		for (int i = 1; i < IsIPictures->NumColumns (); i++)
		{
			m_list_isi_picture.push_back (string (IsIPictures (0, i)));
		}




	m_visual_sensor = Parameter ("VisualSensorChannel");
	m_visual_sensor_thresh = Parameter ("VisualSensorThreshold");
	m_push_button_threshold = Parameter ("PushButtonThreshold");
	m_push_button_channel = Parameter ("PushButtonChannel"); // push botton channel
	// Calculate cue duration parameters
	m_sample_rate = Parameter ("SamplingRate"); //Sampling rate
	m_block_size = Parameter ("SampleBlockSize"); // Sample block size
	m_block_size_msec = (float)(m_block_size) / (float)(m_sample_rate) * 1000; //Sample block size in msec 
	m_stim_visible_blocks = Parameter ("StimulusVisible").InMilliseconds () / m_block_size_msec;
	m_feedback_blocks = Parameter ("FeedbackDuration").InMilliseconds () / m_block_size_msec; // Feedback duration in msec                       
	m_cue_duration_blocks = Parameter ("CueDuration").InMilliseconds () / m_block_size_msec; // duration of cue state
	m_isi_duration_blocks = Parameter ("ISIDuration").InMilliseconds () / m_block_size_msec; // duration of inter stimulus trial state

	m_wait_min_blocks = Parameter ("WaitMin").InMilliseconds () / m_block_size_msec; // Min time duration between cue and stimulus presentation
	m_wait_max_blocks = Parameter ("WaitMax").InMilliseconds () / m_block_size_msec;// Max time duration between cue and stimulus presentation
	m_stimuli_duration_blocks = Parameter ("StimuliDuration").InMilliseconds () / m_block_size_msec; // Waiting time duration since stimulus onset until feedback presentation 

	m_SalientStimuliISIMax_blocks = Parameter ("SalientStimuliISIMax").InMilliseconds () / m_block_size_msec;
	m_SalientStimuliISIMin_blocks = Parameter ("SalientStimuliISIMin").InMilliseconds () / m_block_size_msec;
	m_SalientStimuliDuration_blocks = Parameter ("SalientStimuliDuration").InMilliseconds () / m_block_size_msec;





	// push botton parameters
	m_use_space_button = Parameter ("UseSpaceButton"); // if we use space botton

	m_use_Digital_input = Parameter ("UseDigitalInputButton"); // Use Digital Input 

	// Trial counter parameters
	m_runtime_counter = 0;
	m_trial_counter = 0;
	m_block_delta = 0;
	m_num_trials = 0;

	m_detected_trials = 0;
	m_stim_detected_trials = 0;
	m_undetected_trials = 0;
	m_stim_undetected_trials = 0;
	m_wrong_detect_trials = 0;
	m_stim_wrong_detect_trials = 0;
	v_stimulis.clear ();
	m_threshold_trials = Parameter ("ThresholdTrials");
	m_stim_threshold_trials = Parameter ("StimulatedThresholdTrials");
	m_salient_trials = Parameter ("SalientTrials");
	m_threshold_catch_trials = Round (Parameter ("ThresholdTrials")*Parameter ("ThresholdCatchFraction"));
	m_salient_catch_trials = Round (Parameter ("ThresholdTrials")*Parameter ("ThresholdCatchFraction"));
	m_stim_threshold_catch_trials = Round (Parameter ("StimulatedThresholdTrials")*Parameter ("StimulatedThresholdCatchFraction"));
	m_stim_salient_catch_trials = Round (Parameter ("StimulatedThresholdTrials")*Parameter ("StimulatedThresholdCatchFraction"));


	for (int i = 0; i < m_threshold_trials; ++i)
		v_stimulis.push_back (threshold_stim);

	for (int i = 0; i < m_threshold_catch_trials; ++i)
		v_stimulis.push_back (no_threshold);

	for (int i = 0; i < m_salient_trials; ++i)
		v_stimulis.push_back (salient_stim);

	for (int i = 0; i < m_salient_catch_trials; ++i)
		v_stimulis.push_back (salient_no_threshold);



	for (int i = 0; i < m_stim_threshold_trials; ++i)
		v_stimulis.push_back (stimulated_trial);

	for (int i = 0; i < m_stim_threshold_catch_trials; ++i)
		v_stimulis.push_back (stimulated_catch_no_stim);

	for (int i = 0; i < m_stim_salient_catch_trials; ++i)
		v_stimulis.push_back (stimulated_catch_strong);


	m_random_block.SetBlockSize (v_stimulis.size ());
	_currStimuli = v_stimulis.at (m_random_block.NextElement () - 1);
	m_orientation = 0;

	vector<int> frequencies;
	for (int i = 0; i < PestStimuli->NumColumns () - 1; i++)
		frequencies.push_back (m_threshold_trials / PestStimuli->NumColumns ());

	m_random_generator_orientation.SetFrequencies (frequencies);
	m_wrong_undetect_trials = 0;
	m_stim_wrong_undetect_trials = 0;
	m_wrong_detect_trials = 0;
	m_stim_wrong_detect_trials = 0;
	bciout << "Total number of trials " << v_stimulis.size () << std::endl;
}

void
ReactionTimeVisualPerceptionTask::StartRun ()
{

}


void
ReactionTimeVisualPerceptionTask::Process (const GenericSignal& Input, GenericSignal& Output)
{

	Output = Input; // This passes the signal through unmodified.
	mp_input = &Input;
	// keep track of the states
	++m_runtime_counter;
	State ("ExperimentState") = _currentState;
	State ("CurrentTrial") = m_trial_counter + 1;
	State ("StimType") = _currStimuli;
	State ("StimOrientation") = (m_orientation);
	++m_block_delta;
	switch (_currentState)
	{
		case e_first_trial:
			if (m_block_delta == 1)
			{
				m_text_stimulus.SetTextColor (RGBColor::Yellow);
				m_text_stimulus.SetText ("Get Ready...");
				m_text_stimulus.Present ();
			}
			if (m_block_delta == (2 * m_isi_duration_blocks - 1))
			{
				m_text_stimulus.Conceal ();
				m_fixation.Present ();
			}

			if (m_block_delta == (2 * m_isi_duration_blocks))
			{
				//if (!DoForceCalibration ()) //create a loop until button is pressed
				//	m_block_delta--;
			}



			if (m_block_delta == (2 * m_isi_duration_blocks + 2))
			{
				if (!IsButtonPressed ()) //create a loop until button is pressed
					m_block_delta--;
			}

			if (m_block_delta >= (2 * m_isi_duration_blocks + m_isi_duration_blocks / 2))
			{

				_currentState = e_isi;
				m_block_delta = 0;
			}

			break;
		case e_cue:

			if (m_trial_counter > (v_stimulis.size ()))
			{
				m_fixation.Conceal ();
				m_text_stimulus.SetText ("End of Experiment!");
				m_text_stimulus.Present ();
				State ("Running") = 0;
				_currentState = e_first_trial;
				m_block_delta = 0;
				m_trial_counter = 0;
				PrintSummary ();

			}

			if (m_block_delta == 1)
			{
				m_trial_counter++;
				if (p_cue_sound != NULL)
					p_cue_sound->Play ();
			}
			if (m_block_delta >= m_cue_duration_blocks)
			{
				if (p_cue_sound != NULL && p_cue_sound->IsPlaying ())
					p_cue_sound->Stop ();
				m_block_delta = 0;
				if (IsButtonPressed ())
					_currentState = e_false_start;
				else
					_currentState = e_pre_stimuli;
			}
			break;
		case e_pre_stimuli:
			if (m_block_delta == 1)
			{
				m_rand_blocks = RandomNumber (m_wait_min_blocks, m_wait_max_blocks);	// random delay in presenting the auditory stimuli after CueOff
				switch (_currStimuli)
				{
					case stimulated_catch_no_stim:
					case stimulated_trial:
					case stimulated_catch_strong:
						SetStimulationState (true);
						break;
					default:
						SetStimulationState (false);
				}

			}

			if ((m_block_delta >= SENS_CALIB_DUR) && !VisualSensorCorrect (false))
			{

				m_block_delta = 0;
				_currentState = e_force_calibration;
			}

			if ((m_block_delta >= m_rand_blocks / 2) && OptionalState ("FixationViolated", 0) && OptionalParameter ("EnforceFixation", 0))
			{
				ThresholdStimulusOff ();
				m_block_delta = 0;
				_currentState = e_fixation_violated;
			}

			if (IsButtonPressed ())
			{
				_currentState = e_false_start;
				m_block_delta = 0;
			}

			if (m_block_delta >= m_rand_blocks)
			{
				switch (_currStimuli)
				{
					case salient_stim:
						_currentState = e_stimuli_strong;
						break;
					case salient_no_threshold:
						_currentState = e_stimuli_catch_salient;
						break;
					case no_threshold:
						_currentState = e_stimuli_catch;
						break;
					case threshold_stim:
						_currentState = e_stimuli_threshold;
						break;
					case stimulated_catch_no_stim:
						_currentState = e_stimuli_catch;
						break;

					case stimulated_catch_strong:
						_currentState = e_stimuli_catch_salient;
						break;

					case stimulated_trial:
						_currentState = e_stimuli_threshold;
						break;

				}
				m_block_delta = 0;
			}

			break;
		case e_stimuli_strong:
			if (m_block_delta == 1)
			{
				p_salientstimulus->Present ();

			}

			if (IsButtonPressed ())
			{
				m_block_delta = 0;
				_currentState = e_false_start;
				p_salientstimulus->Conceal ();
			}

			if (m_block_delta >= m_SalientStimuliDuration_blocks)
			{
				m_block_delta = 0;
				_currentState = e_isi_salient;
				p_salientstimulus->Conceal ();

			}
			break;
		case e_isi_salient:
			if (m_block_delta == 1)
			{
				m_SalientStimuliISI_blocks = RandomNumber (m_SalientStimuliISIMin_blocks, m_SalientStimuliISIMax_blocks);	// random delay in presenting the auditory stimuli after CueOff

			}
			if (m_block_delta >= m_SalientStimuliISI_blocks)
			{
				m_block_delta = 0;
				if (_currStimuli == salient_no_threshold)
					_currentState = e_stimuli_catch;
				else
					_currentState = e_stimuli_threshold;
			}
			if (IsButtonPressed ())
			{
				m_block_delta = 0;
				_currentState = e_false_start;
			}
			break;

		case e_stimuli_threshold:
			if (m_block_delta == 1)
			{
				ThresholdStimulusOn ();
			}

			if (OptionalState ("FixationViolated", 0) && OptionalParameter ("EnforceFixation", 0))
			{
				ThresholdStimulusOff ();
				m_block_delta = 0;
				_currentState = e_fixation_violated;
			}
			if ((m_block_delta >= SENS_CALIB_DUR) && !VisualSensorCorrect (true))
			{
				ThresholdStimulusOff ();
				m_block_delta = 0;
				_currentState = e_force_calibration;
			}

			if (m_block_delta == m_stim_visible_blocks)
			{
				ThresholdStimulusOff ();
			}

			if (IsButtonPressed () || (m_block_delta >= m_stimuli_duration_blocks))
			{
				m_block_delta = 0;
				_currentState = e_feedback;
				ThresholdStimulusOff ();
				if (IsButtonPressed ())
				{
					if(State("EStimulation") != 0)
						++m_stim_detected_trials;
					else
						++m_detected_trials;
				}
				else
				{
					if (State ("EStimulation") != 0)
						++m_stim_undetected_trials;
					else
						++m_undetected_trials;
				}

			}
			break;
		case e_fixation_violated:
			if (m_block_delta == 1)
			{
				m_text_stimulus.SetText ("Fixation Violated");
				m_text_stimulus.Present ();
			}

			if (m_block_delta >= m_isi_duration_blocks / 2)
			{
				m_block_delta = 0;
				m_text_stimulus.Conceal ();
				_currentState = e_false_start;
			}

			break;
		case e_stimuli_catch_salient:
			if (m_block_delta == 1)
			{
				m_SalientCatch.Present ();
			}
			if (OptionalState ("FixationViolated", 0) && OptionalParameter ("EnforceFixation", 0))
			{

				m_block_delta = 0;
				_currentState = e_fixation_violated;
				m_SalientCatch.Conceal ();
			}
			if (IsButtonPressed () || (m_block_delta >= m_stimuli_duration_blocks))
			{
				m_block_delta = 0;
				if (m_feedback_blocks != 0)
					m_IsIMask.SetFile (string (Parameter ("IsIPicture")(0, 0)));
				_currentState = e_feedback;

				
				m_SalientCatch.Conceal();
				if (!IsButtonPressed ())
					(State ("EStimulation") != 0) ? ++m_stim_wrong_undetect_trials : ++m_wrong_undetect_trials;
			}

			break;
				
		case e_stimuli_catch:
			if (OptionalState ("FixationViolated", 0) && OptionalParameter ("EnforceFixation", 0))
			{

				m_block_delta = 0;
				_currentState = e_fixation_violated;
			}
			if (IsButtonPressed () || (m_block_delta >= m_stimuli_duration_blocks))
			{
				m_block_delta = 0;
				if (m_feedback_blocks != 0)
					m_IsIMask.SetFile (string (Parameter ("IsIPicture")(0, 0)));
				_currentState = e_feedback;
				if (IsButtonPressed ())
					(State ("EStimulation") != 0) ? ++m_stim_wrong_detect_trials : ++m_wrong_detect_trials;
			}

			break;

		case e_feedback:
			if (m_block_delta == 1)
			{
				if (m_feedback_blocks != 0)
				{

					m_IsIMask.Present ();
					if (p_feedback_yes)
						p_feedback_yes->Play ();
				}

				//mp_fixation->Conceal();
				// mp_text_stimulus->Conceal();
			}



			if (m_block_delta >= m_feedback_blocks)
			{
				m_IsIMask.Conceal ();
				m_block_delta = 0;
				_currentState = e_isi;
			}

			break;
		case e_isi:
			if (m_block_delta == 1)
			{
				PrintSummary ();
				SetStimulationState (false);
				ThresholdStimulusOff();
				if (p_salientstimulus != NULL) p_salientstimulus->Conceal();
				//m_fixation.Conceal();
				m_IsIMask.Conceal();
				m_SalientCatch.Conceal();

				m_IsINoise.Present ();
				//mp_fixation->Conceal();
				// mp_text_stimulus->Conceal();
			}

			if ((m_block_delta >= SENS_CALIB_DUR) && !VisualSensorCorrect (true))
			{
				m_IsINoise.Conceal ();
				m_block_delta = 0;
				_currentState = e_force_calibration;
			}


			if (m_block_delta >= m_isi_duration_blocks)
			{
				m_IsINoise.Conceal ();
				_currStimuli = v_stimulis.at (m_random_block.NextElement () - 1);
				_currentState = e_cue;
				m_block_delta = 0;
			}
			break;
		case e_false_start:
			if (m_block_delta == 1)
			{
				SetStimulationState (false);
				//m_IsIMask.Present();
				//mp_fixation->Conceal();
				// mp_text_stimulus->Conceal();
			}

			if (m_block_delta == (m_cue_duration_blocks / 2)) //
			{
				//m_IsIMask.Conceal();
				m_IsINoise.Present ();
			}

			if (m_block_delta >= m_isi_duration_blocks)
			{
				m_IsINoise.Conceal ();
				m_trial_counter--;
				_currentState = e_cue;
				m_block_delta = 0;
			}
			break;
		case e_force_calibration:
			m_block_delta = 0;
			_currentState = e_false_start;
			if (m_block_delta == 1)
			{
				SetStimulationState (false);
			}
			if (!wait_for_buttonpress)
			{
				//if (DoForceCalibration ())
				//	wait_for_buttonpress = true;
			}
			else if (IsButtonPressed ())
			{
				m_block_delta = 0;
				_currentState = e_false_start;
				wait_for_buttonpress = false;
			}
			break;


	}
}

bool ReactionTimeVisualPerceptionTask::DoForceCalibration ()
{
	return false;
	m_cal_steps++;
	switch (m_calibration)
	{
		case cal_black:
			if (m_cal_steps == 1)
			{

				m_text_stimulus.SetText ("Recalibrate black!");
				m_text_stimulus.Present ();
			}
			if ((m_cal_steps >= SENS_CALIB_DUR) && VisualSensorCorrect (false))
			{
				m_calibration = cal_white;
				m_text_stimulus.Conceal ();
				m_cal_steps = 0;
			}
			break;
		case cal_white:
			if (m_cal_steps == 1)
			{
				m_text_stimulus.SetText ("Recalibrate white!");
				m_text_stimulus.Present ();
				m_IsINoise.Present ();
			}
			if ((m_cal_steps >= SENS_CALIB_DUR) && VisualSensorCorrect (true))
			{
				m_IsINoise.Conceal ();
				m_calibration = cal_finished;
				m_text_stimulus.Conceal ();
				m_cal_steps = 0;
			}
			break;
		case cal_finished: //recheck black, if pass go to false start

			if ((m_cal_steps >= SENS_CALIB_DUR))
			{
				m_calibration = cal_black;
				m_cal_steps = 0;

				if (VisualSensorCorrect (false))
					return true;
			}
			break;

	}
	return false;
}

void
ReactionTimeVisualPerceptionTask::StopRun ()
{
	// The Running state has been set to 0, either because the user has pressed "Suspend",
	// or because the run has reached its natural end.
	PrintSummary ();
	// You know, you can delete methods if you're not using them.
	// Remove the corresponding declaration from ReactionTimeVisualPerceptionTask.h too, if so.
}

void
ReactionTimeVisualPerceptionTask::Halt ()
{
	// Stop any threads or other asynchronous activity.
	// Good practice is to write the Halt() method such that it is safe to call it even *before*
	// Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
	// have already been deleted:  set them to NULL after deletion).
}

/**
Free allocated memory and set pointers to NULL
*/
void ReactionTimeVisualPerceptionTask::FreeMemory ()
{
	__nulldelete (p_salientstimulus);
	__nulldelete (p_cue_sound);
	__nulldelete (p_feedback_yes);
	__nulldelete (p_feedback_no);
	__nulldelete (p_feedback_estim_start);
	__nulldelete (p_feedback_estim_stop);

	for (int i = 0; i < v_threshold_stimulus.size (); ++i)
		delete(v_threshold_stimulus[i]);

	v_threshold_stimulus.clear ();
}

bool ReactionTimeVisualPerceptionTask::VisualSensorCorrect (bool expected)
{
	if (Parameter ("OverrideAutocalibration") == 1)
		return true;
	bool isActive = false;
	int hightime = 0;


	for (unsigned int i = 0; i < m_block_size; i++)
	{

		if (m_use_Digital_input)
		{
			if (State ("DigitalInput" + std::to_string (m_visual_sensor))(i) == 1)
			{
				hightime++;
				if (hightime >= ((m_block_size / 2 == 0) ? 1 : (m_block_size / 2)))
				{
					isActive = true;
					//	bciout << "button pressed" << endl;
					break;
				}
			}
			else
				hightime = 0;
		}
		else
		{
			if (mp_input == NULL)
				return true;
			if (mp_input->Value (m_visual_sensor - 1, i) > m_visual_sensor_thresh)
			{
				hightime++;
				if (hightime >= ((m_block_size / 2 == 0) ? 1 : (m_block_size / 2)))
				{
					isActive = true;
					//	bciout << "button pressed" << endl;
					break;
				}
			}
			else
				hightime = 0;
		}


	}
	return (isActive == expected);

}

void ReactionTimeVisualPerceptionTask::SetStimulationState (bool state)
{
	if (bool(State ("EStimulation")) == state)
		return;
	if (state)
	{
		AppLog << "Stimulation on" << endl;
		if (p_feedback_estim_start != NULL) p_feedback_estim_start->Play ();
		State ("EStimulation") = 1;
	}
	else
	{
		AppLog << "Stimulation Off" << endl;
		if (p_feedback_estim_stop != NULL) p_feedback_estim_stop->Play ();
		State ("EStimulation") = 0;
	}



}

bool ReactionTimeVisualPerceptionTask::IsButtonPressed ()
{

	//Check for keypresses
	bool press = false;

	if (m_use_Digital_input)
	{
		for (unsigned int i = 0; i < m_block_size; i++)
		{
			if (State ("DigitalInput" + std::to_string (m_push_button_channel))(i) == 1)
			{
				return true;
				//bciout << "key pressed" << endl;
			}
		}
	}

	if (!m_use_Digital_input && m_use_space_button)
	{
		for (unsigned int i = 0; i < m_block_size; i++)
		{
			if (State ("KeyDown")(i) == VK_SPACE)
			{
				return true;
				//bciout << "key pressed" << endl;
			}
		}
	}

	if (!m_use_Digital_input && !m_use_space_button && mp_input != NULL)
	{
		for (unsigned int i = 0; i < m_block_size; i++)
		{


			if (mp_input->Value (m_push_button_channel - 1, i) > m_push_button_threshold)
			{
				return true;
			}

		}
	}

	return press;
}

int
ReactionTimeVisualPerceptionTask::RandomNumber (int min, int max)
{
	int random_number = min + m_random_generator.Random () % (max - min);

	//bciout << random_number << endl;

	return random_number;
}

void ReactionTimeVisualPerceptionTask::ThresholdStimulusOn ()
{

	m_orientation = m_random_generator_orientation.NextElement ();
	if (m_feedback_blocks != 0)
		m_IsIMask.SetFile (m_list_isi_picture[m_orientation - 1]);

	v_threshold_stimulus.at (m_orientation - 1)->Present ();

}

void ReactionTimeVisualPerceptionTask::ThresholdStimulusOff ()
{
	if (m_orientation != 0)
	{
		v_threshold_stimulus.at (m_orientation - 1)->Conceal ();
		m_orientation = 0;
	}
}

void ReactionTimeVisualPerceptionTask::PrintSummary ()
{

	AppLog << "#################################################" << endl
		<< "STIMULATION percieved trials / all trials:  (" << m_stim_detected_trials << "/" << m_stim_threshold_trials << ")" << endl
		<< "STIMULATION percieved trials / current trials:  (" << m_stim_detected_trials << "/" << m_stim_detected_trials + m_stim_undetected_trials << ")" << endl
		<< " percieved trials / all trials:  (" << m_detected_trials << "/" << m_threshold_trials + m_salient_trials << ")" << endl
		<< " percieved trials / current trials:  (" << m_detected_trials << "/" << m_undetected_trials + m_detected_trials << ")" << endl
		<< "STIMULATION erroneously  detected trials / all catch trials:  (" << m_stim_wrong_detect_trials << "/" << m_threshold_catch_trials << ")" << endl
		<< "STIMULATION erroneously  missed trials / all catch trials:  (" << m_stim_wrong_undetect_trials << "/" << m_threshold_catch_trials << ")" << endl
		<< "erroneously  detected trials / all catch trials:  (" << m_wrong_detect_trials << "/" << m_threshold_catch_trials << ")" << endl
		<< "erroneously  missed trials / all catch trials:  (" << m_wrong_undetect_trials << "/" << m_threshold_catch_trials << ")" << endl;
}
