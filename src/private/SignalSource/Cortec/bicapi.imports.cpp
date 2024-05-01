// Import definitions for bicapi library
// generated Thu Dec  8 14:20:46 2022 by DylibTool
#ifdef STATIC_LIBBICAPI

namespace Dylib { bool bicapi_Loaded() { return true; } }


#else // STATIC_LIBBICAPI

#include "bicapi.imports.h"
#include "DylibImports.h"
namespace { extern const Dylib::Import* functionImports; }

// Here you may specify a custom error message to be displayed when the library cannot be found.
static const char* notFoundMsg = "";
// Here you may specify an URL to some local or remote help resource.
static const char* notFoundURL = "";
RegisterDylibWithAliases( bicapi, "bicapi", functionImports, notFoundMsg, notFoundURL );

decltype(&__67525e78_createImplantFactory) createImplantFactory = 0;
decltype(&__353f316b_createStimulationCommandFactory) createStimulationCommandFactory = 0;
decltype(&__600d3927_getLibraryVersion) getLibraryVersion = 0;


namespace {
const Dylib::Import functionImports_[] =
{
  { "createImplantFactory", (void**)&createImplantFactory, Dylib::Import::cMangled },
  { "createStimulationCommandFactory", (void**)&createStimulationCommandFactory, Dylib::Import::cMangled },
  { "getLibraryVersion", (void**)&getLibraryVersion, Dylib::Import::cMangled },
  { 0, 0, 0 }
};
const Dylib::Import* functionImports = functionImports_;
}

#endif // STATIC_LIBBICAPI

