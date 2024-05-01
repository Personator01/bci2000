////////////////////////////////////////////////////////////////////////////////
// Authors: Lorenzo@Archimede.dhcp.wustl.edu
// Description: DelsysTrignoAvantiADC implementation
////////////////////////////////////////////////////////////////////////////////
#include "DelsysTrignoAvantiADC.h"

#include "BCIStream.h"
#include "BCIEvent.h"
#include "ThreadUtils.h"

// Make the source filter known to the framework.
RegisterFilter( DelsysTrignoAvantiADC, 1 ); // ADC filters must be registered at location "1" in order to work.

DelsysTrignoAvantiADC::DelsysTrignoAvantiADC()
: mDeviceHandle( 0 ), // Each plain data member should appear in the initializer list.
  mpEmgBuffer( 0 ),
  mpIMUBuffer(0) // Make sure we can call delete[] without crashing even if we never called new[].

{
  mNumberOfSignalChannels = 1;
  mNumberOfOrientationSamplesPerBlock = 1;
  mNumberOfEmgSamplesPerBlock = 1;
  mRatioEmgSamplesToImuSamples = 1;
}

DelsysTrignoAvantiADC::~DelsysTrignoAvantiADC()
{
  // The destructor is where to make sure that all acquired resources are released.
  // Memory deallocation (calling delete[] on a NULL pointer is OK).
  delete[] mpEmgBuffer;
  delete[] mpIMUBuffer;

  // revert the changes
  // modes = originalSettings.
  // isUsed = 
  // originalSettings.setSensorArrayProperties(modes, isUsed)

  // Closing connection to device.
  // 
  TRIGNOSDK_ERROR status = source.closeCommWithServer();

}

void
DelsysTrignoAvantiADC::OnPublish()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source:Server string ServerAddress= localhost localhost % %"
    " // IP address of the Delsys Trigno Server",

    "Source:Signal%20Properties int SourceCh= auto "
    "auto 1 % // number of digitized and stored channels",

    "Source:Signal%20Properties list ChannelNames= 1 auto "
    " // names of amplifier channels",
    
    "Source:Signal%20Properties list SensorMode= 1 auto "
    " // mode of the sensor (e.g. 67: 1xEMG (1482Hz) 4xOrientation (74Hz, 32bits)). cfr SDK User Guide",
    
    "Source:Signal%20Properties list SensorIsRequired= 1 auto"
    " // 0 = sensor not used, 1 = sensor is used",

    "Source:Signal%20Properties list MuscleNames= 1 auto"
    " // names of muscles the sensor is on. e.g. vastus lateralis",

    "Source:Signal%20Properties list SegmentNames= 1 auto "
    " // names of segment the sensor is placed on. e.g. leg, shank, foot",
    
    "Source:Signal%20Properties list ChannelNames= 16 auto % %"
    " // names of amplifier channels. e.g. EMG1",

    "Source:Signal%20Properties int SampleBlockSize= auto "
    "auto 1 % // number of samples transmitted at a time",
    
    "Source:Signal%20Properties float SamplingRate= auto "
    "auto 0.0 % // sample rate (noedit)",
    
    "Source:Signal%20Properties list SourceChGain= 1 auto "
    " // physical units per raw A/D unit",
    
    "Source:Signal%20Properties list SourceChOffset= 1 auto "
    " // raw A/D offset to subtract, typically 0",

    "Source:Legacy string BackwardCompatibilityMode= OFF OFF % %"
    " // A mode that forces sample rate to fixed values. cfr SDK User Guide ",

    "Source:Legacy string Upsampling= OFF OFF % %"
    " // A mode that forces sample rate. cfr SDK User Guide ",

    "Source:IMU float ImuSamplingRate= auto auto % %"
    " // Sample rate for the IMU data. cfr SDK User Guide (noedit)",

 END_PARAMETER_DEFINITIONS
  // %20 is the URL-esque for blank space

  // IMPORTANT NOTE ABOUT BUFFEREDADC AND STATES:
  // * BCI2000 States, or "state variables", are additional data channels stored alongside signal data,
  //   with a resolution of one value per source signal sample.
  // * A State is much like a source signal channel, but differs from a source signal channel in the
  //   following respects:
  //   + You choose the number of bits used to represent the State's value, up to 64.
  //   + Unlike source signals, States are transmitted through all stages of processing, and their values
  //     may be modified during processing, allowing all parts of the system to store state information in
  //     data files.
  //   + When loading a BCI2000 data file for analysis, States appear separately, with names, which is
  //     typically more useful for trigger information than being just another channel in the signal.
  //   + States may be set synchronously from inside a filter's Process() handler, or asynchronously using
  //     a "bcievent" interface.
  //   + If your amplifier provides a digital input, or another kind of "trigger" information, it makes sense
  //     to store this information in form of one or more States. From a user perspective, it is probably most
  //     useful if physically distinct amplifier input sockets correspond to States, and single bits to certain
  //     lines or values communicated through such a socket.
  //   + If the amplifier API asynchronously reports trigger information through a callback mechanism, you
  //     should register a callback that uses the "bcievent" interface to set states asynchronously.
  //     This example provides a "MyAsyncTriggers" "event state", and a sample callback function.
  //   + If the amplifier API sends trigger information in the same way as it does for signal data, you should
  //     use a "State channel" to represent it in your source module. This example provides a "MySyncTriggers"
  //     state, and writes to it when acquiring data.

  BEGIN_STREAM_DEFINITIONS
    "ErrorStream 8 0", // <name> <bit width> <initial value>
  END_STREAM_DEFINITIONS

}

