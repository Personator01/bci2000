// XLDataExportReceiver.cpp : Implementation of CXLDataExportReceiver
#include "stdafx.h"
#include "XLDataExportClient.h"
#include "XLDataExportReceiver.h"
#include "Utils\GuidUtil.h"
#include <string>
#include <exception>
#include <iomanip> 
#define SERVERPORT 1030

/////////////////////////////////////////////////////////////////////////////
// CXLDataExportReceiver

CXLDataExportReceiver::CXLDataExportReceiver ()
{

}

CXLDataExportReceiver::~CXLDataExportReceiver ()
{
}

STDMETHODIMP CXLDataExportReceiver::InterfaceSupportsErrorInfo (REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_IXLDataExportReceiver
	};
	for (int i = 0; i < sizeof (arr) / sizeof (arr[0]); i++)  // Unicode OK
	{
		if (InlineIsEqualGUID (*arr[i], riid))
			return S_OK;
	}
	return S_FALSE;
}

static const TCHAR* month_tbl[] = {
	_T ("---"), _T ("JAN"), _T ("FEB"), _T ("MAR"), _T ("APR"), _T ("MAY"), _T ("JUN"), _T ("JUL"), _T ("AUG"), _T ("SEP"), _T ("OCT"), _T ("NOV"), _T ("DEC")
};

/*
enum
{
	EEG_HEADBOX32 = 1			// EEG32
	EEG_HEADBOX128 = 3,        		// EEG128, EEG128FS
	AMB_L_HEADBOX = 8,          	// Mobee32, Mobee32-O2
	EMU_L_HEADBOX = 9,          	// Mobee-24
	SLEEP_IPUSB_HEADBOX = 14,    	// Connex / Brain Monitor
	AMBULATORY_USB_HEADBOX = 15,	// Trex
	EMU40_HEADBOX = 17,			// EMU40
	EEG32USB_HEADBOX = 19,		// EEG32u
	EEG512_HEADBOX = 20,        //Quantum
	BRAINBOX_HEADBOX = 21,		// NeuroLink IP
	BIOLOGIC_NETLINK_HEADBOX = 22,	// Bio-logic Netlink
	BIOLOGIC_TRAVELER_HEADBOX = 23,	// Bio-logic Traveler
	V32_HEADBOX,				// 26
	V44_HEADBOX,				// 27
};
*/
static const char * headboxType[] = {
	"UNKNOWN",	// 0
	 "EEG32",	// 1
	 "UNKNOWN",	// 2
	 "EEG128",	// 3
	 "UNKNOWN",	// 4
	"UNKNOWN",	// 5
	"UNKNOWN",	// 6
	"UNKNOWN",	// 7
	"MOBEE32",	// 8
	"MOBEE24",	// 9
	"UNKNOWN",	// 10
	"UNKNOWN",	// 11
	"UNKNOWN",	// 12
	"UNKNOWN",	// 13
	"CONNEX/Brain Monitor",	// 14
	"TREX",		// 15
	"UNKNOWN",	// 16
	"EMU40",	// 17
	"UNKNOWN",	// 18
	"EEG32u",	// 19
	"QUANTUM",	// 20
	"NEUROLINK IP",	// 21
	"NETLINK",	// 22
	"TRAVELER",	// 23
	"UNKNOWN",	// 24
	"UNKNOWN",	// 25
	"V32",	// 26
	"V44",	// 27
};

static void ErrorResponse (void* parent, uint8_t cmd, uint8_t* payload, uint32_t payload_size)
{
	std::cout << "Error response received for command 0x" << std::hex << cmd << std::endl;
}

static NatusDeviceInformation GetInformation (void* parent)
{
	return ((CXLDataExportReceiver*)parent)->_information;
	
}

static NatusChannelInformation GetChannelInformation(void* parent)
{
	//_channelNames.ChannelNames = new uint8_t[256];
	//memcpy(_channelNames.ChannelNames, "ab cd ef gh ij", sizeof("ab cd ef gh ij"));
	//_channelNames.StreamNames = new uint8_t[256];
	//memcpy(_channelNames.StreamNames, "C S", sizeof("C S"));
	//std::cout << _channelNames.ChannelNames << std::endl;
	//std::cout << _channelNames.StreamNames << std::endl;
	return ((CXLDataExportReceiver*)parent)->_channelNames;
}


STDMETHODIMP CXLDataExportReceiver::StartExportData (/*[in]*/ XLDataExportPatientMetadata patient, /*[in]*/ XLDataExportStudyMetadata study)
{
	try
	{
		freopen("logfile.txt", "w", stdout);
		std::cout << "Logfile for CXLDataExportReceiver" << std::endl;
		_server.Verbose(true);
		_server.Startup(SERVERPORT);

		_server.RegisterErrorCallback((NatusResponse)ErrorResponse, this);
		_server.RegisterInformationCallback((NatusInformationRequest)GetInformation, this);
		_server.RegisterChannelInformationCallback((NatusChannelInfoRequest)GetChannelInformation, this);

		_information.NumberOfChannels = study.numberOfChannels;
		_information.SamplingRate = study.samplingFreq;
		memcpy(_information.Identifier, headboxType[study.headboxType], strlen(headboxType[study.headboxType]));

		//_server.CheckForConnection();
	}
	catch (std::exception e)
	{
		std::cout << "Exception during StartExportData: " << e.what() << std::endl;
	}
	return S_OK;
}

STDMETHODIMP CXLDataExportReceiver::StopExportData ()
{
	try
	{
		_server.Disconnect();
		_server.UnregisterCallbacks();
		std::cout << "StopExportData has been called! " << std::endl;
	}
	catch (std::exception e)
	{
		std::cout << "Exception during StopExportData: " << e.what() << std::endl;
	}
	// TODO: Add your implementation code here
	return S_OK;

}

STDMETHODIMP CXLDataExportReceiver::PassExportData (/*[in]*/ TStamp samplestamp, /*[in]*/ short numberOfChannels, /*[in, size_is(numberOfChannels)]*/ float wave_data[])
{

	try
	{

		_server.SendStreamData (wave_data, numberOfChannels);
	}
	catch (std::exception e)
	{
		std::cout << e.what () << std::endl;
		
		try
		{
			_server.Disconnect();
		}
		catch (std::exception e)
		{
			std::cout << "Exception during PassExportData " << e.what() << std::endl;
		}
		_server.Startup (SERVERPORT);
		
	}

	return S_OK;
}


