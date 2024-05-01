###########################################################################
## $Id: IncludeExtension.cmake 7465 2023-06-30 15:23:43Z mellinger $
## Authors: griffin.milsap@gmail.com

SET( BCI2000_APP_FILES
  ${BCI2000_APP_FILES}
  ${BCI2000_EXTENSION_DIR}/Gauge.cpp
  ${BCI2000_EXTENSION_DIR}/Scale.cpp
  ${BCI2000_EXTENSION_DIR}/GaugeGallery/AnalogGauge.cpp
  ${BCI2000_EXTENSION_DIR}/GaugeGallery/BarGauge.cpp
  ${BCI2000_EXTENSION_DIR}/GaugeGallery/BarPlotGauge.cpp
  ${BCI2000_EXTENSION_DIR}/GaugeGallery/HistoryGauge.cpp
  ${BCI2000_EXTENSION_DIR}/GaugeGallery/ScrollingHistoryGauge.cpp
  ${BCI2000_EXTENSION_DIR}/GaugeGallery/XYGauge.cpp
  ${BCI2000_SRC_DIR}/private/shared/utils/Quantiles.cpp
  ${BCI2000_EXTENSION_DIR}/GaugeApp.cpp
)
