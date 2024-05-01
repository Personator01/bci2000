###########################################################################
## $Id: CMakeLists.txt 6350 2021-08-03 15:19:29Z madamek $
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}Private/" )

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( SignalSource )
ADD_SUBDIRECTORY( SignalProcessing )
ADD_SUBDIRECTORY( Application )
ADD_SUBDIRECTORY( Extensions )
ADD_SUBDIRECTORY( tools )
#ADD_SUBDIRECTORY( SuperStimulusPresentation )
