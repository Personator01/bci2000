////////////////////////////////////////////////////////////////////////////////
// $Id: tbsi.cpp 7464 2023-06-30 15:04:08Z mellinger $
// Authors: belsten@neurotechcenter.org, yichuanwang@neurotechcenter.org, vines@trianglebiosystems.com
// Description: Class that represents the TBSI USB-SPI wireless stimulator and 
//				dongle.
// Known Issues: TBSI API is currently under development.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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
////////////////////////////////////////////////////////////////////////////////
#include "tbsi.h"

//global definitions not defined in class
BYTE* TBSI::tbsiWrBuf = new BYTE[TBSI::BUFFER_SIZE];
BYTE* TBSI::tbsiRdBuf = new BYTE[TBSI::BUFFER_SIZE];
bool verbose_printout = FALSE;

//Raw strings with properly formatted headers and footers. Still need everything else to be set.
std::string rawCmdPacket = "AAAA000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000005555";

//CMD_ACK strings
std::string CMD_ACK_WAKE_STIM = "01"; //Turn ON
std::string NACK_WAKE_STIM = "11"; //Turn ON
std::string CMD_ACK_SLEEP_STIM = "02"; //Turn OFF
std::string NACK_SLEEP_STIM = "12"; //Turn OFF
std::string CMD_ACK_TRIGGER_GUI_STIM = "03"; //Software trigger
std::string NACK_TRIGGER_GUI_STIM = "13"; //Software trigger
std::string CMD_ACK_SEARCH_STIM = "23";
std::string NACK_SEARCH_STIM = "33";
std::string CMD_ACK_SELECT_STIM = "24";
std::string NACK_SELECT_STIM = "34";
std::string CMD_ACK_GET_DONGLE_STATUS = "66";
std::string NACK_GET_DONGLE_STATUS = "76";


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
	else return notHandled;
}

TBSI::TBSI(bool enable_verbose_printout)
{
	verbose_printout = enable_verbose_printout;
}


TBSI::~TBSI()
{
	delete[] tbsiWrBuf;
	delete[] tbsiRdBuf;
}

