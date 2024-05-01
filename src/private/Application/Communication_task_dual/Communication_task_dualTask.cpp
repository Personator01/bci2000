////////////////////////////////////////////////////////////////////////////////
// Authors: lingl@DESKTOP-GH0R7CA.wucon.wustl.edu
// Description: Communication_task_dualTask implementation
////////////////////////////////////////////////////////////////////////////////

#include "Communication_task_dualTask.h"
#include "BCIStream.h"
#include <math.h>  
#include <fstream>

Slider* slider = nullptr;

RegisterFilter( Communication_task_dualTask, 3 );
     // Change the location if appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations within SignalProcessing modules begin with "2."
     //       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
     //  - filter locations within Application modules begin with "3."


Communication_task_dualTask::Communication_task_dualTask() :
  mrDisplay( Window() )
{
  // C++ does not initialize simple types such as numbers, or pointers, by default.
  // Rather, they will have random values.
  // Take care to initialize any member variables here, so they have predictable values
  // when used for the first time.
}

Communication_task_dualTask::~Communication_task_dualTask()
{
  Halt();
  if (my_slider_1) delete my_slider_1;
  if (my_slider_2) delete my_slider_2;
  if (m_img_container) delete m_img_container;
  if (slider) slider = nullptr;
  if (m_instruction_container) delete m_instruction_container;
  if (m_dim_1a_txt) delete m_dim_1a_txt;
  if (m_dim_1b_txt) delete m_dim_1b_txt;
  if (m_dim_2a_txt) delete m_dim_2a_txt;
  if (m_dim_2b_txt) delete m_dim_2b_txt;
  if (m_opt_1_txt) delete m_opt_1_txt;
  if (m_opt_2_txt) delete m_opt_2_txt;
  if (m_feedback_container) delete m_feedback_container;
  if (m_role_txt) delete m_role_txt;

  // If you have allocated any memory with malloc(), call free() here.
  // If you have allocated any memory with new[], call delete[] here.
  // If you have created any object using new, call delete here.
  // Either kind of deallocation will silently ignore null pointers, so all
  // you need to do is to initialize those to zero in the constructor, and
  // deallocate them here.
}

void
Communication_task_dualTask::SharedPublish()
{
    std::string instructs_vec = "";
    int instructs_size = 10;
    for (int i = 0; i < instructs_size; i++) {
        instructs_vec += "../tasks/Communication_task/Instructions/Slide" + std::to_string(i) + ".jpg ";
    }

  // Define any parameters that the filter needs....
 BEGIN_PARAMETER_DEFINITIONS
    //Stimuli
    "Application:Stimuli matrix StimuliMatrix= "
    //"{Sender ImageReceiver ImageSender Dimension1a Dimension1b Dimension2a Dimension2b Option1 Option2 Jitter CorrectResponse Training Category Difficulty} "
	//
	"1 1 auto" 
    //"{} "
    " // Stimuli information matrix",
    "Application:Stimuli int StimuliWidth= 15 0 0 100"
    " // StimulusWidth in percent of screen width (zero for original pixel size)",
     //Sequence
    "Application:Sequence intlist InstructionImagesSeque= " + std::to_string(instructs_size) + " " + instructs_vec + " % % % "
    "//Sequence of instruction images path",
     "Application:Sequence int InstructionWidth= 100 0 0 100"
     " // Instruction width in percent of screen width (zero for original pixel size)",
    "Application:Sequence intlist BreakTrialSequece= 1 auto % % "
     "//Sequence of break trials",
     "Application:Sequence float FeedbackDuration= auto auto 0 % "
     " //Duration of feedback(ITI)",
	 //"Application:Sequence intlist StimuliSequence= 1 auto % % "
	 //" // Sequence of stimuli images",
    //Experiment
     //slider
     "Application:Experiment int SliderWidth= 35 0 0 100"
     " // SliderWidth in percent of screen width (zero for original pixel size)",
    "Application:Experiment string BarColor= 0xA2A2A2 0xA2A2A2 0x00000000 0xFFFFFFFF // "
    "Color of slider bar (color)",
    "Application:Experiment string AxeActiveColor= 0xFF0000 0xFF0000 0x00000000 0xFFFFFFFF // "
    "Color of active slider axe (color)",
    "Application:Experiment string AxeInActiveColor= 0x000000 0x000000 0x0000RoleHeight0000 0xFFFFFFFF // "
    "Color of inactive slider axe (color)",
    "Application:Experiment string DimensionFontColor= 0xA2A2A2 0xA2A2A2 0x00000000 0xFFFFFFFF // "
    "Color of the text under the slider (color)",
    "Application:Experiment int DimensionHeight= 2 2 0 % // "
    "Height of text in percent of screen height",
     //option
     "Application:Experiment string OptionFontActiveColor= 0x000000 0x000000 0x00000000 0xFFFFFFFF // "
     "Color of the active options (color)",
     "Application:Experiment string OptionFontInActiveColor= 0xA2A2A2 0xA2A2A2 0x00000000 0xFFFFFFFF // "
     "Color of the inactive options (color)",
     "Application:Experiment int OptionHeight= 3 3 0 % // "
     "Height of text in percent of screen height",
     //sender/reciever text
     "Application:Experiment string RoleFontColor= 0x000000 0x000000 0x00000000 0xFFFFFFFF // "
     "Color of the role text (color)",
     "Application:Experiment int RoleHeight= 3 3 0 % // "
     "Height of text in percent of screen height",
     //feedback
     "Application:Experiment int FeedbackWidth= 2 2 0 % // "
     "Width of feedback in percent of screen width",

     //photodiode
     "Application:PhotoDiodePatch int PhotoDiodePatch= 1 1 0 1 "
     "// Display photo diode patch (boolean)",
     "Application:PhotoDiodePatch float PhotoDiodePatchHeight= 0.065 1 0 1 "
     "// Photo diode patch height in relative coordinates",
     "Application:PhotoDiodePatch float PhotoDiodePatchWidth= 0.05 1 0 1 "
     "// Photo diode patch width in relative coordinates",
     "Application:PhotoDiodePatch float PhotoDiodePatchLeft= 0 1 0 1 "
     "// Photo diode patch left in relative coordinates",
     "Application:PhotoDiodePatch float PhotoDiodePatchTop= 0.935 1 0 1 "
     "// Photo diode patch top in relative coordinates",
     "Application:PhotoDiodePatch int PhotoDiodePatchShape= 0 0 0 1 "
     "// Photo diode patch shape: 0 rectangle, 1 ellipse (enumeration)",
     "Application:PhotoDiodePatch int PhotoDiodePatchActiveColor= 0x0 0 0 0xffffffff "
     "// Photo diode patch color when active (color)",
     "Application:PhotoDiodePatch int PhotoDiodePatchInactiveColor= 0xffffff 0 0 0xffffffff "
     "// Photo diode patch color when inactive, use 0xff000000 for transparent (color)",

	 //InitialTrialNumber
	 //"Application:Sequence int InitialTrialNumber= auto auto % %"
	 //" // Trial number to start on",
 END_PARAMETER_DEFINITIONS
}

