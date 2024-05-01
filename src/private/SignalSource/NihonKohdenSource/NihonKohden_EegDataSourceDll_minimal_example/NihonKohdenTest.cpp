#include <Windows.h>
#include <iostream>
#include <stdexcept>
#include "CmdParamInfo.h"

namespace {
  unsigned long	(__stdcall *InitializeDll)(int nDllMode, const char* pszLicense, const char* pszLogFolder, const int nId);
  bool			(__stdcall *EndDll)(unsigned long ulIdentifier);
  int				(__stdcall *InitializeReaderMode)(unsigned long ulIdentifier, const READER_MODE_INIT_INFO& stInitInfo, void (CALLBACK* pStateChangeHandler)(int nState, int nSubState, void* pAddInfo));
  int				(__stdcall *ReaderModeConnect)(unsigned long ulIdentifier);
  int				(__stdcall *ReaderModeClose)(unsigned long ulIdentifier);
  int				(__stdcall *ReaderModeEnd)(unsigned long ulIdentifier);
  int				(__stdcall *GetDataFrameCount)(unsigned long ulIdentifier, UINT& nFrameCount);
  int				(__stdcall *GetFloatData)(unsigned long ulIdentifier, UINT nFrameCount, UINT& nReadCount, DWORD* pTimeStamp, float* pData, WORD* pDigitalMark, UINT64* pSampleNo);
  int				(__stdcall *GetReconnectStatus)(unsigned long ulIdentifier);
  int				(__stdcall *ResetReadOffset)(unsigned long ulIdentifier);

  int				(__stdcall *MMFileGetSamplingRate)(unsigned long ulIdentifier, unsigned int& nRate);
  int				(__stdcall *MMFileGetElectrodeCount)(unsigned long ulIdentifier, unsigned int& nCount);
  int				(__stdcall *MMFileGetElectrodeName)(unsigned long ulIdentifier, MMFILE_ELECTRODE_NAME& stName);
  int				(__stdcall *MMFileGetElectrodeName2)(unsigned long ulIdentifier, MMFILE_ELECTRODE_NAME& stName);
  int				(__stdcall *MMFileGetElectrodeCode)(unsigned long ulIdentifier, MMFILE_ELECTRODE_CODE& stCode);
  int				(__stdcall *MMFileGetPatientItemCount)(unsigned long ulIdentifier, unsigned int& nCount);
  int				(__stdcall *MMFileGetPatientItemAttr)(unsigned long ulIdentifier, unsigned long ulItemID, MMFILE_PATIENT_ITEM_ATTR& stAttr);
  int				(__stdcall *MMFileGetPatientItemName)(unsigned long ulIdentifier, unsigned long ulItemID, char* pszName, int nNameLength);
  int				(__stdcall *MMFileGetPatientItemData)(unsigned long ulIdentifier, unsigned long ulItemID, char* pszData, int nDataLength);
  int				(__stdcall *MMFileGetOpenReadAppCount)(unsigned long ulIdentifier, unsigned int& nCount);
  int				(__stdcall *MMFileGetFileVersion)(unsigned long ulIdentifier, unsigned short& wVersion);
  int				(__stdcall *MMFileGetFileRevision)(unsigned long ulIdentifier, unsigned short& wRevision);
  int				(__stdcall *MMFileGetCreateDateTime)(unsigned long ulIdentifier, char* pszTime, int nTimeLength);
  int				(__stdcall *MMFileGetUpdateDateTime)(unsigned long ulIdentifier, char* pszTime, int nTimeLength);
  int				(__stdcall *MMFileGetGUID)(unsigned long ulIdentifier, char* pszGUID, int nGUIDLength);
  int				(__stdcall *MMFileGetReadAppCount)(unsigned long ulIdentifier, unsigned int& nCount);
  int				(__stdcall *MMFileGetWriteAppHandle)(unsigned long ulIdentifier, unsigned long& ulHandle);
  int				(__stdcall *MMFileGetReadAppHandle)(unsigned long ulIdentifier, unsigned int nAppNo, unsigned long& ulHandle);
  int				(__stdcall *MMFileGetStoreTime)(unsigned long ulIdentifier, unsigned int& nTime);
  int				(__stdcall *MMFileGetSetterCount)(unsigned long ulIdentifier, unsigned int& nCount);
  int				(__stdcall *MMFileGetDigitalMarkCount)(unsigned long ulIdentifier, unsigned int& nCount);
  int				(__stdcall *MMFileGetSetterDataCount)(unsigned long ulIdentifier, unsigned int nSetterNo, unsigned int& nCount);
  int				(__stdcall *MMFileGetDataSourceList)(unsigned long ulIdentifier, MMFILE_DATA_SOURCE_INFO stList[], int& nListCount);
  int				(__stdcall *MMFileGetJunctionBoxType)(unsigned long ulIdentifier, unsigned long& ulType);
  int				(__stdcall *MMFileGetMiniJunctionBoxType)(unsigned long ulIdentifier, unsigned long& ulType);
  int				(__stdcall *MMFileGetQI123ACount)(unsigned long ulIdentifier, unsigned int& nCount);
  int				(__stdcall *MMFileGetQI123AInfo)(unsigned long ulIdentifier, unsigned int nIndex, MMFILE_QI123A_INFO& stInfo);
  int				(__stdcall *MMFileGetDataType)(unsigned long ulIdentifier, unsigned long& ulType);
  int				(__stdcall *MMFileGetDataSourceType)(unsigned long ulIdentifier, unsigned long& ulType);
  int				(__stdcall *MMFileGetDataSourceExInfo)(unsigned long ulIdentifier, unsigned long& ulInfo);
}

