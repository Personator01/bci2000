###########################################################################
## Authors: Lorenzo@Archimede.dhcp.wustl.edu
## Description: Build information for the DelsysTrignoAvanti source module
###########################################################################
if(WIN32)
# Set the executable name
SET( EXECUTABLE_NAME DelsysTrignoAvanti )

# Set the project specific sources
SET( SRC_PROJECT
  DelsysTrignoAvantiADC.cpp
  imports/DelsysClient.cpp
)

SET( HDR_PROJECT
 DelsysTrignoAvantiADC.h
 imports/DelsysClient.h
)

#INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/custom/DelsysTrignoAvanti/lib )

BCI2000_INCLUDE( SOURCEFILTER )

bci2000_add_signal_source_module(
  DelsysTrignoAvanti NOBUNDLE
  "${SRC_PROJECT}"
  imports/DelsysClient.h
  SUPPORT batch/*=>/batch
  SUPPORT parms/*=>/parms/fragments/amplifiers
)
endif(WIN32)
