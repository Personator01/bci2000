###########################################################################
## Authors: schalklab@HR18821
## Description: Build information for the KastnerCueingTask application module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( 3DAPI )

if(WIN32)

bci2000_add_application_module(
  KastnerCueingTask NOBUNDLE
  KastnerCueingTask.cpp

  KastnerCueingTask_Intro.bat
  KastnerCueingTask_Test.bat
)

endif(WIN32)
