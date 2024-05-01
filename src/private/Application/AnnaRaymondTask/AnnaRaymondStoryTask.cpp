////////////////////////////////////////////////////////////////////////////////
// Authors: schalklab@HR18818.wucon.wustl.edu
// Description: AnnaRaymondStoryTask implementation
////////////////////////////////////////////////////////////////////////////////

#include "AnnaRaymondStoryTask.h"
#include "BCIStream.h"
#include "MyTextStimulus.h"
#include "myImageStimulus.h"
#include "wavePlayerStimulus.h"

RegisterFilter( AnnaRaymondStoryTask, 3 );
     // Change the location if appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations within SignalProcessing modules begin with "2."
     //       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
     //  - filter locations within Application modules begin with "3."


AnnaRaymondStoryTask::AnnaRaymondStoryTask() 
{
  // C++ does not initialize simple types such as numbers, or pointers, by default.
  // Rather, they will have random values.
  // Take care to initialize any member variables here, so they have predictable values
  // when used for the first time.
}

AnnaRaymondStoryTask::~AnnaRaymondStoryTask()
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
AnnaRaymondStoryTask::Publish()
{
    std::map<std::string, std::string> stimulusMap = initStimulusMatrix();
  // Define any parameters that the filter needs....

 BEGIN_PARAMETER_DEFINITIONS
    //window
    "Application:Window int WindowWidth= 1920 1920 0 % "
    " // width of application window",
    "Application:Window int WindowHeight= 1080 1080 0 % "
    " // height of application window",
    "Application:Window int WindowLeft= 0 0 % % "
    " // screen coordinate of application window's left edge",
    "Application:Window int WindowTop= 0 0 % % "
    " // screen coordinate of application window's top edge",
     "Application:Window string BackgroudColor= 0x000000 0xFFFFFF 0x000000 0xFFFFFF // "
     "Color of window backgroud(color)",
     //stimulus 
     //stimulus matrix
     "Application:Stimuli matrix StimuliMatrix= "
     "{caption icon audio} "  +                                           // row labels
     stimulusMap["column"] +                                              //column labels
     stimulusMap["caption"] +
     stimulusMap["icon"] +
     stimulusMap["audio"] +
     " // texts and audios to be played for different stimuli",
     //audioVolume
     "Application:Stimuli float AudioVolume= 100 100 0 % // "
     "Volume for audio playback in percent",
     //button or keypress switch
     //"Application:Stimuli int segmentSwitch= 1 1 0 1 // "
     //"Segment by button(boolean) (otherwise segment by keypress)",
     "Application:Stimuli string TextBackgroudColor= 0x000000 0xFFFFFF 0x000000 0xFFFFFF // "
     "Color of caption stimulus backgroud(color)",
     "Application:Stimuli string TextColor= 0xFFFFFF 0xFFFFFF 0x000000 0xFFFFFF // "
     "Color of caption stimulus(color)",
     //sequencing
     //sequence
     "Application:Sequencing intlist Sequence= " +
     stimulusMap["sequence"] +
     " % % % // "
     "Sequence in which stimuli are presented ",
     //prerunduration
     //"Application:Sequencing float PreRunDuration= 1s 1 0 % "
     //"// pause preceding first sequence",
 END_PARAMETER_DEFINITIONS



 BEGIN_STATE_DEFINITIONS
   "StimulusCode   16 0 0 0",
   "repeate       8 0 0 0", 
 END_STATE_DEFINITIONS

}

void
AnnaRaymondStoryTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
    Parameter("WindowHeight");
    Parameter("WindowWidth");
    Parameter("WindowLeft");
    Parameter("WindowTop");
    Parameter("SampleBlockSize");
    Parameter("Sequence");
    Parameter("StimuliMatrix");
    Parameter("AudioVolume");
    Parameter("BackgroudColor");
    Parameter("TextBackgroudColor");
    Parameter("TextColor");



    State("StimulusCode");
    State("repeate");
    State("KeyDown");
    State("Running");

    //check

}


