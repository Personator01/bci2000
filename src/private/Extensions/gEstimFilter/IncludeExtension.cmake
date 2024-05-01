###########################################################################
## $Id: IncludeExtension.cmake 7465 2023-06-30 15:23:43Z mellinger $
## Authors: kaleb.goering@gmail.com

# Add the gEstimFilter to all application modules

IF( NOT WIN32 )
  MESSAGE( "**** gEstimFilter failed: Not supported on this platform" )
  RETURN()
ENDIF()

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	SET( BCI2000_APP_FILES
	   ${BCI2000_APP_FILES}
	   ${BCI2000_EXTENSION_DIR}/gEstimFilter.cpp
	   ${BCI2000_EXTENSION_DIR}/lib/gEstimAPI.imports.cpp
	   ${BCI2000_EXTENSION_DIR}/lib/gEstimAPI.dll
	   ${BCI2000_EXTENSION_DIR}/lib/gEstimFactorySettings.dll
	   ${BCI2000_EXTENSION_DIR}/lib/ftd2xx.dll
	   COMPANION ${BCI2000_EXTENSION_DIR}/lib/gEstimFactorySettings.h5
	)
ELSE()
	MESSAGE("**** gEstimFilter failed: Not supported on x86 platforms")
	RETURN()
ENDIF()
