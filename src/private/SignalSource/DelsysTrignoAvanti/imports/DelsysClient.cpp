////////////////////////////////////////////////////////////////////////////////
/// \file Library for communication with the Trigno Delsys Server  
/// Authors: Lorenzo@Archimede.dhcp.wustl.edu
/// Description: DelsysTrignoAvantiADC implementation
// 
// 
// The trigno Server should be up with the App provided:
// SensorBaseControl.exe
// which can be downloaded here:
// https://delsys.com/support/software
// and then find Trigno Software SDK server.
// 
// 
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "DelsysClient.h"
#include <vector>

/** \var sockstream mCommStream
\brief Stream for the Trigno server commands
*/
sockstream mCommStream;														

/** \var sockstream mEmgStream
\brief stream for the Trigno server emg stream
*/
sockstream mEmgStream;														

/** \var sockstream mAccStream;
\brief Stream for the Trigno server acc stream
*/
sockstream mAccStream;													

/** \var sockstream mLegacyEmgStream;
\brief Stream for the Trigno server emg stream
*/
sockstream mLegacyEmgStream;										

/** \var sockstream mLegacyAuxStream;
\brief Stream for the Trigno server auxiliary commands
*/
sockstream mLegacyAuxStream;											

/** \var client_tcpsocket mCommSocket;
	* .\brief Socket for the Trigno server commands
	*/
client_tcpsocket mCommSocket;

/** \var client_tcpsocket mEmgSocket;
	* .\brief Socket for the Trigno emg stream
	*/
client_tcpsocket mEmgSocket;

/** \var client_tcpsocket mAccSocket;
	* .\brief Socket for the Trigno auxiliary stream
	*/
client_tcpsocket mAccSocket;

/** \var client_tcpsocket mLegacyEmgSocket;
	* .\brief Socket for the Trigno emg (legacy)
	*  \warning legacy
	*/
client_tcpsocket mLegacyEmgSocket;

/** \var
	*  \brief Socket for the Trigno auxiliary data (legacy)
	* .\warning legacy
	*/
client_tcpsocket mLegacyAuxSocket;


TRIGNOSDK_ERROR DelsysTrigno::initServer(void) {

	return TRIGNOSDK_ALL_OK;
}

TRIGNOSDK_ERROR DelsysTrigno::initSensorArray(int* inIdsSensorsOfInterest, int inNumberSensors)
{
	return TRIGNOSDK_ALL_OK;
}

TRIGNOSDK_ERROR DelsysTrigno::initBaseStation(const char* inBackwardsCompatibility, const char* inUpsampling)
{
	
	TRIGNOSDK_ERROR error = this->mBaseStation.setBaseStationProperties(inBackwardsCompatibility, inUpsampling);
	//get every paramter and save it into original
	this->mBaseStation.getBaseStationProperties();

	//set every parameter to the new base station

	return error;
}

TRIGNOSDK_ERROR DelsysTrigno::initDelsysTrignoServerCommunication(void) {

	//read local host from parameters
	//concatenate with the define
	//string with server and port
	TRIGNOSDK_ERROR error = TRIGNOSDK_ALL_OK;

	mCommSocket.open("localhost:50040");
	mCommStream.open(mCommSocket);
	if (!mCommStream.is_open())
	{
		bcierr << "Could not connect to server at command port" << "127.0.0.1:50040" << ". "
			<< "Make sure the Delsys Control Utility is running";

		error = TRIGNOSDK_COMM_SOCKET_FAIL;
	}
	else
	{
		bciout << "Command socket opened successfully";
	}

	//emg stream
	//read form param and concatenate
	mEmgSocket.open("localhost:50043");
	mEmgStream.open(mEmgSocket);
	if (!mEmgStream.is_open())
	{
		bcierr << "Could not connect to server at EMG port" << "127.0.0.1:50043" << ". "
			<< "Make sure the Delsys Control Utility is running";

		error = TRIGNOSDK_EMG_SOCKET_FAIL;
	}
	else
	{
		bciout << "Emg socket and strem opened successfully";
	}

	//auxiliary stream
	//read form param and concatenate
	mAccSocket.open("localhost:50044");
	mAccStream.open(mAccSocket);
	if (!mAccStream.is_open())
	{
		bcierr << "Could not connect to server at AUX port" << "127.0.0.1:50044" << ". "
			<< "Make sure the Delsys Control Utility is running";

		error = TRIGNOSDK_IMU_SOCKET_FAIL;
	}
	else
	{
		bciout << "Orientation socket and strem opened successfully";
	}

	return error;
}

