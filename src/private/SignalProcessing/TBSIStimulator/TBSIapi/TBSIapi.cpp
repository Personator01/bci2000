// TBSIapi.cpp : Defines the exported functions for the DLL application.
//

#include "TBSIapi.h"

//Taken from the code (tbsi.h & tbsi.cpp) given by Rob/Harry from Triangle Biosystems
// Please look into Stimware's Setup.ini for detailed configuration
const static BYTE  csIndex = 0;
const static BYTE  spiMode = 0x3B;
const static BYTE  SPI_DELAY_INTERVAL_MASK = 1;
const static DWORD CMD_PACKET_SIZE = 128;
const static DWORD ACK_PACKET_SIZE = 32;
const static DWORD BUFFER_SIZE = 255;
const static BYTE  RELEASE_AFTER_TRANSFER = 1;
const static int   WRITE_MAX_WAIT_MS = 500;
const static int   READ_MAX_WAIT_MS = 500;
const static int   ACK_NACK_MAX_WAIT_MS = 7000; //approximate amount of time before dongle gives up. Don't send any other commands while waiting on ack.
const static int   DRAIN_READ_MAX_WAIT_MS = 100;
const static int   GET_RTR_TIME_DELAY_MS = 2;

//Custom errors overlayed on top of SLAB_USB_SPI error codes. Only use indexes not used by SLAB.
const static BYTE ACK_PACKET_TIMEOUT_ERROR = 0x06;
const static BYTE ACK_PACKET_RTR_TIMEOUT_ERROR = 0x08;
const static BYTE ACK_PACKET_TRANSFER_ERROR = 0x09;


void checkErrCode(int& errCode);
void tbsiSendReceive(int& errCode, CP213x_DEVICE hDevice, char* input, bool &acked);
void parseReceiveAck(int& errCode, BYTE* readBuf, int size, int expected_size, bool &acked);
void TBSI_InterfaceInit(int& errCode, CP213x_DEVICE hDevice, BYTE csIndex, BYTE spiMode);
void TBSI_ChipSelect(int& errCode, CP213x_DEVICE hDevice, BYTE csIndex, BYTE csMode);
void TBSI_SpiDelayConfig(int& errCode, CP213x_DEVICE hDevice);
void TBSI_PinConfig(int& errCode, CP213x_DEVICE hDevice, int  pinOffset, BYTE config);
void TBSI_SendCmd(int& errCode, CP213x_DEVICE hDevice, char* input);
DWORD TBSI_ReceiveAck(int& errCode, CP213x_DEVICE hDevice, bool &acked);
bool Get_RTR_State(int& errCode, CP213x_DEVICE hDevice);

void  encodeData(char* input, int length, BYTE* encoded);
char* decodeData(BYTE* encoded, int bytes_length, int word_position);


//Raw strings with properly formatted headers and footers. Still need everything else to be set.
Pattern rawCmdPacket = "AAAA000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000005555";

//CMD_ACK strings
char CMD_ACK_WAKE_STIM[3] = "01"; //Turn ON
char NACK_WAKE_STIM[3] = "11"; //Turn ON
char CMD_ACK_SLEEP_STIM[3] = "02"; //Turn OFF
char NACK_SLEEP_STIM[3] = "12"; //Turn OFF
char CMD_ACK_TRIGGER_GUI_STIM[3] = "03"; //Software trigger
char NACK_TRIGGER_GUI_STIM[3] = "13"; //Software trigger
char CMD_ACK_SEARCH_STIM[3] = "23";
char NACK_SEARCH_STIM[3] = "33";
char CMD_ACK_SELECT_STIM[3] = "24";
char NACK_SELECT_STIM[3] = "34";
char CMD_ACK_GET_DONGLE_STATUS[3] = "66";
char NACK_GET_DONGLE_STATUS[3] = "76";
char CMD_ACK_TRIGGER_GUI_STIM_STOP[3] = "42";
char NACK_TRIGGER_GUI_STIM_STOP[3] = "52";
char CMD_ACK_HIBERNATE_STIM[3] = "82";
char CMD_ACK_DOWNLOAD_PATTERN[3] = "08";
char NACK_DOWNLOAD_PATTERN[3] = "18";
char CMD_ACK_HW_TRIGGER[3] = "40";
char NACK_HW_TRIGGER[3] = "50";

///////////////////////////////////////////////
// ack and nack codes for the following commands are needed
// nack- hibernate
// ack and nack for hardware trigger command - Enable and disable
//////////////////////////////////////////////

TBSI_API_RESULT SearchDongle(DeviceIndex & largest_device_index)
{
	//Find out how many dongles are connected to PC USB ports. Only 1 device handled for now.
	int errCode = CP213x_GetNumDevices(&largest_device_index);

	if (largest_device_index != 1) {
		std::cerr << "Check your device connection. Only 1 dongle should be plugged in" << std::endl;
		return NO_DEVICE_FOUND;
	}
	checkErrCode(errCode);

	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		std::cout << "TBSI_Status: GetNumDevices: " << largest_device_index << std::endl;
		return NO_ERROR_API;
	}
	return NO_DEVICE_FOUND;
}

