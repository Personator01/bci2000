###########################################################################
## Authors:
## Description: Build information for the TBSIStimulator project
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( MATLAB )

IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	find_library(INCLUDE_LIB
		HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x64/
		NAMES SLAB_USB_SPI
	)
	 SET( FILES_PROJECT
     		${FILES_PROJECT}
      		extlib/dylib/x64/SLAB_USB_SPI.dll
	)
	find_library(INCLUDE_API_LIB
		HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x64/
		NAMES TBSIAPI
	)
	 SET( FILES_PROJECT
     		${FILES_PROJECT}
      		extlib/dylib/x64/TBSIAPI.dll
	)
ELSE()
	find_library(INCLUDE_LIB
		HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x86/
		NAMES SLAB_USB_SPI
	)
	SET( FILES_PROJECT
     		${FILES_PROJECT}
      		extlib/dylib/x86/SLAB_USB_SPI.dll
	)
	find_library(INCLUDE_API_LIB
		HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x86/
		NAMES TBSIAPI
	)
	 SET( FILES_PROJECT
     		${FILES_PROJECT}
      		extlib/dylib/x86/TBSIAPI.dll
	)
ENDIF()

bci2000_add_signal_processing_module(
  TBSIStimulator NOBUNDLE
  PipeDefinition.cpp
  TBSIStimulatorFilter.cpp
  Pulse.cpp
)


TARGET_LINK_LIBRARIES(TBSIStimulator ${INCLUDE_LIB} ${INCLUDE_API_LIB})