TRIGNOSDK_ERROR DelsysTrigno::initDelsysTrignoServerCommunication(std::string ip) {

	//read local host from parameters
//concatenate with the define
//string with server and port
	TRIGNOSDK_ERROR error = TRIGNOSDK_ALL_OK;

	mCommSocket.open(ip + ":50040");
	mCommStream.open(mCommSocket);
	if (!mCommStream.is_open())
	{
		bcierr << "Could not connect to server at command port" << "127.0.0.1:50040" << ". "
			<< "Make sure the Delsys Control Utility is running";

		error = TRIGNOSDK_COMM_SOCKET_FAIL;
	}
	else
	{
		bciout << "Command socket opened successfully";
	}

	//emg stream
	//read form param and concatenate
	mEmgSocket.open(ip + ":50043");
	mEmgStream.open(mEmgSocket);
	if (!mEmgStream.is_open())
	{
		bcierr << "Could not connect to server at EMG port" << "127.0.0.1:50043" << ". "
			<< "Make sure the Delsys Control Utility is running";

		error = TRIGNOSDK_EMG_SOCKET_FAIL;
	}
	else
	{
		bciout << "Emg socket and strem opened successfully";
	}

	//auxiliary stream
	//read form param and concatenate
	mAccSocket.open(ip + ":50044");
	mAccStream.open(mAccSocket);
	if (!mAccStream.is_open())
	{
		bcierr << "Could not connect to server at AUX port" << "127.0.0.1:50044" << ". "
			<< "Make sure the Delsys Control Utility is running";

		error = TRIGNOSDK_IMU_SOCKET_FAIL;
	}
	else
	{
		bciout << "Orientation socket and strem opened successfully";
	}

	return error;

}

TRIGNOSDK_ERROR DelsysTrigno::checkServerProtocolVersion(void) const
{

	TRIGNOSDK_ERROR error = TRIGNOSDK_ALL_OK;
	char tmp[128] = { 0 };	//string buffer for various uses
	if (mCommSocket.is_open())
	{
		int ready = mCommSocket.wait_for_read(10000);
		if (ready) {
			int n = mCommSocket.recv(tmp, sizeof(tmp), 0);
			tmp[n - 2] = 0;	//back up over second CR/LF just for printing purposes

			if (!strncmp(tmp, "Delsys Trigno System Digital Protocol Version", 45))
			{// if it replies with "Delsys Trigno System Digital Protocol Version", we don't care after this
				bciout << "Protocol version: " << tmp;
			}
			else
			{
				bciwarn << "Command port did not respond with the expected protocol version";
				error = TRIGNOSDK_INVALID_BASE_PROTOCOL_VERSION_FAIL;
			}
		}
		else 
		{
			bciwarn << "server protocol timeout";
			error = TRIGNOSDK_COMM_SOCKET_FAIL;
		}
	}
	else
	{
		bciwarn << "Command port was not open or not listening. Make sure the base is connected, the Trigno Control Utility is running and local host is ok. ";
		error = TRIGNOSDK_COMM_SOCKET_FAIL;
	}

	return error;

}

TRIGNOSDK_ERROR DelsysTrigno::getSensorArrayProperties(bool* inIsUsed) {

	//from command port, read all infos relavant for the sensor array 
	//original configs
	bciout << " Getting current sensor configs";

	for (int IDsensor = 0; IDsensor < NSENSORS_MAX; ++IDsensor)
	{
		this->mSensorArray[IDsensor].id = IDsensor + 1;

		//if we see an one in position IDsensor
		if (inIsUsed[IDsensor])
		{
			// Double check it's paired
			this->mSensorArray[IDsensor].getSensorProperties();
			
		}
	}
	return TRIGNOSDK_ALL_OK;
}

TRIGNOSDK_ERROR DelsysTrigno::setSensorArrayProperties(int* modes, bool* isUsed) {

	bciout << " Configuring sensors: ";

	for (int IDsensor = 0; IDsensor < NSENSORS_MAX; ++IDsensor)
	{
		this->mSensorArray[IDsensor].id = IDsensor + 1;

		//if we see an one in position IDsensor
		if (isUsed[IDsensor])
		{
			//set the this
			this->mSensorArray[IDsensor].isNeededInThisExperiment = true;
			

			// parse the modes, This code assumes mode is 0 to 999 followed by 
			constexpr int MAX_MODE_DIGITS = 4;

			char mode[MAX_MODE_DIGITS] = { 0 };
			sprintf_s(mode, "%d", modes[IDsensor]);
				
			this->mSensorArray[IDsensor].setSensorProperties(mode);

		}
		else {
			//set the this
			this->mSensorArray[IDsensor].isNeededInThisExperiment = false;
		}
	}
	
	return TRIGNOSDK_ALL_OK;
}

