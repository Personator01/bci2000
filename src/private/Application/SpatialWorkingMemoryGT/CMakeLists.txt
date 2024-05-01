###########################################################################
## Authors: schalklab@HR18826
## Description: Build information for the SpatialWorkingMemoryGT application module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( 3DAPI )

if( WIN32 )
  bci2000_add_application_module(
    SpatialWorkingMemoryGT NOBUNDLE
    SpatialWorkingMemoryGTTask.cpp
    ${BCI2000_SRC_DIR}/contrib/Extensions/StimBoxFilter/lib/gSTIMbox.imports.cpp
  )
endif()

