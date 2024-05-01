/////////////////////////////////////////////////////////////////////////////
// $Id: BioPacInputLogger.cpp 7993 2024-04-04 20:44:05Z mellinger $
// Author: lingling@neurotechcenter.org
// Description: A logger component that records data from BioPac amplifiers.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2023: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
/////////////////////////////////////////////////////////////////////////////
#include "BioPacInputLogger.h"
#include "BCIStream.h"
#include "BCIEvent.h"
//test
#include <fstream>
#include <iostream>
#include <iomanip>
//#include "Debugging.h"

Extension(BioPacInputLogger);

BioPacInputLogger::BioPacInputLogger()
{
    PublishEnabler("BioPacLogger");
}

BioPacInputLogger::~BioPacInputLogger()
{
    if (mBioPacLogger) {
        Halt();
        if (mpBuffer != nullptr) {
            delete[] mpBuffer;
            mpBuffer = nullptr;
        }
        // Closing connection to device.
        MPRETURNCODE retval = getStatusMPDev();
        if (retval != MPNOTCON) {
            stopAcquisition();
            disconnectMPDev();
        }
    }
}

void BioPacInputLogger::Publish()
{
    //SuggestDebugging << "myBreakpoint";
    bool enabled = (OptionalParameter("BioPacLogger", 0) > 0);
    if (enabled){
        BEGIN_PARAMETER_DEFINITIONS
            "Source:BioPacLogger int BioPacCh= 2 2 1 16"
            " % // number of digitized and stored channels in BioPac",

            //"Source:BioPacLogger intlist BioPacChList= 1 auto % % % "
            //" // list of active channel index",

            //"Source:BioPacLogger list BioPacSourceChType= 1 auto "
            //" // only RSP EGG EMG EDA is allowed, list of amplifier type, the order is corresponded to the order of BioPacChList",

            "Source:BioPacLogger int BioPacBlockSize= 200 200 1 % "
            " // number of samples acquired from BioPac at a time",

            "Source:BioPacLogger float BioPacSamplingRate= 1000 1000 0 2000 "
            " // BioPac sample rate",

            "Source:BioPacLogger matrix BioPacChMatrix= "
            "{BioPac%20Channel%20Id BioPac%20Signal%20Type} "                                           // row labels
            "{1 2} "
            "3 1 "
            "EMG RSP "
            "//Information of connected channels and their corresponding signal types(RSP EGG EMG EDA)",

            //"Source:BioPacLogger list BioPacChannelNames= 1 auto % % % "
            //" // names of amplifier channels",

        END_PARAMETER_DEFINITIONS

        for (int i = 0; i < BP_CHANNEL_NUMBER_MAX; i++) {
            StringUtils::String eventDef;
            eventDef << "BioPac_ch" << i + 1 << " 32 0 ";
            BEGIN_EVENT_DEFINITIONS
                eventDef.c_str()
            END_EVENT_DEFINITIONS
        }   

        //test
        //BEGIN_EVENT_DEFINITIONS
        //    "BioPac_timestamp 32 0"
        //END_EVENT_DEFINITIONS
    }
}

void BioPacInputLogger::Preflight() const
{
    bool enabled = (OptionalParameter("BioPacLogger", 0) > 0);
    if (enabled) {
        // Also check that the values of any parameters are sane:
        if (Parameter("BioPacSamplingRate").InHertz() > BP_SAMPLING_RATE) {
            bcierr << "logger does not support a sampling rate greater than " << BP_SAMPLING_RATE << "HZ" << std::endl;
        }

        if (Parameter("BioPacCh") > BP_CHANNEL_NUMBER_MAX) {
            bcierr << "Amplifier has a maximum of " << BP_CHANNEL_NUMBER_MAX << " available channels!" << std::endl;
        }

        //if (Parameter("BioPacChannelNames")->NumValues() != Parameter("BioPacCh"))
        //    bcierr << "A Channel name must be defined for each Channel!";
       
        if (Parameter("BioPacBlockSize") < 100 || Parameter("BioPacBlockSize") > 8000) {
            bcierr << "BioPac blockSize preferably greater than 100 and less than 8000" << std::endl;
        }

        if (Parameter("BioPacChMatrix")->NumColumns() != Parameter("BioPacCh")){
            bcierr << "The number of the colum in BioPacChMatrix is not equal to the size of BioPacCh!" << std::endl;
        }

        //check the channel index range[1 16] in the matrix
        std::vector<int> pre_ch_vec;
        ParamRef biopac_matrix = Parameter("BioPacChMatrix");
        for (int i = 0; i != biopac_matrix->NumColumns(); ++i) {
            if (biopac_matrix->RowLabels().Exists("BioPac Channel Id")) {
                if (biopac_matrix("BioPac Channel Id", i) < 1 || biopac_matrix("BioPac Channel Id", i) > 17) {
                    bcierr << "The channel index " << biopac_matrix("BioPac Channel Id", i) << " in BioPacChMatrix must be between 1 and 16." << std::endl;
                }
                pre_ch_vec.push_back(biopac_matrix("BioPac Channel Id", i));
            }
        }
        //check the duplicates in the matrix
        std::sort(pre_ch_vec.begin(), pre_ch_vec.end());
        for (int i = 0; i != pre_ch_vec.size() - 1 ; ++i) {
            if (pre_ch_vec[i] == pre_ch_vec[i + 1]) {
                bcierr << "No duplicate channel index allowed!! Duplicate channel index: " << pre_ch_vec[i] << std::endl;
            }
        }
    }
}