TRIGNOSDK_ERROR DelsysTrigno::baseStation::getBaseStationProperties(void) {

	//from command port, read all infos relavant for the base station
	//original configs
	bciout << " Getting base station configs:";

	//get current setting from the base station and save them into original base station
	getBackwardCompatibility();
	getUpsampling();
	getFrameInterval();
	getMaxSamplesEmg();
	getMaxSamplesAux();
	getEndianness();
	getBaseFirmware();
	getBaseSerial();

	bciout << "backwards compatibility: " << this->backwardsCompatibility;
	bciout << "upsampling: " << this->upsampling;
	bciout << "frame interval: " << this->frameInterval;
	bciout << "max samples emg: " << this->emgMaxSamples;
	bciout << "max samples aux: " << this->auxMaxSamples;
	bciout << "big endianness: " << this->bigEndianness;
	bciout << "fw: " << this->firmwareRevision;
	bciout << "serial: " << this->baseSerial;

	return TRIGNOSDK_ALL_OK;
}

TRIGNOSDK_ERROR DelsysTrigno::baseStation::setBaseStationProperties(const char* inBackwardsCompatibility, const char* inUpsampling) {

	//from command port, read all infos relavant for the base station
	//original configs
	bciout << " Configuring base station";

	this->setBackwardCompatibility(inBackwardsCompatibility);
	this->setUpsampling(inUpsampling);

	//endianness and upsampling we can leave as is for now

	return TRIGNOSDK_ALL_OK;

}

TRIGNOSDK_ERROR DelsysTrigno::readFrame(char* pOutEmgBlockData, char* pOutOrientationBlockData, int inNumberEmgSamplesPerBlock, int inNumberImuSamplesPerBlock){

	constexpr uint16_t TRIGNOSDK_TIMEOUTMS = 15;  // frames have a fixed 13.5ms, so that would be more than 1 frame delay. A block might be 3 frame, for instance

	TRIGNOSDK_ERROR err = TRIGNOSDK_ALL_OK;

	//emg read
	int ready = mEmgSocket.wait_for_read(TRIGNOSDK_TIMEOUTMS);
	//peek at the number of data available
	if (ready)
	{
		mEmgStream.read(pOutEmgBlockData, sizeof(float) * NSENSORS_MAX * inNumberEmgSamplesPerBlock);
	}
	else
	{
		err = TRIGNOSDK_TIMEOUT_READ_EMG_FAIL;
		bciout << "frame drop"; //feedback online is useful
	}

	//auxiliary read
	ready = mAccSocket.wait_for_read(TRIGNOSDK_TIMEOUTMS);
	if (ready)
	{
		mAccStream.read(pOutOrientationBlockData, sizeof(float) * inNumberImuSamplesPerBlock * NSENSORS_MAX * (NCHANNELS_MAX - 1));
	}
	else
	{
		//a little logic to encode 2 faults into a single error variable:
		//if you have already seen an emg fail, update to both failure
		//otherwise it's only the IMU read that failed
		if (err == TRIGNOSDK_TIMEOUT_READ_EMG_FAIL) { err = TRIGNOSDK_TIMEOUT_READ_BOTH_FAIL; }
		else { err = TRIGNOSDK_TIMEOUT_READ_IMU_FAIL; }

		bciout << "frame drop"; //feedback online is useful
	}

	return err;

}

TRIGNOSDK_ERROR DelsysTrigno::closeCommWithServer(void)
{

	if (mCommSocket.is_open() && mCommStream.is_open())
	{
		mCommSocket.close();
		mCommStream.close();
	}

	if (mEmgSocket.is_open() && mEmgStream.is_open())
	{
		mEmgSocket.close();
		mEmgStream.close();
	}
	if (mAccSocket.is_open() && mAccStream.is_open())
	{
		mAccSocket.close();
		mAccStream.close();
	}

	return TRIGNOSDK_ALL_OK;

}

