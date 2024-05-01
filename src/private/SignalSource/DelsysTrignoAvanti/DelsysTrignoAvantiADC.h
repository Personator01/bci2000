////////////////////////////////////////////////////////////////////////////////
// Authors: Lorenzo@Archimede.dhcp.wustl.edu
// Description: DelsysTrignoAvantiADC header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_DELSYSTRIGNOAVANTIADC_H  // makes sure this header is not included more than once
#define INCLUDED_DELSYSTRIGNOAVANTIADC_H

#include "BufferedADC.h"
#include "DelsysClient.h"

class DelsysTrignoAvantiADC : public BufferedADC
{
 public:
  DelsysTrignoAvantiADC();
  ~DelsysTrignoAvantiADC();
  void OnPublish() override;
  void OnAutoConfig() override;
  void OnPreflight( SignalProperties& Output ) const override;
  void OnInitialize( const SignalProperties& Output ) override;
  void OnStartAcquisition() override;
  void DoAcquire( GenericSignal& Output ) override;
  void OnStopAcquisition() override;
  void OnTrigger( int ); // for asynchronous triggers only

 private:

  void* mDeviceHandle;
  char* mpEmgBuffer;
  char* mpIMUBuffer;
  
  /** \var int mNumberOfSignalChannels
   *	\brief total number of signal channels in the stream. e.g. should be 16 emg + 144 imu + 1 error. 
   */
  int mNumberOfSignalChannels;
  
  /** \var int mNumberOfOrientationChannelsPerBlock;
   *	\brief number of orientation samples in a block.
   */
  int mNumberOfOrientationSamplesPerBlock;
  
  /** \var int mNumberOfEmgChannelsPerBlock;
   *	\brief number of emg samples in a block.
   */
  int mNumberOfEmgSamplesPerBlock;
  
  /** \var int mRatioEmgSamplesToImuSamples;
   *	\brief the emg and imu are sampled at different frequencies typically. This is the ratio of the sampling frequencies 
   */
  int mRatioEmgSamplesToImuSamples;

  /** \var sensor gOriginalSensorArray[NSENSORS_MAX];
	 *	\brief Global containing all the sensors and their variable of interest for the current configurations.
	 */
  DelsysTrigno source;

	/** \var baseStation gBaseStation;
	 *	\brief Global containing all the info for the base for the starting configuration
	 *	Saving the original configuration so that at the end I can revert them back to how I found it.
	 *	Always place things back where you found them after using them.
	 */
  DelsysTrigno originalSettings;

};

#endif // INCLUDED_DELSYSTRIGNOAVANTIADC_H
