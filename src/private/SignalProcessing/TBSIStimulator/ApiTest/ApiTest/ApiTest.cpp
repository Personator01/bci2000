// ApiTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "./extlib/include/TBSIapi.h"
#include <chrono>
#include <thread>

void close(DongleHandle devHandle);

int main()
{
	std::cout << "starting program" << std::endl;
	
	TBSI_API_RESULT result = NO_ERROR_API;
	bool deviceOpen = false;
	DongleHandle devhandle = NULL;
	int sleepTime = 5;
	bool deviceStatus = false;

	//Testing Dongle Functions
	result = SetupDongle(&devhandle, deviceOpen);
	std::cout << "Device Open " << deviceOpen << std::endl;
	std::cout << "Result " << result << std::endl;
	if (result != NO_ERROR_API)
		return 0;

	std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
	result = GetDongleStatus(devhandle, deviceStatus);
	std::cout << "Dongle status : " << deviceStatus << std::endl;
	std::cout << "Dongle Status result: " << result << std::endl;
	if (result != NO_ERROR_API)
	{
		close(devhandle);
		return 0;
	}
	//Testing Stimulator functions
	uint32_t n = 2;
	StimulatorAdd stimulators[2];
	strncpy_s(stimulators[0],"0024",STIMULATOR_LENGTH);
	strncpy_s(stimulators[1],"0068",STIMULATOR_LENGTH);
	std::cout << "Finding the following stimulators: " << stimulators[0] <<"  "<< stimulators[1] << std::endl;
	uint32_t found_stims;
	StimulatorAdd found_stimulators[2];
	result = FindStimulators(devhandle, stimulators, n, found_stimulators,found_stims);
	std::cout << "Find Stimulator result: " << result << std::endl;
	std::cout << "Stimulators found: " << found_stims << std::endl;
	for (uint32_t i = 0; i < found_stims; i++)
		std::cout << found_stimulators[i] << std::endl;
		
	if (result != NO_ERROR_API)
	{
		close(devhandle);
		return 0;
	}
	std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
	result = ConnectStimulator(devhandle, stimulators[0]);
	std::cout << "Connect Stimulator result: " << result << std::endl;
	
	if (result == NO_ERROR_API)
	{
		//Wake up stimulator
		result = SetStimulatorState(devhandle, awake);
		std::cout << "Wake Stimulator result: " << result << std::endl;
		//testing setting hardware trigger
		
		/*result = SetHwTriggerState(devhandle, true);
		std::cout << "Hw Trigger Enable Stimulator result: " << result << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
		result = SetHwTriggerState(devhandle, false);
		std::cout << "Hw Trigger Disable Stimulator result: " << result << std::endl;
		*/

		//download pattern
		Pattern pattern = "555500000100000000000100000000000000320000006200FA0090010300EE02000000000000A4085C076117010001000100000000080000000000000100000000000000000064000000000064000000000032000000E7032003FFFF00000000000000000000A4085C070A00010001000000000000080000000000000100AAAA";
		result = DownloadPattern(devhandle, pattern);
		std::cout << "Download Pattern Stimulator result: " << result << std::endl;
		if (result != NO_ERROR_API)
		{
			close(devhandle);
			return 0;
		}

		for (int i = 1; i <= 10; i++)
		{
			std::cout << "iteration: " << i << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(3));
			//trigger pattern
			if (result == NO_ERROR_API)
			{
				result = SetTriggerState(devhandle, true);
				std::cout << "Start Trigger Stimulator result: " << result << std::endl;
			}
		}

		//stop triggering pattern
		if (result == NO_ERROR_API)
		{
			result = SetTriggerState(devhandle, false);
			std::cout << "Stop Trigger Stimulator result: " << result << std::endl;

		}
		
		if (result == NO_ERROR_API) {
			std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
			//Sleep stimulator
			result = SetStimulatorState(devhandle, sleep);
			std::cout << "Sleep Stimulator result: " << result << std::endl;
		}
	}

    //Closing Dongle
	result = CloseDongleConnection(devhandle);
	std::cout << "Close dongle result: " << result << std::endl;
	
	return 0;
}

void close(DongleHandle devhandle) {
	//Closing Dongle
	TBSI_API_RESULT result = CloseDongleConnection(devhandle);
	std::cout << "Close dongle result: " << result << std::endl;
}