###########################################################################
## $Id: CMakeLists.txt 7711 2023-11-10 18:39:15Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for AttentionTask in Qt

# Set the executable name
SET( EXECUTABLE_NAME AttentionTask )

# Set the project specific sources
SET( SRC_PROJECT
  AttentionTask.cpp
  Cursor.cpp
)
SET( HDR_PROJECT
  AttentionTask.h
  Cursor.h
)

# Be sure to compile against EXTLIB dependencies
BCI2000_INCLUDE( "DSOUND" )
BCI2000_INCLUDE( "3DAPI" )

# Create the signal processing module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