TRIGNOSDK_ERROR DelsysTrigno::sendStartCommand(void) {

	TRIGNOSDK_ERROR error = TRIGNOSDK_ALL_OK;	
	std::string reply = SendCommandWithResponse("START\r\n\r\n");

	if ( reply == "OK" ) { error = TRIGNOSDK_ALL_OK; }
	else {
		error = TRIGNOSDK_START_COMMAND_FAIL;
		bciwarn << "start failed";
	}

	return error;
}

TRIGNOSDK_ERROR DelsysTrigno::sendStopCommand(void) {

	TRIGNOSDK_ERROR error = TRIGNOSDK_ALL_OK;
	std::string reply = SendCommandWithResponse("QUIT\r\n\r\n");

	//expected reply: OK. cfr SDK User Guide
	if ( reply == "OK" ) { error = TRIGNOSDK_ALL_OK; }
	else
	{
		error = TRIGNOSDK_STOP_COMMAND_FAIL;
		bciwarn << "Stop command did not return ok.";
	}

	return error;
}

std::string DelsysTrigno::SendCommandWithResponse(const char* pInCommand)
{
	constexpr int COMMAND_TIMEOUT = 5000; //start command is the slowest, takes a few seconds at least
	int n = 0;
	bool ready = false;

	int error = mCommSocket.send(pInCommand, (int)strlen(pInCommand), 0);
	std::string response = "";

	if (error == -1){
		bciwarn << "Send error, cmd:" << pInCommand;
	}
	else // sent successfully
	{
		ready = mCommSocket.wait_for_read(COMMAND_TIMEOUT);
		if (ready) {

			uint16_t bytesAvailable = mCommSocket.in_avail();
			std::vector<char> tmp(bytesAvailable, 0);
			n = mCommSocket.recv(tmp.data(), tmp.size(), 0);

			if (n == -1)
			{
				bciwarn << "Reception error, cmd:" << pInCommand;
			}
			else // sent and received successfully
			{
				//append removing the last 4 chars, i.e. \r\n\r\n at the end
				response += std::string(tmp.data(), tmp.size() - 4);
			}
		}
		else 
		{
			bciwarn << "timeout, cmd: " << pInCommand;
		}
	}

	return response;
}

//base property getters
void DelsysTrigno::baseStation::getBackwardCompatibility(void)
{
	std::string backwardCompatible = SendCommandWithResponse("BACKWARDS COMPATIBILITY?\r\n\r\n");
	if (backwardCompatible == "YES")
	{
		this->backwardsCompatibility = 1;
	}
	else if (backwardCompatible == "NO")
	{
		this->backwardsCompatibility = 0;
	}
	else
	{
		bciwarn << "Invalid backwards compatibility.";
	}
}

void DelsysTrigno::baseStation::getUpsampling(void)
{
	std::string upsampling = SendCommandWithResponse("UPSAMPLING?\r\n\r\n");
	
	if (upsampling == "UPSAMPLING ON")
	{
		this->upsampling = 1;
	}
	else if (upsampling == "UPSAMPLING OFF")
	{
		this->upsampling = 0;
	}
	else
	{
		bciwarn << "Invalid upsampling: " << upsampling;
	}
}
	
void DelsysTrigno::baseStation::getFrameInterval(void)
{
	std::string frameInterval = SendCommandWithResponse("FRAME INTERVAL?\r\n\r\n");
	float frameIntervalNumerical = stof(frameInterval);
	if (frameIntervalNumerical > 0.0 && frameIntervalNumerical < 1.0) //range of sensible values
	{
		this->frameInterval = frameIntervalNumerical;
	}
	else 
	{
		this->frameInterval = 0.0;
		bciwarn << "Invalid frame interval";
	} 
}
	
void DelsysTrigno::baseStation::getMaxSamplesEmg(void) {
	
	std::string maxSamplesEmg = SendCommandWithResponse("MAX SAMPLES EMG?\r\n\r\n");
	uint16_t maxSamplesEmgNumerical = stoi(maxSamplesEmg); //integer type do not match, be careful here. might be wirth masking
	if (maxSamplesEmgNumerical > 0 && maxSamplesEmgNumerical < 1000) //range of sensible values
	{
		this->emgMaxSamples = maxSamplesEmgNumerical;
	}
	else 
	{
		this->emgMaxSamples = 0;
		bciwarn << "Invalid max emg samples";
	}
}
	