void BioPacInputLogger::Initialize()
{
    mBioPacLogger = (OptionalParameter("BioPacLogger", 0) > 0);
    if (mBioPacLogger) {
        // connect the MP106
        MPRETURNCODE retval = getStatusMPDev();
        if (retval != MPNOTCON) {
            stopAcquisition();
            disconnectMPDev();
        }
        retval = connectMPDev(MP160, MPUDP, "auto");
        if (retval != MPSUCCESS) {
            bcierr << "Program failed to connect to MP160 Device" << std::endl;
            bcierr << ErrorCodeTrans(retval) << std::endl;
            bcierr << "Disconnecting...." << std::endl;
            disconnectMPDev();
            throw bcierr << "Exit" << std::endl;
        }

        //sort all the connected channels and save them into a map
        std::map<int, std::string> ch_map;
        Matrix_to_map("BioPacChMatrix", ch_map);

        int ch_index = 0;
        mCh2ChList.clear();
        for (int i = 0; i != BP_CHANNEL_NUMBER_MAX; i++) {
            mAnalogCH[i] = 0;
        }
        for (auto const& entry : ch_map) {
            if (entry.first > 0 && entry.first < 17) {
                mAnalogCH[entry.first - 1] = 1;
                mCh2ChList[ch_index++] = entry.first;
            }                    
        }

        //test
        bciout << "BioPac conntected channel is:  ";
        for (int i = 0; i != 16; ++i) {
            bciout << mAnalogCH[i];
        }

        if(mCh2ChList.size() != Parameter("BioPacCh"))
            bcierr << "The number of active Channel must be equal to SourceCh!";

        //set the channels in biopac
        retval = setAcqChannels(mAnalogCH);
        if (retval != MPSUCCESS) {
            bcierr << "Program failed to set Acquisition Channels" << std::endl;
            throw bcierr << ErrorCodeTrans(retval) << std::endl;
        }

        //set sample rate
        double tempRate = Parameter("BioPacSamplingRate");
        mPeriod = 1000.0 / tempRate;
        bciout << "The period of BioPac is " << mPeriod << " ms/sample" << std::endl;
        //decimal is not allow in mPeriod, because it would cause doublt to int problem in caculating the timestamp in OnExcute()
        if (fmod(mPeriod, 1.0) != 0) {
            throw bcierr << "The period of BioPac is " << mPeriod << " ms/sample. Decimal is not allowed." << std::endl;
        }
        retval = setSampleRate(mPeriod);
        if (retval != MPSUCCESS)
        {
            bcierr << "Program failed to set Sample Rate" << std::endl;
            throw bcierr << ErrorCodeTrans(retval) << std::endl;

        }

        //allocate memory
        //blockSize = Parameter("BioPacSampleBlockSize");
        mEachChSampleNum = Parameter("BioPacBlockSize");
        mNumberOfSignalChannels = Parameter("BioPacCh");
        mBufferSize = mNumberOfSignalChannels * mEachChSampleNum;
        if (mpBuffer != nullptr) {
            delete[] mpBuffer;
            mpBuffer = nullptr;
        }        
        mpBuffer = new double[mBufferSize];
        ::memset(mpBuffer, 0, mBufferSize);
        // Buffer allocation may happen in OnStartAcquisition as well, if more appropriate.
        //test
        bciout << "BufferSize is " << mBufferSize << std::endl;
       
        //find the ch number of EDA and the ch order number of EDA in raw data
        mEDA_order_num.clear();
        std::vector<int> EDA_ch_num;
        int my_num = 0;
        for (std::map<int, std::string>::iterator it = ch_map.begin(); it != ch_map.end(); it++) {
            if (!(it->second.compare("EDA"))) {
                mEDA_order_num.push_back(my_num);
                EDA_ch_num.push_back(it->first);
            }
            my_num++;
        }

        //calibration begin
        if (EDA_ch_num.size() != 0) {
            //display the calibrate window
            int msgboxID = MessageBox(
                NULL,
                "Connect the EDA electrode leads to the transmitter but do NOT connect the leads to the eletrodes and subject. Click \"OK\"",
                "EDA Calibration",
                MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON1 | MB_SYSTEMMODAL
            );

            switch (msgboxID)
            {
            case IDOK:
                for (int i = 0; i < EDA_ch_num.size(); i++) {
                    EDA_calibration(EDA_ch_num[i], mEDA_order_num[i]);
                }
                break;
            case IDCANCEL:
                bcierr << "Calibration is needed! Before acquiring EDA data, we have to finish the calibration. Please push \"Set Config\" again." << std::endl;
                break;
            default:
                bcierr << "Calibration is needed! Before acquiring EDA data, we have to finish the calibration. Please push \"Set Config\" again." << std::endl;
                break;
            }
        }
    }
    disconnectMPDev();
}

