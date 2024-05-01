////////////////////////////////////////////////////////////////////////////////
// Authors:
// Description: ReactionVisualCalibrationTask implementation
////////////////////////////////////////////////////////////////////////////////

#include "ReactionVisualCalibrationTask.h"
#include "BCIStream.h"
#include "RandomGenerator.h"
#include <utility>
#include <math.h>
#include <Windows.h>
#include <cmath>
#include <fstream>
#include <numeric>
#define SENS_CALIB_DUR 10


using namespace std;


RegisterFilter(ReactionVisualCalibrationTask, 3);
// Change the location if appropriate, to determine where your filter gets
// sorted into the chain. By convention:
//  - filter locations within SignalSource modules begin with "1."
//  - filter locations within SignalProcessing modules begin with "2."
//       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
//  - filter locations within Application modules begin with "3."


ReactionVisualCalibrationTask::ReactionVisualCalibrationTask() :
	mrDisplay(Window()), m_push_button_threshold(10000), m_IsINoise(mrDisplay), mp_text_stimulus(mrDisplay), m_random_generator(),
	mp_fixation(mrDisplay), m_random_block(m_random_generator), m_random_generator_orientation(m_random_generator)
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
		"{filename_0 filename_45 filename_90 filename_135 filename_225 filename_270 filename_315}"
		" ..\\tasks\\VisGratingNegative\\0.png  ..\\tasks\\VisGratingNegative\\45.png  ..\\tasks\\VisGratingNegative\\90.png  ..\\tasks\\VisGratingNegative\\135.png  ..\\tasks\\VisGratingNegative\\225.png  ..\\tasks\\VisGratingNegative\\270.png  ..\\tasks\\VisGratingNegative\\315.png ",

		"Application:ReactionTimeTask string NoisePicture= ..\\tasks\\VisGrating\\noise.bmp",
		"Application:ReactionTimeTask string FixationPicture= ..\\tasks\\VisGrating\\fixation.bmp ",
		"Application:ReactionTimeTask string AuditoryCue= .\\sounds\\tick.wav ",
		"Application:ReactionTimeTask string AuditoryFeedbackYes= .\\sounds\\yes.wav",
		"Application:ReactionTimeTask string AuditoryFeedbackNo= .\\sounds\\no.wav",
		"Application:ReactionTimeTask int StaircaseTrials= 10 10 0 % //",
		"Application:ReactionTimeTask int UseSpaceButton= 1 1 0 1 // enable keyboard space button ? (boolean)",
		"Application:ReactionTimeTask int UseDigitalInputButton= 0 1 0 1 // enable Digital input button ? (boolean)",
		"Application:ReactionTimeTask int PushButtonChannel= 2 2 1 % // channel number of push button",
		"Application:ReactionTimeTask int VisualSensorChannel= 1 2 1 % // channel number of Visual Sensor",
		"Application:ReactionTimeTask int PushButtonThreshold= 1e5 1000 0 % // channel number of push button",
		"Application:ReactionTimeTask int VisualSensorThreshold= 1e5 1000 0 % // channel number of push button",
		"Application:ReactionTimeTask int OverrideAutocalibration= 0 0 0 1 // ignore recalibration  (boolean)",
		END_PARAMETER_DEFINITIONS

		BEGIN_PARAMETER_DEFINITIONS
		"Application:ReactionTimeTask int UsePestAlgorithm= 0 0 0 1 // use pest algorithm instead of staircase ? (boolean)",
		"Application:ReactionTimeTask float PestStartLevel= 10 30 000 10000 ",
		"Application:ReactionTimeTask float PestStartStepSize= 4 4 0 25",
		"Application:ReactionTimeTask float PestFinalStepSize= 1 1 0 1",
		"Application:ReactionTimeTask int PestMaxStepSize= 4 4 0 50",
		"Application:ReactionTimeTask int PestWaldFactor= 2 2 1 5",
		"Application:ReactionTimeTask float PestTargetPerformance= 0.5 0.5 0 1",
		"Application:ReactionTimeTask int MaxItrNum= 80 80 20 200",
		"Application:ReactionTimeTask int PerceptionThreshold= 5 5  0 10000 % // final performance ratio",
		"Application:ReactionTimeTask float FeedbackDuration= 500ms 1000ms 0 % // feedback duration",
		"Application:ReactionTimeTask string PerceptionThresholdFilePath= ..\\parms\\VisualThreshold\\fragment_perception_threshold.prm",
		END_PARAMETER_DEFINITIONS

		BEGIN_PARAMETER_DEFINITIONS
		// Timing   
		"Application:ReactionTimeTask float StimulusVisible= 125ms 1000ms 0 % // cue duration",
		"Application:ReactionTimeTask float CueDuration= 300ms 1000ms 0 % // cue duration",
		"Application:ReactionTimeTask float ISIDuration= 1000ms 1000ms 0 % // cue duration",
		"Application:ReactionTimeTask float StimuliDuration= 1500ms 4000ms 0 % // stimuli duration",
		"Application:ReactionTimeTask float FeedbackDuration= 500ms 1000ms 0 % // feedback duration",
		"Application:ReactionTimeTask float WaitMin= 1s 1s 0 % // min time to present stimulus",
		"Application:ReactionTimeTask float WaitMax= 3s 3s 0 % // max time to present stimulus",
		"Application:ReactionTimeTask float RTMin= 200ms 100ms 0 % // min reaction time (False Start)",
		"Application:ReactionTimeTask float RTMax= 1500ms 2000ms 0 % // max reaction time (Too Long)",
		END_PARAMETER_DEFINITIONS

		BEGIN_STATE_DEFINITIONS
		"CurrentTrial		8  0 0 0", // The Current Trial
		"ExperimentState	4  0 0 0", //
		"CueVisual			1  0 0 0", //
		"StimVisual			1  0 0 0", //
		"Threshold			16 0 0 0",
		"StimulusLevel		16 0 0 0",//
		"Performance		16  0 0 0",//
		"StimOrientation	3  0 0 0",
		END_STATE_DEFINITIONS
		// text characteristics
		m_full_rectangle.left = 0.0f;
	m_full_rectangle.right = 1.0f;
	m_full_rectangle.top = 0.2f;
	m_full_rectangle.bottom = 0.8f;


	mp_text_stimulus.SetZOrder(0.0);
	mp_text_stimulus.SetText("");
	mp_text_stimulus.SetTextHeight(0.1f);
	mp_text_stimulus.SetDisplayRect(m_full_rectangle);
	mp_text_stimulus.SetColor(RGBColor::Black);
	mp_text_stimulus.SetTextColor(RGBColor::Green);
	mp_text_stimulus.Present();

	m_cue_image_rectangle.left = 0.35f;
	m_cue_image_rectangle.right = 0.65f;
	m_cue_image_rectangle.top = 0.35f;
	m_cue_image_rectangle.bottom = 0.65f;

	mp_fixation.SetDisplayRect(m_cue_image_rectangle);

	cue_sound = NULL;
	feedback_no = NULL;
	feedback_yes = NULL;
	m_orientation = 0;


}

