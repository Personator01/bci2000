#ifndef BIO_PAC_INPUT_LOGGER_H
#define BIO_PAC_INPUT_LOGGER_H

#include <Environment.h>
#include "Thread.h"
#include "mpdev.h"

#define BP_SAMPLING_RATE 1000
#define BP_CHANNEL_NUMBER_MAX 16
#define BP_BLOCK_SIZE 60

class BioPacInputLogger : public EnvironmentExtension, private Thread
{
public:
    BioPacInputLogger();
    ~BioPacInputLogger();

    // EnvironmentExtension interface
    void Publish() override;
    void Preflight() const override;
    void Initialize() override;
    void StartRun() override;
    void StopRun() override;
    void Halt() override;

    // Thread interface
    int OnExecute() override;

private:
    //members
    double* mpBuffer = nullptr;
    long mBufferSize;
    int mNumberOfSignalChannels;
    double mPeriod;
    BOOL mAnalogCH[BP_CHANNEL_NUMBER_MAX] = { 0 };
    long mEachChSampleNum = 1;
    std::map<int, int> mCh2ChList;
    bool mBioPacLogger = false;
    std::vector<int> mEDA_order_num;// the index of EDA in BioPac raw data

    //methods
    const std::string ErrorCodeTrans(int errorCode);
    uint32_t RawSignalScaleEvent(double rawSignal, double min, double max);
    bool EDA_calibration(int ch_num, int order_num);
    void Matrix_to_map(const std::string& params_name, std::map<int, std::string>& ch_info_map);

};

#endif // !BIO_PAC_INPUT_LOGGER_H
