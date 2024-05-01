////////////////////////////////////////////////////////////////////////////////
// $Id: tbsi.h 7464 2023-06-30 15:04:08Z mellinger $
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
#ifndef TBSI_H
#define TBSI_H

#include <iostream>
#include <string>
#include <map>
#include <Windows.h>
#include "./extlib/include/SLAB_USB_SPI.h"

class TBSI
{
public:
	TBSI(bool enable_verbose_prinout);
	~TBSI(void);

	static void cmdGetDongleStatus(int& errCode, CP213x_DEVICE hDevice);
	static void cmdSearchStim(int& errCode, CP213x_DEVICE hDevice, std::string stimAddress);
	static void cmdWakeStim(int& errCode, CP213x_DEVICE hDevice);
	static void checkErrCode(int& errCode);
	static void tbsiSetup(int& errCode, CP213x_DEVICE& hDevice);
	static void tbsiSendReceive(int& errCode, CP213x_DEVICE hDevice, const std::string& input);
	static void tbsiStartUp(int& errCode, CP213x_DEVICE hDevice, std::string stimAddress);
    static void CloseDevice(int& errCode, CP213x_DEVICE mhDevice);
private:
	static void parseReceiveAck(			  int& errCode, BYTE* readBuf, int size, int expected_size);
	static void TBSI_InterfaceInit(           int& errCode, CP213x_DEVICE hDevice,  BYTE csIndex,    BYTE spiMode);
	static void TBSI_ChipSelect(              int& errCode, CP213x_DEVICE hDevice,  BYTE csIndex,    BYTE csMode);
	static void TBSI_SpiDelayConfig(          int& errCode, CP213x_DEVICE hDevice);
	static void TBSI_PinConfig(               int& errCode, CP213x_DEVICE hDevice,  int  pinOffset,  BYTE config);
	static void TBSI_SendCmd(                 int& errCode, CP213x_DEVICE hDevice, const std::string& input);
	static DWORD TBSI_ReceiveAck(             int& errCode, CP213x_DEVICE hDevice);
	static bool Get_RTR_State(int& errCode, CP213x_DEVICE hDevice);
    


	static void        encodeData(       const std::string& input, BYTE* encoded);
	static std::string decodeData(       BYTE* encoded, int word_length, int word_position);

	// Please look into Stimware's Setup.ini for detailed configuration
	const static BYTE  csIndex                 = 0; 
	const static BYTE  spiMode                 = 0x3B;
	const static BYTE  SPI_DELAY_INTERVAL_MASK = 1;
	const static DWORD CMD_PACKET_SIZE         = 128; 
	const static DWORD ACK_PACKET_SIZE         = 32;
	const static DWORD BUFFER_SIZE             = 255;
	const static BYTE  RELEASE_AFTER_TRANSFER  = 1;
    const static int   WRITE_MAX_WAIT_MS       = 500;
	const static int   READ_MAX_WAIT_MS        = 500;
	const static int   ACK_NACK_MAX_WAIT_MS    = 7000; //approximate amount of time before dongle gives up. Don't send any other commands while waiting on ack.
	const static int   DRAIN_READ_MAX_WAIT_MS  = 100;
	const static int   GET_RTR_TIME_DELAY_MS   = 2;

	//Custom errors overlayed on top of SLAB_USB_SPI error codes. Only use indexes not used by SLAB.
	const static BYTE ACK_PACKET_TIMEOUT_ERROR = 0x06;
	const static BYTE ACK_PACKET_RTR_TIMEOUT_ERROR = 0x08;
	const static BYTE ACK_PACKET_TRANSFER_ERROR= 0x09;

	static BYTE* tbsiWrBuf;
	static BYTE* tbsiRdBuf;
};

#endif