TBSI_API_RESULT OpenDongleConnection(DongleHandle * device_handle, DeviceIndex device_index)
{
	int errCode = CP213x_OpenByIndex(device_index, reinterpret_cast<CP213x_DEVICE*>(device_handle));
	checkErrCode(errCode);
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		std::cout << "TBSI_Status: Connection opened to dongle. Device handle: " << device_handle << std::endl;
		
		TBSI_InterfaceInit(errCode, *device_handle, csIndex, spiMode);				 // cdIndex = 0, spiMode = 0x3b
		TBSI_ChipSelect(errCode, *device_handle, csIndex, CSMODE_ACTIVE_OTHERS_IDLE); // csIndex = 0, spiMode = 2
		TBSI_SpiDelayConfig(errCode, *device_handle);								 // SPI_DELAY_INTERVAL_MASK = 1
		TBSI_PinConfig(errCode, *device_handle, 3, 5);								//pinOffset=GPIO.3=RTR pin=3,0x05=active high...default was active low=0x04

		if (errCode != USB_SPI_ERRCODE_SUCCESS) {
			std::cerr << "TBSI_Status: Dongle configuration failure." << std::endl;
			return NO_CONFIGURATION_SET;
		}
		else
		{
			std::cout << "TBSI_Status: Dongle configuration success! Device handle: " << device_handle << std::endl;
			return NO_ERROR_API;
		}
	}
	else {
		std::cerr << "TBSI Connection to dongle failed" << std::endl;
		return NO_CONNECTION_OPENED;
	}
}

TBSI_API_RESULT GetDongleStatus(DongleHandle device_handle, bool & status)
{
	//init local str with globally defined rawCmdPacket
	Pattern str;
	bool acked = false;
	strncpy_s(str,rawCmdPacket,PATTERN_LENGTH);
	//set 4th and 5th character of string packet to format the cmd characters
	str[4] = CMD_ACK_GET_DONGLE_STATUS[0];
	str[5] = CMD_ACK_GET_DONGLE_STATUS[1];

	std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_GET_DONGLE_STATUS << "' getDongleStatus Cmd." << std::endl;
	
	int errCode = 0;
	tbsiSendReceive(errCode, device_handle, str, acked);
	
	if (errCode != USB_SPI_ERRCODE_SUCCESS || !acked) {
		std::cerr << "TBSI_Status: getDongleStatus Cmd failed." << std::endl;
		status = false;
		return NO_RESPONSE;
	}
	status = true;
	return NO_ERROR_API;
}

TBSI_API_RESULT CloseDongleConnection(DongleHandle device_handle)
{
	//check dongle status before closing
	bool device_status = false;
	GetDongleStatus(device_handle, device_status);
	//device not connected
	if (!device_status)
	{
		std::cout << "No connected device found" << std::endl;
		return NO_CONNECTION_CLOSED_SUCCESSFULLY;
	}
	//if device is connected, close device
	std::cout << "TBSI_Status: Closing connection to device handle: " << device_handle << std::endl;
	int errCode = CP213x_Close(device_handle); //Removed reference &. Should not have needed.
	if (errCode != USB_SPI_ERRCODE_SUCCESS) {
		std::cerr << "TBSI_Status: Error when closing device handle." << std::endl;
		checkErrCode(errCode);
		return NO_CONNECTION_CLOSED_SUCCESSFULLY;
	}
	return NO_ERROR_API;
}

TBSI_API_RESULT SetupDongle(DongleHandle * device_handle, bool &deviceOpen)
{
	DeviceIndex largestDeviceIndex = 0;
	TBSI_API_RESULT result = SearchDongle(largestDeviceIndex);
	if (result == NO_ERROR_API) {
		
		result = OpenDongleConnection(device_handle, largestDeviceIndex - 1);
		deviceOpen = result == NO_ERROR_API ? true : false;
		std::cout << "Result in Setup Dongle " << result << std::endl;
		return result;
	}
	else {
		std::cerr << "No device found" << std::endl;
		return NO_DEVICE_FOUND;
	}
}

TBSI_API_RESULT SetFieldState(DongleHandle device_handle, void * cage_address, bool state)
{
	return TBSI_API_RESULT();
}