void
AnnaRaymondStoryTask::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
    //window
    myWindow->setWindowFlag(Qt::FramelessWindowHint);
    myWindow->setWindowTitle("AnnaRaymondStoryTask");
    myWindow->move(Parameter("WindowLeft"), Parameter("WindowTop"));
    myWindow->resize(Parameter("WindowWidth"), Parameter("WindowHeight"));
    myWindow->setFocusPolicy(Qt::StrongFocus);
    //myWindow->setStyleSheet("background-color:#AAAA00;");
    myWindow->setStyleSheet(transColor(Parameter("BackgroudColor")));
    myWindow->show();

    //get blocksize
    mBlockSize = Parameter("SampleBlockSize");

    //initial stimulus
    stimulusMap.clear();
    ParamRef StimuliMatrix = Parameter("StimuliMatrix");
    float volume = Parameter("AudioVolume");
    for (int i = 0; i != StimuliMatrix->NumColumns(); ++i) {
        //each id associate multiple stimulus
        std::vector<MyStimulus* > stimulusVec;
        if (StimuliMatrix("audio", i) != "") {
            WavePlayerStimulus* audioStimulus = new WavePlayerStimulus();
            audioStimulus->SetSound(StimuliMatrix("audio", i));
            audioStimulus->SetVolume(volume);
            audioStimulus->SetLabel("audio");
            stimulusVec.push_back(audioStimulus);
            bcidbg << "---------------audioPath" << StimuliMatrix("audio", i) << std::endl;
        }
        if (StimuliMatrix("caption", i) != "") {
            std::string sTextBKColor = Parameter("TextBackgroudColor");
            std::string stextColor = Parameter("TextColor");
            QString textBKColor= QString::fromStdString(sTextBKColor.substr(2));
            QString textColor = QString::fromStdString(stextColor.substr(2));

            QString cssStyle = "QLabel{background-color:#"+ textBKColor +"; color:#"+ textColor +"; }";
            MyTextStimulus* textStimulus = new MyTextStimulus(myWindow, cssStyle);
            textStimulus->SetSize(myWindow->width(), myWindow->height());
            textStimulus->SetText(StimuliMatrix("caption", i));
            textStimulus->SetLabel("caption");
            stimulusVec.push_back(textStimulus);
        }
        if (StimuliMatrix("icon", i) != "") {            
            MyImageStimulus* imageStimulus = new MyImageStimulus(myWindow);
            imageStimulus->SetSize(myWindow->width(), myWindow->height());
            imageStimulus->SetImage(StimuliMatrix("icon", i));
            imageStimulus->SetLabel("icon");
            stimulusVec.push_back(imageStimulus);
        }

        //add to stimulusMap
        stimulusMap.insert(std::pair<int, std::vector<MyStimulus*> >(i+1, stimulusVec));
    }

    //initial sequence
    mSequence.clear();
    for (int i = 0; i < Parameter("Sequence")->NumValues(); ++i) {
        mSequence.push_back(Parameter("Sequence")(i));
    }


    

}

void
AnnaRaymondStoryTask::StartRun()
{
    //instructionsTextItem->setText(instructionsVct[0]);
    //myPhase = partOneIntroTextOne;
    State("repeate") = 0;
    State("StimulusCode") = 0;
    mBlocksInPhase = 1;
    mSequencePos = mSequence.begin();
    for(auto s : stimulusMap[*mSequencePos])
        s->Present();
    bciout <<"-----------------present stimulus sequence index: " << *mSequencePos << std::endl;
    State("StimulusCode") = *mSequencePos;

}