/*	
	Description:
		Initialize the CP2130 device, set the SPI channel for Si8902
		and configure the device connection. Should only call once when setting up initial connection.

	Parameters:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle

	Returns:
		Void
*/	
void TBSI::tbsiSetup(int& errCode, CP213x_DEVICE& hDevice)
{
	DWORD numDevices = 0;

	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		//Find out how many dongles are connected to PC USB ports. Only 1 device handled for now.
		errCode = CP213x_GetNumDevices(&numDevices);
		checkErrCode(errCode);
		if (verbose_printout & (errCode == USB_SPI_ERRCODE_SUCCESS)) {
			std::cout << "TBSI_Status: GetNumDevices: " << numDevices << std::endl;
		}
	}
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		errCode = CP213x_OpenByIndex(numDevices - 1, &hDevice);
		checkErrCode(errCode);
		if (verbose_printout & (errCode == USB_SPI_ERRCODE_SUCCESS)) {
			std::cout << "TBSI_Status: Connection opened to dongle. Device handle: " << hDevice << std::endl;
		}
	}

	TBSI_InterfaceInit(errCode, hDevice, csIndex, spiMode);				 // cdIndex = 0, spiMode = 0x3b
	TBSI_ChipSelect(errCode, hDevice, csIndex, CSMODE_ACTIVE_OTHERS_IDLE); // csIndex = 0, spiMode = 2
	TBSI_SpiDelayConfig(errCode, hDevice);								 // SPI_DELAY_INTERVAL_MASK = 1
	TBSI_PinConfig(errCode, hDevice, 3, 5);								//pinOffset=GPIO.3=RTR pin=3,0x05=active high...default was active low=0x04

	if (verbose_printout) {
		if (errCode != USB_SPI_ERRCODE_SUCCESS) std::cerr << "TBSI_Status: Dongle configuration failure." << std::endl;
		else std::cout << "TBSI_Status: Dongle configuration success! Device handle: " << hDevice << std::endl;
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
void TBSI::TBSI_InterfaceInit(int& errCode, CP213x_DEVICE hDevice, BYTE csIndex, BYTE spiMode)
{
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		errCode = CP213x_SetSpiControlByte(hDevice, csIndex, spiMode);
		if (verbose_printout) {
			if (errCode == USB_SPI_ERRCODE_SUCCESS) {
				std::cout << "TBSI_Status: SetSpiControlByte success. Channel: '" << (int)(csIndex) << "', ControlByte: '" << (int)(spiMode) << "'" << std::endl;
			}
			else {
				std::cout << "TBSI_Status: SetSpiControlByte failed." << std::endl;
			}
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
void TBSI::TBSI_ChipSelect(int& errCode, CP213x_DEVICE hDevice, BYTE csIndex, BYTE csMode)
{
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		errCode = CP213x_SetChipSelect(hDevice, csIndex, csMode);
		if (verbose_printout) {
			if (errCode == USB_SPI_ERRCODE_SUCCESS) {
				std::cout << "TBSI_Status: SetChipSelect success. Channel: '" << (int)(csIndex) << "', Mode: '" << (int)(csMode) << "'" << std::endl;
			}
			else {
				std::cout << "TBSI_Status: SetChipSelect failed." << std::endl;
			}
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
void TBSI::TBSI_SpiDelayConfig(int& errCode, CP213x_DEVICE hDevice)
{
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		errCode = CP213x_SetSpiDelay(hDevice, csIndex, SPI_DELAY_INTERVAL_MASK, 0, 0, 0);
		if (verbose_printout) {
			if (errCode == USB_SPI_ERRCODE_SUCCESS) {
				std::cout << "TBSI_Status: SetSpiDelay success. Channel: '" << (int)(csIndex) << "', DelayMode '" << (int)(SPI_DELAY_INTERVAL_MASK) << "'" << std::endl;
			}
			else {
				std::cout << "TBSI_Status: SetSpiDelay failed." << std::endl;
			}
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
void TBSI::TBSI_PinConfig(int& errCode, CP213x_DEVICE hDevice, int pinOffset, BYTE config)
{
	BYTE pinConfig[SIZE_PIN_CONFIG];
	int prev_config = 0;
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		errCode = CP213x_GetPinConfig(hDevice, pinConfig);
		prev_config = pinConfig[pinOffset];
		if (verbose_printout) {
			if (errCode == USB_SPI_ERRCODE_SUCCESS) {
				std::cout << "TBSI_Status: GetPinConfig success. Pin: '" << (int)(pinOffset) << "', Config: '" << (int)(prev_config) << "'" << std::endl;
			}
			else {
				std::cout << "TBSI_Status: GetPinConfig failed." << std::endl;
			}
		}
		checkErrCode(errCode);
		if ((errCode == USB_SPI_ERRCODE_SUCCESS) & (prev_config != config)) {
			pinConfig[pinOffset] = config;
			errCode = CP213x_SetPinConfig(hDevice, pinConfig);
			if (verbose_printout) {
				if (errCode == USB_SPI_ERRCODE_SUCCESS) {
					std::cout << "TBSI_Status: SetPinConfig success. Pin: '" << (int)(pinOffset) << "', Config: '" << (int)(config) << "'" << std::endl;
				}
				else {
					std::cout << "TBSI_Status: SetPinConfig failed." << std::endl;
				}
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
void TBSI::TBSI_SendCmd(int& errCode, CP213x_DEVICE hDevice, const std::string& input)
{
	DWORD count = 0;
	DWORD max_count = 5;
	DWORD bytesTransferred = 0;
	BYTE writeBuf[CMD_PACKET_SIZE];
	BYTE drainBuf[ACK_PACKET_SIZE];
	bool RTR = TRUE;
	if (errCode == USB_SPI_ERRCODE_SUCCESS){
		//Before sending a cmd to dongle with "TransferWrite", flush the read buffer by reading any data currently in it.
		//Hardware triggers to dongle sending acks at too high a frequency (several Hz) to PC may require more work on this section to handle appropriately.
		RTR = Get_RTR_State(errCode, hDevice);
		count = 0;
		while((RTR) & (errCode == USB_SPI_ERRCODE_SUCCESS) & (count < max_count)) {
			errCode = CP213x_TransferReadRtrSync(hDevice, drainBuf, ACK_PACKET_SIZE, ACK_PACKET_SIZE, RELEASE_AFTER_TRANSFER == 1, DRAIN_READ_MAX_WAIT_MS, &bytesTransferred);
			if (verbose_printout) {
				if (errCode == USB_SPI_ERRCODE_SUCCESS) {
					std::cout << "TBSI_Status: Draining read buffer before writing data." << std::endl;
				}
				else {
					std::cout << "TBSI_Status: Draining of read buffer before writing data failed." << std::endl;
				}
			}
			checkErrCode(errCode);
			parseReceiveAck(errCode, drainBuf, bytesTransferred, ACK_PACKET_SIZE);
			RTR = Get_RTR_State(errCode, hDevice);
			count++;
		}
		
		//buffer is clear so begin writing data
		if (errCode == USB_SPI_ERRCODE_SUCCESS) {
			Sleep(2);//consider removing
			//format data for write
			encodeData(input, writeBuf);
			errCode = CP213x_TransferWrite(hDevice, writeBuf, CMD_PACKET_SIZE, RELEASE_AFTER_TRANSFER == 1, WRITE_MAX_WAIT_MS, &bytesTransferred);
			if (verbose_printout) {
				if (errCode == USB_SPI_ERRCODE_SUCCESS) {
					//rewrite to display actual buffer writeBuf instead of input string. Will help catch any mistakes in encodeData function.
					std::cout << "TBSI_Status: W[" << bytesTransferred << "]: " << input << std::endl;
				}
				else {
					std::cout << "TBSI_Status: Write failed." << std::endl;
				}
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
DWORD TBSI::TBSI_ReceiveAck(int& errCode, CP213x_DEVICE hDevice)
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
			if ((bytesTransferred == 0) & (verbose_printout)) {
				//Use more accurate timing method if this throws off any critical timing requirements.
				std::cout << "TBSI_Status: Waiting for dongle ACK or NACK. Count_ms: " << count_ms << std::endl; 
			}
			
		}
		checkErrCode(errCode);
		parseReceiveAck(errCode, readBuf, bytesTransferred, ACK_PACKET_SIZE);
	}
	return bytesTransferred;
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
void TBSI::tbsiSendReceive(int& errCode, CP213x_DEVICE hDevice, const std::string& input)
{
	DWORD bytesRead = 0;
	TBSI_SendCmd(errCode, hDevice, input);
	bytesRead = TBSI_ReceiveAck(errCode, hDevice);
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
void TBSI::encodeData(const std::string& input, BYTE * encoded)
{
	std::map<char, int> hexLookupTable; 
	hexLookupTable['0'] = 0;
	hexLookupTable['1'] = 1;
	hexLookupTable['2'] = 2;
	hexLookupTable['3'] = 3;
	hexLookupTable['4'] = 4;
	hexLookupTable['5'] = 5;
	hexLookupTable['6'] = 6;
	hexLookupTable['7'] = 7;
	hexLookupTable['8'] = 8;
	hexLookupTable['9'] = 9;
	hexLookupTable['A'] = 10;
	hexLookupTable['B'] = 11;
	hexLookupTable['C'] = 12;
	hexLookupTable['D'] = 13;
	hexLookupTable['E'] = 14;
	hexLookupTable['F'] = 15;

	for (int i = 1; i < input.size(); i+=2){
		encoded[i/2] = BYTE(hexLookupTable[input[i-1]]*16 + hexLookupTable[input[i]]);
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
std::string TBSI::decodeData(BYTE * encoded, int bytes_length, int word_position)
{
	std::string ans;

	std::map<int, char> hexLookupTable; 
	hexLookupTable[0] = '0';
	hexLookupTable[1] = '1';
	hexLookupTable[2] = '2';
	hexLookupTable[3] = '3';
	hexLookupTable[4] = '4';
	hexLookupTable[5] = '5';
	hexLookupTable[6] = '6';
	hexLookupTable[7] = '7';
	hexLookupTable[8] = '8';
	hexLookupTable[9] = '9';
	hexLookupTable[10] = 'A';
	hexLookupTable[11] = 'B';
	hexLookupTable[12] = 'C';
	hexLookupTable[13] = 'D';
	hexLookupTable[14] = 'E';
	hexLookupTable[15] = 'F';
	

	//std::cout << "TBSI_Debug: decodeData: bytes_length: " << bytes_length << std::endl;
	//std::cout << "TBSI_Debug: decodeData: word_pos: " << word_position << std::endl;
	//std::cout << "TBSI_Debug: decodeData: sizeof(encoded): " << sizeof(encoded) << std::endl;

	for (int i=0; i < bytes_length; i++){//"i" typically 0 to 31
		//std::cout << "i: " << i << ": upper char: " << hexLookupTable[encoded[i + word_position] / 16] << std::endl;
		//std::cout << "i: " << i << ": lower char: " << hexLookupTable[encoded[i + word_position] % 16] << std::endl;
		ans += hexLookupTable[encoded[i + word_position]/16]; //display upper char
		ans += hexLookupTable[encoded[i + word_position]%16]; //display lower char
	}
	return ans;
}

/*  Description:
		TBSI dongle start up sequence. When the cycle is done, the dongle should
		be ready to trigger Stim.
		
		The four critical commands (searchStim, scanAll, getDongleStatus and wakeUpStim)
		are all specific to the current dongle and stimulator. To obatin the correct command for any other
		dongle, use Stimware to generate such commands and copy them into the function.

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle	
		[in] 

	Return:
		Void
*/
void TBSI::tbsiStartUp(int& errCode, CP213x_DEVICE hDevice, std::string stimAddress) //todo: update parameter list to support startup
{
	cmdGetDongleStatus(errCode, hDevice);
	cmdSearchStim(errCode, hDevice, stimAddress);
	cmdWakeStim(errCode, hDevice); 
	//TBSI::tbsiSendReceive(errCode, hDevice, pattern); //todo: replace with wrapper function
}

/*  Description:
		Call to close device connection. Typically called once when program finishes execution.

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle

	Return:
		Void
*/
void TBSI::CloseDevice(int& errCode, CP213x_DEVICE hDevice)
{
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		if (verbose_printout) {
			std::cout << "TBSI_Status: Closing connection to device handle: " << hDevice << std::endl;
		}
		errCode = CP213x_Close(hDevice); //Removed reference &. Should not have needed.
		if (errCode != USB_SPI_ERRCODE_SUCCESS) {
			std::cerr << "TBSI_Status: Error when closing device handle." << std::endl;
			checkErrCode(errCode);
		}
	}
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
bool TBSI::Get_RTR_State(int& errCode, CP213x_DEVICE hDevice)
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

/*  Description:
		Send command to dongle asking it to return its firmware version and dongle address.

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle

	Return:
		Void
*/
void TBSI::cmdGetDongleStatus(int& errCode, CP213x_DEVICE hDevice)
{
	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		//init local str with globally defined rawCmdPacket
		std::string str = rawCmdPacket;
		//set 4th and 5th character of string packet to format the cmd characters
		str.replace(4, CMD_ACK_GET_DONGLE_STATUS.length(), CMD_ACK_GET_DONGLE_STATUS);

		if (verbose_printout) { 
			std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_GET_DONGLE_STATUS << "' getDongleStatus Cmd." << std::endl;
		}
		tbsiSendReceive(errCode, hDevice, str);
		if (errCode != USB_SPI_ERRCODE_SUCCESS) {
			std::cerr << "TBSI_Status: getDongleStatus Cmd failed." << std::endl;
		}
	}
}

/*  Description:
		Send command to dongle asking it to search for stimulator. Will return Ack if found. Will return Nack if not found.

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle
		[in] stimAddress: Stimulator Address

	Return:
		Void
*/
void TBSI::cmdSearchStim(int& errCode, CP213x_DEVICE hDevice, std::string stimAddress)
{
	//init local str with globally defined rawCmdPacket
	std::string str = rawCmdPacket;
	//set 4th and 5th character of string packet to format the cmd characters
	str.replace(4, CMD_ACK_SEARCH_STIM.length(), CMD_ACK_SEARCH_STIM);
	//set 6th,7th,8th,9th characters to correct stim address.
	str.replace(6, stimAddress.length(), stimAddress);

	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		if (verbose_printout) { 
			std::cout << "TBSI_Status: Sending '0x"<< CMD_ACK_SEARCH_STIM << "' searchStim Cmd." << std::endl; 
		}
		tbsiSendReceive(errCode, hDevice, str);
		if (errCode != USB_SPI_ERRCODE_SUCCESS) {
			std::cerr << "TBSI_Status: searchStim Cmd failed." << std::endl;
		}
	}
}

/*  Description:
		Send command to dongle asking it to wake stimulator up. Correct stimulator must already be searched or selected before calling this function.
		The end result is typically higher current consumption rates within the stimulator and a broader set of commands being allowed.

	Parameter:
		[Ref] errCode: Error Code
		[in] hDevice: USB Interface Handle

	Return:
		Void
*/
void TBSI::cmdWakeStim(int& errCode, CP213x_DEVICE hDevice)
{
	//init local str with globally defined rawCmdPacket
	std::string str = rawCmdPacket;
	//set 4th and 5th character of string packet to format the cmd characters
	str.replace(4, CMD_ACK_WAKE_STIM.length(), CMD_ACK_WAKE_STIM);

	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		if (verbose_printout) {
			std::cout << "TBSI_Status: Sending '0x" << CMD_ACK_WAKE_STIM << "' wakeStim Cmd."  << std::endl;
		}
		tbsiSendReceive(errCode, hDevice, str);
		if (errCode != USB_SPI_ERRCODE_SUCCESS) {
			std::cerr << "TBSI_Status: wakeStim Cmd failed." << std::endl;
		}
	}
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
void TBSI::parseReceiveAck(int& errCode, BYTE* readBuf, int size, int expected_size) 
{

	if (errCode == USB_SPI_ERRCODE_SUCCESS) {
		if ((size != 0) & (size != expected_size)) {
			std::cout << "TBSI_Status: Byte transfer failed." << std::endl;
			errCode = ACK_PACKET_TRANSFER_ERROR; //partial transfer of bytes so throw error.
			checkErrCode(errCode);
		}
		else if (size == 0) {
			std::cout << "TBSI_Status: Read buffer empty." << std::endl;
			errCode = ACK_PACKET_TIMEOUT_ERROR;
			checkErrCode(errCode);
		}
		else if (verbose_printout) {
			std::string ackNackString = decodeData(readBuf, ACK_PACKET_SIZE, 0);
			std::string ackNackCase = decodeData(readBuf, 1, 2);//Get case
			std::string stimAddress;
			std::cout << "TBSI_Status: Received '0x" << ackNackCase << "' ";

			switch (hashit(ackNackCase)) {
			case wakeStimAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "wakeStim Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				break;
			case wakeStimNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "wakeStim Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				break;
			case searchStimAck:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "searchStim Ack. Stim address '0x" << stimAddress << "' responded." << std::endl;
				break;
			case searchStimNack:
				stimAddress = decodeData(readBuf, 2, 3); //Get Stim Address
				std::cout << "searchStim Nack. Stim address '0x" << stimAddress << "' failed to respond." << std::endl;
				break;
			case getDongleStatusAck:
				std::cout << "getDongleStatus Ack." << std::endl;
				break;
			case getDongleStatusNack:
				std::cout << "getDongleStatus Nack." << std::endl;
				break;
			case notHandled:
				std::cout << "which is not handled." << std::endl;
				break;
			default:
				std::cout << "which is not handled." << std::endl;
				break;
			}
			std::cout << "TBSI_Status: R[" << ACK_PACKET_SIZE << "]: " << ackNackString << std::endl;
		}
	}

}


/*
	Description:
		Print CP2130 specific error messages. Clear non-critical errors so software can continue running.
		Also print custom error messages.

	Parameter:
		[in] errCode: CP2130 error code (most can be found in SLAB_USB_SPI.h while the custom ones are found in tbsi.h)

	Return:
		Void
*/
void TBSI::checkErrCode(int& errCode) {
	if (errCode != USB_SPI_ERRCODE_SUCCESS) {
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