###########################################################################
## $Id: CMakeLists.txt 7678 2023-10-26 14:21:43Z mellinger $
## Authors: Alex Belsten belsten@neurotechcenter.org

bci2000_include(Qt)
bci2000_include(OpenGL)
set(GUI_DIR ${BCI2000_SRC_DIR}/shared/gui)

IF (CMAKE_SIZEOF_VOID_P EQUAL 8 AND WIN32 ) 
  # Set the executable name
  SET( EXECUTABLE_NAME gEstimProSwitchingUnit)

  # Set the project specific sources
  SET( SRC_PROJECT
    PipeDefinition.cpp
    gEstimProSwitchingUnit.cpp  
    imports/SwitchingUnitApi.imports.cpp
    lib/x64/SwitchingUnitApi.dll
    lib/x64/Gtec.Devices.SwitchingUnit.dll
    lib/x64/gestimproswitchingunit_ftd2xx.dll

    ${GUI_DIR}/ProgressBarVis.cpp
    ${GUI_DIR}/GUI.cpp
    ${GUI_DIR}/OpenGLContext.cpp
    ${GUI_DIR}/GraphDisplay.cpp
    ${GUI_DIR}/GraphObject.cpp
    ${GUI_DIR}/Shapes.cpp
    ${GUI_DIR}/TextField.cpp
    ${GUI_DIR}/Bitmap.cpp
    ${PROJECT_SRC_DIR}/shared/utils/Qt/QtMain.cpp
    ${BCI2000_SRC_DIR}/shared/utils/Qt/QtProxyThread.cpp
  )

  SET( HDR_PROJECT
    gEstimProSwitchingUnit.h  
    imports/SwitchingUnitApi.h
    imports/SwitchingUnitApi.imports.h
  )

  # Create the signal processing module
  BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )
ELSE()
  MESSAGE( "--- g.EstimProSwitchingUnit libaries not available for this platform. Will not build." )
ENDIF()
