// XLDataExportReceiver.cpp : Implementation of CXLDataExportReceiver
#include "stdafx.h"
#include "XLDataExportClient.h"
#include "XLDataExportReceiver.h"
#include <string>
#include <exception>
#include <iomanip> 
#define SERVERPORT 1030
/////////////////////////////////////////////////////////////////////////////
// CXLDataExportReceiver

CXLDataExportReceiver::CXLDataExportReceiver()
{
}

CXLDataExportReceiver::~CXLDataExportReceiver()
{
}

STDMETHODIMP CXLDataExportReceiver::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IXLDataExportReceiver
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)  // Unicode OK
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

static const TCHAR* month_tbl[] = {
	_T("---"), _T("JAN"), _T("FEB"), _T("MAR"), _T("APR"), _T("MAY"), _T("JUN"), _T("JUL"), _T("AUG"), _T("SEP"), _T("OCT"), _T("NOV"), _T("DEC")
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
	EMU64_BRAIN_MONITOR_HEADBOX,			// 28		Sleep amp
};
*/
static const TCHAR* headboxType[] = {
	_T("UNKNOWN"),	// 0
	_T("EEG32"),	// 1
	_T("UNKNOWN"),	// 2
	_T("EEG128"),	// 3
	_T("UNKNOWN"),	// 4
	_T("UNKNOWN"),	// 5
	_T("UNKNOWN"),	// 6
	_T("UNKNOWN"),	// 7
	_T("MOBEE32"),	// 8
	_T("MOBEE24"),	// 9
	_T("UNKNOWN"),	// 10
	_T("UNKNOWN"),	// 11
	_T("UNKNOWN"),	// 12
	_T("UNKNOWN"),	// 13
	_T("CONNEX/Brain Monitor"),	// 14
	_T("TREX"),		// 15
	_T("UNKNOWN"),	// 16
	_T("EMU40"),	// 17
	_T("UNKNOWN"),	// 18
	_T("EEG32u"),	// 19
	_T("QUANTUM"),	// 20
	_T("NEUROLINK IP"),	// 21
	_T("NETLINK"),	// 22
	_T("TRAVELER")	// 23
	_T("UNKNOWN"),	// 24
	_T("UNKNOWN"),	// 25
	_T("V32"),	// 26
	_T("V44"),	// 27
	_T("Brain Monitor"),	// 28
	_T("Embla NDx"),	// 29
	_T("Embla SDx"),	// 30
	_T("Brain Monitor iX"),	// 31
	_T("Cain") //34
};

static void ErrorResponse(void* parent, uint8_t cmd, uint8_t* payload, uint32_t payload_size)
{
  std::cout << "Error response received for command 0x" << std::hex << cmd << std::endl;
}


static NatusDeviceInformation GetInformation(void* parent)
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

STDMETHODIMP CXLDataExportReceiver::StartExportData(/*[in]*/ XLDataExportPatientMetadata patient, /*[in]*/ XLDataExportStudyMetadata study)
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
    memcpy(_information.Identifier, headboxType[study.headboxType], _tcsclen(headboxType[study.headboxType]));

    //_server.CheckForConnection();
  }
  catch (std::exception e)
  {
    std::cout << "Exception during StartExportData: " << e.what() << std::endl;
  }
  return S_OK;
}

STDMETHODIMP CXLDataExportReceiver::StopExportData()
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

STDMETHODIMP CXLDataExportReceiver::PassExportData(/*[in]*/ TStamp samplestamp, /*[in]*/ short numberOfChannels, /*[in, size_is(numberOfChannels)]*/ float wave_data[])
{
  try
  {

    _server.SendStreamData(wave_data, numberOfChannels);
  }
  catch (std::exception e)
  {
    std::cout << e.what() << std::endl;

    try
    {
      _server.Disconnect();
    }
    catch (std::exception e)
    {
      std::cout << "Exception during PassExportData " << e.what() << std::endl;
    }
    _server.Startup(SERVERPORT);

  }

  return S_OK;
}