TBSI_API_RESULT FindStimulators(DongleHandle device_handle, StimulatorAdd stimulator_address[], uint32_t num_of_stimulators, StimulatorAdd found_stim_add[], uint32_t &found_num_stims)
{	
	//testing parameters received
	std::cout << "number of stimulators: " << num_of_stimulators << std::endl;
	std::cout << "stimulator address: " << stimulator_address[0] <<" " << stimulator_address[1] << std::endl;
	std::cout << "Device handle: " << device_handle << std::endl;
	//init local str with globally defined rawCmdPacket
	Pattern str;
	bool acked = false;
	found_num_stims = 0;
	strncpy_s(str,rawCmdPacket,PATTERN_LENGTH);
	//set 4th and 5th character of string packet to format the cmd characters
	str[4] = CMD_ACK_SEARCH_STIM[0];
	str[5] = CMD_ACK_SEARCH_STIM[1];
	//Search for all stimulators
	for (uint32_t j = 0; j < num_of_stimulators; j++)
	{
		acked = false;
		//set 6th,7th,8th,9th characters to correct stim address.
		for (uint32_t i = 0; i < 4; i++)
			str[i + 6] = stimulator_address[j][i];

		std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_SEARCH_STIM << "' searchStim Cmd." << std::endl;
		std::cout << "Stimulator Address " << stimulator_address[j] << std::endl;
		int errCode = 0;
		tbsiSendReceive(errCode, device_handle, str, acked);

		std::cout << "errcode: " << errCode << std::endl;
		if (errCode != USB_SPI_ERRCODE_SUCCESS || !acked) {
			std::cerr << "TBSI_Status: searchStim Cmd failed." << std::endl;
			std::cerr << "Stimulator Address " << stimulator_address[j] << std::endl;
		}
		else
		{
			strncpy_s(found_stim_add[found_num_stims], stimulator_address[j], STIMULATOR_LENGTH);
			++found_num_stims;
		}
	}
	if (found_num_stims == 0)
		return NO_RESPONSE;
	return NO_ERROR_API;
}

TBSI_API_RESULT ConnectStimulator(DongleHandle device_handle, StimulatorAdd stimulator_address)
{
	//init local str with globally defined rawCmdPacket
	Pattern str;
	bool acked = false;
	strncpy_s(str, rawCmdPacket, PATTERN_LENGTH);
	//First search stimulator
	str[4] = CMD_ACK_SEARCH_STIM[0];
	str[5] = CMD_ACK_SEARCH_STIM[1];
	for (uint32_t i = 0; i < 4; i++)
		str[i + 6] = stimulator_address[i];
	
	std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_SEARCH_STIM << "' searchStim Cmd." << std::endl;
	std::cout << "Stimulator Address " << stimulator_address << std::endl;
	int errCode = 0;
	tbsiSendReceive(errCode, device_handle, str,acked);

	if (errCode != USB_SPI_ERRCODE_SUCCESS || !acked) {
		std::cerr << "TBSI_Status: searchStim Cmd failed." << std::endl;
		std::cerr << "Stimulator Address " << stimulator_address << std::endl;
		return  NO_RESPONSE;
	}

	acked = false;
	//Stimulator Found - select stimulator
	//set 4th and 5th character of string packet to format the cmd characters
	str[4] = CMD_ACK_SELECT_STIM[0];
	str[5] = CMD_ACK_SELECT_STIM[1];
	
	//set 6th,7th,8th,9th characters to correct stim address.
	for (uint32_t i = 0; i < 4; i++)
		str[i + 6] = stimulator_address[i];

	std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_SELECT_STIM << "' selectStim Cmd." << std::endl;
	std::cout << "Stimulator Address " << stimulator_address << std::endl;
	
	tbsiSendReceive(errCode, device_handle, str,acked);

	if (errCode != USB_SPI_ERRCODE_SUCCESS || !acked) {
		std::cerr << "TBSI_Status: selectStim Cmd failed." << std::endl;
		std::cerr << "Stimulator Address " << stimulator_address << std::endl;
		return NO_RESPONSE;
	}
	return NO_ERROR_API;
}

TBSI_API_RESULT SetStimulatorState(DongleHandle device_handle, StimulatorState new_state)
{
	//init local str with globally defined rawCmdPacket
	Pattern str;
	bool acked = false;
	strncpy_s(str, rawCmdPacket, PATTERN_LENGTH);
	switch (new_state)
	{
	case awake:
		str[4] = CMD_ACK_WAKE_STIM[0];
		str[5] = CMD_ACK_WAKE_STIM[1];
		std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_WAKE_STIM << "' wakeStim Cmd." << std::endl;
		break;
	case sleep:
		str[4] = CMD_ACK_SLEEP_STIM[0];
		str[5] = CMD_ACK_SLEEP_STIM[1];
		std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_SLEEP_STIM << "' sleepStim Cmd." << std::endl;
		break;
	case hibernate:
		str[4] = CMD_ACK_HIBERNATE_STIM[0];
		str[5] = CMD_ACK_HIBERNATE_STIM[1];
		std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_HIBERNATE_STIM << "' hibernateStim Cmd." << std::endl;
		break;
	default:
		std::cerr << "Incorrect state" << std::endl;
		return NO_VALID_VALUE;
		break;
	}
	int errCode = 0;
	tbsiSendReceive(errCode, device_handle, str,acked);
	if (errCode != USB_SPI_ERRCODE_SUCCESS || !acked) {
		std::cerr << "TBSI_Status: Set state Cmd failed." << std::endl;
		return NO_RESPONSE;
	}
	return NO_ERROR_API;
}

