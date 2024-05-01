###########################################################################
## $Id: CMakeLists.txt 7968 2024-03-26 18:59:01Z wengelhardt $
## Authors: engelhardt@neurotechcenter.org
## Description: Build information for CCEPFilter module
###########################################################################

SET( EXECUTABLE_NAME CCEPFilter )
SET(vis_src_path ${BCI2000_SRC_DIR}/contrib/ExternalLinks/PythonVisualizations)
SET(vis_dest_path /python/visualizations)

# Set the project specific sources
SET( SRC_PROJECT
  CCEPFilter.cpp
  PipeDefinition.cpp
  DeviceConfiguration.cpp
  NeuroOmegaStimulator.cpp
  ${BCI2000_SRC_DIR}/core/SignalProcessing/SignalSharingDemo/SignalProcessing/SignalSharingDemoFilter.cpp
)
SET( HDR_PROJECT
  CCEPFilter.h
  DeviceConfiguration.h
  ${BCI2000_SRC_DIR}/core/SignalProcessing/SignalSharingDemo/SignalProcessing/SignalSharingDemoFilter.h
)

bci2000_add_signal_processing_module(
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}"

  SUPPORT CCEP_demo.bat=>/batch
  SUPPORT demo_mouse_settings.prm=>/parms/CCEP

  SUPPORT ccepDemo.py=>${vis_dest_path}
  SUPPORT ${vis_src_path}/*=>${vis_dest_path}
  SUPPORT ${vis_src_path}/scripts/*=>${vis_dest_path}/scripts
)