void DelsysTrigno::baseStation::getMaxSamplesAux(void) {

	std::string maxSamplesAux = SendCommandWithResponse("MAX SAMPLES AUX?\r\n\r\n");
	uint16_t maxSamplesAuxNumerical = (uint16_t) stoul(maxSamplesAux);
	if (maxSamplesAuxNumerical < 1000) //range of sensible values
	{
		this->auxMaxSamples = maxSamplesAuxNumerical;
	}
	else 
	{
		this->auxMaxSamples = 0;
		bciwarn << "Invalid max aux samples";
	}
}
	
void DelsysTrigno::baseStation::getEndianness(void) {

	std::string endianness = SendCommandWithResponse("ENDIANNESS?\r\n\r\n");
	if (endianness == "BIG") //range of sensible values
	{
		this->bigEndianness = 1;
	}
	else if (endianness == "LITTLE")
	{
		this->bigEndianness = 0;
	}
	else 
	{
		bciwarn << "Invalid endianness";
	}

}
	
void DelsysTrigno::baseStation::getBaseFirmware(void) {

	std::string baseFirmware = SendCommandWithResponse("BASE FIRMWARE?\r\n\r\n");
	if (baseFirmware.find("Firmware:") != std::string::npos) 
	{
		strncpy_s(this->firmwareRevision, baseFirmware.c_str(), strlen(this->firmwareRevision));
	}
	else
	{
		bciwarn << "Invalid base firmware";
	}

}
	
void DelsysTrigno::baseStation::getBaseSerial(void) {

	std::string baseSerial = SendCommandWithResponse("BASE SERIAL?\r\n\r\n");
	if (baseSerial.find("BID:")!= std::string::npos) //range of sensible values
	{
		strncpy_s(this->baseSerial, baseSerial.c_str(), strlen(this->baseSerial));
	}
	else
	{
		bciwarn << "Invalid base serial";
	}

}

//sensor property getters
void DelsysTrigno::sensor::getSensorProperties(void) {

	this->getSensorPairingStatus();
	//if not, return an error and don't perform the next functions
	if (this->isPaired)
	{
		//read from server
		this->getSensorSerial();
		this->getSensorFirmware();
		this->getSensorType();
		this->getSensorStartIndex();
		this->getSensorTotalChannelCount();
		this->getSensorEmgChannelCount();
		this->getSensorAuxChannelCount();
		this->getSensorMode();

		for (int IDchannel = 0; IDchannel < this->totalChannelCount; IDchannel++) //channels are 0-indexed here, but 1-indexed for the SDK (and debug)
		{
			this->getChannelGain(IDchannel);
			this->getChannelUnits(IDchannel);
			this->getChannelSamplesPerFrame(IDchannel);
			this->getChannelSampleRate(IDchannel);

		}

		//convert to strings for feedback
		char idString[5] = { 0 };
		sprintf_s(idString, "%d", this->id);
		char totalChannelCountString[5] = { 0 };
		sprintf_s(totalChannelCountString, "%d", this->totalChannelCount);
		char emgChannelCountString[5] = { 0 };
		sprintf_s(emgChannelCountString, "%d", this->emgChannelCount);
		char auxChannelCountString[5] = { 0 };
		sprintf_s(auxChannelCountString, "%d", this->auxChannelCount);

		bciout << "Sensor " << idString << " serial " << this->serial;
		bciout << "Sensor " << idString << " fw " << this->firmware;
		bciout << "Sensor " << idString << " type " << this->type;
		bciout << "Sensor " << idString << " start index " << this->startIndex;
		bciout << "Sensor " << idString << " channel count " << totalChannelCountString;
		bciout << "Sensor " << idString << " emg channels " << emgChannelCountString;
		bciout << "Sensor " << idString << " aux channels " << auxChannelCountString;
		bciout << "Sensor " << idString << " mode " << this->mode;
		bciout << "Sensor " << idString << " sample rate emg " << this->channels[0].sampleRateHz;
		bciout << "Sensor " << idString << " sample rate aux " << this->channels[1].sampleRateHz;
	}
	else 
	{
		char idString[5] = { 0 };
		sprintf_s(idString, "%d", this->id);
		bcierr << "Sensor " << idString << " unpaired. Please pair it with the station.";
	}
}

void DelsysTrigno::sensor::getSensorFirmware(void) {

	char command[25] = { 0 };
	sprintf_s(command, "SENSOR %d FIRMWARE?\r\n\r\n", this->id);
	std::string sensorFw = SendCommandWithResponse(command);
	if (sensorFw.find("INVALID") != std::string::npos) //range of sensible values
	{
		bciwarn << "Invalid sensor firmware";
	}
	else
	{
		strncpy_s(this->firmware, sensorFw.c_str(), strlen(this->firmware));
	}
}

