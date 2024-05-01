///////////////////////////////////////////////////////////////////////
// $Id: DelsysClient.h 7464 2023-06-30 15:04:08Z mellinger $
// Author: l.lombardi@neurotechcenter.org
// Description: A file header for the Delsys Trigno SDK. The header contains
//      an SVN Id tag, the author's email address, a short
//      description of the file's content, and a copyright notice.
//
// (C) 2000-2011, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
// Modify the following defines if you have to target a platform prior to the ones specified below.

#ifndef DELSYS_CLIENT_H
#define DELSYS_CLIENT_H

#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <string.h>

#include <sockstream.h>
#include <BCI2000FileReader.h>
#include <BCIStream.h>

/** \def NSENSORS_MAX
 \brief maximum number of sensors per base station.
 */
#define NSENSORS_MAX 16ul

 /** \def NCHANNELS_MAX
  \brief maximum number of 
	per sensor.
  */
#define NCHANNELS_MAX 10

/** \def COMSOCK_PORT
\brief TCP port number in local host IP for commands
*/
#define COMSOCK_PORT 50040

/** \def EMGSOCK_PORT
\brief TCP port number in local host IP for emg stream
*/
#define EMGSOCK_PORT 50043

/** \def AUXSOCK_PORT
\brief TCP port number in local host IP for aux stream
*/
#define AUXSOCK_PORT 50044


/** \typedef TRIGNOSDK_ERROR
\brief List of errors the functions can return
*/
typedef enum
{
	TRIGNOSDK_ALL_OK,															///< 0
	TRIGNOSDK_WSASTARTUP_FAIL,										///< 1
	TRIGNOSDK_INVALID_URL_FAIL,										///< 2
	TRIGNOSDK_COMM_SOCKET_FAIL,										///< 3
	TRIGNOSDK_EMG_SOCKET_FAIL,										///< 4
	TRIGNOSDK_IMU_SOCKET_FAIL,										///< 5
	TRIGNOSDK_IDADDRESS_CONVERSION_FAIL,					///< 6
	TRIGNOSDK_CONNECTION_COMMAND_PORT_FAIL,				///< 7
	TRIGNOSDK_CONNECTION_EMG_PORT_FAIL,						///< 8
	TRIGNOSDK_INVALID_BASE_PROTOCOL_VERSION_FAIL,	///< 9
	TRIGNOSDK_START_COMMAND_FAIL,									///< 10
	TRIGNOSDK_STOP_COMMAND_FAIL,									///< 11
	TRIGNOSDK_TIMEOUT_READ_EMG_FAIL,							///< 12
	TRIGNOSDK_TIMEOUT_READ_IMU_FAIL,							///< 13
	TRIGNOSDK_TIMEOUT_READ_BOTH_FAIL,							///< 14
	TRIGNOSDK_ERROR_ENUM_SIZE
}TRIGNOSDK_ERROR;

class DelsysTrigno
{
	public:

	/** \fn TRIGNOSDK_ERROR initServer(void)
	 *	\brief initializes sockets, and opens them if necessary
	 *	\ret int error code
	 */
	TRIGNOSDK_ERROR initServer(void);

	/** \fn TRIGNOSDK_ERROR initSensorArray(int* inIdsSensorsOfInterest, int inNumberSensors)
   *	\brief loads all the parameters of interest from the base station
   *	\param inIdsSensorsOfInterest	an array containing a list of the sensors used. e.g. [1, 2, 13, 14]
   *	\param inNumberSensors the size of the array above. just for comodity sent as a parameter. e.g. 4
   */
  TRIGNOSDK_ERROR initSensorArray(int* inIdsSensorsOfInterest, int inNumberSensors);

	/** \fn TRIGNOSDK_ERROR initBaseStation(const char* inBackwardsCompatibility, const char* inUpsampling)
	 *	\brief loads all the parameters of interest from the base station
	 *	\return trigno error
	 */
	TRIGNOSDK_ERROR initBaseStation(const char* inBackwardsCompatibility, const char* inUpsampling);

	/** \fn TRIGNOSDK_ERROR sendStartCommand(void);
	 *	\brief writes to comm port to start acquiring
	 */
	TRIGNOSDK_ERROR sendStartCommand(void);

	/** \fn TRIGNOSDK_ERROR sendStopCommand(void);
	 *	\brief writes to comm port to stop acquiring
	 */
	TRIGNOSDK_ERROR sendStopCommand(void);

	/** \fn TRIGNOSDK_ERROR readFrame(char* pOutEmgFrameData, char* pOutOrientationFrameData, int nSamplesPerBlock) 
	 *	\brief reads from the sockets, both emg and aux, the frame of data and parses it to the passed pointers
	 */
	TRIGNOSDK_ERROR readFrame(char* pOutEmgBlockData, char* pOutOrientationBlockData, int inNumberEmgSamplesPerBlock, int inNumberImuSamplesPerBlock);

