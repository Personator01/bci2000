////////////////////////////////////////////////////////////////////////////////
// Authors: schalklab@HR18818.wucon.wustl.edu
// Description: LisaMemycFilter header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_LISAMEMYCFILTER_H  // makes sure this header is not included more than once
#define INCLUDED_LISAMEMYCFILTER_H

#include "GenericFilter.h"

using namespace std;

class LisaMemycFilter : public GenericFilter
{
 public:
  LisaMemycFilter();
  ~LisaMemycFilter();
  void Publish() override;
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;
  void Halt() override;

 private:
   // Use this space to declare any LisaMemycFilter-specific methods and member variables you'll need
	 unsigned int m_smpl_rate, m_blk_size, min_encod_dur_blk;
	 unsigned int m_blk_num = 0;
	 bool is_key_press;
	 float m_one_blk_dur_s;
	 bool enable_cros_filter = 1;
	 vector<int> encod_img_idx_vec;
	 vector<int>::iterator it;
	 int encod_cros;

	 bool checkKeyPress();
};

#endif // INCLUDED_LISAMEMYCFILTER_H