TBSI_API_RESULT GetStimulatorState(DongleHandle device_handle, StimulatorState state)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT SetInitialDelay(DongleHandle device_handle, DelayMicroSeconds delay)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT GetInitialDelay(DongleHandle device_handle, DelayMicroSeconds & delay)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT GetStimulatorId(DongleHandle device_handle, char * uid)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT GetStimulatorAddress(DongleHandle device_handle, StimulatorAdd & add)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT GetStimulatorType(DongleHandle device_handle, char * type)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT GetStimulatorVersion(DongleHandle device_handle, Version version)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT GetStimulatorBatteryVolt(DongleHandle device_handle, float & batteryV)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT GetStimulatorBatteryState(DongleHandle device_handle, BatteryState batteryState)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT GetStimulatorTemperature(DongleHandle device_handle, float & temp)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT GetStimulatorField(DongleHandle device_handle, float & field)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT SetTriggerState(DongleHandle device_handle, bool trigger)
{
	Pattern str;
	bool acked = false;
	strncpy_s(str, rawCmdPacket, PATTERN_LENGTH);
	if(trigger)
	{
		str[4] = CMD_ACK_TRIGGER_GUI_STIM[0];
		str[5] = CMD_ACK_TRIGGER_GUI_STIM[1];
		std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_TRIGGER_GUI_STIM << "' start trigger cmd." << std::endl;
	}
	else
	{
		str[4] = CMD_ACK_TRIGGER_GUI_STIM_STOP[0];
		str[5] = CMD_ACK_TRIGGER_GUI_STIM_STOP[1];
		std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_TRIGGER_GUI_STIM_STOP << "' stop trigger cmd." << std::endl;
	}
	int errCode = 0;
	tbsiSendReceive(errCode, device_handle, str, acked);

	if (errCode != USB_SPI_ERRCODE_SUCCESS || !acked) {
		std::cerr << "TBSI_Status: Set trigger cmd failed." << std::endl;
		return NO_RESPONSE;
	}
	return NO_ERROR_API;
}

TBSI_API_RESULT GetTriggerState(DongleHandle device_handle, bool & trigger)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT SetHwTriggerState(DongleHandle device_handle, bool trigger)
{
	Pattern str;
	bool acked = false;
	strncpy_s(str, rawCmdPacket, PATTERN_LENGTH);
	//std::string disableHWTrigger = "AAAA400000000101000000000001240000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000005555";
	//std::string enableHWTrigger =  "AAAA400000000101000000000001241000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000005555";
	str[4] = '4';
	str[13] = '1';
	str[15] = '1';
	str[27] = '1';
	str[28] = '2';
	str[29] = '4';
	if (trigger)
	{
		str[30] = '1';
		std::cout << "TBSI_Status: sending enable h/w trigger cmd." << std::endl;
	}
	else
	{
		std::cout << "TBSI_Status: sending disable h/w trigger cmd." << std::endl;
	}
	int errCode = 0;
	tbsiSendReceive(errCode, device_handle, str, acked);

	if (errCode != USB_SPI_ERRCODE_SUCCESS || !acked) {
		std::cerr << "TBSI_Status: Set h/w trigger cmd failed." << std::endl;
		return NO_RESPONSE;
	}
	return NO_ERROR_API;
}

TBSI_API_RESULT GetHwTriggerState(DongleHandle device_handle, bool & trigger)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT DownloadPattern(DongleHandle device_handle, Pattern pattern)
{
	int errCode = 0;
	bool acked = false;
	tbsiSendReceive(errCode, device_handle, pattern,acked);

	if (errCode != USB_SPI_ERRCODE_SUCCESS || !acked) {
		std::cerr << "TBSI_Status: Download pattern cmd failed." << std::endl;
		return NO_RESPONSE;
	}
	return NO_ERROR_API;
}

TBSI_API_RESULT GetPattern(DongleHandle device_handle, Pattern pattern)
{
	return TBSI_API_RESULT();
}

TBSI_API_RESULT UpdatePattern(DongleHandle device_handle, char * cmd, char * new_pattern)
{
	return TBSI_API_RESULT();
}

//Private functions which are not exported but are called within the exported functions
//Taken from the code (tbsi.h & tbsi.cpp) given by Rob/Harry from Triangle Biosystems

const static int INPUT_LENGTH = 256;
enum ackNack_code {
	wakeStimAck,
	wakeStimNack,
	sleepStimAck,
	sleepStimNack,
	triggerGuiStimAck,
	triggerGuiStimNack,
	searchStimAck,
	searchStimNack,
	selectStimAck,
	selectStimNack,
	getDongleStatusAck,
	getDongleStatusNack,
	patternDownloadAck,
	patternDownloadNack,
	triggerGuiStimStopAck,
	triggerGuiStimStopNack,
	triggerHwStimAck,
	triggerHwStimNack,
	notHandled,
};

