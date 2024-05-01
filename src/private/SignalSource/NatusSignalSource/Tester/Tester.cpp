// Tester.cpp : Defines the entry point for the console application.
//
#include <cmath>
#include <time.h>
#include "stdafx.h"
#include "NatusDataServer.h"
#include <windows.h>    /* WinAPI */
#include <string>
#include <chrono>
#include <thread>

#define NUM_CH 276
#define SAMPLING_RATE 512
static bool cstate;

static void ErrorResponse(void* parent, uint8_t cmd, uint8_t* payload, uint32_t payload_size)
{
	std::cout << "Error response received for command 0x" << std::hex << cmd << std::endl;
}

static NatusDeviceInformation GetInformation(void* parent)
{
	std::cout << "Get information";
	NatusDeviceInformation inf;
	inf.NumberOfChannels = NUM_CH;
	inf.SamplingRate = SAMPLING_RATE;
	return inf;

}

static NatusChannelInformation GetChannelInformation(void* parent)
{
	std::cout << "Get channel information ";
	NatusChannelInformation channelNames;
	//channelNames.ChannelNames = new uint8_t[256];
	//memcpy(channelNames.ChannelNames, "ab cd ef gh ij", sizeof("ab cd ef gh ij"));
	for (int i = 0; i < 5; i++)
		channelNames.ChannelNames.push_back("A " + std::to_string(i));

	//	std::cout << channelNames.ChannelNames << std::endl;
		//std::cout << channelNames.StreamNames << std::endl;
	return channelNames;
}

int main()
{
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST-1);
	cstate = false;
	double timePassed = 0.0;
	try
	{
		NatusDataServer srv;
		srv.Verbose(true);

		srv.RegisterErrorCallback((NatusResponse)ErrorResponse, NULL);
		srv.RegisterInformationCallback((NatusInformationRequest)GetInformation, NULL);
		srv.RegisterChannelInformationCallback((NatusChannelInfoRequest)GetChannelInformation, NULL);
		srv.Startup(20016);
		float vars[NUM_CH];
		int i = 0;
	//	srv.CheckForConnection();
		while (true)
		{
			//Sleep(0);
			
			
			for (int j = 0; j < SAMPLING_RATE/10; ++j)
			{
				for (int ch = 0; ch < NUM_CH; ch++)
					vars[ch] = std::sin(2 * 3.14 * 1 * (((float)i) / ((float)SAMPLING_RATE)));
				//for (int ii = 0; ii < 4; ii++)
				srv.SendStreamData(vars, NUM_CH);
				i++;
				//std::cout << timePassed << std::endl;
			}
			Sleep(100);
			

		}
		
		srv.Disconnect();
		srv.UnregisterCallbacks();
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}


	getchar();
	return 0;
}