ReactionVisualCalibrationTask::~ReactionVisualCalibrationTask()
{
	Halt();
	// If you have allocated any memory with malloc(), call free() here.
	// If you have allocated any memory with new[], call delete[] here.
	// If you have created any object using new, call delete here.
	// Either kind of deallocation will silently ignore null pointers, so all
	// you need to do is to initialize those to zero in the constructor, and
	// deallocate them here.
	if (cue_sound)
	{
		if (cue_sound->IsPlaying())
			cue_sound->Stop();

		delete(cue_sound);
	}

	if (feedback_no)
	{
		if (feedback_no->IsPlaying())
			feedback_no->Stop();

		delete(feedback_no);
	}

	if (feedback_yes)
	{
		if (feedback_yes->IsPlaying())
			feedback_yes->Stop();

		delete(feedback_yes);
	}
	clearVisualStimuliMap();



}

void
ReactionVisualCalibrationTask::Publish()
{

}

void
ReactionVisualCalibrationTask::Preflight(const SignalProperties& Input, SignalProperties& Output) const
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
	//      bcierr << "Denominator cannot be zero" << endl;
	//
	// Errors issued in this way, during Preflight, still allow the user to open
	// the Config dialog box, fix bad parameters and re-try.  By contrast, errors
	// and C++ exceptions at any other stage (outside Preflight) will make the
	// system stop, such that BCI2000 will need to be relaunched entirely.

	// Note that the ReactionVisualCalibrationTask instance itself, and its members, are read-only during
	// this phase, due to the "const" at the end of the Preflight prototype above.
	// Any methods called by Preflight must also be "const" in the same way.

	Output = Input; // this simply passes information through about SampleBlock dimensions, etc....
	OptionalState("GazeCorrectionMode");
	OptionalState("FixationViolated");
	// create pointer to parameters
	ParamRef PestStimuli = Parameter("PestStimuli");
	ParamRef PestStartLevel = Parameter("PestStartLevel");
	ParamRef PestStartStepSize = Parameter("PestStartStepSize");
	ParamRef PestFinalStepSize = Parameter("PestFinalStepSize");
	ParamRef PestWaldFactor = Parameter("PestWaldFactor");
	ParamRef PestTargetPerformance = Parameter("PestTargetPerformance");
	ParamRef AuditoryCue = Parameter("AuditoryCue");
	if (Parameter("UseDigitalInputButton") == 1)
	{
		State("DigitalInput1");
		State("DigitalInput2");
		State("DigitalInput3");
		State("DigitalInput4");
	}
	else
	{
		OptionalState("DigitalInput1");
		OptionalState("DigitalInput2");
		OptionalState("DigitalInput3");
		OptionalState("DigitalInput4");
	}

	if (Parameter("UseSpaceButton"))
		State("KeyDown");
	else
		OptionalState("KeyDown");
	// touching states
	State("Running");

	State("CurrentTrial");
	State("ExperimentState");
	State("CueVisual");
	State("StimVisual");
	State("Threshold");
	State("StimulusLevel");
	State("Performance");
	State("StimOrientation");
	Parameter("OverrideAutocalibration");
	// touching parameters
	OptionalParameter("EnforceFixation");
	Parameter("StaircaseTrials");
	Parameter("VisualSensorChannel");
	Parameter("VisualSensorThreshold");
	Parameter("PushButtonThreshold");
	Parameter("UsePestAlgorithm");
	Parameter("StimulusVisible");
	Parameter("CueDuration");
	Parameter("StimuliDuration");
	Parameter("SampleBlockSize");
	Parameter("SamplingRate");

	Parameter("PushButtonChannel");
	Parameter("WaitMin");
	Parameter("WaitMax");
	Parameter("ISIDuration");
	Parameter("PestMaxStepSize");
	Parameter("PerceptionThreshold");
	Parameter("PerceptionThresholdFilePath");
	//Parameter("TextFeedbackPreCue");
	Parameter("IsIPicture");

	Parameter("RTMax");
	Parameter("RTMin");
	Parameter("MaxItrNum");
	Parameter("WindowBackgroundColor");
	if (string(Parameter("NoisePicture")).empty())
	{
		bcierr << "No noise picture assigned!" << endl;
	}


	if (string(Parameter("FixationPicture")).empty())
	{
		bcierr << "No fixation picture assigned!" << endl;
	}

	if (string(Parameter("AuditoryFeedbackYes")).empty())
	{
		bciwarn << "No auditory Yes Feedback assigned!" << endl;
	}

	if (string(Parameter("AuditoryFeedbackNo")).empty())
	{
		bciwarn << "No auditory No Feedback assigned!" << endl;
	}

	if (string(AuditoryCue).empty())
	{
		bciwarn << "No auditory Cue assigned!" << endl;
	}



	// check min reaction time is smaller than max reaction time
	if (Parameter("RTMin").InMilliseconds() > Parameter("RTMax").InMilliseconds())
		bcierr << "Parameter RTMin (" << Parameter("RTMin") << ") must be smaller or equal to RTMax (" << Parameter("RTMax") << ")." << endl;

	// check that WaitMin is smaller than WaitMax
	if (Parameter("WaitMin").InMilliseconds() > Parameter("WaitMax").InMilliseconds())
		bcierr << "Parameter WaitMin (" << Parameter("WaitMax") << ") must be smaller or equal to WaitMax (" << Parameter("WaitMax") << ")." << endl;

	double block_size_msec = (double)(Parameter("SampleBlockSize")) / (double)(Parameter("SamplingRate").InHertz()) * 1000;

	if (fmod((double)Parameter("StimulusVisible").InMilliseconds(), block_size_msec) > 1e-6)
		bcierr << "Parameter StimulusVisible (" << Parameter("StimulusVisible") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	// check that WaitMin is a multiple of the sample block time
	if (fmod((double)Parameter("WaitMin").InMilliseconds(), block_size_msec) > 1e-6)
		bcierr << "Parameter WaitMin (" << Parameter("WaitMin") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	// check that ISIDuration is a multiple of the sample block time
	if (fmod((double)Parameter("ISIDuration").InMilliseconds(), block_size_msec) > 1e-6)
		bcierr << "Parameter ISIDuration (" << Parameter("ISIDuration") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	// Check that the PestStimuli matrix has at least 2 columns
	if (Parameter("PestStimuli")->NumColumns() < 2)
		bcierr << "Parameter PestStimuli has " << Parameter("PestStimuli")->NumColumns() << " columns, but must have at least 2 columns." << endl;

	// Check that the PestStimuli matrix has at least 10 rows (i.e., stimuli)
	//if (Parameter("PestStimuli")->NumRows() < 10)
	//	bcierr <<  "Parameter PestStimuli has " << Parameter("PestStimuli")->NumRows() << " rows, but must have at least 10 rows (i.e., number of stimuli)." << endl; 

	// Check that the PestStartStepSize is smaller than the PestFinalStepSize
	if (Parameter("PestStartStepSize") < Parameter("PestFinalStepSize"))
		bcierr << "PestStartStepSize " << Parameter("PestStartStepSize") << " should be larger than PestFinalStepSize " << Parameter("PestFinalStepSize") << endl;

	// check that cueDuration is a multiple of the sample block time
	if (fmod((double)Parameter("CueDuration").InMilliseconds(), block_size_msec) > 1e-6)
		bcierr << "Parameter CueDuration (" << Parameter("CueDuration") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	// check that FeedbackDuration is a multiple of the sample block time
	if (fmod((double)Parameter("FeedbackDuration").InMilliseconds(), block_size_msec) > 1e-6)
		bcierr << "Parameter FeedbackDuration (" << Parameter("FeedbackDuration") << ") must be a multiple of the sample block time (" << block_size_msec << "ms)." << endl;

	int num_stimuli = PestStimuli->NumRows(); // get the total number of different intensities

	// vector of all the visual stimulus intensity values
	std::vector<float> list_attenuation;
	list_attenuation.clear();

	for (unsigned int row = 0; row < num_stimuli; row++)
		list_attenuation.push_back(PestStimuli(row, 0));

	if (Parameter("FeedbackDuration") != 0)
		if (Parameter("IsIPicture")->NumColumns() != (PestStimuli->NumColumns() - 1))
		{
			bcierr << "Number of Interstim Pictures must match Number of possible stimuli pictures for a certain stimuli" << endl;
		}

	float attenuation_min = *std::min_element(std::begin(list_attenuation), std::end(list_attenuation));  // find min intensity value
	float attenuation_max = *std::max_element(std::begin(list_attenuation), std::end(list_attenuation)); // find max intensity value

	// Check that the PestStartLevel is well within the minimum range of the PestStimuli attenuation range
	if ((PestStartLevel - 2.0 * PestStartStepSize) < attenuation_min)
		bcierr << "PestStartLevel (" << PestStartLevel << ") must be at least 2*PestStartStepSize (" << PestStartStepSize << ") larger than the minimum attenuation in the PestStimuli (" << attenuation_min << ")." << endl;

	// Check that the PestStartLevel is well within the maximum range of the PestStimuli attenuation range
	if ((PestStartLevel + 2.0 * PestStartStepSize) > attenuation_max)
		bcierr << "PestStartLevel (" << PestStartLevel << ") must be at most 2*PestStartStepSize (" << PestStartStepSize << ") smaller than the maximum attenuation in the PestStimuli (" << attenuation_max << ")." << endl;




}


