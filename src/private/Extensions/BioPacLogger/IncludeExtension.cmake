###########################################################################
## $Id: IncludeExtension.cmake 7678 2023-10-26 14:21:43Z mellinger $
## Authors: sidhant.sharma@gmail.com

IF( NOT WIN32 )
  MESSAGE( "**** BioPacLogger failed: mpdev.dll not found for this platform" )
  RETURN()
ENDIF()

#add_definitions(-DUSE_QT)

IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
  IF( MSVC )
    SET( MPDEV_LIB_DIR ${BCI2000_EXTENSION_DIR}/extlib/lib/x86/ )
    SET( BCI2000_SIGSRC_LIBS
       ${BCI2000_SIGSRC_LIBS}
       "${MPDEV_LIB_DIR}/mpdev.lib"
    )
  ENDIF()
ELSE()
  IF( MSVC )
    SET( MPDEV_LIB_DIR ${BCI2000_EXTENSION_DIR}/extlib/lib/x64/ )
    SET( BCI2000_SIGSRC_LIBS
       ${BCI2000_SIGSRC_LIBS}
       "${MPDEV_LIB_DIR}/mpdev.lib"
    )
  ENDIF()
ENDIF()

IF( NOT MPDEV_LIB_DIR )
  MESSAGE( "**** BioPacLogger failed: mpdev libraries not found for this platform/compiler" )
  RETURN()
ENDIF()

IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
	list( APPEND BCI2000_SIGSRC_FILES
	${BCI2000_SIGSRC_FILES}
    ${BCI2000_EXTENSION_DIR}/extlib/include/
    ${BCI2000_EXTENSION_DIR}/BioPacInputLogger.cpp
	${BCI2000_EXTENSION_DIR}/extlib/dylib/x86/mpdev.dll
	${BCI2000_EXTENSION_DIR}/extlib/dylib/x86/xerces-c_3_1.dll
	#../../../../src/shared/modules/CoreModule_Qt.cpp
    #../../../../src/shared/utils/Qt/QtMain.cpp
)
ELSE()
	list( APPEND BCI2000_SIGSRC_FILES
		${BCI2000_SIGSRC_FILES}
		${BCI2000_EXTENSION_DIR}/extlib/include/
		${BCI2000_EXTENSION_DIR}/BioPacInputLogger.cpp
		${BCI2000_EXTENSION_DIR}/extlib/dylib/x64/mpdev.dll
		${BCI2000_EXTENSION_DIR}/extlib/dylib/x64/xerces-c_3_1.dll
		#../../../../src/shared/modules/CoreModule_Qt.cpp
		#../../../../src/shared/utils/Qt/QtMain.cpp
)
ENDIF()

