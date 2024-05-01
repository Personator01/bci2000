###########################################################################
## $Id: IncludeExtension.cmake 7465 2023-06-30 15:23:43Z mellinger $
## Authors: jezhill@gmail.com

IF( NOT WIN32 )
  MESSAGE( "**** AudioInputFilter failed: not supported on this platform" )
  RETURN()
ENDIF()

SET( BCI2000_SIGSRC_FILES
   ${BCI2000_SIGSRC_FILES}
   ${BCI2000_EXTENSION_DIR}/AudioInput.cpp
   ${BCI2000_EXTENSION_DIR}/AudioInputFilter.cpp
)