	/** \fn int initDelsysTrignoServerCommunication(void)
	 *	\brief initializes sockets on localhost, and opens them if necessary
	 *	\ret int error code
	 */
	TRIGNOSDK_ERROR initDelsysTrignoServerCommunication(void);

	/** \fn int initDelsysTrignoServerCommunication(std::string ip)
 *	\brief initializes sockets on dedicated ip, and opens them if necessary
 *	\ret int error code
 */
	TRIGNOSDK_ERROR initDelsysTrignoServerCommunication(std::string ip);

	/** \fn TRIGNOSDK_ERROR closeCommWithServer(void);
	 *	\brief closes and cleans sockets
	 *	\ret int error code
	 */
	TRIGNOSDK_ERROR closeCommWithServer(void);
	
	/** \fn int TRIGNOSDK_ERROR checkServerProtocolVersion(void) const
	 *	\brief reads from commSock the SDK
	 *	\ret int error code from the TRIGNOSDK enum
	 *  \warning the function initDelsysTrignoServerCommunication() and connectDelsysTrignoServer()
	 *	have to be called before calling this function
	 */
	TRIGNOSDK_ERROR checkServerProtocolVersion(void) const;

	/** \fn TRIGNOSDK_ERROR checkAllPortsAreValid(void) const;
	 *	\brief tries to open all sockets and returns an OR of all the opening states
	 *	\ret int error code from the TRIGNOSDK enum
	 */
	TRIGNOSDK_ERROR checkAllSocketsAreOpen(void) const;

	/** \fn static double getEmgSamplingFrequencyFromMode(int mode);
	 *	\brief returns the theoretical emg sampling frequency for the mode selected
	 *	\param inMode integer containing the mode number for the sensor (e.g. 67). cfr SDK User Guide
	 *	\ret double expected sampling frequency
	 *	\warning The function does not actually read from the comm port the sampling frequency, that should be done separately
	 */
	static double getEmgSamplingFrequencyFromMode(int inMode);

	/** \fn static double getAuxSamplingFrequencyFromMode(int inMode);
	 *	\brief returns the theoretical imu sampling frequency for the mode selected
	 *	\param inMode integer containing the mode number for the sensor (e.g. 67). cfr SDK User Guide
	 *	\ret double expected sampling frequency
	 *	\warning The function does not actually read from the comm port the sampling frequency, that should be done separately
	 */
	static double getAuxSamplingFrequencyFromMode(int inMode);

	/** \fn TRIGNOSDK_ERROR getSensorArrayProperties(void)
	 *	\brief reads from comm port all the info regarding
	 *  sensor properties.
	 */
	TRIGNOSDK_ERROR getSensorArrayProperties(bool* inIsUsed);

	/** \fn TRIGNOSDK_ERROR setSensorArrayProperties(int* inModes, bool* inIsUsed);
	 *	\brief writes to the command port all the sensor related
	 *	\param inUsed array of booleans (16) indicating if the sensor is needed in this experiment
	 *	\param inModes array of ints (16) indicating the sensor mode. e.g. 67, cfr SDK User Guide
	 */
	TRIGNOSDK_ERROR setSensorArrayProperties(int* inModes, bool* inIsUsed);

	private:

	/** \typedef baseStation
	 *	\brief struct containing all the features of the base station
	 */
	class baseStation
	{
		public:

		char baseSerial[50];							///< base station serial number
		char protocolVersion[50];						///< server communication protocol release.
		char hardwareRevision[50];						///< hardware revision
		char firmwareRevision[100];						///< firmware revision
		bool startIsTriggered;							///< start trigger status 
		bool stopIsTriggered;							///< start trigger status
		bool backwardsCompatibility;					///< backwards compatibility bool. 0 = Sampling Rate scale, 1 = Resampling to the fastest (respectively for EMG and AUX). cfr SDK user guide 
		bool upsampling;								///< upsampling bool. 0 = no upsampling, 1 = upsample. Only used in backwards compatibiliyty ON. cfr SDK User Guide
		uint16_t emgMaxSamples;							///< maximum number of samples in a frame of EMG
		uint16_t auxMaxSamples;							///< maximum number of samples in a frame of AUX
		float frameInterval;							///< fixed value at 13.5ms.
		bool bigEndianness;								///< endianness of the communication. 1 = big, 0 = little
		bool isMaster;									///< master/slave communication

