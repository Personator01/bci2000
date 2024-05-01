////////////////////////////////////////////////////////////////////////////////
// Authors: schalklab@HR18818.wucon.wustl.edu
// Description: BioPacADC implementation
////////////////////////////////////////////////////////////////////////////////
#include "BioPacADC.h"

#include "BCIStream.h"
#include "BCIEvent.h"
#include "ThreadUtils.h"

#include <qmessagebox.h>
// In order to help you write a source module, exchange of information
// between amplifier and the BCI2000 source module is indicated by the use of
// macros.
// Once you are done with writing the source module, each occurrence of
// GET_FROM_AMP_API(), CALL_AMP_API(), and AMP_API_SYNC_GET_DATA() should
// have been replaced with actual calls to the amplifier API, or constants.
// By removing or disabling those macros' definitions below, you can then
// make sure that the compiler will notify you of any oversights.

// Depending on the kind of amplifier you have, occurrences of GET_FROM_AMP_API()
// may be read through the amplifier API, or obtained from documentation.
// Make the source filter known to the framework.

RegisterFilter( BioPacADC, 1 ); // ADC filters must be registered at location "1" in order to work.

BioPacADC::BioPacADC()
:mpBuffer( 0 ) // Make sure we can call delete[] without crashing even if we never called new[].
{
}

BioPacADC::~BioPacADC()
{
  // The destructor is where to make sure that all acquired resources are released.
  // Memory deallocation (calling delete[] on a NULL pointer is OK).
    if (mpBuffer != nullptr) {
        delete[] mpBuffer;
        mpBuffer = nullptr;
    }
  // Closing connection to device.
  retval = getStatusMPDev();
  if (retval != MPNOTCON) {
      stopAcquisition();
      disconnectMPDev();
  }
}

void
BioPacADC::OnPublish()
{
  // Declare any parameters that the filter needs....

 BEGIN_PARAMETER_DEFINITIONS

    "Source:Signal%20Properties int SourceCh= 2 "
       "2 1 16 // number of digitized and stored channels",

   /* "Source:Signal%20Properties intlist SourceChList= 1 9 % % % "
     " // list of active source channel index",

    "Source:Signal%20Properties list SourceChType= 1 auto "
     " // only RSP EGG EMG EDA is allowed, list of source signal type, the order is corresponded to the order of SourceChList",*/

    "Source:Signal%20Properties int SampleBlockSize= auto "
       "auto 1 % // number of samples transmitted at a time",

    "Source:Signal%20Properties float SamplingRate= auto "
       "auto 0.0 % // sample rate",

    "Source:Signal%20Properties list SourceChGain= 1 auto "
       " // physical units per raw A/D unit",

    "Source:Signal%20Properties list SourceChOffset= 1 auto "
       " // raw A/D offset to subtract, typically 0",

    "Source:Signal%20Properties list ChannelNames= 1 auto "
       " // names of amplifier channels",

     "Source:Signal%20Properties matrix SourceChMatrix= "
     "{Source%20Channel%20Id Source%20Signal%20Type} "                                           // row labels
     "{1 2 3 4} "
     "1 2 3 12 "
     "RSP EGG EMG EDA "
     "//Information of connected channels and their corresponding signal types(RSP EGG EMG EDA)",

     "Source:Signal%20Properties matrix StreamChMatrix= "
     "{Stream%20Channel%20Id Stream%20Signal%20Type} "                                           // row labels
     "0 "
     "//Information of connected channels and their corresponding signal types(RSP EGG EMG EDA)",

     //"Source:Signal%20Properties intlist StreamChList= 1 0 % % % "
     //" // channel index list of stream signal",

     //"Source:Signal%20Properties list StreamChType= 1 auto "
     //" // only RSP EGG EMG EDA is allowed, list of stream type, the order is corresponded to the order of StreamChList",

 END_PARAMETER_DEFINITIONS

     for (int i = 0; i < BP_CHANNEL_NUMBER_MAX; i++) {
         StringUtils::String streamDef;
         streamDef << "BioPac_ch" << i + 1 << " 32 0 0 0";
         BEGIN_STREAM_DEFINITIONS
             streamDef.c_str()
         END_STREAM_DEFINITIONS
     }

  // ...and likewise any state variables.

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

  //BEGIN_STREAM_DEFINITIONS
  //  "BioPacADCSyncTriggers 8 0", // <name> <bit width> <initial value>
  //END_STREAM_DEFINITIONS

  //BEGIN_EVENT_DEFINITIONS
  //  "BioPacADCAsyncTriggers 8 0", // <name> <bit width> <initial value>
  //END_EVENT_DEFINITIONS
}

