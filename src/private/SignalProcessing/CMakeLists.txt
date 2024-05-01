#############################################################################
## $Id: CMakeLists.txt 6777 2022-06-01 17:00:02Z lingling $
## Authors: jezhill@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}SignalProcessing/" )

#ADD_SUBDIRECTORY( ARSignalProcessing_Hijacked )
#ADD_SUBDIRECTORY( NeuroFeedbackSignalProcessing )
#ADD_SUBDIRECTORY( IIRFilterBankSignalProcessing )
#ADD_SUBDIRECTORY( StimResponseSignalProcessing )
#ADD_SUBDIRECTORY( ReferenceViewFilter )
#ADD_SUBDIRECTORY( CP2130 )
#ADD_SUBDIRECTORY( TBSIStimulator )
#add_subdirectory( CorTecFilter )
ADD_SUBDIRECTORY( g.EstimSwitchingUnit )
ADD_SUBDIRECTORY( LisaMemycFilter )