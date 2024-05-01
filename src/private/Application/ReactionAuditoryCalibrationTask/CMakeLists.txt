###########################################################################
## $Id: CMakeLists.txt 7465 2023-06-30 15:23:43Z mellinger $
## Authors:
## Description: Build information for the ReactionPest application module
##              
##              
###########################################################################

if( WIN32 )
# Set the executable name
SET( EXECUTABLE_NAME ReactionAuditoryCalibrationTask )

# Set the project specific sources
SET( SRC_PROJECT
  ReactionAuditoryCalibrationTask.cpp
)

SET( HDR_PROJECT
  ReactionAuditoryCalibrationTask.h
)


LIST( APPEND SRC_PROJECT ${BCI2000_SRC_DIR}/shared/modules/CoreMain.cpp )
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

endif( WIN32 )
