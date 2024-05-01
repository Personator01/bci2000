###########################################################################
## Authors:
## Description: Build information for the NatusSignalSource source module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( MATH )
if( WIN32 )
add_definitions(-D_ATL_ALLOW_CHAR_UNSIGNED)
endif( WIN32 )
bci2000_add_signal_source_module(
  NatusSignalSource NDA
  NatusADC.cpp
  NatusService/NatusClient.cpp
  NatusService/NatusPackageInterface.cpp
  StimulusPresentation_Natus.bat
)