void BioPacInputLogger::StartRun()
{
    if(mBioPacLogger){
        MPRETURNCODE retval = connectMPDev(MP160, MPUDP, "auto");
        if (retval != MPSUCCESS) {
            throw bcierr << "Program failed to connect to MP device"
                         << ErrorCodeTrans(retval);
        }
        //set the channels in biopac
        retval = setAcqChannels(mAnalogCH);
        if (retval != MPSUCCESS) {
            throw bcierr << "Program failed to set Acquisition Channels"
                         << ErrorCodeTrans(retval);
        }
        //set sample rate
        retval = setSampleRate(mPeriod);
        if (retval != MPSUCCESS) {
            throw bcierr << "Program failed to set Sample Rate" 
                         << ErrorCodeTrans(retval);
        }
        retval = startMPAcqDaemon();
        if (retval != MPSUCCESS) {
            throw bcierr << "Program failed to Start Acquisition Daemon"
                         << ErrorCodeTrans(retval);
        }
        retval = startAcquisition();
        if (retval != MPSUCCESS)
        {
            throw bcierr << "Program failed to Start Acquisition"
                         << ErrorCodeTrans(retval);
        }
        Thread::Start();
    }
}

void BioPacInputLogger::StopRun()
{
    if (mBioPacLogger) {
        Thread::Terminate();
        stopAcquisition();
        disconnectMPDev();
    }
    //test
    //ofstream myfile("../data/time_stamp.txt");
    //if (myfile.is_open()) {
    //    for (auto it = time_stamp.begin(); it != time_stamp.end(); it++)
    //    {
    //        //myfile << std::fixed << std::setprecision(8) << *it;
    //        myfile << *it;
    //        myfile << '\n';
    //    }
    //}
}

void BioPacInputLogger::Halt()
{
    if (mBioPacLogger) {
        StopRun();
    }    
}

int BioPacInputLogger::OnExecute()
{    
    if (!mBioPacLogger) return 1;
    DWORD valuesRead = 0;
    //PrecisionTime blockstart = PrecisionTime::Now();
    //uint32_t event_counter = 0;
    while (!Thread::Terminating()){
        //PrecisionTime blockstart = PrecisionTime::Now();
        //acquire data from BioPac API
        //test
        //PrecisionTime time_begin = PrecisionTime::Now();
        if (receiveMPData(mpBuffer, mBufferSize, &valuesRead) != MPSUCCESS)
        {
            bcierr << "Failed to receive MP data" << std::endl;
            // using of getMPDaemonLAstError is a good practice
            bcierr << "Failed to Recv data from Acq Daemon. Last ERROR=" << getMPDaemonLastError()
                   << ", Read=" << valuesRead;
            stopAcquisition();
        }
        //test
        //PrecisionTime time_duration = -time_begin + PrecisionTime::Now();
        //time_stamp.push_back(time_duration);
        if (valuesRead != mBufferSize) {
            bciout << "The target number of requested data " << mBufferSize << " is not equal to the actually recieved number of data " << valuesRead << ". Some data might lost" << std::endl;
        }
        //PrecisionTime blockstart = PrecisionTime::UnsignedDiff(PrecisionTime::Now(), std::round(eachChSampleNum * mPeriod));
        PrecisionTime blockstart = PrecisionTime::Now();
        //uint32_t event_counter = 0;
        const double* pSignalData = reinterpret_cast<double*>(mpBuffer);
        // Copy values from raw buffer into event.
        for (int sample = 0; sample < mEachChSampleNum; sample++)
        {
            for (int ch = 0; ch < mNumberOfSignalChannels; ch++)
            {   
                //test
                //rawdata_vec.push_back(pSignalData[ch + sample * mNumberOfSignalChannels]);
                //bcievent << "BioPac_9" << " " << event_counter << std::flush;
                double temp_d = pSignalData[ch + sample * mNumberOfSignalChannels];
                uint32_t temp_ui;
                if (std::find(mEDA_order_num.begin(), mEDA_order_num.end(), ch) != mEDA_order_num.end()) {
                    temp_ui = RawSignalScaleEvent(temp_d, 0.0, 50.0);
                }
                else {
                    temp_ui = RawSignalScaleEvent(temp_d, -10.0, 10.0);
                }  
                //test
                //uint32_t mPeriod = 1; // 1000ms / 1000 hz
                //uint32_t event_time_stamp = blockstart + event_counter * mPeriod;
                //BCIEvent event(event_time_stamp);
                //BCIEvent event_2(event_time_stamp);             
                BCIEvent event((std::round(sample * mPeriod)) + blockstart);

                if(!mCh2ChList.empty()){
                    event << "BioPac_ch" << mCh2ChList[ch] << " " << temp_ui << std::flush;
                    //event_2 << "BioPac_timestamp " << event_time_stamp << std::flush;
                }
                else {
                    bcierr << "Couldn't save the data to correct events. Please make sure the number of BioPacCh is equal to the number of BioPacChList " << std::endl;
                }                
            }
            //blockstart = blockstart + mPeriod;
            //event_counter++;
        }
    }
    return 0;
}