void
DelsysTrignoAvantiADC::OnAutoConfig()
{
  // The user has pressed "Set Config" and some parameters may be set to "auto",
  // indicating that they should be set from the current amplifier configuration or default.

  int channels = 160;
  Parameter( "SourceCh" ) = channels; 
  channels = ActualParameter( "SourceCh" );
  Parameter( "ChannelNames" )->SetNumValues( channels );
  Parameter( "SourceChGain" )->SetNumValues( channels );
  Parameter( "SourceChOffset" )->SetNumValues( channels );
  for( int i = 0; i < channels; ++i )
  {
    // In case of auto name, all channels are named generically Ch_i.
    const char* name = "Ch";
    Parameter( "ChannelNames" )( i ) << name << "_" << i;

    // No units for gain because for emgs are in V and IMUs are in quaternions.
    constexpr double gainFactor = 1.0;
    Parameter( "SourceChGain" )( i ) << gainFactor;
    Parameter( "SourceChOffset" )( i ) = 0;
  }

  // In case of auto, set the number of samples per block to 20
  constexpr int samplesPerBlock = 20;
  Parameter( "SampleBlockSize" ) = samplesPerBlock;

  //in case of auto, set sampling rate to 2kHz
  constexpr double samplingRate = 2000.0;
  Parameter( "SamplingRate" ) << samplingRate << "Hz";

  //in case of auto, set sampling rate to the expected for that modality 
 //(which we trust for now, otherwise preflight will take care of it later)
  const int mode = (int)Parameter("SensorMode")(0).ToNumber();
  const double emgSamplingRate = DelsysTrigno::getEmgSamplingFrequencyFromMode(mode);
  const double imuSamplingRate = DelsysTrigno::getAuxSamplingFrequencyFromMode(mode);
  Parameter("SamplingRate") << emgSamplingRate << "Hz";
  Parameter("ImuSamplingRate") << imuSamplingRate << "Hz";

  bool isUsed[NSENSORS_MAX] = { 0 };
  for (int IDsensor = 0; IDsensor < NSENSORS_MAX; IDsensor++) {
    isUsed[IDsensor] = (bool)Parameter("SensorIsRequired")(IDsensor).ToNumber();
  }

  //ip address, string in dotted notation.
  std::string ipstring = Parameter("ServerAddress").ToString();

  //read the initial settings and set aside. We'll usem at the end to revert the setting back to original
  originalSettings.closeCommWithServer(); //just in case we left it open
  originalSettings.initDelsysTrignoServerCommunication(ipstring);
  originalSettings.checkServerProtocolVersion();
  originalSettings.getSensorArrayProperties(isUsed);
  originalSettings.closeCommWithServer();

  //open using the handle with the configs we will actually be using
  source.initDelsysTrignoServerCommunication(ipstring);
}

