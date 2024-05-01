###########################################################################
## Authors: Markus Adamek, Alexander Belsten
## Description: Build information for the Cortec source module
###########################################################################

if( WIN32 )
# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( MATH )

# Allowed versions: 200, 230
# After changing the version, in order for the changes to take effect, rebuild the CortecADC project.
set(BICAPI_VERSION 230)
add_compile_definitions(BICAPI_VERSION=${BICAPI_VERSION})

IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
    set(LIB_VARIANT lib32)
ELSE()
    set(LIB_VARIANT lib64)
ENDIF()

set(BICAPI_DIR cortec_api_1.0.${BICAPI_VERSION})
INCLUDE_DIRECTORIES( ${SRC_PROJECT}/${BICAPI_DIR}/include )
set(FILES_PROJECT
    ${FILES_PROJECT}
    CortecADC.cpp
    bicapi$<$<CONFIG:Debug>:d>.imports.cpp
    ${SRC_PROJECT}/${BICAPI_DIR}/${LIB_VARIANT}/bicapi$<$<CONFIG:Debug>:d>.dll
    ${SRC_PROJECT}/${BICAPI_DIR}/${LIB_VARIANT}/ftd2xx.dll
    ${SRC_PROJECT}/${BICAPI_DIR}/${LIB_VARIANT}/log4cplusU.dll
    ${SRC_PROJECT}/${BICAPI_DIR}/${LIB_VARIANT}/log4cplusUD.dll
)

bci2000_add_signal_source_module(
  CortecADC NOBUNDLE
  ${FILES_PROJECT}
)

ENDIF( WIN32 )