// For asynchronous trigger notification, register this callback with the amplifier API.
static void
TriggerCallback( void* pData, int trigger )
{
  reinterpret_cast<BioPacADC*>( pData )->OnTrigger( trigger );
}

void
BioPacADC::OnTrigger( int trigger )
{
  // The following line will create a time-stamped entry into an event queue.
  // Once the next block of data arrives, the queued trigger value will be applied
  // to the BioPacADCAsyncTriggers state variable at the sample location that
  // corresponds to the time stamp.
  //bcievent << "BioPacADCAsyncTriggers " << trigger;
}

void
BioPacADC::OnAutoConfig()
{
  // The user has pressed "Set Config" and some parameters may be set to "auto",
  // indicating that they should be set from the current amplifier configuration.
  // In this handler, we behave as if any parameter were set to "auto", and the
  // framework will transparently make sure to preserve user-defined values.

  // Device availability (or connection parameters) may have changed, so close
  // and reopen the connection to the device before proceeding.
  retval = getStatusMPDev();
  if (retval != MPNOTCON) {
      stopAcquisition();
      disconnectMPDev();
  }
  // initial the source module parameter in case of "auto"
  Parameter("SamplingRate") = BP_SAMPLING_RATE;
  Parameter("SourceCh") = BP_CHANNEL_NUMBER_MAX;
  Parameter("SampleBlockSize") = BP_BLOCK_SIZE;
  //get the actual channels, either the value of above(auto) or the input by configuration window
  int channels = ActualParameter("SourceCh");
  //initialize other paramter with the actual channels number
  Parameter( "ChannelNames" )->SetNumValues( channels );
  Parameter( "SourceChGain" )->SetNumValues( channels );
  Parameter( "SourceChOffset" )->SetNumValues( channels );
  //Parameter("SourceChMatrix")->SetNumColumns(channels);

  // get the actual channels by other parameter input
  if (ActualParameter("ChannelNames")->NumValues() != channels)
      channels = ActualParameter("ChannelNames")->NumValues();

  if (ActualParameter("SourceChGain")->NumValues() != channels)
      channels = ActualParameter("SourceChGain")->NumValues();

  if (ActualParameter("SourceChOffset")->NumValues() != channels)
      channels = ActualParameter("SourceChOffset")->NumValues();
  
  if (ActualParameter("SourceChMatrix")->NumColumns() != channels) {
      channels = ActualParameter("SourceChMatrix")->NumColumns();
      bcierr << "The number of the colum in SourceChMatrix is not equal to the size of SourceCh!" << endl;
  }
  //update channels again
  Parameter("SourceCh") = channels;
  Parameter("ChannelNames")->SetNumValues(channels);
  Parameter("SourceChGain")->SetNumValues(channels);
  Parameter("SourceChOffset")->SetNumValues(channels);
  //Parameter("SourceChMatrix")->SetNumColumns(channels);

  //check the duplicates in channel index
  map<int, string> temp_check_map;
  vector<string> temp_check_vct = { "SourceChMatrix", "StreamChMatrix" };
  Matrix_to_map(temp_check_vct, 3, temp_check_map);

  //sort the source ch id
  map<int, string> auto_ch_map; 
  vector<string> temp_ch_vct = { "SourceChMatrix" };
  Matrix_to_map(temp_ch_vct, 1, auto_ch_map);

  int i = 0;
  for(auto const& entry : auto_ch_map)
  {
    string ch_name = entry.second;
    // For EEG amplifiers, channel names should use 10-20 naming if possible.
    //string s = "ch_" + to_string(Parameter("SourceChList")(i));
    //Parameter( "ChannelNames" )( i ) = s.substr(0, s.find("."));
    string s = to_string(entry.first);
    Parameter("ChannelNames")(i) = "ch_" + s.substr(0, s.find(".")) + "_" + ch_name;
    bciout << "ch info: " << s << "  " << ch_name;
    // Always provide correct physical unit for documentation and display purposes.
    if (!(ch_name.compare("EMG")) || !(ch_name.compare("EGG")))
    {
        Parameter("SourceChGain")(i) = "1mV";
    }
    else if (!(ch_name.compare("RSP"))) {
        Parameter("SourceChGain")(i) = "1V";
    }
    else if (!(ch_name.compare("EDA"))) {
        Parameter("SourceChGain")(i) = "1muS"; // microseiemens 
    }
    else {
        throw bcierr << "The channel name " << ch_name << " isn't supported! Only RSP, EDA, EMG, EGG is allowed." << endl;
    }
    
    // For most amplifiers, offset removal is done within the amp hardware or driver. Otherwise, you may set this to a nonzero value.
    Parameter( "SourceChOffset" )( i ) = 0;
    i++;
  }

  // blocksize varies proportionly as samplingrate changes
  double s_ratio = (double)BP_BLOCK_SIZE / (double)BP_SAMPLING_RATE;
  Parameter("SampleBlockSize") = std::round(s_ratio * ActualParameter("SamplingRate"));
  bciout << "SampleBlockSize is set as " << Parameter("SampleBlockSize") << " automatically" << endl;
}