void
DelsysTrignoAvantiADC::OnPreflight( SignalProperties& Output ) const
{
  // The user has pressed "Set Config" and we need to sanity-check everything.
  // For example, check that all necessary parameters and states are accessible:
  State( "ErrorStream" );
  
  // Sensors required
  uint16_t nSensorUsed = 0;
  for (int IDsensor = 0; IDsensor < NSENSORS_MAX; IDsensor++)
  {
    if (Parameter("SensorIsRequired")(IDsensor).ToNumber() > 2.0)
    {
      bcierr << "Invalid value in the sensor is required field: should be a boolean";
    }

    //get the number of sensors in this experiment. 
    if (((int)Parameter("SensorIsRequired")(IDsensor).ToNumber()) == 1)
    {
      nSensorUsed++;
    }
  }

  //check that at least one sensor should be selected.
  if (nSensorUsed == 0)
  {
    bcierr << "At least one sensor should be used. Double check your sensors used booleans.";
  }

  // sampling frequency check.
  double fs = Parameter("SamplingRate").InHertz();
  if (fs < 100.0 || fs > 4000.0)
  {
    bcierr << "Amplifier does not support a sampling rate of " << Parameter("SamplingRate") <<
      "It should be in the range 100 - 4000Hz depending on the mode for emg";
  }

  //check that the mode is valid
  constexpr double MINMODE = 0.0;
  constexpr double MAXMODE = 380.0;
  for (int IDsensor = 0; IDsensor < NSENSORS_MAX; IDsensor++)
  {
    double mode = Parameter("SensorMode")(IDsensor).ToNumber();
    if (mode < MINMODE || mode > MAXMODE)
    {
      bcierr << "Amplifier does not support a sensor mode of " << Parameter("SensorMode")(IDsensor).ToString() <<
        "It should be between 1 and 379. cfr SDK User Guide.";
    }

    //consistency between mode and sampling frequency
    //check consistency with the sampling frequency in case someone manually edited it since we know a priori corresponds to that mode:
    double fsExpected = DelsysTrigno::getEmgSamplingFrequencyFromMode(mode);
    
    //overwrite the expected sampling frequency in case of backwards compatibility shenaningans;
    std::string backwardsCompatibility = Parameter("BackwardCompatibilityMode").ToString();
    std::string upsampling = Parameter("Upsampling");
    if ((backwardsCompatibility == "ON") && (upsampling == "OFF")) { fsExpected = 1111.111; }
    if ((backwardsCompatibility == "ON") && (upsampling == "ON")) { fsExpected = 2000.0; }
    
    double constexpr tol = 1.0; //Hz, maximum tolerance between sampling frequency set and expected for that mode.
    if (abs(fs - fsExpected) > tol)
    {
      bcierr << "Inconsistent mode-sampling rate pair. The sampling rate for mode " << mode <<
        "should be " << fsExpected << ". cfr SDK User Guide.";
    }

    
    

  }

  //Sample block size check: block size should be an integer multiple of the ratio between emg and imu sampling freqeuncy. 
  //this way, in a block we will have an integer amount of both samples
  const double emgSamplingFreq = Parameter("SamplingRate").InHertz();
  const double imuSamplingFreq = Parameter("ImuSamplingRate").InHertz();
  const double frameTime = 0.0135; //s, cfr SDKs
  const int emgSamplesPerFrame = lround(frameTime * emgSamplingFreq);
  const int imuSamplesPerFrame = lround(frameTime * imuSamplingFreq);
  const int ratioEmgToImuSamplingFrequency = lround((emgSamplingFreq / imuSamplingFreq)); //integer rounding, just in case of float point evil
  const int sampleBlockSize = lround(Parameter("SampleBlockSize").ToNumber());            //integer rounding, just in case of float point evil
  if ((sampleBlockSize % emgSamplesPerFrame) != 0 || (sampleBlockSize % imuSamplesPerFrame) != 0)
  {

    bciwarn << "A sample block size of " << Parameter("SampleBlockSize") << "does not contain an integer amount of samples for both emg and imu in a frame,"
            "so the timing might a little jittery. In case of issues, consider using an integer multiple of" << (emgSamplesPerFrame) << "and " << (imuSamplesPerFrame);

  }

  //chek the server connection.
  Parameter("ServerAddress").ToString();

  if (source.checkAllSocketsAreOpen() != TRIGNOSDK_ALL_OK)
  {
    bcierr << "Could not open sockets. Double check that the base is connected, "
      "the Trigno Control Utility is running, and the server settings are correct.";
  }

  if (source.checkServerProtocolVersion() != TRIGNOSDK_ALL_OK)
  {
    bcierr << "Invalid server protocol. Double check that the base is connected, "
      "the Trigno Control Utility is running, and the server settings are correct.";
  }

  // The amplifer outputs samples as float 32 bits, 4 bytes.
  SignalType sigType = SignalType::float32;
  int samplesPerBlock = Output.Elements();
  int numberOfSignalChannels = Output.Channels();
  int numberOfSyncStates = 1;
  Output = SignalProperties( numberOfSignalChannels + numberOfSyncStates, samplesPerBlock, sigType );
  // A channel name starting with @ indicates a trigger channel.
  Output.ChannelLabels()[numberOfSignalChannels] = "@ErrorStream";
}