void
AnnaRaymondStoryTask::Process( const GenericSignal& Input, GenericSignal& Output )
{   
    State("repeate") = 0;
    checkAudioStimulusEnd();

    keyPressValue keyValue = checkKeyPress();

    if (keyValue == key_space) {
        //stop the previous stimulus
        if (mSequencePos < mSequence.end())
        {
            for (auto s : stimulusMap[*mSequencePos])
                s->Conceal();
            //show the next stimulus
            mSequencePos++;
        }
        if (mSequencePos < mSequence.end()) {
            for (auto s : stimulusMap[*mSequencePos]){
                s->Present();
            }
            State("StimulusCode") = *mSequencePos;
            bciout << "-----------------present stimulus sequence index:" << *mSequencePos << std::endl;
         }
        else {
            bciout << "End of the sequence" << std::endl;
        }
    }
    else if (keyValue == key_r) {
        for (auto s : stimulusMap[*mSequencePos])
            s->Repeate();
        State("repeate") = 1;
        State("StimulusCode") = *mSequencePos;
    }
    else if (keyValue == key_q) {
        State("Running") = 0;
    }


    Output = Input;
    ++mBlocksInPhase;
    
}

void
AnnaRaymondStoryTask::StopRun()
{  
    if(mSequencePos!=mSequence.end()){
        for (auto s : stimulusMap[*mSequencePos])
            s->Conceal();
    }
    State("StimulusCode") = 0;
}

void
AnnaRaymondStoryTask::Halt(){
   
    stimulusMap.clear();
    mSequence.clear();

}


AnnaRaymondStoryTask::keyPressValue
AnnaRaymondStoryTask::checkKeyPress() {
    for (unsigned int i = 0; i < mBlockSize; i++)
    {
         if (State("KeyDown")(i) == Qt::Key_Space)
             return key_space;
        else if (State("KeyDown")(i) == Qt::Key_Enter)
            return key_enter;
        else if (State("KeyDown")(i) == Qt::Key_Q)
            return key_q;
        else if (State("KeyDown")(i) == Qt::Key_R)
            return key_r;
    }

    return key_none;
}


std::map<std::string,std::string>
AnnaRaymondStoryTask::initStimulusMatrix() {
    std::map<std::string, std::string> matrixMap;
    std::string column,caption, icon, audio, sequence;

    ////caption
    //caption = "+ ";
    //icon = "% ";
    //audio = "% ";
    //textPicture
    for (int i = 1; i < 10; i++) {
        icon += "../tasks/AnnaRaymondStoryTask/resource/Images/" + std::to_string(i) + ".png ";
        audio += "% ";
        caption += "% ";
    }
    //audio
    audio += "../tasks/AnnaRaymondStoryTask/resource/Audio/Waking_Up_edited.wav ../tasks/AnnaRaymondStoryTask/resource/Audio/practice_edited.wav ";
    icon += "% % ";
    caption += "+ + ";
    for (int i = 1; i < 41; i++) {
        audio += "../tasks/AnnaRaymondStoryTask/resource/Audio/q" + std::to_string(i) + ".wav ";
        caption += "+ ";
        icon += "% ";
    }
    //column
    column = "{ ";
    for (int i=1; i < 52; i++) {
        column += std::to_string(i) + " ";
    }

    column += " }";

    //sequence
    sequence = "52 1 2 10 3 4 10 5 6 7 ";
    for (int i = 12; i < 52; i++) {
        sequence += (std::to_string(i) + " ");
    }
    sequence += "8 11 9";


    matrixMap.insert(std::pair<std::string, std::string>("column", column));
    matrixMap.insert(std::pair<std::string, std::string>("caption", caption));
    matrixMap.insert(std::pair<std::string, std::string>("icon", icon));
    matrixMap.insert(std::pair<std::string, std::string>("audio", audio));
    matrixMap.insert(std::pair<std::string, std::string>("sequence", sequence));

    return matrixMap;
}

void
AnnaRaymondStoryTask::checkAudioStimulusEnd() { 
    if (mBlocksInPhase>1 && mSequencePos < mSequence.end()) {
        for (auto s : stimulusMap[*mSequencePos]){            
            if (s->Lable() == "audio") {
                if (!(s->isPlaying())) {
                    State("StimulusCode") = 0;
                }
            }
        }
    }
}

QString 
AnnaRaymondStoryTask::transColor(std::string scolor) {
    return QString ( "background-color: #" + QString::fromStdString(scolor.substr(2)) + ";" );
}