ackNack_code hashit(std::string const& inString) {
	if (inString == CMD_ACK_WAKE_STIM) return wakeStimAck;
	else if (inString == NACK_WAKE_STIM) return wakeStimNack;
	else if (inString == CMD_ACK_SEARCH_STIM) return searchStimAck;
	else if (inString == NACK_SEARCH_STIM) return searchStimNack;
	else if (inString == CMD_ACK_SELECT_STIM) return selectStimAck;
	else if (inString == NACK_SELECT_STIM) return selectStimNack;
	else if (inString == CMD_ACK_GET_DONGLE_STATUS) return getDongleStatusAck;
	else if (inString == NACK_GET_DONGLE_STATUS) return getDongleStatusNack;
	else if (inString == CMD_ACK_SLEEP_STIM) return sleepStimAck;
	else if (inString == NACK_SLEEP_STIM) return sleepStimNack;
	else if (inString == CMD_ACK_DOWNLOAD_PATTERN) return patternDownloadAck;
	else if (inString == NACK_DOWNLOAD_PATTERN) return patternDownloadNack;
	else if (inString == CMD_ACK_TRIGGER_GUI_STIM) return triggerGuiStimAck;
	else if (inString == NACK_TRIGGER_GUI_STIM) return triggerGuiStimNack;
	else if (inString == CMD_ACK_TRIGGER_GUI_STIM_STOP) return triggerGuiStimStopAck;
	else if (inString == NACK_TRIGGER_GUI_STIM_STOP) return triggerGuiStimStopNack;
	else if (inString == CMD_ACK_HW_TRIGGER) return triggerHwStimAck;
	else if (inString == NACK_HW_TRIGGER) return triggerHwStimNack;
	else return notHandled;
}

static BYTE* tbsiWrBuf;
static BYTE* tbsiRdBuf;

/*
	Description:
		Print CP2130 specific error messages. Clear non-critical errors so software can continue running.
		Also print custom error messages.

	Parameter:
		[in] errCode: CP2130 error code (most can be found in SLAB_USB_SPI.h while the custom ones are found in tbsi.h)

	Return:
		Void
*/
void checkErrCode(int & errCode)
{
	if (errCode != USB_SPI_ERRCODE_SUCCESS) 
	{
		switch (errCode) {
		case ACK_PACKET_TIMEOUT_ERROR:
			std::cerr << "TBSI_Error: [0x0" << std::hex << errCode << "]: Ack packet timeout from dongle." << std::endl;
			errCode = USB_SPI_ERRCODE_SUCCESS; //clear error since this case happens often when the stimulator fails to respond. allow code to continue running.
			break;
		case ACK_PACKET_RTR_TIMEOUT_ERROR:
			std::cerr << "TBSI_Error: [0x0" << std::hex << errCode << "]: RTR timeout from dongle." << std::endl;
			break;
		case ACK_PACKET_TRANSFER_ERROR:
			std::cerr << "TBSI_Error: [0x0" << std::hex << errCode << "]: Failed to transfer all bytes in ack packet." << std::endl;
			break;
		case USB_SPI_ERRCODE_HWIF_DEVICE_ERROR:
			std::cerr << "TBSI_Error: [0x" << std::hex << errCode << "]: " <<
				"Device Hardware Interface Error. Occurred while communicating with " <<
				"the device or while retrieving device information. " <<
				"Check if other programs are using the device and rerun. USB port likely corrupted. " <<
				"Restart PC or use another USB port not on the same hub." << std::endl;
			break;
		case USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT:
			std::cerr << "TBSI_Error: [0x" << std::hex << errCode << "]: Dongle time out. " <<
				"Try reconnecting dongle and rerunning program." << std::endl;
			break;
		case USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR:
			std::cerr << "TBSI_Error: [0x" << std::hex << errCode << "]: Dongle connection lost. Please disconnect and reconnect." << std::endl;
			break;
		case USB_SPI_ERRCODE_PIPE_READ_FAIL:
			std::cerr << "TBSI_Error: [0x" << std::hex << errCode << "]: Read error." << std::endl;
			break;
		default:
			std::cerr << "TBSI_Error: [0x" << std::hex << errCode << "]: Error not handled." << std::endl;
			break;
		}
	}
}

/*
	Description:
		Perform one communication cycle (send message & receive ack)

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle
		[in] writeBuf: Buffer that contains the pattern

		[out] readBuf: Buffer that contains the ACK information

	Return:
		Void
*/
void tbsiSendReceive(int & errCode, CP213x_DEVICE hDevice, char * input, bool &acked)
{
	DWORD bytesRead = 0;
	TBSI_SendCmd(errCode, hDevice, input);
	bytesRead = TBSI_ReceiveAck(errCode, hDevice,acked);
}

