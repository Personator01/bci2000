###########################################################################
## Authors: lingl@DESKTOP-GH0R7CA.wucon.wustl.edu
## Description: Build information for the Communication_task_dual application module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( 3DAPI )

bci2000_add_application_module(
  Communication_task_dual NOBUNDLE
  Communication_task_dualTask.cpp
  HyperscanningApplicationBase/HyperscanningApplicationBase.cpp
  Slider.cpp
)