void DelsysTrigno::sensor::getSensorSerial(void) {
	
	char command[25] = { 0 };
	sprintf_s(command, "SENSOR %d SERIAL?\r\n\r\n", this->id);
	std::string sensorSerial = SendCommandWithResponse(command);
	if (sensorSerial.find("SID") != std::string::npos)
	{
		strncpy_s(this->serial, sensorSerial.c_str(), strlen(this->serial));
	}
	else
	{
		bciwarn << "Invalid sensor serial";
	}
}

void DelsysTrigno::sensor::getSensorType(void) {
	
	char command[25] = { 0 };
	sprintf_s(command, "SENSOR %d TYPE?\r\n\r\n", this->id);
	std::string sensorType = SendCommandWithResponse(command);
	if ((sensorType.find("O") != std::string::npos) || 
			(sensorType.find("K") != std::string::npos) ||
			(sensorType.find("A") != std::string::npos) ||
			(sensorType.find("D") != std::string::npos)) // O = avanti, K = input analog, A = legacy, D = standard. Add more here if needed
	{
		strncpy_s(this->type, sensorType.c_str(),strlen(this->type));
	}
	else
	{
		bciwarn << "Invalid sensor type. For now only Avanti and Analog O and K are allowed. Add to the function getSensorType for new sensor types";
	}
}

void DelsysTrigno::sensor::getSensorStartIndex(void) {

	char command[35] = { 0 };
	sprintf_s(command, "SENSOR %d STARTINDEX?\r\n\r\n", this->id);
	std::string startIndex = SendCommandWithResponse(command);
	uint32_t startIndexNumerical = (uint32_t) stoul(startIndex);
	if (startIndexNumerical < 3000){ 
		this->startIndex = startIndexNumerical;
	}
	else
	{
		bciwarn << "Invalid sensor start index";
		this->startIndex = 0;
	}
}

void DelsysTrigno::sensor::getSensorTotalChannelCount(void) {

	char command[35] = { 0 };
	sprintf_s(command, "SENSOR %d CHANNELCOUNT?\r\n\r\n", this->id);
	std::string channelCount = SendCommandWithResponse(command);
	uint32_t channelCountNumerical = (uint32_t) stoul(channelCount);
	if (channelCountNumerical < NCHANNELS_MAX) { //range of sensible values, O - 10

		this->totalChannelCount = (channelCountNumerical & 0x000000FF);
	}
	else
	{
		bciwarn << "Invalid sensor total channel count";
		this->totalChannelCount = 0;
	}
}

void DelsysTrigno::sensor::getSensorEmgChannelCount(void) {

	char command[35] = { 0 };
	sprintf_s(command, "SENSOR %d EMGCHANNELCOUNT?\r\n\r\n", this->id);
	std::string channelCount = SendCommandWithResponse(command);
	uint32_t channelCountNumerical = (uint32_t) stoul(channelCount);
	if (channelCountNumerical < NCHANNELS_MAX) { //range of sensible values, O - 10

		this->emgChannelCount = (channelCountNumerical & 0x000000FF);
	}
	else
	{
		bciwarn << "Invalid sensor emg channel count";
		this->emgChannelCount = 0;
	}

}

void DelsysTrigno::sensor::getSensorAuxChannelCount(void) {

	char command[35] = { 0 };
	sprintf_s(command, "SENSOR %d AUXCHANNELCOUNT?\r\n\r\n", this->id);
	std::string channelCount =SendCommandWithResponse(command);
	uint32_t channelCountNumerical = (uint32_t) stoul(channelCount);
	if (channelCountNumerical < NCHANNELS_MAX) { 
		this->auxChannelCount = (channelCountNumerical & 0x000000FF); 
	}
	else
	{
		bciwarn << "Invalid sensor aux channel count";
		this->auxChannelCount = 0;
	}

}

void DelsysTrigno::sensor::getSensorMode(void) {

	constexpr uint32_t MAXMODE = 400;//cfr SDK user guide
	
	char command[35] = { 0 };
	sprintf_s(command, "SENSOR %d MODE?\r\n\r\n", this->id);
	std::string sensorMode = SendCommandWithResponse(command);
	uint32_t modeNumerical = (uint32_t) stoul(sensorMode);
	if (modeNumerical < MAXMODE)
	{
		strncpy_s(this->mode, sensorMode.c_str(), strlen(this->mode)); 
	}
	else
	{
		bciwarn << "Invalid sensor mode";
	}
}

