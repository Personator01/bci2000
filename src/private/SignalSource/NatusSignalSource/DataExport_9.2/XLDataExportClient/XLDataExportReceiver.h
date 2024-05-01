// XLDataExportReceiver.h : Declaration of the CXLDataExportReceiver

#ifndef __XLDATAEXPORTRECEIVER_H_
#define __XLDATAEXPORTRECEIVER_H_

#include "resource.h"       // main symbols
#include "..\..\NatusService\NatusDataServer.h"
/////////////////////////////////////////////////////////////////////////////
// CXLDataExportReceiver
class ATL_NO_VTABLE CXLDataExportReceiver: 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CXLDataExportReceiver, &CLSID_XLDataExportReceiver>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CXLDataExportReceiver>,
	public IXLDataExportReceiver
{
public:
	CXLDataExportReceiver();
	~CXLDataExportReceiver();

DECLARE_REGISTRY_RESOURCEID(IDR_XLDATAEXPORTRECEIVER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CXLDataExportReceiver)
	COM_INTERFACE_ENTRY(IXLDataExportReceiver)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CXLDataExportReceiver)
END_CONNECTION_POINT_MAP()


// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IXLDataExportReceiver
public:
	STDMETHOD(PassExportData)(/*[in]*/ TStamp samplestamp, /*[in]*/ short numberOfChannels, /*[in, size_is(numberOfChannels)]*/ float wave_data[]);
	STDMETHOD(StopExportData)();
	STDMETHOD(StartExportData)(XLDataExportPatientMetadata patient, XLDataExportStudyMetadata study);
  volatile bool _streamData;
  NatusDeviceInformation _information;
  NatusChannelInformation _channelNames;

private:
  NatusDataServer _server;

};

static void ErrorResponse(void* parent, uint8_t cmd, uint8_t* payload, uint32_t payload_size);
static NatusDeviceInformation GetInformation(void* parent);

#endif //__XLDATAEXPORTRECEIVER_H_
