###########################################################################
## Authors:
## Description: Build information for the BB8Control application module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( 3DAPI )
if( WIN32 )
bci2000_add_application_module(
  BB8Control
  BB8ControlTask.cpp
  SpheroRAW/SpheroRAWItf.h
  SpheroRAW/BluetoothWrapper_BLED112.h
  SpheroRAW/SpheroDevice.h
  BLED112/cmd_def.cpp
  BLED112/cmd_def.h
  BLED112/apitypes.h
)
endif( WIN32 )