void
BioPacADC::OnPreflight( SignalProperties& Output ) const
{
  // The user has pressed "Set Config" and we need to sanity-check everything.
  // For example, check that all necessary parameters and states are accessible:
  //State( "BioPacADCAsyncTriggers" );
  //State( "BioPacADCSyncTriggers" );

  // Also check that the values of any parameters are sane:
  if(Parameter("SamplingRate").InHertz() > BP_SAMPLING_RATE){
    bcierr << "Amplifier does not support a sampling rate greater than " << BP_SAMPLING_RATE <<"HZ" << endl;
  }

  if (Parameter("SourceCh") > BP_CHANNEL_NUMBER_MAX) {
      bcierr << "Amplifier has a maximum of "<< BP_CHANNEL_NUMBER_MAX << " available channels!" << endl;
  }

  if (Parameter("ChannelNames")->NumValues() != Parameter("SourceCh")){
      bcierr << "A Channel name must be defined for each Channel!";      
  }

  if (Parameter("SourceChGain")->NumValues() != Parameter("SourceCh"))
      bcierr << "A Channel gain must be defined for each Channel!";

  if (Parameter("SourceChOffset")->NumValues() != Parameter("SourceCh"))
      bcierr << "A Channel offset must be defined for each Channel!";

  if (Parameter("SourceChMatrix")->NumColumns() != Parameter("SourceCh"))
      bcierr << "The number of the colum in SourceChMatrix is not equal to the size of SourceCh!" << endl;
  //check the channel index range[1,16]
  map<int, string> pre_stream_ch_map;
  ParamRef source_matrix = Parameter("SourceChMatrix");
  ParamRef stream_matrix = Parameter("StreamChMatrix");
  for (int i = 0; i != source_matrix->NumColumns(); ++i) {
      if (source_matrix->RowLabels().Exists("Source Channel Id")) {
          if (source_matrix("Source Channel Id", i) < 1 || source_matrix("Source Channel Id", i) > 16) {
              bcierr << "The channel index " << source_matrix("Source Channel Id", i) << " in SourceChMatrix must be between 1 and 16." << endl;
          }
          //pre_stream_ch_map.insert(std::make_pair(source_matrix("Source Channel Id", i), source_matrix("Source Signal Type", i)));
      }
  }
  for (int i = 0; i != stream_matrix->NumColumns(); ++i) {
      if (stream_matrix->RowLabels().Exists("Stream Channel Id") && stream_matrix->RowLabels().Exists("Stream Signal Type")) {          
          if (stream_matrix("Stream Channel Id", i) < 1 || stream_matrix("Stream Channel Id", i) > 16) {
              bcierr << "The channel index " << stream_matrix("Source Channel Id", i) << " in StreamChMatrix must be between 1 and 16." << endl;
          }
          pre_stream_ch_map.insert(std::make_pair(stream_matrix("Stream Channel Id", i), stream_matrix("Stream Signal Type", i)));
      }
  }

  // Errors issued in this way, during the Preflight phase, still allow the user
  // to open the Config dialog box, fix bad parameters and re-try.  By contrast,
  // errors and C++ exceptions at any other stage (outside Preflight) will make
  // the system stop, such that BCI2000 will need to be relaunched entirely.

  // Internally, signals are always represented by GenericSignal::ValueType == double.
  // Here, you choose the format in which data will be stored, which may be
  // int16, int32, or float32.
  // Typically, you will choose the format that your amplifier natively provides, in
  // order to avoid loss of precision when converting, e.g., int32 to float32.
  SignalType sigType = SignalType::float32;
  int samplesPerBlock = Parameter("SampleBlockSize");
  int numberOfSignalChannels = Parameter("SourceChMatrix")->NumColumns();
  int numberOfSyncStates = 0;
  int numberOfStream = Parameter("StreamChMatrix")->NumColumns();
  Output = SignalProperties( numberOfSignalChannels + numberOfSyncStates + numberOfStream, samplesPerBlock, sigType );
  // A channel name starting with @ indicates a trigger channel.
  //Output.ChannelLabels()[numberOfSignalChannels] = "@BioPacADCSyncTriggers";
  // Note that the BioPacADC instance itself, and its members, are read-only during the
  // Preflight phase---note the "const" at the end of the OnPreflight prototype above.
  // Any methods called by OnPreflight must also be declared as "const" in the same way.

  //append the streams
  int s_index = numberOfSignalChannels;
  for (auto const &entry : pre_stream_ch_map) {
        StringUtils::String streamDef;
        streamDef << "@BioPac_ch" << entry.first;
        Output.ChannelLabels()[s_index] = streamDef.c_str();
        bciout << "raw index: " << s_index << "    channelLabel: " << streamDef.c_str() << endl;         
        s_index++;
  }
}