		// base station getters and setters
		/** \fn void TRIGNOSDK_ERROR getBaseStationProperties(void)
		 *	\brief reads from comm port all the info regarding
		 *  communication protocol and base station properties.
		 */
		TRIGNOSDK_ERROR getBaseStationProperties(void);

		/** \fn TRIGNOSDK_ERROR setBaseStationProperties(void)
		 *	\brief writes to comm port all the wanted base station properties.
		 */
		TRIGNOSDK_ERROR setBaseStationProperties(const char* inBackwardsCompatibility, const char* inUpsampling);

		private:
		
		/** \fn void getBackwardCompatibility(baseStation* pBaseStation);
		*		\brief reads from comm port the backward compatibility setting.
		*/
		void getBackwardCompatibility(void);

		/** \fn void getUpsampling(baseStation* pBaseStation);
		 *	\brief reads from comm port the upsampling setting.
		 */
		void getUpsampling(void);

		/** \fn void getMaxSamplesEmg(baseStation* pBaseStation);
		 *		\brief reads from comm port the maximum number of emg samples per frame setting.
		 */
		void getMaxSamplesEmg(void);

		/** \fn void getMaxSamplesAux(baseStation* pBaseStation);
		 *	\brief reads from comm port the maximum number of aux samples per frame setting.
		 */
		void getMaxSamplesAux(void);

		/** \fn void getFrameInterval(baseStation* pBaseStation);
		 *	\brief reads from comm port the frame interval setting.
		 *  \warning this should always return 13.5ms
		 */
		void getFrameInterval(void);

		/** \fn void getEndianness(baseStation* pBaseStation);
		 *	\brief reads from comm port the endianness of the communication setting
		 */
		void getEndianness(void);

		/** \fn void getBaseFirmware(baseStation* pBaseStation);
		 *	\brief reads from comm port the firmware revision of the base station
		 */
		void getBaseFirmware(void);

		/** \fn void getBaseSerial(baseStation* pBaseStation);
		 *	\brief reads from comm port the serial of the base station
		 */
		void getBaseSerial(void);

		/** \fn void setBackwardCompatibility(const char* command);
		 *	\brief sets the backwards compatibilty configuration
		 *	\param inSetting is a string with either 'ON' or 'OFF'
		 */
		void setBackwardCompatibility(const char* inSetting);

		/** \fn void setUpsampling(const char* inSetting);
		 *	\brief sets the upsampling configuration
		 *	\param inSetting is a string with either 'ON' or 'OFF'
		 */
		void setUpsampling(const char* inSetting);

		/** \fn void setEndianness(const char* inSetting);
		 *	\brief sets the endianness configuration
		 *	\param inSetting is a string with either 'BIG' or 'LITTLE'
		 */
		void setEndianness(const char* inSetting);


	};

	/** \typedef channel
	 *	\brief struct containing all the features of each channel
	 */
	typedef struct
	{

		uint32_t gain;									///< emg sensor gain
		char units[50];									///< emg sensor gain units 
		uint32_t samplesPerFrame;						///< the native samples per frame
		uint32_t sampleRateHz;							///< the native sample rate (in Hz)


	}channel;

	/** \typedef sensor
	 * \brief struct containing all the features of each sensor
	 */
	class sensor
	{
		public:
		uint8_t id;										///< numerical id, 1-16 on the sensor itself
		char serial[50];								///< sensor serial number
		char firmware[50];							///< sensor firmware release.
		channel channels[NCHANNELS_MAX];				///< array of channels in the sensor. e.g 1 = EMG, 2 = ACCX, 3 = ACCY
		uint32_t startIndex;								///< starting index for this sensor in the buffer, for convenience 
		uint8_t totalChannelCount;						///< total number of channels for this sensor. \warning should be less than NCHANNELS_MAX
		uint8_t emgChannelCount;						///< number of eeg channels in the sensor e.g. 1 for the avanti
		uint8_t auxChannelCount;						///< number of auxiliary channels in the sensor e.g. 9 for the full 9 DoF IMU 
		char type[50];									///< total number of channels for this sensor. \warning should be less than NCHANNELS_MAX
		char mode[50];									///< current mode of a given sensor. e.g. for Avanti, 40  = EMG (2148Hz) cfr. SDK User GUIDE
		uint32_t sampleRate;							///< sampling rate, depends on the mode.
		bool isPaired;									///< pairing status bool. 0 = not paired, 1 = paired

		//from the experiment file
		bool isNeededInThisExperiment;					///< used in this experiment bool. 0 = not used, 1 = used. should match isPaired in most cases.
		char muscleName[50];							///< name of the muscle associated

		/** \fn void getSensorProperties(void);
		 *	\brief get all infos from the SDKs into the instance
		 *	\warning the channels are 1-indexed in the SDK but 0 indexed in the code (as usual)
		 */
		void getSensorProperties(void);
		
