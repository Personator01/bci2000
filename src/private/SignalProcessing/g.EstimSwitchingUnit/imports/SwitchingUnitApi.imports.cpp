// Import definitions for SwitchingUnitApi library
// generated Tue Feb 28 12:00:01 2023 by DylibTool
#ifdef STATIC_LIBSWITCHINGUNITAPI

namespace Dylib { bool SwitchingUnitApi_Loaded() { return true; } }


#else // STATIC_LIBSWITCHINGUNITAPI

#include "SwitchingUnitApi.imports.h"
#include "DylibImports.h"
namespace { extern const Dylib::Import* functionImports; }

// Here you may specify a custom error message to be displayed when the library cannot be found.
static const char* notFoundMsg = "";
// Here you may specify an URL to some local or remote help resource.
static const char* notFoundURL = "";
RegisterDylibWithAliases( SwitchingUnitApi, "SwitchingUnitApi", functionImports, notFoundMsg, notFoundURL );

decltype(&__6225121b_GSU_AddStimulationSetting) GSU_AddStimulationSetting = 0;
decltype(&__3c082060_GSU_ClearStimulationSettingList) GSU_ClearStimulationSettingList = 0;
decltype(&__613f3a2c_GSU_Close) GSU_Close = 0;
decltype(&__6b706a3d_GSU_ConfigureImpedanceCheck) GSU_ConfigureImpedanceCheck = 0;
decltype(&__2f246a7d_GSU_GetAmplifierGroundChannels) GSU_GetAmplifierGroundChannels = 0;
decltype(&__372a525c_GSU_GetAudioOutChannel) GSU_GetAudioOutChannel = 0;
decltype(&__0e36261b_GSU_GetConstants) GSU_GetConstants = 0;
decltype(&__6560291a_GSU_GetDeviceLabel) GSU_GetDeviceLabel = 0;
decltype(&__74664f19_GSU_GetDeviceStatus) GSU_GetDeviceStatus = 0;
decltype(&__022a5a51_GSU_GetFirmwareVersion) GSU_GetFirmwareVersion = 0;
decltype(&__0a2a535f_GSU_GetHardwareVersion) GSU_GetHardwareVersion = 0;
decltype(&__517e5307_GSU_GetImpedanceCheckResult) GSU_GetImpedanceCheckResult = 0;
decltype(&__257e5346_GSU_GetImpedanceCheckResultAt) GSU_GetImpedanceCheckResultAt = 0;
decltype(&__37172555_GSU_GetNumberOfAvailableDevices) GSU_GetNumberOfAvailableDevices = 0;
decltype(&__0d035055_GSU_GetNumberOfModules) GSU_GetNumberOfModules = 0;
decltype(&__115a4646_GSU_GetSelftestResult) GSU_GetSelftestResult = 0;
decltype(&__6b6f4f00_GSU_GetSerialNumber) GSU_GetSerialNumber = 0;
decltype(&__043e4344_GSU_GetSoftwareVersion) GSU_GetSoftwareVersion = 0;
decltype(&__74575569_GSU_GetState) GSU_GetState = 0;
decltype(&__6424021b_GSU_GetStimulationSetting) GSU_GetStimulationSetting = 0;
decltype(&__10406b62_GSU_GetStimulationSettingListIterator) GSU_GetStimulationSettingListIterator = 0;
decltype(&__77506606_GSU_GetStimulationSettingListLength) GSU_GetStimulationSettingListLength = 0;
decltype(&__52595f6d_GSU_GetSwitchingMode) GSU_GetSwitchingMode = 0;
decltype(&__625f7666_GSU_GetTriggerSource) GSU_GetTriggerSource = 0;
decltype(&__08233031_GSU_Open) GSU_Open = 0;
decltype(&__3b246a7d_GSU_SetAmplifierGroundChannels) GSU_SetAmplifierGroundChannels = 0;
decltype(&__232a525c_GSU_SetAudioOutChannel) GSU_SetAudioOutChannel = 0;
decltype(&__60575569_GSU_SetState) GSU_SetState = 0;
decltype(&__7024021b_GSU_SetStimulationSetting) GSU_SetStimulationSetting = 0;
decltype(&__46595f6d_GSU_SetSwitchingMode) GSU_SetSwitchingMode = 0;
decltype(&__765f7666_GSU_SetTriggerSource) GSU_SetTriggerSource = 0;


