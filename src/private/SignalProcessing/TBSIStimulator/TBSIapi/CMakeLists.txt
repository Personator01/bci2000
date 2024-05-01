###########################################################################
## $Id: CMakeLists.txt 7465 2023-06-30 15:23:43Z mellinger $
## Authors: belsten@neurotechcenter.org 2018-09-13 
## Description: Build information for the TBSIapi

cmake_minimum_required(VERSION 3.14)

IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	SET( EXTLIB_PROJECT ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x64/ )
ELSE()
	SET( EXTLIB_PROJECT ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x86/ )
ENDIF()

add_library(TBSIAPI SHARED 
 	    TBSIapi.cpp
	    ${EXTLIB_PROJECT}/SLAB_USB_SPI.dll 
)

TARGET_LINK_LIBRARIES(TBSIAPI ${EXTLIB_PROJECT}/SLAB_USB_SPI.lib )


#C:/BCI2000.x64/src/private/SignalProcessing/TBSIStimulator/TBSIapi/extlib/dylib/x86/SLAB_USB_SPI.dll 