		/** \fn void setSensorProperties(char* mode);
		 *	\brief set all infos (i.e. mode) to the sensor
		 *	\warning the channels are 1-indexed in the SDK but 0 indexed in the code (as usual)
		 */
		void setSensorProperties(char* mode);

		private:
		//sensor getters
		/** \fn void getSensorFirmware(sensor* pSensor);
		 *	\brief reads from comm port the firmware of the sensor
		 */
		void getSensorFirmware(void);

		/** \fn void getSensorStartIndex(sensor* pSensor);
		 *	\brief reads from comm port the start index of the sensor.
		 *  \warning should be 4 x sensorID, tbc
		 */
		void getSensorStartIndex(void);

		/** \fn void getSensorTotalChannelCount(sensor* pSensor);
		 *	\brief reads from comm port the total number of channels of the sensor.
		 */
		void getSensorTotalChannelCount(void);

		/** \fn void getSensorEmgChannelCount(sensor* pSensor);
		 *	\brief reads from comm port the number of emg channels of the sensor.
		 */
		void getSensorEmgChannelCount(void);

		/** \fn void getSensorAuxChannelCount(sensor* pSensor);
		 *	\brief reads from comm port the number of aux channels of the sensor.
		 */
		void getSensorAuxChannelCount(void);

		/** \fn void getSensorType(sensor* pSensor);
		 *	\brief reads from comm port the type of the sensor.
		 *  \warn expects either O (Avanti) or K (Analog) for now. To be extended if more sensor types are needed
		 */
		void getSensorType(void);

		/** \fn void getSensorMode(sensor* pSensor);
		 *	\brief reads from comm port the mode of the sensor.
		 */
		void getSensorMode(void);

		/** \fn void getSensorSerial(sensor* pSensor);
		 *	\brief reads from comm port the serial of the sensor.
		 */
		void getSensorSerial(void);

		/** \fn void getSensorPairingStatus(sensor* pSensor);
		 *	\brief reads from comm port if the sensor is paired or not.
		 */
		void getSensorPairingStatus(void);

		/** \fn void getChannelGain(int IDsensor, channel* pChannel);
		 *	\brief reads from comm port the gain of the sensor for the specific channel.
		 *  \param inIDchannel the indef of the channel. 1 indexed. 1 is typicallt EMG, the rest IMU
		 */
		void getChannelGain(int inIDchannel);

		/** \fn void getChannelUnits(int IDsensor, channel* pChannel);
		 *	\brief reads from comm port the units of the sensor data for the specific channel
		 *  \param inIDchannel the indef of the channel. 1 indexed. 1 is typicallt EMG, the rest IMU
		 *	\warning only implemented Volts and g as possible units. extend for more possibilities.
		 */
		void getChannelUnits(int inIDchannel);

		/** \fn void getChannelSamplesPerFrame(int IDsensor, channel* pChannel);
		 *	\brief reads from comm port the number of samples per frame for the specific channel
		 *  \param inIDchannel the indef of the channel. 1 indexed. 1 is typicallt EMG, the rest IMU
		 */
		void getChannelSamplesPerFrame(int inIDchannel);

		/** \fn void getChannelSampleRate(int IDsensor, channel* pChannel)
		 *  \brief reads from comm port the sample rate of the sensor for the specific channel (Hz).
		 *  \param inIDchannel the indef of the channel. 1 indexed. 1 is typicallt EMG, the rest IMU
		 */
		void getChannelSampleRate(int inIDchannel);

		/** \fn void setSensorMode(const char* inSetting)
		 *  \brief sets the sensor mode for the current sensor
		 *  \param inSetting an string with the mode number. e.g. '67' for EMG + orientation. cfr SDK user guide
		 *  \warning N.B. the input is a string with the numerical value, not an integer. so '67', not 67
		 */
		void setSensorMode(const char* inSetting);

		/** \fn void pair(void)
		 *  \brief pairs the sensors
		 */
		void pair(void);

	};


	sensor mSensorArray[NSENSORS_MAX];
	
	baseStation mBaseStation;

	/** \fn void SendCommandWithResponse(const char* inCommand, char* outResponse)
	 *  \brief Send a single TCP command and waits for a single TCP response on the commSocket
	 *  \param[in] inCommand	pointer to the string we want to send as command
	 *	\param[in] outResponse	pointer to the string we received as commnd
	 *	\warning Both inCommand and outReponse have to end with \r\\n\r\\n because it's a requirement of single TCP package
	 *	protocol with the server
	 */
	static std::string SendCommandWithResponse(const char* pInCommand);

};
#endif // !DELSYS_SDK_IMPORT