void
DelsysTrignoAvantiADC::OnInitialize( const SignalProperties& Output )
{
  //parse the parameter lists into a local variable.
  int modes[NSENSORS_MAX] = { 0 }; 
  bool isUsed[NSENSORS_MAX] = { 0 }; 
  for (int IDsensor = 0; IDsensor < NSENSORS_MAX; IDsensor++) {
       modes[IDsensor] = (int) Parameter("SensorMode")(IDsensor).ToNumber(); 
       isUsed[IDsensor] = (bool) Parameter("SensorIsRequired")(IDsensor).ToNumber();
       //careful about truncation error and double representation, but ok for now.
  }

  //load the parameters into the function.
  const char* backwardsCompatibility = Parameter("BackwardCompatibilityMode").c_str();
  const char* upsampling = Parameter("Upsampling").c_str();

  //now our actual settings for the experiment
  TRIGNOSDK_ERROR err = TRIGNOSDK_ALL_OK;
  err = source.initBaseStation(backwardsCompatibility, upsampling);
  err = source.getSensorArrayProperties(isUsed);                  // read all settings
  err = source.setSensorArrayProperties(modes, isUsed);           // change the ones we care about
  err = source.getSensorArrayProperties(isUsed);                  // re-read all settings to double check.

  //load member variables for the process function
  mNumberOfSignalChannels = Parameter( "SourceCh" );

  const double emgSamplingFreq = Parameter("SamplingRate").InHertz();
  const double imuSamplingFreq = Parameter("ImuSamplingRate").InHertz();
  const int ratioEmgSamplesToImuSamples = lround((emgSamplingFreq / imuSamplingFreq)); //integer rounding, just in case of float point evil
  mNumberOfEmgSamplesPerBlock = Output.Elements(); 
  mNumberOfOrientationSamplesPerBlock = Output.Elements() / ratioEmgSamplesToImuSamples;
  mRatioEmgSamplesToImuSamples = ratioEmgSamplesToImuSamples;

  uint32_t nBytesEmgPerBlock = sizeof(float) * mNumberOfEmgSamplesPerBlock * NSENSORS_MAX;                                // 4 bytes/sample * 60 sample/(block*sensors*channel)  * 16 sensor * 1 channel. 
  uint32_t nBytesIMUPerBlock = sizeof(float) * mNumberOfOrientationSamplesPerBlock * NSENSORS_MAX * (NCHANNELS_MAX - 1);  // 4 bytes/sample * 3 sample/(block*sensors*channel)  * 16 sensor * 9 channels.

  delete[] mpEmgBuffer;
  mpEmgBuffer = new char[nBytesEmgPerBlock];
  ::memset(mpEmgBuffer, 0, nBytesEmgPerBlock);

  delete[] mpIMUBuffer;
  mpIMUBuffer = new char[nBytesIMUPerBlock];
  ::memset(mpIMUBuffer, 0, nBytesIMUPerBlock);

}

void
DelsysTrignoAvantiADC::OnStartAcquisition()
{
  // This method is called from the acquisition thread once the system is initialized.
  //start acquisition
  TRIGNOSDK_ERROR error = source.sendStartCommand();
  if (error == TRIGNOSDK_ALL_OK) { bciout << "Start command ok"; }
  //countermeasures in case of failure? retry perhaps?
 
  // It takes roughly 5s to start everything from wireshark initial debugging. We could also read from comm port for the string 
  
  // grace period: read the first n frames and discard them. hopefully that will help
  constexpr uint8_t nFramesToBeDiscared = 40; // obtimised by eye, about 0.5s of data comes at a different time rate for some reason, so wait, read and discard it
  ThreadUtils::SleepForMs(13 * nFramesToBeDiscared); //13ms is the (fixed, supposed) frame rate of the port, not respected in the initial period 
  for (int idFrame = 0; idFrame < nFramesToBeDiscared; idFrame++)
  {
    source.readFrame(mpEmgBuffer, mpIMUBuffer, mNumberOfEmgSamplesPerBlock, mNumberOfOrientationSamplesPerBlock);
  }
  
  //now move onto what the valid acquisitions

}