void
BioPacADC::OnInitialize( const SignalProperties& Output )
{
  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The system will now transition into an "Initialized" state.

  // If the user should be able to control amplifier configuration through BCI2000,
  // you will need to use the amplifier's API here in order to apply parameter settings.
  // Preflight() checks should ensure that Initialize() never fails due to misconfiguration.
  // Still, unforeseen cases or hardware conditions may lead to failure.
  
    // connect the MP106
    retval = getStatusMPDev();
    if (retval != MPNOTCON) {
        stopAcquisition();
        disconnectMPDev();
    }
    retval = connectMPDev(MP160, MPUDP, "auto");
    if (retval != MPSUCCESS) {
        bcierr << "Program failed to connect to MP160 Device" << endl;
        bcierr << ErroCodeTrans(retval) << endl;
        bcierr << "Disconnecting...." << endl;
        disconnectMPDev();
        throw bcierr << "Exit" << endl;
    }

    //sort all the connected channels and save them into a map
    map<int, string> ch_map;
    vector<string> ch_vec = { "SourceChMatrix", "StreamChMatrix" };
    Matrix_to_map(ch_vec, 3, ch_map);
    int idx_raw = 0;
    raw_idx_of_stream_ch_vec.clear();
    for (map<int, string>::iterator it = ch_map.begin(); it != ch_map.end(); it++) {
        bciout << it->first << " " << it->second << endl;
        //get the raw index of stream channel
        for (int i = 0; i != Parameter("StreamChMatrix")->NumColumns(); ++i) {
            if (it->first == Parameter("StreamChMatrix")("Stream Channel Id", i)) {
                raw_idx_of_stream_ch_vec.push_back(idx_raw);
                break;
            }
        }
        idx_raw++;
    }
    for (auto const& vec : raw_idx_of_stream_ch_vec) {
        bciout << "raw index of stream ch: " << vec;
    }

    //initialize the channels in MP160
    for (int i = 0; i != BP_CHANNEL_NUMBER_MAX; i++) {
        analogCH[i] = 0;
    }
    for (auto const &entry : ch_map) {
        if (entry.first > 0 && entry.first < 17) {
            analogCH[entry.first - 1] = 1;
        }
    }
    //test
    bciout << "BioPac conntected channel is:  ";
    for (int i = 0; i != 16; ++i) {
        bciout << analogCH[i];
    }
    //set the channels in biopac
    retval = setAcqChannels(analogCH);
    if (retval != MPSUCCESS) {
        bcierr << "Program failed to set Acquisition Channels" << endl;
        throw bcierr << ErroCodeTrans(retval) << endl;
    }

    //set sample rate
    double tempRate = Parameter("SamplingRate");
    mPeriod = 1000.0 / tempRate;
    bciout << "The period is " << mPeriod << " ms/sample" << endl;
    retval = setSampleRate(mPeriod);
    if (retval != MPSUCCESS)
    {
        bcierr << "Program failed to set Sample Rate" << endl;
        throw bcierr << ErroCodeTrans(retval) << endl;
    }

    // The signal properties can no longer be modified, but the const limitation has gone, so
    // the BioPacADC instance itself can be modified. Allocate any memory you need
    // store any information you need in private member variables.
    size_of_source_matrix = Parameter("SourceChMatrix")->NumColumns();
    size_of_stream_matrix = Parameter("StreamChMatrix")->NumColumns();
    mNumberOfSignalChannels = size_of_source_matrix + size_of_stream_matrix;
    bufferSize = Output.Channels() * Output.Elements();
    if (mpBuffer != nullptr) {
        delete[] mpBuffer;
        mpBuffer = nullptr;
    }
    mpBuffer = new double[bufferSize];
    ::memset(mpBuffer, 0, bufferSize);
    // Buffer allocation may happen in OnStartAcquisition as well, if more appropriate.

    blockSize = Parameter("SampleBlockSize");

    //find the ch number of EDA and the ch order number of EDA in raw data    
    std::vector<int> EDA_ch_num;
    EDA_order_num.clear();
    int my_num = 0;
    for (map<int, string>::iterator it = ch_map.begin(); it != ch_map.end(); it++) {
        if (!(it->second.compare("EDA"))) {
            EDA_order_num.push_back(my_num);
            EDA_ch_num.push_back(it->first);
        }
        my_num++;
    }
   
    //calibration begin
    if (EDA_ch_num.size() != 0) {
        QMessageBox* msgBox = new QMessageBox;
        msgBox->setText("EDA Calibration:\nConnect the EDA electrode leads to the transmitter but do NOT connect the leads to the eletrodes and subject. Click \"Calibrate\"");
        msgBox->addButton("Calibrate", QMessageBox::AcceptRole);
        msgBox->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        int ret = msgBox->exec();

        switch (ret) {
        case QMessageBox::AcceptRole:
            for (int i = 0; i < EDA_ch_num.size(); i++) {
                EDA_calibration(EDA_ch_num[i], EDA_order_num[i]);
            }
            break;
        default:
            // should never be reached
            bcierr << "Calibration is needed! Before acquiring EDA data, we have to finish the calibration. Please push \"Set Config\" again." << endl;
            break;
        }
        //release the memory
        if (msgBox)
            delete msgBox;
        msgBox = NULL;
    }
}

