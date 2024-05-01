SET(BCI2000_PRIVATE_EXTLIB  "${BCI2000_SRC_DIR}/private/extlib" )

if (BUILD_PRIVATE OR BUILD_PRIVATE_EXTENSIONS)
INCLUDE_EXTENSION( AudioInputFilter "${BCI2000_SRC_DIR}/private/Extensions/AudioInputFilter" )
INCLUDE_EXTENSION( GaugeExtension "${BCI2000_SRC_DIR}/private/Extensions/GaugeExtension" )
INCLUDE_EXTENSION( gEstimFilter "${BCI2000_SRC_DIR}/private/Extensions/gEstimFilter" NDA )
INCLUDE_EXTENSION( BioPacLogger "${BCI2000_SRC_DIR}/private/Extensions/BioPacLogger" )
INCLUDE_EXTENSION( WebcamLogger2 "${BCI2000_SRC_DIR}/private/Extensions/WebcamLogger2" )
endif()