/*  Description:
		Parse returned ack/nacks from dongle. Expand as needed to support future commands.
		Only a few added for example. Use global enum at top of this file to define other cases.
		Further work could be done here to pass up valuable info to be handled and passed around elsewhere.
		Data that can be passed up includes stimulator state (On/Awake, Off/Asleep, Pattern, Hibernated...),
		battery voltage, microcontroller temperature C, charge state, inductive field strength, power ok field voltage and more.

	Parameter:
		[Ref] errCode: Error Code
		[in] readBuf: Buffer that contains the ACK packet information returned from dongle
		[in] size: Size of readBuf

	Return:
		Void
*/
void parseReceiveAck(int & errCode, BYTE * readBuf, int size, int expected_size, bool &ack)
{
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		if ((size != 0) & (size != expected_size)) {
			std::cout << "TBSI_Status: Byte transfer failed." << std::endl;
			errCode = ACK_PACKET_TRANSFER_ERROR; //partial transfer of bytes so throw error.
			checkErrCode(errCode);
			ack = false;
		}
		else if (size == 0) {
			std::cout << "TBSI_Status: Read buffer empty." << std::endl;
			errCode = ACK_PACKET_TIMEOUT_ERROR;
			checkErrCode(errCode);
			ack = false;
		}
		else {
			char* ackNackString = decodeData(readBuf, ACK_PACKET_SIZE, 0);
			char* ackNackCase = decodeData(readBuf, 1, 2);//Get case
			char* stimAddress;
			std::cout << "TBSI_Status: Received '0x" << ackNackCase << "' ";

			switch (hashit(ackNackCase)) {
			case wakeStimAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "wakeStim Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				ack = true;
				break;
			case wakeStimNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "wakeStim Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				ack = false;
				break;
			case searchStimAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "searchStim Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				ack = true;
				break;
			case searchStimNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "searchStim Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				ack = false;
				break;
			case selectStimAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "SelectStim Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				ack = true;
				break;
			case selectStimNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "selectStim Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				ack = false;
				break;
			case sleepStimAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "SleepStim Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				ack = true;
				break;
			case sleepStimNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "sleepStim Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				ack = false;
				break;
			case patternDownloadAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "Pattern Download Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				ack = true;
				break;
			case patternDownloadNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "Pattern Download Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				ack = false;
				break;
			case triggerGuiStimAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "Trigger GUI Stim Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				ack = true;
				break;
			case triggerGuiStimNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "Trigger GUI Stim Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				ack = false;
				break;
			case triggerGuiStimStopAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "Trigger GUI Stop Stim Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				ack = true;
				break;
			case triggerGuiStimStopNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "Trigger GUI Stop Stim Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				ack = false;
				break;
			case triggerHwStimAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "Trigger H/w Stim Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				ack = true;
				break;
			case triggerHwStimNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "Trigger H/w Stim Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				ack = false;
				break;
			case getDongleStatusAck:
				std::cout << "getDongleStatus Ack." << std::endl;
				ack = true;
				break;
			case getDongleStatusNack:
				std::cout << "getDongleStatus Nack." << std::endl;
				ack = false;
				break;
			case notHandled:
				std::cout << "which is not handled." << std::endl;
				ack = false;
				break;
			default:
				std::cout << "which is not handled." << std::endl;
				ack = false;
				break;
			}
			std::cout << "TBSI_Status: R[" << ACK_PACKET_SIZE << "]: " << ackNackString << std::endl;
		}
	}
}

/*
	Description:
		Initialize the CP2130 Device for TBSI

	Parameters:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle
		[in] csIndex: The SPI channel number
		[in] spiMode: The SPI control word for SPI configuration

	Returns:
		Void
*/
void TBSI_InterfaceInit(int & errCode, CP213x_DEVICE hDevice, BYTE csIndex, BYTE spiMode)
{
	if (errCode == USB_SPI_ERRCODE_SUCCESS)
	{
		errCode = CP213x_SetSpiControlByte(hDevice, csIndex, spiMode);
		if (errCode == USB_SPI_ERRCODE_SUCCESS) {
			std::cout << "TBSI_Status: SetSpiControlByte success. Channel: '" << (int)(csIndex) << "', ControlByte: '" << (int)(spiMode) << "'" << std::endl;
		}
		else {
			std::cout << "TBSI_Status: SetSpiControlByte failed." << std::endl;
		}
		checkErrCode(errCode);
	}
}

/*
	Description:
		Initialize the CP2130 device and select the SPI channel

	Parameters:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle
		[in] csIndex: The SPI channel number
		[in] spiMode: The SPI mode

	Returns:
		Void
*/
void TBSI_ChipSelect(int & errCode, CP213x_DEVICE hDevice, BYTE csIndex, BYTE csMode)
{
	if (errCode == USB_SPI_ERRCODE_SUCCESS)
	{
		errCode = CP213x_SetChipSelect(hDevice, csIndex, csMode);
		if (errCode == USB_SPI_ERRCODE_SUCCESS) {
			std::cout << "TBSI_Status: SetChipSelect success. Channel: '" << (int)(csIndex) << "', Mode: '" << (int)(csMode) << "'" << std::endl;
		}
		else {
			std::cout << "TBSI_Status: SetChipSelect failed." << std::endl;
		}
		checkErrCode(errCode);
	}
}