void
BioPacADC::OnStartAcquisition()
{
  // This method is called from the acquisition thread once the system is initialized.
  // You should use this space to start up your amplifier using its API.  Any initialization
  // here is done in the acquisition thread, so non-thread-safe APIs should work if initialized here.
    retval = startMPAcqDaemon();
    if (retval != MPSUCCESS)
    {
        bcierr << "Program failed to Start Acquisition Daemon" << endl;
        bcierr << ErroCodeTrans(retval) << endl;
        stopAcquisition();
        throw bcierr << "Stopping..." << endl;        
    }

    retval = startAcquisition();
    if (retval != MPSUCCESS)
    {
        bcierr << "Program failed to Start Acquisition" << endl;
        bcierr << ErroCodeTrans(retval) << endl;
        stopAcquisition();
        throw bcierr << "Stopping..." << endl;        
    }
}

void
BioPacADC::DoAcquire( GenericSignal& Output )
{
  // Now we're acquiring a single SampleBlock of data in the acquisition thread, which is stored
  // in an internal buffer until the main thread is ready to process it.

  // Internally, BufferedADC writes this data to a buffer, then locks a mutex and pushes the data
  // into the GenericSignal output in the main thread.  The size of this buffer is parameterized by
  // "SourceBufferSize" declared by BufferedADC, which will be interpreted as follows:
  // * When set to a naked number, the number will be rounded to the nearest integer, and specify
  //   the number of BCI2000 data blocks (cf the SampleBlockSize parameter). The buffer's
  //   duration in time will vary with changes to the SamplingRate and SampleBlockSize parameters.
  // * When set to a number followed with an SI time unit, the buffer's size will be automatically
  //   chosen to match the specified time duration as close as possible. By default, the value is 2s.
  //   SI time units must be appended without white space, and consist of
  //   a valid SI prefix (such as m for milli=1e-3, mu for micro=1e-6, k for kilo=1e3),
  //   followed with a lowercase s (for seconds).

  // Keep in mind that even though we're writing this data from another thread, the main thread
  // cannot continue without data, so be sure this is done in a timely manner
  // or the system will not be able to perform in real-time.

  // IMPORTANT NOTES ABOUT BUFFERING

  // Ideally, the BCI2000 processing time ("Roundtrip time") is always shorter than the physical
  // duration of a sample block, i.e. every single block of data has been processed before its
  // successor arrives. In that ideal case, buffering makes no difference, because the main thread will
  // always wait for the acquisition thread to deliver the next block of data into the internal
  // buffer, and copy it from there immediately.

  // If, on average, processing takes longer than the physical duration of a sample block, buffering
  // will not help to improve things either, because the buffer will be filled faster than
  // it is being emptied, and thus it will overflow after a certain time. Depending on buffering strategy,
  // buffer overflow may be dealt with by looping (i.e., overwriting data that has not been delivered yet),
  // or by blocking (not accepting any new data before there is space in the buffer).
  // For scientific purposes -- as opposed to, e.g., entertainment applications -- silently omitting
  // data is not an option, so BufferedADC will use the blocking strategy, and deliver all data blocks,
  // but delayed by the physical duration of its buffer.

  // So the _only_ situation in which buffering is actually useful is for cases when processing is not
  // able to keep up with data acquisition for short amounts of time. Typical examples are lengthy 
  // computations that happen from time to time, such as updating a classifier matrix, or initialization
  // work in algorithm implementations that are not factored out into initialization and update operations
  // (otherwise, you would just do lengthy operations inside Initialize()).
  // In such cases, you should set the SourceBufferSize parameter to an estimate of the longest expected
  // delay.

  // Call the amplifier API's function for synchronous data transfer here.

    DWORD valuesRead = 0;
    //timestamp_vec.push_back(PrecisionTime::Now());
    retval = receiveMPData(mpBuffer, bufferSize, &valuesRead);
    if (retval != MPSUCCESS)
    {
        bcierr << ErroCodeTrans(retval) << endl;

        bcierr << "Failed to receive MP data" << endl;
        // using of getMPDaemonLAstError is a good practice
        char szbuff3[512];
        ::memset(szbuff3, 0, 512);
        sprintf_s(szbuff3, "Failed to Recv data from Acq Daemon. Last ERROR=%d, Read=%d", getMPDaemonLastError(), valuesRead);
        bcierr << szbuff3 << endl;
        stopAcquisition();
    }
    if (valuesRead != bufferSize) {
        bciout << "The target number of requested data " << bufferSize << " is not equal to the actually recieved number of data " << valuesRead << ". Some data might lost" << endl;
    }
  
  const double* pSignalData = reinterpret_cast<double*>( mpBuffer );
  // Copy values from raw buffer into output signal.
  int source_idx = 0;
  int stream_idx = size_of_source_matrix;
  for( int ch = 0; ch < mNumberOfSignalChannels; ch++ )
  {
    bool isStreamCh = std::find(raw_idx_of_stream_ch_vec.begin(), raw_idx_of_stream_ch_vec.end(), ch) != raw_idx_of_stream_ch_vec.end();
    //bciout << "Channel Label:  " << Output.Properties().ChannelLabels()[ch] << "     isStreamCh: " << isStreamCh << endl;
    for( int sample = 0; sample < Output.Elements(); sample++ )
    {
      // Check the amplifier's documentation whether data are sent in
      // channel-major or sample-major format.
        
        if (isStreamCh) {//rescale the raw data
            double temp_d = pSignalData[ch + sample * mNumberOfSignalChannels];
            uint32_t temp_ui;
            if (std::find(EDA_order_num.begin(), EDA_order_num.end(), ch) != EDA_order_num.end()) {
                temp_ui = rawSignalScaleEvent(temp_d, 0.0, 50.0);
            }
            else {
                temp_ui = rawSignalScaleEvent(temp_d, -10.0, 10.0);
            }

            Output(stream_idx, sample) = temp_ui;
        }
        else {
            Output(source_idx, sample) = pSignalData[ch + sample * mNumberOfSignalChannels];
        }      
      // When getting garbage, try this instead:
      // Output( ch, el ) = pSignalData[el + ch * Output.Elements()];
    }
    if (isStreamCh) stream_idx++;
    else source_idx++;
  }
  //const char* pSyncTriggerData = 0;
  //GET_FROM_AMP_API( pSyncTriggerData, mpBuffer );
  //for( int sample = 0; sample < Output.Elements(); sample++ )
  //{
  //  // just an example, unlikely to work for a certain amplifier
  //  Output( mNumberOfSignalChannels, sample ) = pSyncTriggerData[sample];
  //}
}