void
ReactionVisualCalibrationTask::Initialize(const SignalProperties& Input, const SignalProperties& Output)
{
	wait_for_buttonpress = false;
	m_overrideCalib = Parameter("OverrideAutocalibration");
	// Create pointers to paramters
	ParamRef PestStimuli = Parameter("PestStimuli"); //Input stimulus
	ParamRef PestStartLevel = Parameter("PestStartLevel"); //Initial value for intensity presenting to subject
	ParamRef PestStartStepSize = Parameter("PestStartStepSize"); // Initial step size
	ParamRef PestFinalStepSize = Parameter("PestFinalStepSize"); // Smallest possible step size
	ParamRef PestWaldFactor = Parameter("PestWaldFactor"); // Wald factor, PEST toleration of error
	ParamRef PestTargetPerformance = Parameter("PestTargetPerformance"); // Target perception threshold
	ParamRef IsIPictures = Parameter("IsIPicture");
	// Read PEST parameters
	m_use_pest = Parameter("UsePestAlgorithm");
	m_PestStartLevel = Parameter("PestStartLevel");                 // starting value for intensity            
	m_PestFinalStepSize = Parameter("PestFinalStepSize");           // smallest value for intensity change 
	m_PestStartStepSize = Parameter("PestStartStepSize");           // smallest value for intensity change 
	m_PestWaldFactor = Parameter("PestWaldFactor");              // number of missed trials x target likelihood without intensity change     
	m_PestTargetPerformance = Parameter("PestTargetPerformance");       // target performance ratio    
	m_PestStepsize = m_PestStartStepSize;
	m_Pest_num_correct = 0;
	m_Pest_doubled = 0;
	m_PestMaxstepsize = Parameter("PestMaxStepSize");

	m_max_num_trials = Parameter("MaxItrNum");
	// Feedback duration in msec
	// Calculate cue duration parameters
	m_sample_rate = Parameter("SamplingRate"); //Sampling rate
	m_block_size = Parameter("SampleBlockSize"); // Sample block size

	m_block_size_msec = (float)(m_block_size) / (float)(m_sample_rate) * 1000; //Sample block size in msec        
	m_stim_visible_blocks = Parameter("StimulusVisible").InMilliseconds() / m_block_size_msec;
	m_feedback_blocks = Parameter("FeedbackDuration").InMilliseconds() / m_block_size_msec;
	m_cue_duration_blocks = Parameter("CueDuration").InMilliseconds() / m_block_size_msec; // duration of cue state
	m_isi_duration_blocks = Parameter("ISIDuration").InMilliseconds() / m_block_size_msec; // duration of inter stimulus trial state

	m_wait_min_blocks = Parameter("WaitMin").InMilliseconds() / m_block_size_msec; // Min time duration between cue and stimulus presentation
	m_wait_max_blocks = Parameter("WaitMax").InMilliseconds() / m_block_size_msec;// Max time duration between cue and stimulus presentation
	m_stimuli_duration_blocks = Parameter("StimuliDuration").InMilliseconds() / m_block_size_msec; // Waiting time duration since stimulus onset until feedback presentation 


	m_visual_intensity = m_PestStartLevel;
	// Reaction time parameters
	m_max_rt = Parameter("RTMax");
	m_min_rt = Parameter("RTMin");


	// push botton parameters
	m_visual_sensor = Parameter("VisualSensorChannel");
	m_visual_sensor_thresh = Parameter("VisualSensorThreshold");
	m_push_button_threshold = Parameter("PushButtonThreshold");
	m_push_button_channel = Parameter("PushButtonChannel"); // push botton channel
	m_use_space_button = Parameter("UseSpaceButton"); // if we use space botton
	m_use_Digital_input = Parameter("UseDigitalInputButton"); // Use Digital Input 

	// Trial counter parameters
	m_runtime_counter = 0;
	m_trial_counter = 0;
	m_block_delta = 0;
	m_is_first_trial = true; // first state
	m_num_trials = 0;
	m_Pest_samelevelcounter = 0;

	m_reaction_time.clear(); // reaction time vector
	m_stimulus_Level.clear(); // Stimulus intensity vector for all trials
	m_stimulus_Level_copy.clear(); // just a copy helped for coding!
	m_PEST_performance.clear(); // Subject performance vector
	m_Pest_percentage_check.clear(); // final perception percentages based on subject's performance in the total of trials
	m_Pesttrialcounter = 0; // trial counter
	m_Pest_history_correct.clear(); // Subject's performance

	// pushe botton flag
	m_pushed_botton_flag = 0;

	// State initiation
	m_current_state = e_cue;

	if (States->Exists("GazeCorrectionMode"))
	{
		State("GazeCorrectionMode") = 1;
		// Disable drawing of your stimuli in here
	}

	// Determine number of Stimuli
	m_num_stimuli = Parameter("PestStimuli")->NumRows();

	// Read attenuation from Stimuli
	m_list_attenuation.clear();
	for (unsigned int row = 0; row < m_num_stimuli; row++)
		m_list_attenuation.push_back(PestStimuli(row, 0));

	// Determine range and stepping of attenuation in Stimuli
	m_attenuation_min = *std::min_element(std::begin(m_list_attenuation), std::end(m_list_attenuation));
	m_attenuation_max = *std::max_element(std::begin(m_list_attenuation), std::end(m_list_attenuation));
	m_attenuation_stepping = PestStimuli(1, 0) - PestStimuli(2, 0);
	m_PestMinlevel = m_attenuation_min; // Smallest possible stimulus intensity

	clearVisualStimuliMap();
	mp_input = NULL;
	m_cue_image_rectangle.left = 0.0f;
	m_cue_image_rectangle.right = 1.0f;
	m_cue_image_rectangle.top = 0.0f;
	m_cue_image_rectangle.bottom = 1.0f;

	mp_fixation.SetFile(Parameter("FixationPicture"));

	mp_fixation.SetZOrder(1.0);
	mp_fixation.SetDisplayRect(m_cue_image_rectangle);
	mp_fixation.SetAspectRatioMode(GUI::AspectRatioModes::AdjustBoth);
	mp_fixation.Conceal();
	m_staircase_steps.clear();
	m_staircase_steps.push_back(0);
	m_IsINoise.SetFile(Parameter("NoisePicture"));
	m_IsINoise.SetZOrder(0.0);
	m_IsINoise.SetDisplayRect(m_cue_image_rectangle);
	m_IsINoise.SetAspectRatioMode(GUI::AspectRatioModes::AdjustBoth);
	m_IsINoise.Conceal();

	if (cue_sound)
	{
		delete (cue_sound);
		cue_sound = NULL;
	}
	if (feedback_no)
	{
		delete (cue_sound);
		cue_sound = NULL;
	}

	if (feedback_yes)
	{
		delete (cue_sound);
		cue_sound = NULL;
	}

	if (string(Parameter("AuditoryCue")).empty())
		cue_sound = NULL;
	else
	{
		cue_sound = new WavePlayer();
		cue_sound->SetFile(Parameter("AuditoryCue"));
	}

	if (string(Parameter("AuditoryFeedbackYes")).empty())
		feedback_yes = NULL;
	else
	{
		feedback_yes = new WavePlayer();
		feedback_yes->SetFile(Parameter("AuditoryFeedbackYes"));
	}

	if (string(Parameter("AuditoryFeedbackNo")).empty())
		feedback_no = NULL;
	else
	{
		feedback_no = new WavePlayer();
		feedback_no->SetFile(Parameter("AuditoryFeedbackNo"));
	}


	m_cue_image_rectangle.left = 0.0f;
	m_cue_image_rectangle.right = 1.0f;
	m_cue_image_rectangle.top = 0.0f;
	m_cue_image_rectangle.bottom = 1.0f;


	m_FeedbackStimulus = NULL;

	for (unsigned int row = 0; row < m_num_stimuli; row++)
	{

		m_list_visual_stimuli.push_back(std::vector<ImageStimulus*>());
		for (uint8_t col = 1; col < PestStimuli->NumColumns(); col++)
		{
			ImageStimulus* stim = new ImageStimulus(mrDisplay);
			stim->SetFile(string(PestStimuli(row, col)));
			stim->SetPresentationMode(VisualStimulus::ShowHide);
			stim->SetDisplayRect(m_cue_image_rectangle);
			stim->SetAspectRatioMode(GUI::AspectRatioModes::AdjustBoth);
			stim->SetZOrder(0.0); //always on top
			stim->Conceal();
			m_list_visual_stimuli[row].push_back(stim);
		}

	}
	m_stimuliIdx.clear();
	m_list_isi_picture.clear();
	if (m_feedback_blocks != 0)
		for (int i = 0; i < IsIPictures->NumColumns(); i++)
		{
			ImageStimulus* stim = new ImageStimulus(mrDisplay);
			stim->SetFile(string(IsIPictures(0, i)));
			stim->SetPresentationMode(VisualStimulus::ShowHide);
			stim->SetDisplayRect(m_cue_image_rectangle);
			stim->SetAspectRatioMode(GUI::AspectRatioModes::AdjustBoth);
			stim->SetZOrder(0.0); //always on top
			stim->Conceal();
			m_list_isi_picture.push_back(stim);
		}

	vector<int> frequencies;
	for (int i = 0; i < PestStimuli->NumColumns() - 1; i++)
		frequencies.push_back(m_num_stimuli / PestStimuli->NumColumns());

	m_random_generator_orientation.SetFrequencies(frequencies);
	mp_stim = NULL;

}