namespace {
const Dylib::Import functionImports_[] =
{
  { "GSU_AddStimulationSetting", (void**)&GSU_AddStimulationSetting, Dylib::Import::cMangled },
  { "GSU_ClearStimulationSettingList", (void**)&GSU_ClearStimulationSettingList, Dylib::Import::cMangled },
  { "GSU_Close", (void**)&GSU_Close, Dylib::Import::cMangled },
  { "GSU_ConfigureImpedanceCheck", (void**)&GSU_ConfigureImpedanceCheck, Dylib::Import::cMangled },
  { "GSU_GetAmplifierGroundChannels", (void**)&GSU_GetAmplifierGroundChannels, Dylib::Import::cMangled },
  { "GSU_GetAudioOutChannel", (void**)&GSU_GetAudioOutChannel, Dylib::Import::cMangled },
  { "GSU_GetConstants", (void**)&GSU_GetConstants, Dylib::Import::cMangled },
  { "GSU_GetDeviceLabel", (void**)&GSU_GetDeviceLabel, Dylib::Import::cMangled },
  { "GSU_GetDeviceStatus", (void**)&GSU_GetDeviceStatus, Dylib::Import::cMangled },
  { "GSU_GetFirmwareVersion", (void**)&GSU_GetFirmwareVersion, Dylib::Import::cMangled },
  { "GSU_GetHardwareVersion", (void**)&GSU_GetHardwareVersion, Dylib::Import::cMangled },
  { "GSU_GetImpedanceCheckResult", (void**)&GSU_GetImpedanceCheckResult, Dylib::Import::cMangled },
  { "GSU_GetImpedanceCheckResultAt", (void**)&GSU_GetImpedanceCheckResultAt, Dylib::Import::cMangled },
  { "GSU_GetNumberOfAvailableDevices", (void**)&GSU_GetNumberOfAvailableDevices, Dylib::Import::cMangled },
  { "GSU_GetNumberOfModules", (void**)&GSU_GetNumberOfModules, Dylib::Import::cMangled },
  { "GSU_GetSelftestResult", (void**)&GSU_GetSelftestResult, Dylib::Import::cMangled },
  { "GSU_GetSerialNumber", (void**)&GSU_GetSerialNumber, Dylib::Import::cMangled },
  { "GSU_GetSoftwareVersion", (void**)&GSU_GetSoftwareVersion, Dylib::Import::cMangled },
  { "GSU_GetState", (void**)&GSU_GetState, Dylib::Import::cMangled },
  { "GSU_GetStimulationSetting", (void**)&GSU_GetStimulationSetting, Dylib::Import::cMangled },
  { "GSU_GetStimulationSettingListIterator", (void**)&GSU_GetStimulationSettingListIterator, Dylib::Import::cMangled },
  { "GSU_GetStimulationSettingListLength", (void**)&GSU_GetStimulationSettingListLength, Dylib::Import::cMangled },
  { "GSU_GetSwitchingMode", (void**)&GSU_GetSwitchingMode, Dylib::Import::cMangled },
  { "GSU_GetTriggerSource", (void**)&GSU_GetTriggerSource, Dylib::Import::cMangled },
  { "GSU_Open", (void**)&GSU_Open, Dylib::Import::cMangled },
  { "GSU_SetAmplifierGroundChannels", (void**)&GSU_SetAmplifierGroundChannels, Dylib::Import::cMangled },
  { "GSU_SetAudioOutChannel", (void**)&GSU_SetAudioOutChannel, Dylib::Import::cMangled },
  { "GSU_SetState", (void**)&GSU_SetState, Dylib::Import::cMangled },
  { "GSU_SetStimulationSetting", (void**)&GSU_SetStimulationSetting, Dylib::Import::cMangled },
  { "GSU_SetSwitchingMode", (void**)&GSU_SetSwitchingMode, Dylib::Import::cMangled },
  { "GSU_SetTriggerSource", (void**)&GSU_SetTriggerSource, Dylib::Import::cMangled },
  { 0, 0, 0 }
};
const Dylib::Import* functionImports = functionImports_;
}

#endif // STATIC_LIBSWITCHINGUNITAPI

