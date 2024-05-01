###########################################################################
## $Id: CMakeLists.txt 7886 2024-03-02 20:23:15Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the ParallelSwitch example

IF( BUILD_DEMOS )

  # Set the executable name
  SET( NAME SimpleExample )

  # Set the project specific sources
  SET( SRC_PROJECT
    SimpleExample.cpp
    ${PROJECT_SRC_DIR}/shared/utils/Lib/sockstream.cpp
  )

  set( DIR_NAME Demos/AppConnector )
  SET_OUTPUT_DIRECTORY( ${PROJECT_BUILD_ROOT}/src/contrib/AppConnectorApplications/SimpleExample )
  BCI2000_ADD_TARGET( INFO "AppConnector example" CONSOLEAPP ${NAME} ${SRC_PROJECT} )

  IF( WIN32 AND NOT FAILED )
    TARGET_LINK_LIBRARIES( ${NAME} ws2_32 )
  ENDIF()

ENDIF( BUILD_DEMOS )