void
ReactionVisualCalibrationTask::StartRun()
{

}

void ReactionVisualCalibrationTask::clearVisualStimuliMap()
{
	// Free memory from previously used visual stimuli
	for (unsigned int idx = 0; idx < m_list_visual_stimuli.size(); idx++)
	{
		m_list_visual_stimuli.clear();
	}
	m_list_visual_stimuli.clear();

	for (int i = 0; i < m_list_isi_picture.size(); i++)
	{
		delete m_list_isi_picture[i];
	}
	m_list_isi_picture.clear();
}

bool ReactionVisualCalibrationTask::VisualSensorCorrect(bool expected)
{
	bool isActive = false;
	int hightime = 0;
	if (m_overrideCalib)
		return true;

	for (unsigned int i = 0; i < m_block_size; i++)
	{

		if (m_use_Digital_input)
		{
			if (State("DigitalInput" + std::to_string(m_visual_sensor))(i) == 1)
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
			if (mp_input->Value(m_visual_sensor - 1, i) > m_visual_sensor_thresh)
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

bool ReactionVisualCalibrationTask::DoForceCalibration()
{
	return false;
	m_cal_steps++;
	switch (m_calibration)
	{
	case cal_black:
		if (m_cal_steps == 1)
		{

			mp_text_stimulus.SetText("Recalibrate black!");
			mp_text_stimulus.Present();
		}
		if ((m_cal_steps >= SENS_CALIB_DUR) && VisualSensorCorrect(false))
		{
			m_calibration = cal_white;
			mp_text_stimulus.Conceal();
			m_cal_steps = 0;
		}
		break;
	case cal_white:
		if (m_cal_steps == 1)
		{
			mp_text_stimulus.SetText("Recalibrate white!");
			mp_text_stimulus.Present();
			m_IsINoise.Present();
		}
		if ((m_cal_steps >= SENS_CALIB_DUR) && VisualSensorCorrect(true))
		{
			m_IsINoise.Conceal();
			m_calibration = cal_finished;
			mp_text_stimulus.Conceal();
			m_cal_steps = 0;
		}
		break;
	case cal_finished: //recheck black, if pass go to false start

		if ((m_cal_steps >= SENS_CALIB_DUR))
		{
			m_calibration = cal_black;
			m_cal_steps = 0;

			if (VisualSensorCorrect(false))
				return true;
		}
		break;

	}
	return false;
}

void
ReactionVisualCalibrationTask::Process(const GenericSignal& Input, GenericSignal& Output)
{

	Output = Input; // This passes the signal through unmodified.

	mp_input = &Input;

	m_runtime_counter++;

	if (m_trial_counter >= m_num_stimuli)
	{
		return;
	}

	// Go to first trial state if it is the first trial
	if (m_is_first_trial)
	{
		m_current_state = e_first_trial;
	}

	// keep track of the states
	State("ExperimentState") = m_current_state;
	State("CurrentTrial") = m_trial_counter + 1;
	State("Threshold") = abs(m_visual_intensity);
	State("StimOrientation") = (m_orientation)+1;
	switch (m_current_state)
	{

		//////////////////////////////////////////////////////////////////////////
	case e_first_trial:
	{
		m_block_delta++;
		if (m_block_delta == 1)
		{
			// This state provides instructions for the subject, e.g. Get ready

			// present "get ready" statement
			mp_text_stimulus.SetTextColor(RGBColor::Yellow);
			mp_text_stimulus.SetText("Get Ready...");
			mp_text_stimulus.Present();
		}
		if (m_block_delta == (2 * m_isi_duration_blocks - 2))
		{

			mp_text_stimulus.Conceal();
			mp_fixation.Present();
		}

		if (m_block_delta == (2 * m_isi_duration_blocks))
		{
			//if (DoForceCalibration ()) //create a loop until button is pressed
			//	m_block_delta--;
		}


		if (m_block_delta == 2 * m_isi_duration_blocks + 1)
		{

			// go to next state
			m_is_first_trial = false;
			if (!IsButtonPressed()) //create a loop until button is pressed
				m_block_delta--;
			else
				m_IsINoise.Present();

		}



		if (m_block_delta >= (3 * m_isi_duration_blocks))
		{
			m_IsINoise.Conceal();
			m_current_state = e_initial_staircase;
			m_block_delta = 0;
			m_is_first_trial = false;
		}


	}

	break;
	case e_wait_paradigm:
		m_block_delta++;
		if (m_block_delta == 1)
		{
			mp_text_stimulus.SetText("Press to start paradigm");
			mp_text_stimulus.Present();
		}

		if (m_block_delta == 10)
		{
			if (IsButtonPressed())
			{
				mp_text_stimulus.Conceal();
				m_block_delta = 0;
				m_current_state = e_cue;

			}
			else
				m_block_delta--;
		}

		break;

	case e_initial_staircase: //find treshold estimate
		m_block_delta++;
		if (Parameter("StaircaseTrials") == 0)
		{
			m_current_state = e_wait_paradigm;
			m_block_delta = 0;
		}


		if (IsButtonPressed())
		{
			StimulusOff();
			if (m_staircase_steps.size() > Parameter("StaircaseTrials"))
			{
				m_current_state = e_wait_paradigm;
				m_block_delta = 0;
				m_visual_intensity = Round(std::accumulate(m_staircase_steps.begin(), m_staircase_steps.end(), 0.0) / m_staircase_steps.size());
				m_PestStartLevel = m_visual_intensity;
				AppLog << "Finished mean Staircase result: " << to_string(m_visual_intensity) << endl;
			}
			else
			{
				AppLog << "Initial Staircase result: " << to_string(m_staircase_steps.back()) << endl;
				m_staircase_steps.push_back(0);
				m_block_delta = 0;
				m_current_state = e_wait_button;
			}
		}
		if (m_block_delta == 1)
		{
			StimulusOff();
			AppLog << "Stim off at " << m_block_delta << endl;

		}

		if (m_block_delta >= m_stimuli_duration_blocks/2)
		{
			//StimulusOff();
			if (m_staircase_steps.back()++ >= m_attenuation_max)
				m_staircase_steps.back() = m_attenuation_max;

			PrepareStimulus(m_staircase_steps.back());
			StimulusOn();
			AppLog << "Stim on at " << m_block_delta << endl;
			m_block_delta = 0;
		}

		break;

	case e_wait_button:
		m_block_delta++;
		if (m_block_delta == 1)
		{
			m_IsINoise.Present();
		}
		if (!IsButtonPressed() && m_block_delta >= m_isi_duration_blocks) //wait until button is released
		{
			m_IsINoise.Conceal();
			m_current_state = e_initial_staircase;
			m_block_delta = 0;
		}

		break;
		///////////////////////////////////////////////////////////////////////
	case e_cue:
	{

		State("Performance") = 0;
		//State("StimulusLevel") = 0;
		m_block_delta++;

		if (m_block_delta == 1)
		{

			AppLog << "=============================================" << endl;
			AppLog << "---------------------------------------------" << endl;
			AppLog << "Trial #" << m_Pesttrialcounter + 1 << endl;
			AppLog << "---------------------------------------------" << endl;
			AppLog << "State: Presenting cues ..." << endl;
			AppLog << "stimulus Intensity : " << m_visual_intensity << endl;
			State("StimulusLevel") = abs(m_visual_intensity);
			PrepareStimulus(m_visual_intensity);
			if (m_cue_duration_blocks != 0 && cue_sound != NULL)
				cue_sound->Play();
		}



		if (m_PestStepsize <= m_PestFinalStepSize)
		{
			AppLog << "==== Finished !!! =====" << endl;
			State("Running") = 0;
			mp_fixation.Conceal();
			mp_text_stimulus.SetText("Finished! ");
			mp_text_stimulus.Present();


			SummarizeExperiment();
			m_current_state = e_first_trial;
		}



		if (m_block_delta >= m_cue_duration_blocks)
		{

			if (cue_sound != NULL && cue_sound->IsPlaying())
				cue_sound->Stop();

			if (IsButtonPressed())
			{
				m_current_state = e_false_start;
				m_block_delta = 0;
			}
			else
			{
				m_current_state = e_pre_stimuli;
				m_block_delta = 0;
			}
		}

	}
	break;
	case	e_fixation_violated:
		m_block_delta++;
		if (m_block_delta == 1)
		{
			AppLog << "Fixation violated" << endl;
			mp_text_stimulus.SetText("Fixation Violated");
			mp_text_stimulus.Present();
		}

		if (m_block_delta >= m_isi_duration_blocks / 2)
		{
			m_block_delta = 0;
			mp_text_stimulus.Conceal();
			m_current_state = e_false_start;
		}
		break;
		////////////////////////////////////////////////////////////////////
	case e_pre_stimuli:
	{
		m_block_delta++;


		if (m_block_delta == 1)
		{
			m_rand_blocks = RandomNumber(m_wait_min_blocks, m_wait_max_blocks);	// random delay in presenting the auditory stimuli after CueOff
			AppLog << "State: Waiting for " << ((float)(m_rand_blocks)* m_block_size_msec) << "ms ..." << endl;
			StimulusOff();
		}

		if ((m_block_delta >= m_rand_blocks / 2) && OptionalState("FixationViolated", 0) && OptionalParameter("EnforceFixation", 0))
		{

			m_block_delta = 0;
			m_current_state = e_fixation_violated;
			StimulusOff();
		}

		if ((m_block_delta >= SENS_CALIB_DUR) && !VisualSensorCorrect(false))
		{

			m_block_delta = 0;
			m_current_state = e_force_calibration;
		}

		if (IsButtonPressed())
		{
			m_current_state = e_false_start;
			m_block_delta = 0;
		}

		if (m_block_delta >= m_rand_blocks)
		{
			m_current_state = e_stimuli;
			m_block_delta = 0;
		}

	}
	break;
	///////////////////////////////////////////////////////////////////
	case e_stimuli:
	{
		m_block_delta++;
		if (OptionalState("FixationViolated", 0) && OptionalParameter("EnforceFixation", 0))
		{

			m_block_delta = 0;
			m_current_state = e_fixation_violated;
			StimulusOff();
		}
		if (m_block_delta == 1)
		{
			AppLog << "State: Presenting Visual stimuli ..." << endl;
			StimulusOn();// Visual stimuli onset

		}

		if ((m_block_delta >= SENS_CALIB_DUR) && !VisualSensorCorrect(true))
		{
			StimulusOff();
			m_block_delta = 0;
			m_current_state = e_force_calibration;
		}

		if (m_block_delta == m_stim_visible_blocks)
		{
			StimulusOff();
		}

		// Wait for a duration or a push botton and then go to the next state
		if (IsButtonPressed() || (m_block_delta >= m_stimuli_duration_blocks))
		{
			StimulusOff();
			if (IsButtonPressed())
			{
				if (ReactionTime() <= m_max_rt & ReactionTime() >= m_min_rt)
				{
					m_pushed_botton_flag = 1;
					m_reaction_time.push_back(ReactionTime());// Recording the reaction time if there is a botton press
					m_PEST_performance.push_back(m_pushed_botton_flag);// Recording the performance of subject, pushed the botton or not?
					m_stimulus_Level.push_back(m_visual_intensity); // recording the intensity of the presented stimuli

					State("Performance") = (unsigned int)(m_pushed_botton_flag);
					// Offset of the stimuli

					m_current_state = e_feedback;
					m_block_delta = 0;
				}
				else if (ReactionTime() < m_min_rt || ReactionTime() > m_max_rt)
				{
					m_current_state = e_false_start;
					m_block_delta = 0;
				}
			}
			else if (!IsButtonPressed() & m_block_delta != 0)
			{
				m_pushed_botton_flag = 0;
				m_reaction_time.push_back(ReactionTime());
				m_PEST_performance.push_back(m_pushed_botton_flag);// Recording the performance of subject, pushed the botton or not?
				m_stimulus_Level.push_back(m_visual_intensity); // recording the intensity of the presented stimuli
				State("Performance") = (unsigned int)(m_pushed_botton_flag);
				m_current_state = e_feedback;
				m_block_delta = 0;
			}

		}


	}
	break;
	////////////////////////////////////////////////////////////////
	case e_feedback:
	{
		m_block_delta++;

		if (m_block_delta == 1)
		{
			AppLog << "State: Presenting feedback ..." << endl;
			if ((m_FeedbackStimulus != NULL) && (m_feedback_blocks != 0))
				m_FeedbackStimulus->Present();
			//VisualMarkerOff();

			// present "Good job!" statement if the subject pushed the botton
			if (m_pushed_botton_flag == 1)
			{
				AppLog << "State: Response time = " << m_reaction_time[m_Pesttrialcounter] << "ms ..." << endl;
				m_Pest_num_correct++;
				if (m_feedback_blocks != 0 && feedback_yes != NULL)
					feedback_yes->Play();
				/*mp_text_stimulus->SetTextColor(RGBColor::Yellow);
				mp_text_stimulus->SetText("Good job!");
				mp_text_stimulus-> Present();*/

			}
			else
			{
				if (m_feedback_blocks != 0 && feedback_no != NULL)
					feedback_no->Play();
			}

		}

		// present the feedback text for a while and then go to next state
		if (m_block_delta >= m_feedback_blocks)
		{
			if ((m_FeedbackStimulus != NULL) && (m_feedback_blocks != 0))
				m_FeedbackStimulus->Conceal();

			m_current_state = e_isi;
			m_block_delta = 0;
		}
	}
	break;
	/////////////////////////////////////////////////////////////
	case e_isi:
	{
		m_block_delta++;

		if (m_block_delta == 1)
		{
			bciout << "Isi start";
			AppLog << "State: Inter Stimulus Interval (ISI) ..." << endl;
			if (m_isi_duration_blocks != 0)
				m_IsINoise.Present();
			//mp_fixation->Conceal();
			// mp_text_stimulus->Conceal();
		}

		if ((m_block_delta >= SENS_CALIB_DUR) && !VisualSensorCorrect(true))
		{
			m_IsINoise.Conceal();
			m_block_delta = 0;
			m_current_state = e_force_calibration;
		}

		// wait for isi duration and then go to next state
		if (m_block_delta >= m_isi_duration_blocks)
		{
			bciout << "Isi end";

			if (m_isi_duration_blocks != 0)
				m_IsINoise.Conceal();
			//mp_fixation->Present();
			if (m_use_pest)
				m_current_state = e_PEST;
			else
				m_current_state = e_staircase;
			m_block_delta = 0;
		}
	}
	break;
	///////////////////////////////////////////////////////////
	case e_prePEST:
	{
		m_block_delta++;

		if (m_block_delta == 1)
		{
			if (m_PestStepsize > m_PestFinalStepSize)
			{
				m_current_state = e_PEST;
				m_block_delta = 0;
			}
		}
	}
	break;
	case e_staircase:
		if (m_stimuliIdx.empty())
		{
			std::vector<int> freq;
			for (int i = int(m_PestStartLevel) - 2; i <= int(m_PestStartLevel) + 2; i++)
			{
				if (i <= m_attenuation_max && i >= m_attenuation_min)
				{
					freq.push_back(m_max_num_trials);
					m_stimuliIdx.push_back(i);
				}
			}

			for (int i = 0; i < freq.size(); i++)
				freq[i] /= m_stimuliIdx.size();

			m_random_block.SetFrequencies(freq);
		}
		m_visual_intensity = m_stimuliIdx.at(m_random_block.NextElement() - 1);
		m_Pesttrialcounter++;

		if (m_Pesttrialcounter == m_max_num_trials)
		{
			mp_fixation.Conceal();
			mp_text_stimulus.SetTextColor(RGBColor::Yellow);
			mp_text_stimulus.SetText("End of experiment");
			mp_text_stimulus.Present();
			AppLog << "==== Finished !!! =====" << endl;
			State("Running") = 0;
			m_current_state = e_first_trial;
			m_block_delta = 0;
			SummarizeExperiment();
		}
		else
		{
			m_current_state = e_cue;
			m_block_delta = 0;
		}


		break;
		////////////////////////////////////////////////////////////
	case e_PEST:
	{
		m_block_delta++;
		if (m_block_delta == 1)
		{

			m_Pesttrialcounter++;

			if (m_Pesttrialcounter == m_max_num_trials)
			{
				mp_fixation.Conceal();
				mp_text_stimulus.SetTextColor(RGBColor::Yellow);
				mp_text_stimulus.SetText("End of experiment");
				mp_text_stimulus.Present();
				AppLog << "==== Finished !!! =====" << endl;
				State("Running") = 0;
				SummarizeExperiment();
			}

			if (((m_PestTargetPerformance * m_num_trials) - m_PestWaldFactor < m_Pest_num_correct) && (m_Pest_num_correct < (m_PestTargetPerformance * m_num_trials) + m_PestWaldFactor))
			{
				m_num_trials++;
				m_current_state = e_cue;
				m_block_delta = 0;
				m_Pest_samelevelcounter++;
			}
			else
			{
				m_Pest_samelevelcounter = 0;

				if (m_Pest_num_correct >= ((m_num_trials*m_PestTargetPerformance) + m_PestWaldFactor))
				{
					m_Pest_history_correct.append("d");
				}
				if (m_Pest_num_correct <= ((m_num_trials*m_PestTargetPerformance) - m_PestWaldFactor))
				{
					m_Pest_history_correct.append("u");
				}

				// PEST rule #1
				if (m_Pest_history_correct.length() > 1)
				{
					m_historylength = m_Pest_history_correct.length();
					a = m_Pest_history_correct[m_historylength - 1];
					b = m_Pest_history_correct[m_historylength - 2];

					if (a.compare(b) != 0)
						m_PestStepsize = m_PestStepsize / 2;
				}

				// PEST rule #3,4
				if (m_Pest_history_correct.length() > 2)
				{
					if (m_Pest_history_correct.length() == 3)
					{
						m_historylength = m_Pest_history_correct.length();
						a = m_Pest_history_correct.substr(m_historylength - 3);

						if (!(a.compare("ddd")) || !(a.compare("uuu")))
						{
							m_PestStepsize = m_PestStepsize * 2;
							m_Pest_doubled = 1;

						}
					}
					if (m_Pest_history_correct.length() > 3)
					{
						m_historylength = m_Pest_history_correct.length();
						a = m_Pest_history_correct.substr(m_historylength - 4);

						if (!(a.compare("uddd")) || !(a.compare("duuu")))
						{
							if (m_Pest_doubled == 1)
								m_Pest_doubled = 0;
							else
							{
								m_PestStepsize = m_PestStepsize * 2;
								m_Pest_doubled = 1;
							}

						}
						if (!(a.compare("uuuu")) || !(a.compare("dddd")))
						{
							m_PestStepsize = m_PestStepsize * 2;
							m_Pest_doubled = 1;
						}
					}
				}


				// Exception to rule #3 (TAYLOR & CREELMAN, 1967)
				if (m_PestStepsize > m_PestMaxstepsize)
					m_PestStepsize = m_PestMaxstepsize;




				m_historylength = m_Pest_history_correct.length();
				a = m_Pest_history_correct.substr(m_historylength - 1);

				if (!(a.compare("u")))
				{
					m_visual_intensity = m_visual_intensity + m_PestStepsize;
					AppLog << "intensity is increased" << endl;
				}
				else
				{
					m_visual_intensity = m_visual_intensity - m_PestStepsize;
					AppLog << "intensity is decreased" << endl;
				}

				if (m_visual_intensity <= m_PestMinlevel)
					m_visual_intensity = m_PestMinlevel + m_PestFinalStepSize;

				m_num_trials = 0;
				m_Pest_num_correct = 0;
				m_current_state = e_cue;
				m_block_delta = 0;
			}

		}
	}
	break;
	//////////////////////////////
	case e_false_start:
		m_block_delta++;

		if (m_block_delta == 1)
		{
			AppLog << "State: Inter Stimulus Interval (ISI) ..." << endl;

			//mp_fixation->Conceal();
			// mp_text_stimulus->Conceal();
		}

		if (m_block_delta == (m_isi_duration_blocks / 2)) //
		{

			m_IsINoise.Present();
		}


		// wait for isi duration and then go to next state
		if (m_block_delta >= m_isi_duration_blocks)
		{
			m_IsINoise.Conceal();
			//mp_fixation->Present();
			m_current_state = e_cue;
			m_block_delta = 0;
		}

		break;

	case e_force_calibration:
		m_block_delta++;
		m_block_delta = 0;
		m_current_state = e_false_start;
		if (!wait_for_buttonpress)
		{
			//	if (DoForceCalibration ())
		//			wait_for_buttonpress = true;
		}
		else if (IsButtonPressed())
		{
			m_block_delta = 0;
			m_current_state = e_false_start;
			wait_for_buttonpress = false;
		}
		break;
		///////////////////////////////////////
	}
}

void
ReactionVisualCalibrationTask::StopRun()
{
	Parameter("PerceptionThreshold") = m_visual_intensity;

	std::ofstream f(Parameter("PerceptionThresholdFilePath"));
	f << Parameters->ByPath("PerceptionThreshold");

	//	Parameters->ByName("PerceptionThreshold").Serialize(f);

	f.close();

}

void
ReactionVisualCalibrationTask::Halt()
{
	// Stop any threads or other asynchronous activity.
	// Good practice is to write the Halt() method such that it is safe to call it even *before*
	// Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
	// have already been deleted:  set them to NULL after deletion).
}

unsigned int
ReactionVisualCalibrationTask::CalculateStimulusIndex(float attenuation)
{


	float k = (float)(m_num_stimuli - 1) / (float)(m_attenuation_max - m_attenuation_min);
	float d = (float)(m_num_stimuli - 1) - (float)(k * m_attenuation_max);

	float y = attenuation * k + d;
	int index = float(y);

	if (index > m_num_stimuli - 1)
		index = m_num_stimuli - 1;

	if (index < 0)
		index = 0;

	return (index);

}
void ReactionVisualCalibrationTask::PrepareStimulus(float attenuation)
{
	bciout << "Preparation of Stimuli " << attenuation << endl;
	unsigned int index = CalculateStimulusIndex(attenuation);
	m_orientation = m_random_generator_orientation.NextElement() - 1;
	AppLog << "Orientation: " << to_string(m_orientation + 1) << "of " << to_string(m_list_visual_stimuli[index].size()) << endl;
	mp_stim = m_list_visual_stimuli[index].at(m_orientation);
	if (m_feedback_blocks != 0)
		m_FeedbackStimulus = m_list_isi_picture[m_orientation];
}

void
ReactionVisualCalibrationTask::StimulusOn()
{

	bciout << "visual stimulus started" << endl;
	if (mp_stim != NULL)
		mp_stim->Present();


}

void
ReactionVisualCalibrationTask::StimulusOff()
{
	//if(mp_stim->MayBePresented())
	if (mp_stim != NULL)
		mp_stim->Conceal();
	bciout << "visual stimulus stopped" << endl;
}

bool
ReactionVisualCalibrationTask::IsButtonPressed()
{

	//Check for keypresses
	bool press = false;

	if (m_use_Digital_input)
	{
		for (unsigned int i = 0; i < m_block_size; i++)
		{
			if (State("DigitalInput" + std::to_string(m_push_button_channel))(i) == 1)
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
			if (State("KeyDown")(i) == VK_SPACE)
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


			if (mp_input->Value(m_push_button_channel - 1, i) > m_push_button_threshold)
			{
				return true;
			}

		}
	}

	return press;
}

int
ReactionVisualCalibrationTask::RandomNumber(int min, int max)
{
	int random_number = min + m_random_generator.Random() % (max - min);

	//bciout << random_number << endl;

	return random_number;
}

float ReactionVisualCalibrationTask::ReactionTime()
{
	unsigned int offset = 0;
	unsigned int i = 0;
	bool press = false;
	if (m_use_Digital_input)
	{
		for (i = 0; i < m_block_size; i++)
		{
			if (State("DigitalInput" + std::to_string(m_push_button_channel))(i) == 1)
			{
				press = true;
				offset = i;
				break;
				//bciout << "key pressed" << endl;
			}
		}
	}
	else
		if (m_use_space_button)
		{
			//Check for keypresses
			for (i = 0; i < m_block_size; i++)
			{
				if (State("KeyDown")(i) == VK_SPACE)
				{
					press = true;
					offset = i;
					break;
				}
			}
		}
		else
			if (mp_input != NULL)
			{
				for (i = 0; i < m_block_size; i++)
				{


					if (mp_input->Value(m_push_button_channel - 1, i) > m_push_button_threshold)
					{
						press = true;
						offset = i;
						//					bciout << "button pressed with offset " << offset << endl;
						break;
					}
				}
			}




	if (press)
	{
		return (float)(m_block_delta - 2) * m_block_size_msec + (float)(offset + 1) / (float)(m_block_size)* m_block_size_msec;
	}
	else
	{
		return -1;
	}

}
void ReactionVisualCalibrationTask::SummarizeExperiment()
{
	float rt_sumation = 0;
	unsigned int rt_counter = 0;


	for (unsigned int i = 0; i < m_reaction_time.size(); i++)
	{
		if (m_reaction_time[i] != -1)
		{
			rt_sumation += m_reaction_time[i];
			rt_counter++;
		}
	}

	AppLog << "=============================================" << endl;
	AppLog << "*************** S U M M A R Y ***************" << endl;
	AppLog << "=============================================" << endl;
	AppLog << "---------------------------------------------" << endl;
	//AppLog << "Total Runtime: " << ( m_Pesttrialcounter * m_block_size_msec ) / 1000 << "s" << endl;
	AppLog << "Total trials: " << m_Pesttrialcounter + 1 << endl;
	AppLog << "Total trials push botton: " << rt_counter << endl;
	AppLog << "Total missed trials: " << m_reaction_time.size() - rt_counter << endl;

	AppLog << "average reaction time: " << rt_sumation / rt_counter << endl;

	AppLog << "stimulus level: " << " ";
	for (unsigned int i = 0; i < m_stimulus_Level.size(); i++)
	{
		AppLog << m_stimulus_Level[i] << " ";
	}
	AppLog << endl;

	AppLog << "subject performance: " << " ";
	for (unsigned int i = 0; i < m_PEST_performance.size(); i++)
	{
		AppLog << m_PEST_performance[i] << " ";
	}
	AppLog << endl;
	m_stimulus_Level_copy = m_stimulus_Level;
	std::sort(m_stimulus_Level_copy.begin(), m_stimulus_Level_copy.end());
	auto last = std::unique(m_stimulus_Level_copy.begin(), m_stimulus_Level_copy.end());
	m_stimulus_Level_copy.erase(last, m_stimulus_Level_copy.end());

	AppLog << "unique stimulus level: " << " ";
	for (unsigned int i = 0; i < m_stimulus_Level_copy.size(); i++)
	{
		AppLog << m_stimulus_Level_copy[i] << " ";
	}
	AppLog << endl;
	float counter;
	float summation_count;
	AppLog << "Performance percentage: " << " ";
	for (unsigned int i = 0; i < m_stimulus_Level_copy.size(); i++)
	{
		counter = 0;
		summation_count = 0;
		for (unsigned int j = 0; j < m_stimulus_Level.size() - 1; j++)
		{
			if (m_stimulus_Level_copy[i] == m_stimulus_Level[j])
			{
				counter++;
				summation_count += m_PEST_performance[j];
			}
		}
		m_Pest_percentage_check.push_back(summation_count / counter);
		AppLog << m_Pest_percentage_check[i] << " ";
	}

	int high_idx = std::upper_bound(m_Pest_percentage_check.begin(), m_Pest_percentage_check.end(), m_PestTargetPerformance) - m_Pest_percentage_check.begin() - 1;

	if ((high_idx != 0) && (m_PestTargetPerformance - m_Pest_percentage_check[high_idx - 1]) < (m_Pest_percentage_check[high_idx] - m_PestTargetPerformance))
		m_visual_intensity = m_stimulus_Level_copy[high_idx - 1];
	else
		m_visual_intensity = m_stimulus_Level_copy[high_idx];

	State("Threshold") = abs(m_visual_intensity);
	AppLog << "perception threshold: " << m_visual_intensity << endl;
	AppLog << endl;
}