void
BioPacADC::OnStopAcquisition()
{
  // This method is called from the acquisition thread just before it exits.  Use this method
  // to shut down the amplifier API (undoing whatever was done in OnStartAcquisition).
  // Immediately after this returns, the system will go into an un-initialized state and
  // OnHalt will be called in the main thread.
  stopAcquisition();
  disconnectMPDev();

    //test
    /*ofstream myfile("../data/timestamp.txt");
    if (myfile.is_open()) {
        for (auto it = timestamp_vec.begin(); it != timestamp_vec.end(); it++)
        {
            myfile << *it;
            myfile << '\n';
        }
    }*/
}


const string 
BioPacADC::ErroCodeTrans(int errorCode) {
    switch (errorCode)
    {
    case 1:
        return "successful execution";
        break;
    case 2:
        return "error communicating with the device drivers";
        break;
    case 3:
        return "a process is attached to the DLL, only one process may use the DLL";
        break;
    case 4:
        return "invalid parameter(s)";
        break;
    case 5:
        return "MP device is not connected";
        break;
    case 6:
        return "MP device is ready";
        break;
    case 7:
        return "MP device is waiting for pre-trigger (pre-triggering is not implemented)";
        break;
    case 8:
        return "MP device is waiting for trigger";
        break;
    case 9:
        return "MP device is busy";
        break;
    case 10:
        return "there are no active channels, in order to acquire data at least one analog channel must be active";
        break;
    case 11:
        return "generic communication error";
        break;
    case 12:
        return "the function is incompatible with the selected MP device or communication method";
        break;
    case 13:
        return "the specified MP160/MP150 is not in the network";
        break;
    case 14:
        return "MP device overwrote samples that had not been transferred from the device (buffer overflow)";
        break;
    case 15:
        return "error allocating memory";
        break;
    case 16:
        return "internal socket error";
        break;
    case 17:
        return "MP device returned a data pointer that is less than the last data pointer";
        break;
    case 18:
        return "error with the specified preset file";
        break;
    case 19:
        return "preset file parsing error, the XML file must be valid according to the schema";
        break;
    default:
        return "Not recognized erroCode " + errorCode;
        break;
    }
}