void DelsysTrigno::sensor::getSensorPairingStatus(void) {

	char command[25] = { 0 };
	sprintf_s(command, "SENSOR %d PAIRED?\r\n\r\n", this->id);
	std::string isPaired = SendCommandWithResponse(command);

	if (isPaired == "YES"){ this->isPaired = true; }
	else									{	this->isPaired = false; }
}

void DelsysTrigno::sensor::getChannelGain(int IDchannel) { 

	char command[45] = { 0 };
	sprintf_s(command, "SENSOR %d CHANNEL %d GAIN?\r\n\r\n", this->id, IDchannel + 1); //1 indexed in the SDK. Cfr SDK User Guide
	std::string gain = SendCommandWithResponse(command);
	uint32_t gainNumerical = (uint32_t) stoul(gain);
	if (gainNumerical < 10000)	{ this->channels[IDchannel].gain= gainNumerical; }
	else												{ this->channels[IDchannel].gain = 0; }
}

void DelsysTrigno::sensor::getChannelUnits(int IDchannel) { 

	char command[45] = { 0 };
	sprintf_s(command, "SENSOR %d CHANNEL %d UNITS?\r\n\r\n", this->id, IDchannel+1); //channels are 1-indexed
	std::string units = SendCommandWithResponse(command);
	if ((units.find("V") != std::string::npos) || 
			(units.find("g") != std::string::npos) || 
			(units.find("Q") != std::string::npos)) //Volts for EMG, g for accelerometer, Q for IMU quaternions
	{
		strncpy_s(this->channels[IDchannel].units, units.c_str(), strlen(this->channels[IDchannel].units)); //careful here about max sizes,
	}
	else
	{
		bciwarn << "Invalid units for channel" << IDchannel;
	}
}

void DelsysTrigno::sensor::getChannelSamplesPerFrame(int IDchannel) {

	char command[45] = { 0 };
	sprintf_s(command, "SENSOR %d CHANNEL %d SAMPLES?\r\n\r\n", this->id, IDchannel+1);
	std::string samplesPerFrame = SendCommandWithResponse(command);
	uint32_t samplesPerFrameNumerical = (uint32_t) stoul(samplesPerFrame); 
	if (samplesPerFrameNumerical < 200) 
	{
		this->channels[IDchannel].samplesPerFrame = samplesPerFrameNumerical; 
	}
	else
	{
		this->channels[IDchannel].samplesPerFrame = 0;
		bciwarn << "Invalid samples for channel" << IDchannel+1;
	}
}

void DelsysTrigno::sensor::getChannelSampleRate(int IDchannel) { 

	char command[45] = { 0 };
	sprintf_s(command, "SENSOR %d CHANNEL %d RATE?\r\n\r\n", this->id, IDchannel+1);
	std::string samplesRate = SendCommandWithResponse(command);
	uint32_t samplesRateNumerical = (uint32_t) stoul(samplesRate);
	if (samplesRateNumerical < 5000) //range of sensible values, O = avanti, K = input analog
	{
		this->channels[IDchannel].sampleRateHz = samplesRateNumerical; 
	}
	else
	{
		this->channels[IDchannel].sampleRateHz = 0;
		bciwarn << "Invalid rate for channel" << IDchannel + 1;
	}
}

//base property setter
void DelsysTrigno::baseStation::setBackwardCompatibility(const char* inSetting)
{
	char command[128] = { 0 }; 
	sprintf_s(command, "BACKWARDS COMPATIBILITY %s\r\n\r\n", inSetting);
	std::string response = SendCommandWithResponse(command);
	if ((!strncmp(inSetting, "ON", 2)) && (response == "OK"))
	{
		this->backwardsCompatibility = true;
	}
	else if( !strncmp(inSetting, "OFF", 3) && (response == "OK") )
	{
		this->backwardsCompatibility = false;
	}
	else
	{
		bciwarn << "Unable to set backwards compatibility.";
	}
}

void DelsysTrigno::baseStation::setUpsampling(const char* inSetting)
{
	char command[128] = { 0 };
	sprintf_s(command, "UPSAMPLE %s\r\n\r\n", inSetting);
	std::string response = SendCommandWithResponse(command);
	if ((!strncmp(inSetting, "ON", 2)) && (response == "OK"))
	{
		this->upsampling = true;
	}
	else if ((!strncmp(inSetting, "OFF", 3)) && (response == "OK"))
	{
		this->upsampling = false;
	}
	else
	{
		bciwarn << "Invalid upsampling: " << inSetting << " response: " << response;
	}
}