void
Communication_task_dualTask::SharedPreflight( const SignalProperties& Input, SignalProperties& Output ) const
{
    float FeedbackDuration = Parameter("FeedbackDuration").InSampleBlocks();
    float sampleRate = Parameter("SamplingRate");//Sampling rate
    unsigned int blockSize = Parameter("SampleBlockSize");// Sample block size
    float sBlockMs = ((float)(blockSize) / (float)(sampleRate)) * 1000;

    if (Parameter("StimuliMatrix")->NumColumns() == 0) {
        bcierr << "StimuliMatrix should't be empty!" << std::endl;
    }
    if (Parameter("InstructionImagesSeque")->NumValues() == 0) {
        bcierr << "InstructionImagesSeque should't be empty!" << std::endl;
    }
    if (Parameter("BreakTrialSequece")->NumValues() == 0) {
        bcierr << "BreakTrialSequece should't be empty!" << std::endl;
    }
    if (FeedbackDuration < 1) {
        bcierr << "FeedbackDuration must be >= 1 sampleBlock duration(" + std::to_string(sBlockMs) + " ms)." << std::endl;
    }
    if (Parameter("InitialTrialNumber") < 0) {
        bciout<< "InitialTrialNumber is " << Parameter("InitialTrialNumber") << ", which should't be greater or equal to 0!" << std::endl;
    }
    if (Parameter("StimuliSequence")->NumValues() == 0) {
        bcierr << "StimuliSequence should't be empty!" << std::endl;
    }

    //check the resource path!

    Parameter("StimuliWidth");
    Parameter("BarColor");
    Parameter("AxeActiveColor");
    Parameter("AxeInActiveColor");
    Parameter("DimensionFontColor");
    Parameter("DimensionHeight"); 
    Parameter("OptionFontActiveColor");
    Parameter("OptionHeight");
    Parameter("RoleFontColor");
    Parameter("RoleHeight");
    Parameter("FeedbackWidth");
    Parameter("SliderWidth");
    Parameter("InstructionWidth");
    //Parameter("InitialTrialNumber");

    State("JoystickButtons1");
    State("JoystickButtons2");
    State("JoystickButtons3");
    State("JoystickButtons4");
    State("JoystickButtons9");
    State("JoystickButtons10");
    State("KeyDown");
    State("TrialNumber");
    State("PhaseNumber");
    State("BarOneValue");
    State("BarTwoValue");
    State("BarOneActive");
    State("BarTwoActive");
    State("ResponseValue");
    State("isReadyStart0");
    State("isReadyStart1");
    State("SenderLock");
    State("ReceiverLock");
    State("ClientNumber");
    State("StimulusCode");

  Output = Input; // this simply passes information through about SampleBlock dimensions, etc....

}