bool 
BioPacADC::EDA_calibration(int ch_num, int order_num) {
    bciout << "Starting Calibration..." << endl;
    retval = startAcquisition();
    if (retval != MPSUCCESS)
    {
        bciout << "Program failed to Start Acquisition" << endl;
        bciout << "startAcquisition returned with " << ErroCodeTrans(retval) << " as a return code." << endl;        
        stopAcquisition();
        throw bciout << "Stopping..." << endl;
        return 0;
    }
    //get the most recent sample
    int numsamples_buff = 100;
    double* data_buff = new double[mNumberOfSignalChannels * numsamples_buff];
    ::memset(data_buff, 0, mNumberOfSignalChannels * numsamples_buff);
    retval = getMPBuffer(numsamples_buff, data_buff);
    if (retval != MPSUCCESS)
    {
        bciout << "Program failed to get data" << endl;
        bciout << "getMPBuffer(...) returned with " << ErroCodeTrans(retval) << " as a return code." << endl;
        stopAcquisition();
        throw bciout << "Stopping..." << endl; 
        return 0;
    }
    //stopAcquisition();
    // average the baseline data
    double data_average = 0.0;
    double data_sum = 0.0;
    for (int i = 0; i < numsamples_buff; i++) {
        data_sum += data_buff[order_num + i * mNumberOfSignalChannels];
    }
    data_average = data_sum / numsamples_buff;   
    bciout << "The average value of EDA baseline data is " << data_average << endl;
    stopAcquisition();
    //calibrate
    DWORD ch_id = ch_num - 1;
    double sacled_sample_1 = 0;
    double sacled_sample_2 = 50;
    retval = setAnalogChScale(ch_id, data_average, sacled_sample_1, data_average + 10, sacled_sample_2);
    if (retval != MPSUCCESS)
    {
        bciout << "Program failed to EDA calibration" << endl;
        bciout << "setAnalogChScale(...) returned with " << ErroCodeTrans(retval) << " as a return code." << endl;
        stopAcquisition();
        throw bciout << "Stopping..." << endl; 
        return 0;
    }
    bciout << "Calibration done!" << endl;
    return 1;
}

