//TBSIapi.h - contains the declarations of the api functions
#pragma once


#ifdef TBSIAPI_EXPORTS
#define TBSIAPI_API __declspec(dllexport)
#else
#define TBSIAPI_API __declspec(dllimport)
#endif

#include <iostream>
#include <Windows.h>
#include "./extlib/include/SLAB_USB_SPI.h"

#define STIMULATOR_LENGTH 5
#define PATTERN_LENGTH 257 //256 bits for pattern and 1 bit for NULL character

enum StimulatorState {
	awake,
	sleep,
	hibernate
};

enum BatteryState {
	low,
	normal,
	fully_charged
};

enum TBSI_API_RESULT {
	NO_ERROR_API,
	NO_RESPONSE,
	NO_DEVICE_FOUND,
	NO_CONNECTION_OPENED,
	NO_CONFIGURATION_SET,
	NO_CONNECTION_CLOSED_SUCCESSFULLY,
	NO_VALID_VALUE
};

TBSIAPI_API typedef void*	DongleHandle;
TBSIAPI_API typedef DWORD	DeviceIndex;
TBSIAPI_API typedef char	StimulatorAdd[STIMULATOR_LENGTH];
TBSIAPI_API typedef char	Pattern[PATTERN_LENGTH];
TBSIAPI_API typedef int		DelayMicroSeconds;
TBSIAPI_API typedef char*	Version;

//Every function returns a integer value corresponding to the succes/failure or error code occured.
//Dongle 

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	//TBSI_API_RESULT SearchDongle(DongleHandle device_handle[], int &num_of_devices); //Search for dongle - return a list of all the dongles connected
	TBSIAPI_API TBSI_API_RESULT SearchDongle(DeviceIndex &largest_device_index); //Search for dongle - return a list of all the dongles connected
	TBSIAPI_API TBSI_API_RESULT OpenDongleConnection(DongleHandle* device_handle, DeviceIndex device_index); // Connect to Dongle
	TBSIAPI_API TBSI_API_RESULT GetDongleStatus(DongleHandle device_handle, bool &status); //Get dongle status
	TBSIAPI_API TBSI_API_RESULT CloseDongleConnection(DongleHandle device_handle); //Close connection with dongle
	TBSIAPI_API TBSI_API_RESULT SetupDongle(DongleHandle* device_handle, bool &deviceOpen); //Search for the dongle and if found connect with it. can use SearchDongles and SetupDongles functions. limitation - handles only 1 device connection

//Cage
	TBSIAPI_API TBSI_API_RESULT SetFieldState(DongleHandle device_handle, void* cage_address, bool state); //To start/stop the inductive field of the cage. The battery of the implant starts charging when inside the inductive field

//Recording Device


//Stimulator
	TBSIAPI_API TBSI_API_RESULT FindStimulators(DongleHandle device_handle, StimulatorAdd stimulator_address[], uint32_t num_of_stimulators, StimulatorAdd found_stim_add[], uint32_t &found_num_stims); //search for stimulators with the given stimulator addresses
	TBSIAPI_API TBSI_API_RESULT ConnectStimulator(DongleHandle device_handle, StimulatorAdd stimulator_address); //To connect to a particular stimulator

	TBSIAPI_API TBSI_API_RESULT SetStimulatorState(DongleHandle device_handle, StimulatorState new_state); //change stimulator's state
	TBSIAPI_API TBSI_API_RESULT GetStimulatorState(DongleHandle device_handle, StimulatorState state); //get stimulator state
//Possible values of new_state - wake_up, sleep, hibernate or pattern

//Initial Delay - Can move this commands to the updatePattern command
	TBSIAPI_API TBSI_API_RESULT SetInitialDelay(DongleHandle device_handle, DelayMicroSeconds delay); //set initial delay
	TBSIAPI_API TBSI_API_RESULT GetInitialDelay(DongleHandle device_handle, DelayMicroSeconds &delay); //get initial delay

//Functions to get stimulator's info
	TBSIAPI_API TBSI_API_RESULT GetStimulatorId(DongleHandle device_handle, char* uid); //get stimualator uid
	TBSIAPI_API TBSI_API_RESULT GetStimulatorAddress(DongleHandle device_handle, StimulatorAdd &add); //get stimulator address
	TBSIAPI_API TBSI_API_RESULT GetStimulatorType(DongleHandle device_handle, char* type); //get stimulator type
	TBSIAPI_API TBSI_API_RESULT GetStimulatorVersion(DongleHandle device_handle, Version version); //get stimulator version
	TBSIAPI_API TBSI_API_RESULT GetStimulatorBatteryVolt(DongleHandle device_handle, float &batteryV); //get stimulator battery volt
	TBSIAPI_API TBSI_API_RESULT GetStimulatorBatteryState(DongleHandle device_handle, BatteryState batteryState); //get stimulator battery state
	TBSIAPI_API TBSI_API_RESULT GetStimulatorTemperature(DongleHandle device_handle, float &temp); //get stimulator temperature
	TBSIAPI_API TBSI_API_RESULT GetStimulatorField(DongleHandle device_handle, float &field); //get stimulator surrounding field

//Trigger functions
	TBSIAPI_API TBSI_API_RESULT SetTriggerState(DongleHandle device_handle, bool trigger); //to start(trigger = true)/stop(trigger = false) trigger
	TBSIAPI_API TBSI_API_RESULT GetTriggerState(DongleHandle device_handle, bool &trigger); //get trigger state

// H/W Trigger functions
	TBSIAPI_API TBSI_API_RESULT SetHwTriggerState(DongleHandle device_handle, bool trigger); //to enable(trigger = true)/disable(trigger = false) external h/w triggers
	TBSIAPI_API TBSI_API_RESULT GetHwTriggerState(DongleHandle device_handle, bool &trigger); //get h/w trigger state

//Pattern functions
	TBSIAPI_API TBSI_API_RESULT DownloadPattern(DongleHandle device_handle, Pattern pattern); //download pattern on the stimulator
	TBSIAPI_API TBSI_API_RESULT GetPattern(DongleHandle device_handle, Pattern pattern); //get pattern sent to stimulator
	TBSIAPI_API TBSI_API_RESULT UpdatePattern(DongleHandle device_handle, char* cmd, char* new_pattern); //to update a part(specific configuration) of patterns

#ifdef __cplusplus
}
#endif // __cplusplus