const std::string
BioPacInputLogger::ErrorCodeTrans(int errorCode) {
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
        return "Not recognized errorCode " + errorCode;
        break;
    }
}

uint32_t
BioPacInputLogger::RawSignalScaleEvent(double rawSignal, double min, double max) {
    //scale raw signal to [0,1]
    double scaled_signal = (rawSignal - min) / (max - min);
    scaled_signal = (scaled_signal <= 0) ? 0 : scaled_signal;
    scaled_signal = (scaled_signal >= 1) ? 1 : scaled_signal;

    return static_cast<uint32_t>(round(scaled_signal * std::numeric_limits<uint32_t>::max()));

}

bool
BioPacInputLogger::EDA_calibration(int ch_num, int order_num) {
    bciout << "Starting Calibration..." << std::endl;
    MPRETURNCODE retval = startAcquisition();
    if (retval != MPSUCCESS)
    {
        bciout << "Program failed to Start Acquisition" << std::endl;
        bciout << "startAcquisition returned with " << ErrorCodeTrans(retval) << " as a return code." << std::endl;
        stopAcquisition();
        throw bciout << "Stopping..." << std::endl;
        return 0;
    }
    //get the most recent sample
    int numsamples_buff = 100;
    double* data_buff = new double[mNumberOfSignalChannels * numsamples_buff];
    ::memset(data_buff, 0, mNumberOfSignalChannels * numsamples_buff);
    retval = getMPBuffer(numsamples_buff, data_buff);
    if (retval != MPSUCCESS)
    {
        bciout << "Program failed to get data" << std::endl;
        bciout << "getMPBuffer(...) returned with " << ErrorCodeTrans(retval) << " as a return code." << std::endl;
        stopAcquisition();
        throw bciout << "Stopping..." << std::endl;
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
    bciout << "The average value of EDA baseline data is " << data_average << std::endl;
    stopAcquisition();
    //calibrate
    DWORD ch_id = ch_num - 1;
    double sacled_sample_1 = 0;
    double sacled_sample_2 = 50;
    retval = setAnalogChScale(ch_id, data_average, sacled_sample_1, data_average + 10, sacled_sample_2);
    if (retval != MPSUCCESS)
    {
        bciout << "Program failed to EDA calibration" << std::endl;
        bciout << "setAnalogChScale(...) returned with " << ErrorCodeTrans(retval) << " as a return code." << std::endl;
        stopAcquisition();
        throw bciout << "Stopping..." << std::endl;
        return 0;
    }
    bciout << "Calibration done!" << std::endl;
    return 1;
}


/***
* fist check if there are duplicates in the matrix
* parameter_flag 1: source channel 2: stream channel 3:both
*/
void
BioPacInputLogger::Matrix_to_map(const std::string& params_name, std::map<int, std::string>& ch_info_map) {
    std::vector<int> check_dup_vec;
    ParamRef matrix = Parameter(params_name);
        
    for (int i = 0; i != matrix->NumColumns(); ++i) {
        if (matrix->RowLabels().Exists("BioPac Channel Id") && matrix->RowLabels().Exists("BioPac Signal Type")) {
            ch_info_map[matrix("BioPac Channel Id", i)] = matrix("BioPac Signal Type", i);
            check_dup_vec.push_back(matrix("BioPac Channel Id", i));
        }
    }
    
    //check if there are duplicates in ch index 
    std::sort(check_dup_vec.begin(), check_dup_vec.end());
    for (int i = 0; i != check_dup_vec.size() - 1; i++) {
        if (check_dup_vec[i] == check_dup_vec[i + 1]) {
            bcierr << "No duplicate channel index allowed!! Duplicate channel index: " << check_dup_vec[i] << std::endl;
        }
    }
}