void
Communication_task_dualTask::SharedInitialize( const SignalProperties& Input, const SignalProperties& Output )
{
  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The signal properties can no longer be modified, but the const limitation has gone, so
  // the Communication_task_dualTask instance itself can be modified. Allocate any memory you need, start any
  // threads, store any information you need in private member variables.
    
    //initialize from parameters
    m_sample_rate = Parameter("SamplingRate");
    m_block_size = Parameter("SampleBlockSize");
    feedback_dur_in_block = Parameter("FeedbackDuration").InSampleBlocks();

    //random sequence
    random_sequence.clear();
    StringUtils::String s_s;
    for (int i = 0; i < Parameter("StimuliSequence")->NumValues(); i++) {
        random_sequence.push_back(Parameter("StimuliSequence")(i));
        s_s += std::to_string( Parameter("StimuliSequence")(i) ) + ",  ";
    }
    AppLog << "Random sequence: " << s_s << std::endl;

    //StimuliMatrix
    img_recv_v.clear(); img_send_v.clear(); dimt_1a_v.clear(); dimt_1b_v.clear(); dimt_2a_v.clear(); dimt_2b_v.clear();
    opt_1_v.clear(); opt_2_v.clear(); jitter_v.clear(); sender_v.clear(); cor_rsps_v.clear();
    
    for (int i = 0; i < Parameter("StimuliMatrix")->NumColumns(); i++) {
        if (Parameter("StimuliMatrix")->RowLabels().Exists("Sender")) {
            sender_v.push_back(Parameter("StimuliMatrix")("Sender", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("ImageReceiver")) {
            img_recv_v.push_back(Parameter("StimuliMatrix")("ImageReceiver", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("ImageSender")) {
            img_send_v.push_back(Parameter("StimuliMatrix")("ImageSender", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("Dimension1a")) {
            dimt_1a_v.push_back(Parameter("StimuliMatrix")("Dimension1a", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("Dimension1b")) {
            dimt_1b_v.push_back(Parameter("StimuliMatrix")("Dimension1b", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("Dimension2a")) {
            dimt_2a_v.push_back(Parameter("StimuliMatrix")("Dimension2a", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("Dimension2b")) {
            dimt_2b_v.push_back(Parameter("StimuliMatrix")("Dimension2b", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("Option1")) {
            opt_1_v.push_back(Parameter("StimuliMatrix")("Option1", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("Option2")) {
            opt_2_v.push_back(Parameter("StimuliMatrix")("Option2", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("Jitter")) {
            jitter_v.push_back(Parameter("StimuliMatrix")("Jitter", random_sequence[i]));
        }
        if (Parameter("StimuliMatrix")->RowLabels().Exists("CorrectResponse")) {
            cor_rsps_v.push_back(Parameter("StimuliMatrix")("CorrectResponse", random_sequence[i]));
        }
    }
    trial_num = sender_v.size();
    //inver Sender if needed
    //for test
    //client_number = 1;
    //client_number = State("ClientNumber");
    //if (client_number) {//need to reverse
    //    for (int i = 0; i < sender_v.size(); i++) {
    //        sender_v[i] = (sender_v[i] == 1 ? 2 : 1);
    //    }
    //}
    is_reverse_sender = false;
     
    
    //sequece
    instructs_list.clear();
    break_trial_list.clear();
    for (int i = 0; i < Parameter("InstructionImagesSeque")->NumValues(); i++) {
        instructs_list.push_back(Parameter("InstructionImagesSeque")(i));
    }
    for (int i = 0; i < Parameter("BreakTrialSequece")->NumValues(); i++) {
        break_trial_list.push_back(Parameter("BreakTrialSequece")(i));
    }

    //GUI
    //slider
    if (my_slider_1) delete my_slider_1;
    if (my_slider_2) delete my_slider_2;
    my_slider_1 = nullptr;
    my_slider_2 = nullptr;

    int slider_size_mode = GUI::ScalingMode::AdjustHeight;
    double slider_width = Parameter("SliderWidth") / 100.0;
    if (slider_width < 0) {
        slider_size_mode = GUI::ScalingMode::AdjustBoth;
    }
    my_slider_1 = new Slider(mrDisplay);
    GUI::Rect rect_1 = { (1 - slider_width) / 2, 0.65, (1 + slider_width) / 2, 0.67 };
    my_slider_1->SetObjectRect(rect_1);
    my_slider_1->SetLineColor(RGBColor(Parameter("BarColor")));
    my_slider_1->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
    my_slider_1->SetScalingMode(slider_size_mode);
    my_slider_1->SetVisible(false);

    my_slider_2 = new Slider(mrDisplay);
    GUI::Rect rect_2 = { (1 - slider_width) / 2, 0.8, (1 + slider_width) / 2, 0.82 };
    my_slider_2->SetObjectRect(rect_2);
    my_slider_2->SetLineColor(RGBColor(Parameter("BarColor")));
    my_slider_2->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
    my_slider_2->SetScalingMode(slider_size_mode);
    my_slider_2->SetVisible(false);

    //image
    if (m_img_container) delete m_img_container;
    m_img_container = nullptr;

    int img_size_mode = GUI::ScalingMode::AdjustHeight;
    double stimulus_width = Parameter("StimuliWidth") / 100.0;
    if (stimulus_width < 0) {
        img_size_mode = GUI::ScalingMode::AdjustBoth;
    }
    GUI::Rect rect_img = { (1 - stimulus_width) / 2, 0.3, (1 + stimulus_width) / 2, 0.3 };
    m_img_container = new ImageStimulus(mrDisplay);
    m_img_container->SetObjectRect(rect_img);
    m_img_container->SetScalingMode(img_size_mode);
    m_img_container->SetRenderingMode(GUI::RenderingMode::Opaque);
    //m_img_container->SetFile("../tasks/Communication_task/Stimuli/africanelephant_africanelephant_noise_mean0.1_var0.1.png");
    m_img_container->Conceal();
    
    //dimension text
    if (m_dim_1a_txt) delete m_dim_1a_txt;
    if (m_dim_1b_txt) delete m_dim_1b_txt;
    if (m_dim_2a_txt) delete m_dim_2a_txt;
    if (m_dim_2b_txt) delete m_dim_2b_txt;
    m_dim_1a_txt = nullptr;
    m_dim_1b_txt = nullptr;
    m_dim_2a_txt = nullptr;
    m_dim_2b_txt = nullptr;
    
    double dim_txt_height = Parameter("DimensionHeight") / 100.0;

    GUI::Rect rect_dim_1a = { my_slider_1->ObjectRect().left, my_slider_1->ObjectRect().top + 0.03, my_slider_1->ObjectRect().left, my_slider_1->ObjectRect().top + 0.03 + dim_txt_height};
    m_dim_1a_txt = new TextStimulus(mrDisplay);
    //m_dim_1a_txt->SetText("Indoors");
    m_dim_1a_txt->SetTextHeight(1.0);
    m_dim_1a_txt->SetTextColor(RGBColor(Parameter("DimensionFontColor")));
    m_dim_1a_txt->SetScalingMode(GUI::ScalingMode::AdjustBoth);
    m_dim_1a_txt->SetObjectRect(rect_dim_1a);
    m_dim_1a_txt->Conceal();

    GUI::Rect rect_dim_1b = { my_slider_1->ObjectRect().right, my_slider_1->ObjectRect().top + 0.03, my_slider_1->ObjectRect().right, my_slider_1->ObjectRect().top + 0.03 + dim_txt_height };
    m_dim_1b_txt = new TextStimulus(mrDisplay);
    //m_dim_1b_txt->SetText("Outdoors");
    m_dim_1b_txt->SetTextHeight(1.0);
    m_dim_1b_txt->SetTextColor(RGBColor(Parameter("DimensionFontColor")));
    m_dim_1b_txt->SetScalingMode(GUI::ScalingMode::AdjustBoth);
    m_dim_1b_txt->SetObjectRect(rect_dim_1b);
    m_dim_1b_txt->Conceal();

    GUI::Rect rect_dim_2a = { my_slider_2->ObjectRect().left, my_slider_2->ObjectRect().top + 0.03, my_slider_2->ObjectRect().left, my_slider_2->ObjectRect().top + 0.03 + dim_txt_height };
    m_dim_2a_txt = new TextStimulus(mrDisplay);
    //m_dim_2a_txt->SetText("Noisy");
    m_dim_2a_txt->SetTextHeight(1.0);
    m_dim_2a_txt->SetTextColor(RGBColor(Parameter("DimensionFontColor")));
    m_dim_2a_txt->SetScalingMode(GUI::ScalingMode::AdjustBoth);
    m_dim_2a_txt->SetObjectRect(rect_dim_2a);
    m_dim_2a_txt->Conceal();

    GUI::Rect rect_dim_2b = { my_slider_2->ObjectRect().right, my_slider_2->ObjectRect().top + 0.03, my_slider_2->ObjectRect().right, my_slider_2->ObjectRect().top + 0.03 + dim_txt_height };
    m_dim_2b_txt = new TextStimulus(mrDisplay);
    //m_dim_2b_txt->SetText("Silent");
    m_dim_2b_txt->SetTextHeight(1.0);
    m_dim_2b_txt->SetTextColor(RGBColor(Parameter("DimensionFontColor")));
    m_dim_2b_txt->SetScalingMode(GUI::ScalingMode::AdjustBoth);
    m_dim_2b_txt->SetObjectRect(rect_dim_2b);
    m_dim_2b_txt->Conceal();

    //option text
    if (m_opt_1_txt) delete m_opt_1_txt;
    if (m_opt_2_txt) delete m_opt_2_txt;
    m_opt_1_txt = nullptr;
    m_opt_2_txt = nullptr;

    double opt_txt_height = Parameter("OptionHeight") / 100.0;

    GUI::Rect rect_opt_1 = { my_slider_2->ObjectRect().left - 0.03, my_slider_2->ObjectRect().top + 0.1, my_slider_2->ObjectRect().left - 0.03, my_slider_2->ObjectRect().top + 0.1 + opt_txt_height };
    m_opt_1_txt = new TextStimulus(mrDisplay);
    //m_opt_1_txt->SetText("Rake");
    m_opt_1_txt->SetTextHeight(1.0);
    m_opt_1_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));
    m_opt_1_txt->SetScalingMode(GUI::ScalingMode::AdjustBoth);
    m_opt_1_txt->SetObjectRect(rect_opt_1);
    m_opt_1_txt->Conceal();

    GUI::Rect rect_opt_2 = { my_slider_2->ObjectRect().right + 0.03, my_slider_2->ObjectRect().top + 0.1, my_slider_2->ObjectRect().right + 0.03, my_slider_2->ObjectRect().top + 0.1 + opt_txt_height };
    m_opt_2_txt = new TextStimulus(mrDisplay);
    //m_opt_2_txt->SetText("Otter");
    m_opt_2_txt->SetTextHeight(1.0);
    m_opt_2_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));
    m_opt_2_txt->SetScalingMode(GUI::ScalingMode::AdjustBoth);
    m_opt_2_txt->SetObjectRect(rect_opt_2);
    m_opt_2_txt->Conceal();

    //feedback text
    if (m_feedback_container) delete m_feedback_container;
    m_feedback_container = nullptr;

    int feedback_size_mode = GUI::ScalingMode::AdjustHeight;
    double feedback_txt_width = Parameter("FeedbackWidth") / 100.0;
    if (feedback_txt_width < 0) {
        feedback_size_mode = GUI::ScalingMode::AdjustBoth;
    }
   
    GUI::Rect rect_feedback = { (1 - feedback_txt_width) / 2, my_slider_2->ObjectRect().top + 0.12, (1 + feedback_txt_width) / 2, my_slider_2->ObjectRect().top + 0.12 };
    m_feedback_container = new ImageStimulus(mrDisplay);
    m_feedback_container->SetObjectRect(rect_feedback);
    m_feedback_container->SetScalingMode(feedback_size_mode);
    m_feedback_container->SetRenderingMode(GUI::RenderingMode::Opaque);
    m_feedback_container->SetFile("../tasks/Communication_task/Markers/check_marker.png");
    m_feedback_container->Conceal();

    //role text
    if (m_role_txt) delete m_role_txt;
    m_role_txt = nullptr;

    double role_txt_height = Parameter("RoleHeight") / 100.0;

    GUI::Rect rect_role = { 0.5, (1 - role_txt_height) / 2, 0.5, (1 + role_txt_height) / 2 }; //center
    m_role_txt = new TextStimulus(mrDisplay);
    m_role_txt->SetText("Receiver");
    m_role_txt->SetTextHeight(1.0);
    m_role_txt->SetTextColor(RGBColor(Parameter("RoleFontColor")));
    m_role_txt->SetScalingMode(GUI::ScalingMode::AdjustBoth);
    m_role_txt->SetObjectRect(rect_role);
    m_role_txt->Conceal();

   //instruction
    if (m_instruction_container) delete m_instruction_container;
    m_instruction_container = nullptr;

    int instruction_size_mode = GUI::ScalingMode::AdjustHeight;
    double instruction_width = Parameter("InstructionWidth") / 100.0;
    if (instruction_width < 0) {
        instruction_size_mode = GUI::ScalingMode::AdjustBoth;
    }
    GUI::Rect rect_instruction = { (1 - instruction_width) / 2, 0.5, (1 + instruction_width) / 2, 0.5 };
    m_instruction_container = new ImageStimulus(mrDisplay);
    m_instruction_container->SetObjectRect(rect_instruction);
    m_instruction_container->SetScalingMode(instruction_size_mode);
    m_instruction_container->SetRenderingMode(GUI::RenderingMode::Opaque);
    m_instruction_container->SetFile(instructs_list[0]);
    m_instruction_container->Conceal();

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
            mPhotoDiodePatch.pShape = new RectangularShape(mrDisplay, -1);
            break;
        case ellipse:
        default:
            mPhotoDiodePatch.pShape = new EllipticShape(mrDisplay, -1);
            break;
        }
        mPhotoDiodePatch.pShape->SetHeight(Parameter("PhotoDiodePatchHeight"))
            .SetWidth(Parameter("PhotoDiodePatchWidth"))
            .SetPositionX(Parameter("PhotoDiodePatchLeft") + Parameter("PhotoDiodePatchWidth") / 2.0)
            .SetPositionY(Parameter("PhotoDiodePatchTop") + Parameter("PhotoDiodePatchHeight") / 2.0);
        Patch(0);
    }
}

void Communication_task_dualTask::SharedAutoConfig( const SignalProperties& Input ) {
	Parameters->Load( "../parms/CommunicationTask/HyperScanningParameters.prm", true );
	bciout << "Loaded server parameters";
}

void
Communication_task_dualTask::SharedStartRun()
{
    //initialize
    client_number = State("ClientNumber");
    if (client_number && (!is_reverse_sender) ) {//need to reverse
        for (int i = 0; i < sender_v.size(); i++) {
            sender_v[i] = (sender_v[i] == 1 ? 2 : 1);
        }
        is_reverse_sender = true;
    }

  // The user has just pressed "Start" (or "Resume")
    //GUI
    slider = my_slider_1;
    my_slider_1->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
    my_slider_1->SetAxesXPosition(0.5);
    my_slider_2->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
    my_slider_2->SetAxesXPosition(0.5);
   
    my_slider_1->SetVisible(false);
    my_slider_2->SetVisible(false);
    m_img_container->Conceal();
    m_dim_1a_txt->Conceal();
    m_dim_1b_txt->Conceal();
    m_dim_2a_txt->Conceal();
    m_dim_2b_txt->Conceal();
    m_opt_1_txt->Conceal();
    m_opt_2_txt->Conceal();
    m_feedback_container->Conceal();
    m_role_txt->Conceal();
    m_instruction_container->Conceal();
    m_opt_1_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));
    m_opt_2_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));

    //photodiode
    Patch(0);

    //statemachine
    m_block_in_phase = 0;
    m_current_trial_num = Parameter( "InitialTrialNumber" );
    role_dur_in_block = 0;

    if (m_current_trial_num > 0) {
        my_phase = wait_to_begin;
    }
    else {
        my_phase = instruction;
    }

    //others
    instructs_idx = 0;
    m_pre_button_state_joybt1 = 0;
    m_pre_button_state_joybt2 = 0;
    m_pre_button_state_joybt3 = 0;
    m_pre_button_state_joybt4 = 0;
    m_pre_button_state_joybt9 = 0;
    m_pre_button_state_joybt10 = 0;
    m_pre_axe_v_1 = 0.0;
    m_pre_axe_v_2 = 0.0;
    pre_opt = 0;

    //State
    State("PhaseNumber") = my_phase;
    State("TrialNumber") = m_current_trial_num;
	State("BarOneValue") = 0.5 * multi_step;
    State("BarTwoValue") = 0.5 * multi_step;
    State("ResponseValue") = 0;
    State("isReadyStart0") = 0;
    State("isReadyStart1") = 0;
    State("BarOneActive") = 0;
    State("BarTwoActive") = 0;
    State("SenderLock") = 0;
    State("ReceiverLock") = 0;
    State("StimulusCode") = 0;

}


void
Communication_task_dualTask::SharedProcess( const GenericSignal& Input, GenericSignal& Output )
{
    m_block_in_phase++;
    State("PhaseNumber") = my_phase;

    switch (my_phase)
    {
    case instruction:
        if (m_block_in_phase == 1) {
            m_instruction_container->SetFile(instructs_list[instructs_idx++]);
            m_instruction_container->Present();
        }
        if (CheckKeyBoardPress("KeyDown", 32)) {//space
            if (instructs_idx < instructs_list.size()) {
                m_instruction_container->SetFile(instructs_list[instructs_idx++]);
                m_instruction_container->Present();
            }
            else {
                //try to avoid to fresh the GUI frequenctly
                m_instruction_container->Conceal();
                m_role_txt->SetText("We are waiting for another player, once she/he is ready, we're ready to go!");
                m_role_txt->Present();

                if (client_number) {
                    State("isReadyStart1") = 1;
                }
                else {
                    State("isReadyStart0") = 1;
                }
            }
        }

        if (State("isReadyStart0") && State("isReadyStart1")) {//both of the two players are ready,move to next phase
            m_instruction_container->Conceal();
            m_role_txt->Conceal();
            my_phase = role_text;
            m_block_in_phase = 0;
            //State("isReadyStart0") = 0;
            //State("isReadyStart1") = 0;           
        }
        break;
    case wait_to_begin:
        if (m_block_in_phase == 1) {
            m_role_txt->SetText("Please press SPACE to continue the game.");
            m_role_txt->Present();
        }

        if (CheckKeyBoardPress("KeyDown", 32)) {//space
            m_role_txt->SetText("We are waiting for another player, once she/he is ready, we're ready to go!");
            if (client_number) {
                State("isReadyStart1") = 1;
            }
            else {
                State("isReadyStart0") = 1;
            }
        }

        if (State("isReadyStart0") && State("isReadyStart1")) {//both of the two players are ready,move to next phase
            m_role_txt->Conceal();
            my_phase = role_text;
            m_block_in_phase = 0;       
        }
        break;
    case role_text:
        if (m_block_in_phase == 1) {
            if (m_current_trial_num < sender_v.size()) {
                if (sender_v[m_current_trial_num] == 1) {
                    m_role_txt->SetText("Sender");
                }
                else {
                    m_role_txt->SetText("Receiver");
                }                
            }
            else {
                my_phase = end;
                m_block_in_phase = 0;
            }
            m_role_txt->Present();

            if(m_current_trial_num < jitter_v.size()){
                double temp_one_block_in_ms = (double)m_block_size / m_sample_rate * 1000.0;
                double temp_block = (double)jitter_v[m_current_trial_num] / temp_one_block_in_ms;
                role_dur_in_block = static_cast<int>(std::floor(temp_block));
            }

            State("StimulusCode") = random_sequence[m_current_trial_num] + 1;
            State("TrialNumber") = m_current_trial_num + 1;
            AppLog << "=================================" << std::endl;
            AppLog << "Trial #" << m_current_trial_num + 1 << "/" << sender_v.size() << std::endl;
        }
        else if(m_block_in_phase >= role_dur_in_block){
            my_phase = image_present;
            m_block_in_phase = 0;
            m_role_txt->Conceal();
            State("BarOneActive") = 1;
            State("BarTwoActive") = 0;
        }
        break;
    case image_present:
        if (m_block_in_phase == 1) {
            Patch(1);
            my_slider_1->SetVisible(true);
            my_slider_1->SetAxesColor(RGBColor(Parameter("AxeActiveColor")));
            my_slider_2->SetVisible(true);
            m_dim_1a_txt->SetText(dimt_1a_v[m_current_trial_num]);
            m_dim_1b_txt->SetText(dimt_1b_v[m_current_trial_num]);
            m_dim_2a_txt->SetText(dimt_2a_v[m_current_trial_num]);
            m_dim_2b_txt->SetText(dimt_2b_v[m_current_trial_num]);
            m_dim_1a_txt->Present();
            m_dim_1b_txt->Present();
            m_dim_2a_txt->Present();
            m_dim_2b_txt->Present();
            if (m_current_trial_num < sender_v.size()) {
                if (sender_v[m_current_trial_num] == 2) {//receiver
                    m_opt_1_txt->SetText(opt_1_v[m_current_trial_num]);
                    m_opt_2_txt->SetText(opt_2_v[m_current_trial_num]);
                    m_opt_1_txt->Present();
                    m_opt_2_txt->Present();
                    m_img_container->SetFile(img_recv_v[m_current_trial_num]);
                }
                else {//sender
                    m_img_container->SetFile(img_send_v[m_current_trial_num]);
                }              
            }
            else {
                my_phase = end;
                m_block_in_phase = 0;
            }

            m_img_container->Present();
        }
        else{
            //update the position of axes
            if (m_current_trial_num < sender_v.size() && sender_v[m_current_trial_num] == 1) {//sender
                if (State("SenderLock") == 0) {
                    MoveSliderAxes(slider_step);
                }
                else if (State("ReceiverLock") == 0) {
                    UpdateOptions();
                }
            }
            else if (m_current_trial_num < sender_v.size() && sender_v[m_current_trial_num] == 2) {//receiver
                if (State("SenderLock") == 0) {
                    UpdateSliderAxes();
                }
                else if(State("ReceiverLock") == 0){//receiver adjust begin
                    my_slider_1->SetAxesColor(RGBColor(Parameter("AxeActiveColor")));
                    my_slider_2->SetAxesColor(RGBColor(Parameter("AxeActiveColor")));
                    MoveOptions();
                }
            }
            if (State("ReceiverLock") == 1) { // feedback
                my_phase = feedback;
                m_block_in_phase = 0;
            }
        }
        break;
    case feedback:
        if (m_block_in_phase == 1) {
            if (m_current_trial_num < cor_rsps_v.size() && cor_rsps_v[m_current_trial_num] == State("ResponseValue")) {
                m_feedback_container->SetFile("../tasks/Communication_task/Markers/check_marker.png");
            }
            else {
                m_feedback_container->SetFile("../tasks/Communication_task/Markers/wrong_marker.png");
            }
            m_feedback_container->Present();
        }
        else if (m_block_in_phase >= feedback_dur_in_block) {
            m_block_in_phase = 0;
            m_current_trial_num++;
            //check if it is break trial
            if (m_current_trial_num >= trial_num) {
                my_phase = end;
            }
            else if (std::find(break_trial_list.begin(), break_trial_list.end(), m_current_trial_num) != break_trial_list.end()) {
                my_phase = block_break;
            }
            else {
                my_phase = role_text;
            }
            //reset
            //GUI
            slider = my_slider_1;
            my_slider_1->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
            my_slider_1->SetAxesXPosition(0.5);
            my_slider_2->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
            my_slider_2->SetAxesXPosition(0.5);

            my_slider_1->SetVisible(false);
            my_slider_2->SetVisible(false);
            m_img_container->Conceal();
            m_dim_1a_txt->Conceal();
            m_dim_1b_txt->Conceal();
            m_dim_2a_txt->Conceal();
            m_dim_2b_txt->Conceal();
            m_opt_1_txt->Conceal();
            m_opt_2_txt->Conceal();
            m_feedback_container->Conceal();
            m_role_txt->Conceal();
            m_instruction_container->Conceal();
            m_opt_1_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));
            m_opt_2_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));

            //photodiode
            Patch(0);

            //statemachine
            role_dur_in_block = 0;

            //others
            m_pre_button_state_joybt1 = 0;
            m_pre_button_state_joybt2 = 0;
            m_pre_button_state_joybt3 = 0;
            m_pre_button_state_joybt4 = 0;
            m_pre_button_state_joybt9 = 0;
            m_pre_button_state_joybt10 = 0;
            m_pre_axe_v_1 = 0.0;
            m_pre_axe_v_2 = 0.0;
            pre_opt = 0;

            //State
            State("BarOneValue") = 0.5 * multi_step;
            State("BarTwoValue") = 0.5 * multi_step;
            State("ResponseValue") = 0;
            State("isReadyStart0") = 0;
            State("isReadyStart1") = 0;
            State("BarOneActive") = 0;
            State("BarTwoActive") = 0;
            State("SenderLock") = 0;
            State("ReceiverLock") = 0;
        }
        break;
    case block_break:
        if (m_block_in_phase == 1) {
            m_role_txt->SetText("Well done! Are you ready for the next block? \n\n Press SPACEBAR");
            m_role_txt->Present();
        }
        else {
            if (CheckKeyBoardPress("KeyDown", 32)) {
                if (client_number) {
                    State("isReadyStart1") = 1;
                }
                else {
                    State("isReadyStart0") = 1;
                }
            }
            //waiting for both sides are ready. advance to next phase
            if (State("isReadyStart0") && State("isReadyStart1")) {
                m_role_txt->Conceal();
                my_phase = role_text;
                m_block_in_phase = 0;
                //State("isReadyStart0") = 0;
                //State("isReadyStart1") = 0;
            }
        }
        break;
    case end:
        if (m_block_in_phase == 1) {
            m_role_txt->SetText("Well done! You have finished all the trials.");
            m_role_txt->Present();
        }
        break;
    default:
        break;
    }

    Output = Input; // This passes the signal through unmodified.
}

void
Communication_task_dualTask::SharedStopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // or because the run has reached its natural end.
  // You know, you can delete methods if you're not using them.
  // Remove the corresponding declaration from Communication_task_dualTask.h too, if so.
}

void
Communication_task_dualTask::SharedHalt()
{
  // Stop any threads or other asynchronous activity.
  // Good practice is to write the Halt() method such that it is safe to call it even *before*
  // Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
}

/*
check the event which will last for a while
*/
bool
Communication_task_dualTask::CheckLongEvent(std::string event_name, int value, int& m_pre_button_state) {
    for (unsigned int i = 0; i < m_block_size; i++) {
        if (m_pre_button_state == 0 && State(event_name)(i) == value) {
            m_pre_button_state = State(event_name)(i);
            return true;
        }
        m_pre_button_state = State(event_name)(i);
    }
    return false;
}

/*
move the axes of slider by keyboard or Logitech controller
*/
void Communication_task_dualTask::MoveSliderAxes(float step) {
    if (step > 1.0f || step < 0.0f) 
        bcierr << "step should be between 0 and 1" << std::endl;
    if (CheckKeyBoardPress("KeyDown", 38) || CheckLongEvent("JoystickButtons4", 1, m_pre_button_state_joybt4)) {// up
        slider = my_slider_1;
        my_slider_1->SetAxesColor(RGBColor(Parameter("AxeActiveColor")));
        my_slider_2->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
        Patch(1);
        State("BarOneActive") = 1;
        State("BarTwoActive") = 0;
    }
    else if (CheckKeyBoardPress("KeyDown", 40) || CheckLongEvent("JoystickButtons2", 1, m_pre_button_state_joybt2)) {//down
        slider = my_slider_2;
        my_slider_1->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
        my_slider_2->SetAxesColor(RGBColor(Parameter("AxeActiveColor")));
        Patch(1);
        State("BarOneActive") = 0;
        State("BarTwoActive") = 1;
    }
    else if (CheckKeyBoardPress("KeyDown", 37) || CheckLongEvent("JoystickButtons1", 1, m_pre_button_state_joybt1)) {//left
        float axes_position = slider->AxesXPosition() - step;
        if (axes_position < 0.0f) {
            axes_position = 0.0f;
        }
        else if (axes_position > 1.0f) {
            axes_position = 1.0f;
        }
        axes_position = std::round(axes_position * multi_step) / multi_step;
        slider->SetAxesXPosition(axes_position);
        bciout << "Slider value: " << axes_position << std::endl;
        Patch(1);
        //bciout << "axes_position1: " << axes_position << std::endl;

        int axes_position_int = static_cast<int>(axes_position * multi_step);
        if(State("BarOneActive"))
            State("BarOneValue") = axes_position_int;
        else if(State("BarTwoActive"))
            State("BarTwoValue") = axes_position_int;

        bciout << "State slider value: " << axes_position_int << std::endl;
    }
    else if (CheckKeyBoardPress("KeyDown", 39) || CheckLongEvent("JoystickButtons3", 1, m_pre_button_state_joybt3)) {//right
        float axes_position = slider->AxesXPosition() + step;
        if (axes_position < 0.0f) {
            axes_position = 0.0f;
        }
        else if (axes_position > 1.0f) {
            axes_position = 1.0f;
        }
        axes_position = std::round(axes_position * multi_step) / multi_step;
        slider->SetAxesXPosition(axes_position);
        bciout << "Slider value: " << axes_position << std::endl;
        Patch(1);
        //bciout << "axes_position2: " << axes_position << std::endl;

        int axes_position_int = static_cast<int>(axes_position * multi_step);
        if (State("BarOneActive"))
            State("BarOneValue") = axes_position_int;
        else
            State("BarTwoValue") = axes_position_int;
        bciout << "State slider value: " << axes_position_int << std::endl;
    }
    else if (CheckKeyBoardPress("KeyDown", 32) || CheckLongEvent("JoystickButtons10", 1, m_pre_button_state_joybt10)) {//confirm
        if (m_current_trial_num < opt_1_v.size() && m_current_trial_num < opt_2_v.size()) {
            m_opt_1_txt->SetText(opt_1_v[m_current_trial_num]);
            m_opt_2_txt->SetText(opt_2_v[m_current_trial_num]);
        }
        else {
            bcierr << "m_current_trial_num is greater than than the number of totaltrial!" << std::endl;
        }
        m_opt_1_txt->Present();
        m_opt_2_txt->Present();
        Patch(1);
        State("SenderLock") = 1;
        State("BarOneActive") = 0;
        State("BarTwoActive") = 0;
        my_slider_1->SetAxesColor(RGBColor(Parameter("AxeActiveColor")));
        my_slider_2->SetAxesColor(RGBColor(Parameter("AxeActiveColor")));
    }
    else {
        Patch(0);
    }
    
}

void
Communication_task_dualTask::UpdateSliderAxes() {
    if (State("BarOneActive")) {
        float axes_value = (float)State("BarOneValue") / (float)multi_step;
        if (std::fabs(m_pre_axe_v_1 - axes_value) > std::numeric_limits<float>::epsilon()) {
            my_slider_1->SetAxesXPosition(axes_value);
            m_pre_axe_v_1 = axes_value;
            Patch(1);
        }
        else {
            Patch(0);
        }
        my_slider_1->SetAxesColor(RGBColor(Parameter("AxeActiveColor")));
        my_slider_2->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
    }
    else if (State("BarTwoActive")) {
        float axes_value = (float)State("BarTwoValue") / (float)multi_step;
        if (std::fabs(m_pre_axe_v_2 - axes_value) > std::numeric_limits<float>::epsilon()) {
            my_slider_2->SetAxesXPosition(axes_value);
            m_pre_axe_v_2 = axes_value;
            Patch(1);
        }
        else {
            Patch(0);
        }
        my_slider_2->SetAxesColor(RGBColor(Parameter("AxeActiveColor")));
        my_slider_1->SetAxesColor(RGBColor(Parameter("AxeInActiveColor")));
    }
}

void
Communication_task_dualTask::MoveOptions() {
    if (CheckKeyBoardPress("KeyDown", 37) || CheckLongEvent("JoystickButtons1", 1, m_pre_button_state_joybt1)) {//left
        m_opt_1_txt->SetTextColor(RGBColor(Parameter("OptionFontActiveColor")));
        m_opt_2_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));
        State("ResponseValue") = 1;
        Patch(1);
    }
    else if (CheckKeyBoardPress("KeyDown", 39) || CheckLongEvent("JoystickButtons3", 1, m_pre_button_state_joybt3)) {//right
        m_opt_1_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));
        m_opt_2_txt->SetTextColor(RGBColor(Parameter("OptionFontActiveColor")));
        State("ResponseValue") = 2;
        Patch(1);
    }
    else if (CheckKeyBoardPress("KeyDown", 32) || CheckLongEvent("JoystickButtons10", 1, m_pre_button_state_joybt10)) {//confirm
         State("ReceiverLock") = 1;
        Patch(1);
    }
    else {
        Patch(0);
    }
}