/*
	Description:
		SPI delay configuration

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle

	Return:
		Void
*/
void TBSI_SpiDelayConfig(int & errCode, CP213x_DEVICE hDevice)
{
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		errCode = CP213x_SetSpiDelay(hDevice, csIndex, SPI_DELAY_INTERVAL_MASK, 0, 0, 0);
		if (errCode == USB_SPI_ERRCODE_SUCCESS) {
			std::cout << "TBSI_Status: SetSpiDelay success. Channel: '" << (int)(csIndex) << "', DelayMode '" << (int)(SPI_DELAY_INTERVAL_MASK) << "'" << std::endl;
		}
		else {
			std::cout << "TBSI_Status: SetSpiDelay failed." << std::endl;
		}
		checkErrCode(errCode);
	}
}

/*
	Description:
		CP2130 pin configuration. For now only used to set RTR pin to active high so TransferReadSynchRtr function can be used.
		Calling TransferReadSynch lead to a lockup issue.

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle
		[in] pinOffset: The pin number
		[in] config: The configuration word

	Return:
		Void
*/
void TBSI_PinConfig(int & errCode, CP213x_DEVICE hDevice, int pinOffset, BYTE config)
{
	BYTE pinConfig[SIZE_PIN_CONFIG];
	int prev_config = 0;
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		errCode = CP213x_GetPinConfig(hDevice, pinConfig);
		prev_config = pinConfig[pinOffset];
		if (errCode == USB_SPI_ERRCODE_SUCCESS) {
			std::cout << "TBSI_Status: GetPinConfig success. Pin: '" << (int)(pinOffset) << "', Config: '" << (int)(prev_config) << "'" << std::endl;
		}
		else {
			std::cout << "TBSI_Status: GetPinConfig failed." << std::endl;
		}
		checkErrCode(errCode);
		if ((errCode == USB_SPI_ERRCODE_SUCCESS) & (prev_config != config)) {
			pinConfig[pinOffset] = config;
			errCode = CP213x_SetPinConfig(hDevice, pinConfig);
			if (errCode == USB_SPI_ERRCODE_SUCCESS) {
				std::cout << "TBSI_Status: SetPinConfig success. Pin: '" << (int)(pinOffset) << "', Config: '" << (int)(config) << "'" << std::endl;
			}
			else {
				std::cout << "TBSI_Status: SetPinConfig failed." << std::endl;
			}
		
		checkErrCode(errCode);
		}
	}
}

/*
	Description:
		Transfer a pattern packet. For specs on the pattern, please look at
		TBSI documentation on info packets

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle
		[in] writeBuf: Buffer that contains the pattern

	Return:
		Void
*/
void TBSI_SendCmd(int & errCode, CP213x_DEVICE hDevice, char * input)
{
	DWORD count = 0;
	DWORD max_count = 5;
	DWORD bytesTransferred = 0;
	BYTE writeBuf[CMD_PACKET_SIZE];
	BYTE drainBuf[ACK_PACKET_SIZE];
	bool RTR = TRUE;
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		//Before sending a cmd to dongle with "TransferWrite", flush the read buffer by reading any data currently in it.
		//Hardware triggers to dongle sending acks at too high a frequency (several Hz) to PC may require more work on this section to handle appropriately.
		RTR = Get_RTR_State(errCode, hDevice);
		count = 0;
		while ((RTR) & (errCode == USB_SPI_ERRCODE_SUCCESS) & (count < max_count)) {
			errCode = CP213x_TransferReadRtrSync(hDevice, drainBuf, ACK_PACKET_SIZE, ACK_PACKET_SIZE, RELEASE_AFTER_TRANSFER == 1, DRAIN_READ_MAX_WAIT_MS, &bytesTransferred);
			if (errCode == USB_SPI_ERRCODE_SUCCESS) {
				std::cout << "TBSI_Status: Draining read buffer before writing data." << std::endl;
			}
			else {
				std::cout << "TBSI_Status: Draining of read buffer before writing data failed." << std::endl;
			}

			checkErrCode(errCode);
			bool ack = false;
			parseReceiveAck(errCode, drainBuf, bytesTransferred, ACK_PACKET_SIZE, ack);
			RTR = Get_RTR_State(errCode, hDevice);
			count++;
		}

		//buffer is clear so begin writing data
		if (errCode == USB_SPI_ERRCODE_SUCCESS) {
			//Sleep(2);//consider removing
			//format data for write
			encodeData(input,INPUT_LENGTH, writeBuf);
			errCode = CP213x_TransferWrite(hDevice, writeBuf, CMD_PACKET_SIZE, RELEASE_AFTER_TRANSFER == 1, WRITE_MAX_WAIT_MS, &bytesTransferred);
			if (errCode == USB_SPI_ERRCODE_SUCCESS) {
				//rewrite to display actual buffer writeBuf instead of input string. Will help catch any mistakes in encodeData function.
				std::cout << "TBSI_Status: W[" << bytesTransferred << "]: " << input << std::endl;
			}
			else {
				std::cout << "TBSI_Status: Write failed." << std::endl;
			}
			
			checkErrCode(errCode);
		}
	}
}

