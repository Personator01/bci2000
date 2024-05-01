###########################################################################
## Authors:
## Description: Build information for the NeuralynxNetComADC source module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( MATH )

if( WIN32 )

INCLUDE_DIRECTORIES( ${SRC_PROJECT}/lib/include)
# BCI2000_INCLUDE( "IIRSOURCEFILTER" )

set ( FILES_PROJECT
  NeuralynxNetComADC.cpp
  NetComCommands.cpp
  NeuralynxUDPConverter.cpp
  CircularBuffer.h
)


IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
	
	 set(	FILES_PROJECT	
		${FILES_PROJECT}
		${SRC_PROJECT}/lib/NetComClient3.dll
		)
		
ELSE()
	
	set(	FILES_PROJECT	
		${FILES_PROJECT}
		${SRC_PROJECT}/lib/NetComClient3_x64.dll
		)
		
ENDIF()

bci2000_add_signal_source_module(
  NeuralynxNetComADC NOBUNDLE NDA
 ${FILES_PROJECT}
)

if (TARGET NeuralynxNetComADC)
	IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
	target_link_libraries(NeuralynxNetComADC ${CMAKE_CURRENT_LIST_DIR}/lib/NetComClient3.lib)
	ELSE()
	target_link_libraries(NeuralynxNetComADC ${CMAKE_CURRENT_LIST_DIR}/lib/NetComClient3_x64.lib)
	ENDIF()
endif()

endif( WIN32 )