void
Communication_task_dualTask::UpdateOptions() {
    if (State("ResponseValue") == 1) {
        m_opt_1_txt->SetTextColor(RGBColor(Parameter("OptionFontActiveColor")));
        m_opt_2_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));
    }
    else if (State("ResponseValue") == 2) {
        m_opt_1_txt->SetTextColor(RGBColor(Parameter("OptionFontInActiveColor")));
        m_opt_2_txt->SetTextColor(RGBColor(Parameter("OptionFontActiveColor")));
    }

    if (State("ResponseValue") != pre_opt) {
        if (State("ResponseValue") != 0) {
            Patch(1);
        }
        else {
            Patch(0);
        }
        pre_opt = State("ResponseValue");
    }
    else {
        Patch(0);
    }
}

bool
Communication_task_dualTask::CheckKeyBoardPress(std::string event_name, int value) {
    for (unsigned int i = 0; i < m_block_size; i++)
    {
        if (State(event_name)(i) == value) {
            return true;
        }
    }
    return false;
}

void 
Communication_task_dualTask::Patch(bool active)
{
    if (mPhotoDiodePatch.pShape) {
        if (active)
            mPhotoDiodePatch.pShape->SetColor(mPhotoDiodePatch.activeColor).SetFillColor(mPhotoDiodePatch.activeColor);
        else
            mPhotoDiodePatch.pShape->SetColor(mPhotoDiodePatch.inactiveColor).SetFillColor(mPhotoDiodePatch.inactiveColor);
    }
}