void
DelsysTrignoAvantiADC::DoAcquire( GenericSignal& Output )
{
  // Now we're acquiring a single SampleBlock of data in the acquisition thread, which is stored
  // in an internal buffer until the main thread is ready to process it.

  // Internally, BufferedADC writes this data to a buffer, then locks a mutex and pushes the data
  // into the GenericSignal output in the main thread.  The size of this buffer is parameterized by
  
  // Read from ports 
  TRIGNOSDK_ERROR errorStream = source.readFrame(mpEmgBuffer, mpIMUBuffer, mNumberOfEmgSamplesPerBlock, mNumberOfOrientationSamplesPerBlock);

  const float* pSignalData = reinterpret_cast<float*>(mpEmgBuffer);

  // Copy values from raw buffer into output signal.
  constexpr  int mNumberOfEmgChannels = NSENSORS_MAX;                                //16 sensors, each with 1 IMU channels, so 16 channels
  constexpr  int mNumberOfOrientationChannels = (NCHANNELS_MAX - 1) * NSENSORS_MAX;  //16 sensors, each with 9 IMU channels, so 144 channels

  for ( int ch = 0; ch < mNumberOfEmgChannels; ch++ ) //only the 16 emg channels
  {
    for( int sample = 0; sample < mNumberOfEmgSamplesPerBlock; sample++ )
    {
      Output( ch, sample ) = pSignalData[ch + sample * mNumberOfEmgChannels];
    }
  }
    
  const float* pIMUData = reinterpret_cast<float*>(mpIMUBuffer);
  for (int IMUch = 0; IMUch < mNumberOfOrientationChannels; IMUch++)
  { 
    for (int sample = 0; sample < (mNumberOfOrientationSamplesPerBlock); sample++) //index that scans the IMU discrete time
    {   
      // get two adjacent samples in time (i.e. sample space) for this IMU channel
      float IMUSample = pIMUData[IMUch + sample * mNumberOfOrientationChannels];
      float IMUSampleNext = 0.0;

      if (sample != (mNumberOfOrientationSamplesPerBlock - 1))
      { 
        //read the next sample 
        IMUSampleNext = pIMUData[IMUch + (sample + 1) * mNumberOfOrientationChannels]; //consider edge 
      }
      else //last sample
      {   
        //for the last sample, there is no 'next' to interpolate with, 
        //so we opt for a zero level holder, i.e.  constant value
        IMUSampleNext = IMUSample;
      }

      // find the indeces on the emg sample time these two would correspond (due to the different sample rates)
      int startEmgSample = sample * mRatioEmgSamplesToImuSamples;
      int stopEmgSample = (sample + 1) * mRatioEmgSamplesToImuSamples;

      for (int sampleEmg = startEmgSample; sampleEmg < stopEmgSample; sampleEmg++) 
      {
        // upsample to emg rate, using 1st or 0th level order.
        Output(mNumberOfEmgChannels + IMUch, sampleEmg) =
            IMUSample +  (IMUSampleNext - IMUSample) * (sampleEmg - startEmgSample) / (1.0f * (stopEmgSample - startEmgSample));

      }//emg samples
    }//imu samples
  }//imu ch

  //store the error flags
  for (int sample = 0; sample < mNumberOfEmgSamplesPerBlock; sample++)
  {
    Output(mNumberOfEmgChannels + mNumberOfOrientationChannels, sample) = errorStream;
  }
}

void
DelsysTrignoAvantiADC::OnStopAcquisition()
{
  // This method is called from the acquisition thread just before it exits.

  // Stop the acquisition
  TRIGNOSDK_ERROR error = source.sendStopCommand();
  if (error == TRIGNOSDK_ALL_OK) { bciout << "Stop command ok"; }

}

