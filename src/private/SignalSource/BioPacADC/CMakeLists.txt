###########################################################################
## Authors: schalklab@HR18818.wucon.wustl.edu
## Description: Build information for the BioPacADC source module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( MATH )
if(WIN32)

# Set the executable name
SET( EXECUTABLE_NAME BioPacADC)

# Set the project specific sources
SET( SRC_PROJECT
  BioPacADC.cpp
  ../../../shared/modules/CoreModule_Qt.cpp
  ../../../shared/utils/Qt/QtMain.cpp
)

SET( HDR_PROJECT
  # perhaps you will need to add the manufacturer's SDK headers here...
  mpdev.h
)

# specify dll files to be copied
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET( FILES_PROJECT
    ${FILES_PROJECT}
    ${CMAKE_CURRENT_LIST_DIR}/x64/xerces-c_3_1.dll  
	${CMAKE_CURRENT_LIST_DIR}/x64/mpdev.dll
  )
ELSE()
  SET( FILES_PROJECT
    ${FILES_PROJECT}
    ${CMAKE_CURRENT_LIST_DIR}/x86/xerces-c_3_1.dll
	${CMAKE_CURRENT_LIST_DIR}/x86/mpdev.dll
  )
ENDIF()

# LINK_DIRECTORIES(
   # # ...the same goes for directories where third-party libraries are found (see below) 
   # lib/
# )

add_definitions(-DUSE_QT)

bci2000_add_signal_source_module(
  BioPacADC NOBUNDLE
  ${SRC_PROJECT}
  ${HDR_PROJECT}
  ${FILES_PROJECT}
)

if(TARGET BioPacADC)

IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
target_link_libraries(BioPacADC ${CMAKE_CURRENT_LIST_DIR}/x64/mpdev.lib)
ELSE()
target_link_libraries(BioPacADC ${CMAKE_CURRENT_LIST_DIR}/x86/mpdev.lib)
ENDIF()

endif(TARGET BioPacADC)

endif()
