////////////////////////////////////////////////////////////////////////////////
// Authors: schalklab@HR18818.wucon.wustl.edu
// Description: LisaMemycFilter implementation
////////////////////////////////////////////////////////////////////////////////

#include "LisaMemycFilter.h"
#include "BCIStream.h"

using  namespace std;

RegisterFilter( LisaMemycFilter, 2.B );
     // Change the location as appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations witin SignalProcessing modules begin with "2."
     //       (NB: Filter() commands in a separate PipeDefinition.cpp file may override the default location set here with RegisterFilter)
     //  - filter locations within Application modules begin with "3."

// C++ does not initialize simple types such as numbers, or pointers, by default.
// Rather, they will have random values.
// Take care to initialize any member variables here, so they have predictable values
// when used for the first time.
LisaMemycFilter::LisaMemycFilter()
{
}

LisaMemycFilter::~LisaMemycFilter()
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
LisaMemycFilter::Publish()
{
  // Define any parameters that the filter needs....

 BEGIN_PARAMETER_DEFINITIONS

    "Filtering:LisaMemycFilter int EnableLisaMemycFilter= 1 1 0 1 // enable LisaMemycFilter? (boolean)",    
    "Filtering:LisaMemycFilter float MinEncodeDuration= 3.5s 3.5s 0 % //min duration of encoding", 

 END_PARAMETER_DEFINITIONS


  // ...and likewise any state variables:

 BEGIN_STATE_DEFINITIONS

   "SkipCross       8 0 0 0",   

 END_STATE_DEFINITIONS

}

void
LisaMemycFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
    float encod_cros_dur_blk = Parameter("MinEncodeDuration").InSampleBlocks();
    float sampleRate = Parameter("SamplingRate");//Sampling rate
    unsigned int blockSize = Parameter("SampleBlockSize");// Sample block size
    float sBlocks = ((float)(blockSize) / (float)(sampleRate)) ;

    if (encod_cros_dur_blk < 1) {
        bcierr << "MinEncodeDuration must be >=1 sampleBlock duration(" + std::to_string(sBlocks) + " s." << endl;
    }
    else if (fmod(encod_cros_dur_blk, 1.0f) > 0) {
        bciwarn << "Due to a sample block duration is " + std::to_string(sBlocks) + " s. The actual value of MinEncodeDuration will be "
            << floor(encod_cros_dur_blk) * sBlocks << "s rather than " << encod_cros_dur_blk * sBlocks << "s.";
    }

    Parameter("EnableLisaMemycFilter");
    Parameter("MinEncodeDuration");
    Parameter("SampleBlockSize");
    Parameter("SamplingRate");
    Parameter("Stimuli");

    State("SkipCross");
    State("KeyDown");
    State("StimulusCode");
    State("StimulusBegin");

}


void
LisaMemycFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
    m_smpl_rate = Parameter("SamplingRate");
    m_blk_size = Parameter("SampleBlockSize");
    m_one_blk_dur_s = (float)(m_blk_size) / (float)(m_smpl_rate);
    min_encod_dur_blk = Parameter("MinEncodeDuration").InSampleBlocks();
    enable_cros_filter = Parameter("EnableLisaMemycFilter");
    ParamRef StimuliMatrix = Parameter("Stimuli");

    for (int i = 0; i != StimuliMatrix->NumColumns(); ++i) {    
        if (StimuliMatrix("Category", i) == "encodeCross") {
            encod_cros = i + 1;
        }
        if (StimuliMatrix("Category", i) == "encodeImage") {
            encod_img_idx_vec.push_back(i+1);
        }
    }
}

void
LisaMemycFilter::StartRun()
{
    m_blk_num = 0;
    is_key_press = 0;
}


void
LisaMemycFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
    if(enable_cros_filter){

        m_blk_num++;

        //set state value
        if (m_blk_num >= (min_encod_dur_blk - 1) && is_key_press) {
            State("SkipCross") = 1;
            is_key_press = 0;
        }

        //reset
        if (State("StimulusBegin") && State("StimulusCode") != encod_cros) {
            m_blk_num = 0;
            State("SkipCross") = 0;
            is_key_press = 0;
        }
        
  
        //store keydown
        it = find(encod_img_idx_vec.begin(), encod_img_idx_vec.end(), State("StimulusCode"));
        if (it != encod_img_idx_vec.end() || State("StimulusCode") == encod_cros) {
            if (checkKeyPress()) {
                is_key_press = 1;
            }
        }
    }
 
    Output = Input; 
}

void
LisaMemycFilter::StopRun()
{
    m_blk_num = 0;
    is_key_press = 0;
    encod_img_idx_vec.clear();
}

void
LisaMemycFilter::Halt()
{
  // Stop any threads or other asynchronous activity.
  // Good practice is to write the Halt() method such that it is safe to call it even *before*
  // Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
}

bool
LisaMemycFilter::checkKeyPress() {
    for (unsigned int i = 0; i < m_blk_size; i++)
    {
        if (State("KeyDown")(i) == 37 || State("KeyDown")(i) == 39 || State("KeyDown")(i) == 189){
            return 1;        
        }
    }
    return 0;
}