//######################################################################################################################
/*
Copyright (c) since 2014 - Paul Freund

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
//######################################################################################################################
#ifndef INCLUDED_BluetoothWrapperBLED_H  // makes sure this header is not included more than once
#define INCLUDED_BluetoothWrapperBLED_H


#include <string>
#include <vector>
#include <algorithm>
#include "cmd_def.h"
#include "bcistream.h"
#include <winsock2.h>
#include <iomanip>
#include <exception>


//======================================================================================================================

#define BLUETOOTH_SOCKET_INVALID -1
#define BLUETOOTH_ADDRESS bd_addr
#define FIND_CYCLES 10



#define ControlService "2753706865726f2d5475bf2b6f74bb22"//20//  "2753706865726f2d5475bf2b6f74bb22"
//0e32002753706865726f2d5475ba3b6f74bb22
#define	wakeCharacteristic   "2753706865726f2d5475bf2b6f74bb22" //47//    "2753706865726f2d5475bf2b6f74bb22"
#define	txPowerCharacteristic  "2753706865726f2d5475b22b6f74bb22"//23 // "2753706865726f2d5475b22b6f74bb22"
#define	antiDosCharacteristic  "2753706865726f2d5475bd2b6f74bb22" // 42//  "2753706865726f2d5475bd2b6f74bb22"
#define	commandsCharacteristic  "2753706865726f2d5475a12b6f74bb22"//33 //"2753706865726f2d5475a12b6f74bb22"
#define	responseCharacteristic  "2753706865726f2d5475a62b6f74bb22" //16 //"22bb746f2ba675542d6f726568705327"

uint16 bleId=0;
uint16 wakeId=0;
uint16 txId=0;
uint16 antiDOSId=0;
uint16 commandsId=0;
uint16 responseId=0;
int lastReadResult=0;


typedef unsigned char ubyte;
volatile HANDLE serial_handle;

#define MAX_DEVICES 64
bool hasResponded = false; //use for rsp only
volatile bool discovery_in_progress=false; 
volatile bool connection_status_change=true;
int found_devices_count = 0;
bd_addr found_devices[MAX_DEVICES];
std::string found_device_name[MAX_DEVICES];
typedef uint8 BluetoothSocket;
volatile BluetoothSocket lastResponse = BLUETOOTH_SOCKET_INVALID;


std::vector<ubyte> receiveBuffer;


ble_msg_connection_status_evt_t connection_state;

void output(uint8 len1,uint8* data1,uint16 len2,uint8* data2)
{
	DWORD written;

	if(!WriteFile (serial_handle,
		data1,
		len1,
		&written,
		NULL
		))
	{
		bcierr<< "ERROR: Writing data. "  << std::to_string((int)GetLastError()) << std::endl;
	}

	if(!WriteFile (serial_handle,
		data2,
		len2,
		&written,
		NULL
		))
	{
		bcierr<< "ERROR: Writing data. "  << std::to_string((int)GetLastError()) << std::endl;

	}
}

void CloseComHandle()
{
	if(serial_handle != NULL)
		CloseHandle(serial_handle);
	serial_handle = NULL;

}

bool SetComHandle(const char*  comport)
{
	serial_handle = CreateFileA(comport,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if( serial_handle != INVALID_HANDLE_VALUE)
	{
		COMMTIMEOUTS cto; //set timeouts 
		SetCommTimeouts(serial_handle,&cto);
		cto.ReadIntervalTimeout=500;
		cto.ReadTotalTimeoutConstant=750;
		SetCommTimeouts(serial_handle,&cto);
		bglib_output=output;
		return true;
	}
	else
		return false;
}

bool name2Address(bd_addr& addrr, std::string name)
{
	int adr_counter=0;
	int l=name.length();
	char* c_str=NULL;
	c_str=(char*)malloc(l+1);
	char * pch;
	strcpy(c_str,name.c_str());
	pch=strtok(c_str,":");
	try
	{

		do
		{
			if(adr_counter >= BD_ADDR_LENGTH)
				return false;

			addrr.addr[BD_ADDR_LENGTH-adr_counter-1]=std::stoi(pch,0,16); //api stores addresses reversed
			adr_counter++;
			pch=strtok(NULL,":");

		}while (pch != NULL);
	}
	catch(std::exception e)
	{
		free(c_str);
		return false;
	}

	free(c_str);

	if(adr_counter != BD_ADDR_LENGTH)
		return false;

	return true;
}

int read_message()
{
	DWORD rread;
	const struct ble_msg *apimsg;
	struct ble_header apihdr;
	unsigned char data[256];//enough for BLE
	//read header

	if(!ReadFile(serial_handle,
		(unsigned char*)&apihdr,
		4,
		&rread,
		NULL))
	{
		throw Exception("Could not read from COM device");
		//return GetLastError();
	}
	if(!rread)return 0; //return 0 if no data has been collected
	//read rest if needed
	if(apihdr.lolen)
	{
		if(!ReadFile(serial_handle,
			data,
			apihdr.lolen,
			&rread,
			NULL))
		{
			throw Exception("Could not read from COM device");
		}
	}
	apimsg=ble_get_msg_hdr(apihdr);
	if(!apimsg)
	{

		bcierr << "ERROR: Message not found" << std::endl;
		return -1;
	}
	apimsg->handler(data);

	return 1;//return 1 if data has been successfully processed
}

bool cmp_bdaddr(bd_addr first,bd_addr second)
{

	for (int i = 0; i < BD_ADDR_LENGTH; i++) {
		if (first.addr[i] != second.addr[i]) return false;
	}
	return true;
}



//======================================================================================================================

bool BluetoothInitialize() {
	found_devices_count=0;
	ble_cmd_gap_end_procedure();
	read_message();
	//ble_cmd_system_reset(0);
	ble_cmd_connection_get_status(0);
	read_message();

	ble_cmd_gap_set_mode(gap_non_discoverable,gap_non_connectable);
	read_message();
	ble_cmd_sm_set_bondable_mode(1);
	read_message();
	ble_cmd_gap_discover(1);
	read_message();
	return true;
};

void BluetoothCleanup() {


}

bool BluetoothAvailable() {
	return serial_handle != INVALID_HANDLE_VALUE;
}

BLUETOOTH_ADDRESS BluetoothGetDeviceAddress(std::string name) 
{
	bool compareAddress=false;
	bool found=false;
	int trials=FIND_CYCLES;
	BLUETOOTH_ADDRESS retAdr;
	BLUETOOTH_ADDRESS searchAdr;
	if(name2Address(searchAdr,name))
	{
		compareAddress=true;
		//		return searchAdr;
	}
	retAdr.laddr=BLUETOOTH_SOCKET_INVALID;
	for (int i=0; i < FIND_CYCLES; i++)
	{
		read_message();
		Sleep(250);
		for(int i=0; i < found_devices_count;++i)
		{
			if(!compareAddress)
			{
				if(found_device_name[i] == name)
				{
					retAdr= found_devices[i];
					found=true;
					break;
				}
			}
			else
			{
				if(searchAdr.laddr == found_devices[i].laddr)
				{
					ble_cmd_gap_end_procedure();
					read_message();
					retAdr= found_devices[i];
					found=true;
					break;
				}
			}
		}


	}


	return retAdr;
}

void ble_evt_gap_scan_response(const struct ble_msg_gap_scan_response_evt_t *msg)
{
	if (found_devices_count >= MAX_DEVICES) 
	{
		ble_cmd_gap_end_procedure();
		read_message();
	};

	int i;
	char *name = NULL;

	// Check if this device already found
	for (i = 0; i < found_devices_count; i++) {
		if (cmp_bdaddr(msg->sender, found_devices[i])) return;
	}
	found_devices_count++;
	memcpy(found_devices[i].addr, msg->sender.addr, sizeof(bd_addr));

	// Parse data
	for (int j = 0; j < msg->data.len; ) {
		uint8 len = msg->data.data[j++];
		if (!len) continue;
		if (j + len > msg->data.len) break; // not enough data
		uint8 type = msg->data.data[j++];
		switch (type) {
		case 0x09:
			name = (char*)malloc(len);
			memcpy(name, msg->data.data + j, len - 1);
			name[len - 1] = '\0';
		}

		j += len - 1;
	}
	if(name != NULL)
	{
		found_device_name[i]=std::string(name);
		free(name);
	}
	else
	{
		found_device_name[i]="(No Name)";
	}
	bciout << "Found new Device: " << found_device_name[i] << std::endl;

	std::stringstream addressstringstream;
	for(int j=0; j < 6;j++)
	{
		addressstringstream << std::hex << (unsigned int)found_devices[i].addr[j] << ":";
	}

	bciout << "Address: "  << addressstringstream.str();




}


bool BluetoothDevicePaired(std::string name) {

	BLUETOOTH_ADDRESS adr=BluetoothGetDeviceAddress(name);
	if(adr.laddr == BLUETOOTH_SOCKET_INVALID)
		return false;

	if(!cmp_bdaddr(connection_state.address,adr))
		return false;


	return true;
}

void ble_evt_connection_status(const struct ble_msg_connection_status_evt_t *msg)
{
	bciout << "Connection event triggered! "<< std::to_string((int)msg->flags)<< std::endl;

	memcpy(&connection_state,msg,sizeof(ble_msg_connection_status_evt_t));
	connection_status_change = true;
}


void FindServices(BluetoothSocket handle)
{

	hasResponded=false;
	discovery_in_progress=true;
	ble_cmd_attclient_find_information(handle,1,0xffff);
	do
	{
		read_message();
	}while(!hasResponded || discovery_in_progress);


}

BluetoothSocket BluetoothConnect(std::string name) {
	bd_addr adr=BluetoothGetDeviceAddress(name);
	BluetoothSocket handle = BLUETOOTH_SOCKET_INVALID;
	if(adr.laddr != BLUETOOTH_SOCKET_INVALID)
	{
		hasResponded=false;
		connection_status_change=false;
		ble_cmd_gap_connect_direct(adr.addr,gap_address_type_random,60,76,100,0);

		do
		{
			read_message();
		}while(!hasResponded || !connection_status_change);
		handle=connection_state.connection;
		bciout << "Connection Flags: "<< connection_state.flags<< std::endl;
		bciout << "Bonding Handle: "<< connection_state.bonding<< std::endl;

		bciout << "Dumping services" << std::endl;
		if(bleId== 0 && wakeId== 0 && txId==0 && antiDOSId==0 && commandsId==0 && responseId==0)
			FindServices(handle);


	}
	return handle;

}







void ble_evt_connection_raw_rx(const struct ble_msg_connection_raw_rx_evt_t *msg)
{



}

void BluetoothDisconnect(BluetoothSocket socket) {
	hasResponded=false;
	ble_cmd_connection_disconnect(socket);
	while(!hasResponded)
	{
		read_message();
	}

}
bool BluetoothSend(BluetoothSocket &socket,uint16 id, std::vector<ubyte> data) 
{
	//std::reverse(data.begin(),data.end());

	hasResponded=false;
	ble_cmd_attclient_write_command(socket,id,data.size(),data.data());
	while(!hasResponded){read_message();}
	return true;

}

bool BluetoothSend(BluetoothSocket &socket, std::vector<ubyte> data) 
{
	int cmId=0;
	switch(data[2])
	{
	case 0: //core
		cmId=bleId;
		break;
	case 1: //bootloader
		bcierr << "Bootloader id not defined";
		break;
	case 2://sphero
		cmId=commandsId;
		break;
	}
	//std::reverse(data.begin(),data.end());
	return BluetoothSend(socket,cmId,data);


}

void SetTXPower(BluetoothSocket &socket, ubyte txpow) 
{

	hasResponded=false;
	byte test[]= {txpow};
	//std::reverse(data.begin(),data.end());
	ble_cmd_attclient_write_command(socket,txId,1,test);
	while(!hasResponded){read_message();}

}


void AntiDOS(BluetoothSocket &socket) 
{

	hasResponded=false;
	byte test[]= {0x30, 0x31, 0x31, 0x69, 0x33}; //011i3
	//std::reverse(std::begin(test),std::end(test));
	ble_cmd_attclient_write_command(socket,antiDOSId,5,test);

	while(!hasResponded){read_message();}

}

void Wakeup(BluetoothSocket &socket) 
{
	//std::reverse(data.begin(),data.end());
	hasResponded=false;
	byte test[]= {0x01};
	ble_cmd_attclient_write_command(socket,wakeId,1,test);
	while(!hasResponded){read_message();}

}

bool BluetoothReceive(BluetoothSocket &socket, std::vector<ubyte>& data, bool bBlocking = false) 
{
	hasResponded=false;
	discovery_in_progress = true;
	receiveBuffer.clear();
	//ble_cmd_attclient_read_by_handle(socket,responseId);
	ble_cmd_attclient_read_long(socket,responseId);
	do
	{read_message();
	}
	while((discovery_in_progress || !hasResponded) && lastReadResult != 0);//read until buffer is empty
	if(receiveBuffer.size() != 0)
	{
		copy(receiveBuffer.begin(),receiveBuffer.end(),back_inserter(data));

		std::stringstream stream;
		stream << std::hex ;

		for(int i=0; i < receiveBuffer.size() ; i++)
		{
			stream << "0x" << (int)receiveBuffer[i] << " ";
		}
		std::string result( stream.str() );
		bciout <<"Received Data: "<<  result << std::endl;


	}
	return true; //if there is nothing to read, trying to read from handle throws error
}
//======================================================================================================================
//stubs




void  ble_evt_attclient_attribute_value(const struct ble_msg_attclient_attribute_value_evt_t  *msg)
{
	for(int i=0; i < msg->value.len;i++)
	{
		receiveBuffer.push_back((ubyte)msg->value.data[i]);
	}

	//bciout << "ble_evt_attclient_attribute_value" << std::to_string((int)msg->atthandle) << std::endl;
}


bool compareUUID(std::string str,const uint8* b, int size)
{
	int j=0;
	int buffer;
	bool valid=true;
	for(int i=0; i < size; i++)
	{
		auto substr=str.substr(j,2);
		if(j+2 > str.size())
			break;

		buffer=std::stoi(substr,nullptr,16);
		if(buffer != b[i])
		{
			valid=false;
			break;
		}
		j+=2;
	}
	return valid;
}

void  ble_evt_attclient_find_information_found(const struct ble_msg_attclient_find_information_found_evt_t  *msg)
{


	std::string wakestring=	wakeCharacteristic;  
	std::string txtring=	txPowerCharacteristic;
	std::string dosstring=	antiDosCharacteristic; 
	std::string cmdstring=	commandsCharacteristic;
	std::string rspstring=	responseCharacteristic;
	std::string blestring= ControlService;


	if(compareUUID(wakestring,msg->uuid.data,msg->uuid.len))
	{
		bciout << "Wake handle found!" << std::endl;
		wakeId=msg->chrhandle;
	}

	if(compareUUID(txtring,msg->uuid.data,msg->uuid.len))
	{
		bciout << "Tx handle found!" << std::endl;
		txId=msg->chrhandle;
	}

	if(compareUUID(dosstring,msg->uuid.data,msg->uuid.len))
	{
		bciout << "AntiDOS handle found!" << std::endl;
		antiDOSId=msg->chrhandle;
	}

	if(compareUUID(cmdstring,msg->uuid.data,msg->uuid.len))
	{
		bciout << "Cmd handle found!" << std::endl;
		commandsId=msg->chrhandle;
	}

	if(compareUUID(rspstring,msg->uuid.data,msg->uuid.len))
	{
		bciout << "Response handle found!" << std::endl;
		responseId=msg->chrhandle;
	}


	if(compareUUID(blestring,msg->uuid.data,msg->uuid.len))
	{
		bciout << "BLE handle found!" << std::endl;
		bleId=msg->chrhandle;
	}



}

void  ble_evt_attclient_group_found(const struct ble_msg_attclient_group_found_evt_t  *msg)
{



	std::stringstream stream;


	for(int i=msg->uuid.len-1; i >=0 ; i--)
	{
		stream << std::hex << (int)msg->uuid.data[i];
	}

	bciout <<" ble_evt_attclient_group_found " << std::to_string((int)msg->start) << std::endl;
	std::string result( stream.str() );
	bciout << result << std::endl;


}

void ble_evt_attclient_procedure_completed(const struct ble_msg_attclient_procedure_completed_evt_t  *msg )
{
	discovery_in_progress=false;
	lastReadResult=msg->result;
	//	bciout<<"Process completed" << std::to_string(msg->result) << std::endl;
}

void  ble_evt_connection_disconnected(const struct ble_msg_connection_disconnected_evt_t  *msg)
{
	bciout << "Disconnected from device!" << std::to_string(msg->reason) << std::endl;
}

void  ble_rsp_system_get_info(const struct ble_msg_system_get_info_rsp_t  *msg)
{
	hasResponded=true;
}


void ble_default(const void*v)
{
}

void ble_rsp_attributes_send(const struct ble_msg_attributes_send_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_hardware_set_rxgain(const void*nul)
{
	hasResponded=true;
}

void ble_rsp_system_aes_setkey(const void*nul)
{
	hasResponded=true;
}

void ble_rsp_system_aes_encrypt(const struct ble_msg_system_aes_encrypt_rsp_t*msg)
{
	hasResponded=true;
}

void ble_rsp_system_aes_decrypt(const struct ble_msg_system_aes_decrypt_rsp_t*msg)
{
	hasResponded=true;
}

void ble_rsp_flash_read_data(const struct ble_msg_flash_read_data_rsp_t*msg)
{
	hasResponded=true;
}

void ble_rsp_test_channel_mode(const void*nul)
{
	hasResponded=true;
}

void ble_evt_system_protocol_error(const struct ble_msg_system_protocol_error_evt_t*msg)
{
	bciwarn<< "System protocol error!";
}

void ble_rsp_system_reset(const void* nul)
{
	hasResponded=true;
}

void ble_rsp_system_hello(const void* nul)
{
	hasResponded=true;
}

void ble_rsp_system_address_get(const struct ble_msg_system_address_get_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_system_reg_write(const struct ble_msg_system_reg_write_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_system_reg_read(const struct ble_msg_system_reg_read_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_system_get_counters(const struct ble_msg_system_get_counters_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_system_get_connections(const struct ble_msg_system_get_connections_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_system_read_memory(const struct ble_msg_system_read_memory_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_system_endpoint_tx(const struct ble_msg_system_endpoint_tx_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_system_whitelist_append(const struct ble_msg_system_whitelist_append_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_system_whitelist_remove(const struct ble_msg_system_whitelist_remove_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_system_whitelist_clear(const void* nul)
{hasResponded=true;
}

void ble_rsp_system_endpoint_rx(const struct ble_msg_system_endpoint_rx_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_system_endpoint_set_watermarks(const struct ble_msg_system_endpoint_set_watermarks_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_usb_enable(const struct ble_msg_hardware_usb_enable_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_flash_ps_defrag(const void* nul)
{hasResponded=true;
}

void ble_rsp_flash_ps_dump(const void* nul)
{hasResponded=true;
}

void ble_rsp_flash_ps_erase_all(const void* nul)
{hasResponded=true;
}

void ble_rsp_flash_ps_save(const struct ble_msg_flash_ps_save_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_flash_ps_load(const struct ble_msg_flash_ps_load_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_flash_ps_erase(const void* nul)
{hasResponded=true;
}

void ble_rsp_flash_erase_page(const struct ble_msg_flash_erase_page_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_flash_write_data(const struct ble_msg_flash_write_data_rsp_t * msg)
{hasResponded=true;
}

void ble_rsp_attributes_write(const struct ble_msg_attributes_write_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attributes_read(const struct ble_msg_attributes_read_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attributes_read_type(const struct ble_msg_attributes_read_type_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attributes_user_read_response(const void* nul)
{hasResponded=true;
}

void ble_rsp_attributes_user_write_response(const void* nul)
{hasResponded=true;
}

void ble_rsp_connection_disconnect(const struct ble_msg_connection_disconnect_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_connection_get_rssi(const struct ble_msg_connection_get_rssi_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_connection_update(const struct ble_msg_connection_update_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_connection_version_update(const struct ble_msg_connection_version_update_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_connection_channel_map_get(const struct ble_msg_connection_channel_map_get_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_connection_channel_map_set(const struct ble_msg_connection_channel_map_set_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_connection_features_get(const struct ble_msg_connection_features_get_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_connection_get_status(const struct ble_msg_connection_get_status_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_connection_raw_tx(const struct ble_msg_connection_raw_tx_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_connection_slave_latency_disable(const struct ble_msg_connection_slave_latency_disable_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attclient_find_by_type_value(const struct ble_msg_attclient_find_by_type_value_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attclient_read_by_group_type(const struct ble_msg_attclient_read_by_group_type_rsp_t *msg)
{hasResponded=true;

bciout << "ble_rsp_attclient_read_by_group_type: " << std::to_string(msg->result) << std::endl;
}

void ble_rsp_attclient_read_by_type(const struct ble_msg_attclient_read_by_type_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attclient_find_information(const struct ble_msg_attclient_find_information_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attclient_read_by_handle(const struct ble_msg_attclient_read_by_handle_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attclient_attribute_write(const struct ble_msg_attclient_attribute_write_rsp_t *msg)
{
	bciout << "Write to device" << std::endl;
}

void ble_rsp_attclient_write_command(const struct ble_msg_attclient_write_command_rsp_t *msg)
{hasResponded=true;
//bciout << "Command sent! : " << std::to_string((int)msg->result) << std::endl;
}

void ble_rsp_attclient_indicate_confirm(const struct ble_msg_attclient_indicate_confirm_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attclient_read_long(const struct ble_msg_attclient_read_long_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attclient_prepare_write(const struct ble_msg_attclient_prepare_write_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attclient_execute_write(const struct ble_msg_attclient_execute_write_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_attclient_read_multiple(const struct ble_msg_attclient_read_multiple_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_sm_encrypt_start(const struct ble_msg_sm_encrypt_start_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_sm_set_bondable_mode(const void* nul)
{hasResponded=true;
}

void ble_rsp_sm_delete_bonding(const struct ble_msg_sm_delete_bonding_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_sm_set_parameters(const void* nul)
{hasResponded=true;
}

void ble_rsp_sm_passkey_entry(const struct ble_msg_sm_passkey_entry_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_sm_get_bonds(const struct ble_msg_sm_get_bonds_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_sm_set_oob_data(const void* nul)
{hasResponded=true;
}

void ble_rsp_sm_whitelist_bonds(const struct ble_msg_sm_whitelist_bonds_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_sm_set_pairing_distribution_keys(const struct ble_msg_sm_set_pairing_distribution_keys_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_set_privacy_flags(const void* nul)
{hasResponded=true;
}

void ble_rsp_gap_set_mode(const struct ble_msg_gap_set_mode_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_discover(const struct ble_msg_gap_discover_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_connect_direct(const struct ble_msg_gap_connect_direct_rsp_t *msg)
{hasResponded=true;
bciout << "Response from connect direct " <<  msg->result <<std::endl;
lastResponse=msg->connection_handle;
}

void ble_rsp_gap_end_procedure(const struct ble_msg_gap_end_procedure_rsp_t *msg)
{
	hasResponded=true;
}

void ble_rsp_gap_connect_selective(const struct ble_msg_gap_connect_selective_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_set_filtering(const struct ble_msg_gap_set_filtering_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_set_scan_parameters(const struct ble_msg_gap_set_scan_parameters_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_set_adv_parameters(const struct ble_msg_gap_set_adv_parameters_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_set_adv_data(const struct ble_msg_gap_set_adv_data_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_set_directed_connectable_mode(const struct ble_msg_gap_set_directed_connectable_mode_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_set_initiating_con_parameters(const struct ble_msg_gap_set_initiating_con_parameters_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_gap_set_nonresolvable_address(const struct ble_msg_gap_set_nonresolvable_address_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_io_port_config_irq(const struct ble_msg_hardware_io_port_config_irq_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_set_soft_timer(const struct ble_msg_hardware_set_soft_timer_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_adc_read(const struct ble_msg_hardware_adc_read_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_io_port_config_direction(const struct ble_msg_hardware_io_port_config_direction_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_io_port_config_function(const struct ble_msg_hardware_io_port_config_function_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_io_port_config_pull(const struct ble_msg_hardware_io_port_config_pull_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_io_port_write(const struct ble_msg_hardware_io_port_write_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_io_port_read(const struct ble_msg_hardware_io_port_read_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_spi_config(const struct ble_msg_hardware_spi_config_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_spi_transfer(const struct ble_msg_hardware_spi_transfer_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_i2c_read(const struct ble_msg_hardware_i2c_read_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_i2c_write(const struct ble_msg_hardware_i2c_write_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_set_txpower(const void* nul)
{hasResponded=true;
}

void ble_rsp_hardware_timer_comparator(const struct ble_msg_hardware_timer_comparator_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_io_port_irq_enable(const struct ble_msg_hardware_io_port_irq_enable_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_io_port_irq_direction(const struct ble_msg_hardware_io_port_irq_direction_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_analog_comparator_enable(const void *nul)
{hasResponded=true;
}

void ble_rsp_hardware_analog_comparator_read(const struct ble_msg_hardware_analog_comparator_read_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_analog_comparator_config_irq(const struct ble_msg_hardware_analog_comparator_config_irq_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_hardware_sleep_enable(const struct ble_msg_hardware_sleep_enable_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_test_phy_tx(const void* nul)
{hasResponded=true;
}

void ble_rsp_test_phy_rx(const void* nul)
{hasResponded=true;
}

void ble_rsp_test_phy_end(const struct ble_msg_test_phy_end_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_test_phy_reset(const void* nul)
{hasResponded=true;
}

void ble_rsp_test_get_channel_map(const struct ble_msg_test_get_channel_map_rsp_t *msg)
{hasResponded=true;
}

void ble_rsp_test_debug(const struct ble_msg_test_debug_rsp_t *msg)
{hasResponded=true;
}

void ble_evt_system_boot(const struct ble_msg_system_boot_evt_t *msg)
{
}

void ble_evt_system_debug(const struct ble_msg_system_debug_evt_t *msg)
{
}

void ble_evt_system_endpoint_watermark_rx(const struct ble_msg_system_endpoint_watermark_rx_evt_t *msg)
{
}

void ble_evt_system_endpoint_watermark_tx(const struct ble_msg_system_endpoint_watermark_tx_evt_t *msg)
{
}

void ble_evt_system_script_failure(const struct ble_msg_system_script_failure_evt_t *msg)
{
}

void ble_evt_system_no_license_key(const void* nul)
{
}

void ble_evt_flash_ps_key(const struct ble_msg_flash_ps_key_evt_t *msg)
{
}

void ble_evt_attributes_value(const struct ble_msg_attributes_value_evt_t *msg)
{
}

void ble_evt_attribute_value(const struct ble_msg_attributes_value_evt_t *msg)
{
	//bciout << "Data Received!" << std::endl;


	for(int i=0;msg->value.len;i++)
	{
		receiveBuffer.push_back((ubyte)msg->value.data[i]);
	}
}

void ble_evt_attributes_user_read_request(const struct ble_msg_attributes_user_read_request_evt_t *msg)
{
}

void ble_evt_attributes_status(const struct ble_msg_attributes_status_evt_t *msg)
{
}

void ble_evt_connection_version_ind(const struct ble_msg_connection_version_ind_evt_t *msg)
{
}

void ble_evt_connection_feature_ind(const struct ble_msg_connection_feature_ind_evt_t *msg)
{
}

void ble_evt_attclient_indicated(const struct ble_msg_attclient_indicated_evt_t *msg)
{
}

void ble_evt_attclient_attribute_found(const struct ble_msg_attclient_attribute_found_evt_t *msg)
{
}

void ble_evt_attclient_read_multiple_response(const struct ble_msg_attclient_read_multiple_response_evt_t *msg)
{
	bciout<< "Read multiple out";
}

void ble_evt_sm_smp_data(const struct ble_msg_sm_smp_data_evt_t *msg)
{
}

void ble_evt_sm_bonding_fail(const struct ble_msg_sm_bonding_fail_evt_t *msg)
{
}

void ble_evt_sm_passkey_display(const struct ble_msg_sm_passkey_display_evt_t *msg)
{
}

void ble_evt_sm_passkey_request(const struct ble_msg_sm_passkey_request_evt_t *msg)
{
}

void ble_evt_sm_bond_status(const struct ble_msg_sm_bond_status_evt_t *msg)
{
}

void ble_evt_gap_mode_changed(const struct ble_msg_gap_mode_changed_evt_t *msg)
{
}

void ble_evt_hardware_io_port_status(const struct ble_msg_hardware_io_port_status_evt_t *msg)
{
}

void ble_evt_hardware_soft_timer(const struct ble_msg_hardware_soft_timer_evt_t *msg)
{
}

void ble_evt_hardware_adc_result(const struct ble_msg_hardware_adc_result_evt_t *msg)
{
}

void ble_evt_hardware_analog_comparator_status(const struct ble_msg_hardware_analog_comparator_status_evt_t *msg)
{
}


/**Reset system**/
void ble_rsp_dfu_reset(const void *nul)
{
}

/**set address for flashing**/
void ble_rsp_dfu_flash_set_address(const struct ble_msg_dfu_flash_set_address_rsp_t *msg)
{}

/**Upload binary for flashing. Address will be updated automatically.**/
void ble_rsp_dfu_flash_upload(const struct ble_msg_dfu_flash_upload_rsp_t *msg)
{}
/**Uploading is finished.**/
void ble_rsp_dfu_flash_upload_finish(const struct ble_msg_dfu_flash_upload_finish_rsp_t *msg)
{}
/**Device booted up in dfu, and is ready to receive commands**/
void ble_evt_dfu_boot(const struct ble_msg_dfu_boot_evt_t *msg)
{}


#endif
