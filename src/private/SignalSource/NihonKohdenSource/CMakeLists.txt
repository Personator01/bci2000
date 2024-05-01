###########################################################################
## $Id: CMakeLists.txt 7976 2024-03-28 06:31:50Z mellinger $
## Authors:
## Description: Build information for the NihonKohdenSource source module
###########################################################################

if(WIN32)

# Set the executable name
SET( EXECUTABLE_NAME NihonKohdenSource )

# Set the project specific sources
SET( SRC_PROJECT
  NihonKohdenADC.cpp
  lib/EegDataSource.imports.cpp
)

# specify dll files to be copied
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET( FILES_PROJECT
    ${FILES_PROJECT}
    lib/coff64/EegDataSource.dll
  )
  # EegDataSource.dll truncates pointers to 32bit,
  # so we must disable "high entropy" addresses.
  # see: https://devblogs.microsoft.com/oldnewthing/20150709-00/?p=45181
  add_link_options("/HIGHENTROPYVA:NO" "/LARGEADDRESSAWARE:NO")
ELSE()
  SET( FILES_PROJECT
    ${FILES_PROJECT}
    lib/coff/EegDataSource.dll
  )
ENDIF()

# specify support files 
SET( FILES_PROJECT
  ${FILES_PROJECT}
  SUPPORT batch/*=>/batch
  SUPPORT parms/*=>/parms/fragments/amplifiers
)

# Add the source module to the Makefile or compiler project file:
BCI2000_ADD_SIGNAL_SOURCE_MODULE(
  "${EXECUTABLE_NAME}"
  "${SRC_PROJECT}" "${FILES_PROJECT}"
)

endif()