void LoadDll()
{
  HMODULE hLib = ::LoadLibraryA("EEGDataSource");
  if (!hLib)
    throw std::runtime_error("Could not load EEGDataSource library");
#define RESOLVE(x) x = (decltype(x))::GetProcAddress(hLib, #x); if( !x ) throw std::runtime_error("Could not get address for " #x)
  RESOLVE(InitializeDll);
  RESOLVE(EndDll);

  RESOLVE(InitializeReaderMode);
  RESOLVE(ReaderModeConnect);
  RESOLVE(ReaderModeClose);
  RESOLVE(ReaderModeEnd);
  RESOLVE(GetDataFrameCount);
  RESOLVE(GetFloatData);
  RESOLVE(GetReconnectStatus);
  RESOLVE(ResetReadOffset);

  RESOLVE(MMFileGetSamplingRate);
  RESOLVE(MMFileGetElectrodeCount);
  RESOLVE(MMFileGetElectrodeName);
  RESOLVE(MMFileGetElectrodeName2);
  RESOLVE(MMFileGetElectrodeCode);
  RESOLVE(MMFileGetPatientItemCount);
  RESOLVE(MMFileGetPatientItemAttr);
  RESOLVE(MMFileGetPatientItemName);
  RESOLVE(MMFileGetPatientItemData);
  RESOLVE(MMFileGetOpenReadAppCount);
  RESOLVE(MMFileGetFileVersion);
  RESOLVE(MMFileGetFileRevision);
  RESOLVE(MMFileGetCreateDateTime);
  RESOLVE(MMFileGetUpdateDateTime);
  RESOLVE(MMFileGetGUID);
  RESOLVE(MMFileGetReadAppCount);
  RESOLVE(MMFileGetWriteAppHandle);
  RESOLVE(MMFileGetReadAppHandle);
  RESOLVE(MMFileGetStoreTime);
  RESOLVE(MMFileGetSetterCount);
  RESOLVE(MMFileGetDigitalMarkCount);
  RESOLVE(MMFileGetSetterDataCount);
  RESOLVE(MMFileGetDataSourceList);
  RESOLVE(MMFileGetJunctionBoxType);
  RESOLVE(MMFileGetMiniJunctionBoxType);
  RESOLVE(MMFileGetQI123ACount);
  RESOLVE(MMFileGetQI123AInfo);
  RESOLVE(MMFileGetDataType);
  RESOLVE(MMFileGetDataSourceType);
  RESOLVE(MMFileGetDataSourceExInfo);
#undef RESOLVE
}

unsigned long
Connect()
{
  unsigned long id = InitializeDll( DS_MODE_READER, NULL, NULL, 0 );
  if( id == 0 ) throw std::runtime_error( "Could not initialize NK DLL" );

  READER_MODE_INIT_INFO initInfo = { 0 };
  initInfo.bSelectDataSource = true;
  if( InitializeReaderMode( id, initInfo, nullptr ) )
    throw std::runtime_error( "Could not initialize reader mode!" );
  if( ReaderModeConnect( id ) )
    throw std::runtime_error( "Could not connect to device!" );

  return id;
}

void
Disconnect( unsigned long id )
{
  if( ReaderModeClose( id ) )
    throw std::runtime_error( "Could not close reader mode" );
  if( ReaderModeEnd( id ) )
    throw std::runtime_error( "Could not close dll" );
}

int main(int, char**)
{
  try {
    LoadDll();
    std::cout << "DLL loaded" << std::endl;
    unsigned long id = Connect();
    std::cout << "Connected with id " << id << std::endl;
    Disconnect(id);
    std::cout << "Disconnected" << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  return 0;
}
