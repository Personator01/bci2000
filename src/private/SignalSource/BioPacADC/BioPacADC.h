////////////////////////////////////////////////////////////////////////////////
// Authors: schalklab@HR18818.wucon.wustl.edu
// Description: BioPacADC header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_BIOPACADC_H  // makes sure this header is not included more than once
#define INCLUDED_BIOPACADC_H

#include "BufferedADC.h"
#include "mpdev.h"

using namespace std;

#define BP_SAMPLING_RATE 2000
#define BP_CHANNEL_NUMBER_MAX 16
#define BP_BLOCK_SIZE 64

class BioPacADC : public BufferedADC
{
 public:
  BioPacADC();
  ~BioPacADC();
  void OnPublish() override;
  void OnAutoConfig() override;
  void OnPreflight( SignalProperties& Output ) const override;
  void OnInitialize( const SignalProperties& Output ) override;
  void OnStartAcquisition() override;
  void DoAcquire( GenericSignal& Output ) override;
  void OnStopAcquisition() override;

  void OnTrigger( int ); // for asynchronous triggers only

 private:
  // Use this space to declare any BioPacADC-specific methods and member variables you'll need
  double* mpBuffer = nullptr;
  long bufferSize;
  int mNumberOfSignalChannels;
  MPRETURNCODE retval;
  double mPeriod;
  BOOL analogCH[BP_CHANNEL_NUMBER_MAX] = { 0 };
  int blockSize;
  std::vector<int> EDA_order_num;// the index of EDA in BioPac raw data
  int size_of_source_matrix, size_of_stream_matrix;
  vector<int> raw_idx_of_stream_ch_vec;
  //test
  //vector<double> timestamp_vec;

  //method
  const string ErroCodeTrans(int errorCode);
  bool EDA_calibration(int ch_num, int order_num);
  uint32_t rawSignalScaleEvent(double rawSignal, double min, double max);
  void Matrix_to_map(vector<string> params_name, int parameter_flag, map<int, string> &ch_info_map);
};

#endif // INCLUDED_BIOPACADC_H