void DelsysTrigno::baseStation::setEndianness(const char* inSetting)
{
	char command[128] = { 0 };
	sprintf_s(command, "ENDIAN %s\r\n\r\n", inSetting);
	std::string response = SendCommandWithResponse(command);
	if ((!strncmp(inSetting, "BIG",3)) && (response == "OK"))
	{
		this->bigEndianness = true;
	}
	else if ((!strncmp(inSetting, "LITTLE", 6)) && (response == "OK"))
	{
		this->bigEndianness = false;
	}
	else
	{
		bciwarn << "Invalid endianness";
	}
}

//sensor setters
void DelsysTrigno::sensor::setSensorProperties(char* mode) {

	//set the mode
	this->setSensorMode(mode);

	//pair if unpaired
	this->getSensorPairingStatus();
	//pair if not paired yet
	if (!this->isPaired)
	{
		this->pair();
	}
	
}

void DelsysTrigno::sensor::setSensorMode(const char* inSetting) {

	char command[128] = { 0 };
	sprintf_s(command, "SENSOR %d SETMODE %s\r\n\r\n", this->id, inSetting);
	std::string response = SendCommandWithResponse(command);

	if (response.find("SENSOR") != std::string::npos)
	{
		strncpy_s(this->mode, inSetting, strlen(this->mode)); 
	}
	else
	{
		bciwarn << "Invalid sensor mode";
	}
}

void DelsysTrigno::sensor::pair(void) {

	char command[128] = { 0 };
	sprintf_s(command, "SENSOR %d PAIR\r\n\r\n", this->id);
	std::string response = SendCommandWithResponse(command);

	if ((response.find("PAIR") != std::string::npos) || 
			(response.find("SENSOR") != std::string::npos)) //To be checked, this might be different for trigno
	{
		this->isPaired = true; //careful here about max sizes,
	}
	else
	{
		bciwarn << "cannot pair sensor " << this->id;
	}
}

//range and bandwidth are not documented

double DelsysTrigno::getEmgSamplingFrequencyFromMode(int inMode)
{
	//for simplicity, do it with a switch case.
	float f = 1.0f;
	switch (inMode)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
			//should keep going
			f = 1925.925;
			break;
		case 67:
			f = 1481.48148;
			break;
		case 134:
			f = 2370.370370; //assuming 13.5ms and 32 samples per frame.
			break;
		default:
			f = 2000.0;
			//return an error probably
			bciwarn << "mode" << inMode << "not in the list. Emg sampling frequency may not be reliable";
			break;
	}
	return f;
}

double DelsysTrigno::getAuxSamplingFrequencyFromMode(int inMode)
{
	//for simplicity, do it with a switch case.
	float f = 1.0f;
	switch (inMode)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
		f = 148.148148148;
		break;
	case 40:
		f = 0;
		break;
		//should keep going
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
	case 49:
		f = 74.074074;
		break;
		//should keep going
	case 67:
		f = 74.074074;
		break;
	case 134:
		f = 222.222222;// that is 3 samples per frame
		break;
	default:
		f = 74.074074;
		//return an error probably
		bciwarn << "mode" << inMode << "not in the list.Aux sampling frequency may not be reliable";
		break;
	}
	return f;
}

TRIGNOSDK_ERROR DelsysTrigno::checkAllSocketsAreOpen(void) const
{
	if (!mCommSocket.is_open())
	{
		bciwarn << "Could not connect to server at command port" << "127.0.0.1:50040" << ". "
			<< "Make sure the Delsys Control Utility is running";

		return TRIGNOSDK_COMM_SOCKET_FAIL;
	}
	else
	{
		bciout << "Command socket opened successfully";
	}

	if (!mEmgSocket.is_open())
	{
		bciwarn << "Could not connect to server at emg port" << "127.0.0.1:50043" << ". "
			<< "Make sure the Delsys Control Utility is running";

		return TRIGNOSDK_EMG_SOCKET_FAIL;
	}
	else
	{
		bciout << "Command socket opened successfully";
	}

	if (!mAccSocket.is_open())
	{
		bciwarn << "Could not connect to server at imu port" << "127.0.0.1:50044" << ". "
			<< "Make sure the Delsys Control Utility is running";

		return TRIGNOSDK_IMU_SOCKET_FAIL;
	}
	else
	{
		bciout << "Command socket opened successfully";
	}

	return TRIGNOSDK_ALL_OK;

}


