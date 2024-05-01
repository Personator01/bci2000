// Import definitions for bicapid library
// generated Thu Dec  8 14:22:23 2022 by DylibTool
#ifndef BICAPID_IMPORTS_H
#define BICAPID_IMPORTS_H

#include "Win32Defs.h"

#ifndef STATIC_LIBBICAPID
// Disable function declarations in the original header
// file by #defining function names to point to symbols that
// are not referenced by any code.
#define createImplantFactory __67525e78_createImplantFactory
#define createStimulationCommandFactory __353f316b_createStimulationCommandFactory
#define getLibraryVersion __600d3927_getLibraryVersion

#define __declspec(x)
#endif // STATIC_LIBBICAPID

#if(BICAPI_VERSION==200)
    #include "cortec_api_1.0.200/include/bicapi.h"
#elif(BICAPI_VERSION==230)
    #include "cortec_api_1.0.230/include/bicapi.h"
#endif

#ifndef STATIC_LIBBICAPID
#undef __declspec
// Use #undef to restore function names before declaring
// function pointers with the names originally used to
// declare imports.
#undef createImplantFactory
extern decltype(&__67525e78_createImplantFactory) createImplantFactory;
#undef createStimulationCommandFactory
extern decltype(&__353f316b_createStimulationCommandFactory) createStimulationCommandFactory;
#undef getLibraryVersion
extern decltype(&__600d3927_getLibraryVersion) getLibraryVersion;

#endif // STATIC_LIBBICAPID

namespace Dylib { bool bicapid_Loaded(); }

#endif // BICAPID_IMPORTS_H