uint32_t
BioPacADC::rawSignalScaleEvent(double rawSignal, double min, double max) {
    //scale raw signal to [0,1]
    double scaled_signal = (rawSignal - min) / (max - min);
    scaled_signal = (scaled_signal <= 0) ? 0 : scaled_signal;
    scaled_signal = (scaled_signal >= 1) ? 1 : scaled_signal;

    return static_cast<uint32_t>(round(scaled_signal * numeric_limits<uint32_t>::max()));

}

/***
* fist check if there are duplicates in the matrix
* parameter_flag 1: source channel 2: stream channel 3:both 
*/
void 
BioPacADC::Matrix_to_map(vector<string> params_name, int parameter_flag, map<int, string> &ch_info_map) {
    std::vector<int> check_dup_vec;

    if (params_name.size() == 1) {
        ParamRef matrix = Parameter(params_name[0]);        
        if (parameter_flag == 1) {
            for (int i = 0; i != matrix->NumColumns(); ++i) {
                if (matrix->RowLabels().Exists("Source Channel Id") && matrix->RowLabels().Exists("Source Signal Type")) {
                    ch_info_map.insert(std::make_pair(matrix("Source Channel Id", i), matrix("Source Signal Type", i)));
                    check_dup_vec.push_back(matrix("Source Channel Id", i));
                }
            }
        }
        else if (parameter_flag == 2) {
            for (int i = 0; i != matrix->NumColumns(); ++i) {
                if (matrix->RowLabels().Exists("Stream Channel Id") && matrix->RowLabels().Exists("Stream Signal Type")) {
                    ch_info_map.insert(std::make_pair(matrix("Stream Channel Id", i), matrix("Stream Signal Type", i)));
                    check_dup_vec.push_back(matrix("Stream Channel Id", i));
                }
            }
        }
    }
    else if (params_name.size() == 2) {
        ParamRef matrix = Parameter(params_name[0]);
        ParamRef matrix_2 = Parameter(params_name[1]);
        if (parameter_flag == 3) {
            for (int i = 0; i != matrix->NumColumns(); ++i) {
                if (matrix->RowLabels().Exists("Source Channel Id") && matrix->RowLabels().Exists("Source Signal Type")) {
                    ch_info_map.insert(std::make_pair(matrix("Source Channel Id", i), matrix("Source Signal Type", i)));
                    check_dup_vec.push_back(matrix("Source Channel Id", i));
                }
            }
            for (int i = 0; i != matrix_2->NumColumns(); ++i) {
                if (matrix_2->RowLabels().Exists("Stream Channel Id") && matrix_2->RowLabels().Exists("Stream Signal Type")) {
                    ch_info_map.insert(std::make_pair(matrix_2("Stream Channel Id", i), matrix_2("Stream Signal Type", i)));
                    check_dup_vec.push_back(matrix_2("Stream Channel Id", i));
                }
            }
        }
    }
    else {
        bcierr << "Matrix_to_map() argument error, size of params_name is greater than 2 or params_name is empty";
    }   

    //check if there are duplicates in ch index 
    std::sort(check_dup_vec.begin(), check_dup_vec.end());
    for (int i = 0; i != check_dup_vec.size() - 1; i++) {
        if (check_dup_vec[i] == check_dup_vec[i + 1]) {
            bcierr << "No duplicate channel index allowed!! Duplicate channel index: " << check_dup_vec[i] << endl;
        }
    }
}