/*
	Description:
		Receive ACK message from the TBSI dongle. TransferReadRtrSync waits for RTR to be asserted or timeout before returning.

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle

		[out] readBuf: Buffer that contains the ACK packet information returned from dongle

	Return:
		Void
*/
DWORD TBSI_ReceiveAck(int & errCode, CP213x_DEVICE hDevice, bool &acked)
{
	DWORD bytesTransferred = 0;
	BYTE readBuf[ACK_PACKET_SIZE];
	DWORD count_ms = 0;

	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		Sleep(2);
		//wait in this loop for ack/nack from dongle
		while ((count_ms < ACK_NACK_MAX_WAIT_MS) & (bytesTransferred == 0) & (errCode == USB_SPI_ERRCODE_SUCCESS)) {
			errCode = CP213x_TransferReadRtrSync(hDevice, readBuf, ACK_PACKET_SIZE, ACK_PACKET_SIZE, RELEASE_AFTER_TRANSFER == 1, READ_MAX_WAIT_MS, &bytesTransferred);
			count_ms += READ_MAX_WAIT_MS;
			if (bytesTransferred == 0) {
				//Use more accurate timing method if this throws off any critical timing requirements.
				std::cout << "TBSI_Status: Waiting for dongle ACK or NACK. Count_ms: " << count_ms << std::endl;
			}
		}
		checkErrCode(errCode);
		parseReceiveAck(errCode, readBuf, bytesTransferred, ACK_PACKET_SIZE, acked);
	}
	return bytesTransferred;
}

/*  Description:
		Reads RTR GPIO pin state on dongle. If TRUE then data is Ready To be Read from dongle.
		If FALSE then data is ready to be written to dongle.

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle

	Return:
		[out] RTR: Ready To Read
*/
bool Get_RTR_State(int & errCode, CP213x_DEVICE hDevice)
{
	WORD gpio = 0;
	bool RTR = TRUE;
	byte CS3_Bar_RTR = 0x08;//4th bit mask

	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		Sleep(GET_RTR_TIME_DELAY_MS); // Delay between requests to avoid jamming the device
		errCode = CP213x_GetGpioValues(hDevice, &gpio);
		Sleep(GET_RTR_TIME_DELAY_MS); // Delay between requests to avoid jamming the device
		if (errCode != USB_SPI_ERRCODE_SUCCESS) {
			std::cerr << "TBSI_Error: getGpioValues failed." << std::endl;
			checkErrCode(errCode);
		}
		else {
			//std::cout << "TBSI_Status: GPIO: " << std::hex << gpio << std::endl;
		}
	}
	else {
		std::cerr << "TBSI_Error: Skipped call to getGpioValues due to error." << std::endl;
	}

	RTR = ((gpio & CS3_Bar_RTR) != 0);
	return RTR;
}

/*
	Description:
		Encode readable data string into buffer data

	Parameter:
		[in] input: Data string (Directly copied from Stimware verbose mode log)

		[out] encoded: Encoded data buffer

	Return:
		none
*/
void encodeData(char * input, int length, BYTE * encoded)
{
	int conversion[2];

	for (int i = 1; i < length; i += 2)
	{
		//encoding string
		for (int j = 0; j < 2; j++) {
			switch (input[i - 1 + j])
			{
			case '0':
				 conversion[j] = 0;
				break;
			case '1':
				conversion[j] = 1;
				break;
			case '2':
				conversion[j] = 2;
				break;
			case '3':
				conversion[j] = 3;
				break;
			case '4':
				conversion[j] = 4;
				break;
			case '5':
				conversion[j] = 5;
				break;
			case '6':
				conversion[j] = 6;
				break;
			case '7':
				conversion[j] = 7;
				break;
			case '8':
				conversion[j] = 8;
				break;
			case '9':
				conversion[j] = 9;
				break;
			case 'A':
				conversion[j] = 10;
				break;
			case 'B':
				conversion[j] = 11;
				break;
			case 'C':
				conversion[j] = 12;
				break;
			case 'D':
				conversion[j] = 13;
				break;
			case 'E':
				conversion[j] = 14;
				break;
			case 'F':
				conversion[j] = 15;
				break;
			}
		}

		encoded[i / 2] = BYTE(conversion[0]* 16 + conversion[1]);
	}
}

/*
	Description:
		Decode data buffer into readable data string ( can be used to compare with
		Stimware verbose mode log)

	Parameter:
		[in] encoded: Encoded data buffer
		[in] word_length: Length of data in number of words (1 word = 2 bytes)
		[in] word_position: Offset of when to begin decoding data. Zero-indexed. Use when parsing out data.

	Return:
		The string value of decoded data
*/
char * decodeData(BYTE * encoded, int bytes_length, int word_position)
{
	char* ans = (char*)malloc(257);
	char map[17] = "0123456789ABCDEF";

	int j = -1;
	for (int i = 0; i < bytes_length; i++) {//"i" typically 0 to 31
		ans[++j] = map[encoded[i + word_position] / 16]; //display upper char
		ans[++j] = map[encoded[i + word_position] % 16]; //display lower char
	}
	ans[++j] = '\0';
	return ans;
}
