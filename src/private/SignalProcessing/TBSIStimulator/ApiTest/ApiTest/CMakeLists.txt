###########################################################################
## $Id: CMakeLists.txt 7465 2023-06-30 15:23:43Z mellinger $
## Authors: belsten@neurotechcenter.org 2018-09-13 
## Description: Build information for the TBSIStimulator filter module

IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	find_library(INCLUDE_LIB
		HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x64/
		NAMES SLAB_USB_SPI
	)
	 SET( FILES_PROJECT
     		${FILES_PROJECT}
      		/extlib/dylib/x64/SLAB_USB_SPI.dll
	)
	find_library(INCLUDE_API_LIB
		HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x64/
		NAMES TBSIAPI
	)
	 SET( FILES_PROJECT
     		${FILES_PROJECT}
      		/extlib/dylib/x64/TBSIAPI.dll
	)
ELSE()
	find_library(INCLUDE_LIB
		HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x86/
		NAMES SLAB_USB_SPI
	)
	SET( FILES_PROJECT
     		${FILES_PROJECT}
      		/extlib/dylib/x86/SLAB_USB_SPI.dll
	)
	find_library(INCLUDE_API_LIB
		HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extlib/dylib/x86/
		NAMES TBSIAPI
	)
	 SET( FILES_PROJECT
     		${FILES_PROJECT}
      		/extlib/dylib/x86/TBSIAPI.dll
	)
ENDIF()

add_executable(
	ApiTest
	ApiTest.cpp
)

TARGET_LINK_LIBRARIES(ApiTest ${INCLUDE_LIB} ${INCLUDE_API_LIB})
