###########################################################################
## $Id: IncludeExtension.cmake 7883 2024-03-01 20:59:08Z mellinger $
## Authors: jezhill@gmail.com

if (NOT EXTENSIONS_WEBCAMLOGGER) # avoid linking issues

list(APPEND BCI2000_SIGSRC_INCLUDE_EXTLIB
  FFMPEG
  LIBSCHRIFT
)
list(APPEND BCI2000_SIGSRC_RESOURCES
  FreeMonoBold_ttf=${BCI2000_EXTENSION_DIR}/resources/FreeMonoBold.ttf
)
list( APPEND BCI2000_SIGSRC_FILES
   ${BCI2000_EXTENSION_DIR}/WebcamLogger.cpp
   ${BCI2000_EXTENSION_DIR}/WebcamController.cpp
   ${BCI2000_EXTENSION_DIR}/WebcamClient.h
#   ${BCI2000_EXTENSION_DIR}/win32mf/WebcamThread.cpp
#   ${BCI2000_EXTENSION_DIR}/win32mf/WebcamPipeline.cpp
#   ${BCI2000_EXTENSION_DIR}/win32mf/SaveToFileTransform.cpp
#   ${BCI2000_EXTENSION_DIR}/win32mf/GrabberTransform.cpp
#   ${BCI2000_EXTENSION_DIR}/win32mf/AudioMediaType.cpp
#   ${BCI2000_EXTENSION_DIR}/win32mf/VideoMediaType.cpp
#   ${BCI2000_EXTENSION_DIR}/win32mf/MFInit.h
   ${BCI2000_EXTENSION_DIR}/ffmpeg/WebcamThread.cpp
   ${BCI2000_EXTENSION_DIR}/ffmpeg/SaveToFile.cpp
   ${BCI2000_EXTENSION_DIR}/ffmpeg/AudioMediaType.cpp
   ${BCI2000_EXTENSION_DIR}/ffmpeg/VideoMediaType.cpp   
   ${BCI2000_SRC_DIR}/shared/utils/Rendering/TextRenderer.cpp
   ${BCI2000_SRC_DIR}/shared/utils/AV/AVEncoder.cpp
   ${BCI2000_SRC_DIR}/shared/utils/AV/AVLog.cpp
   ${BCI2000_SRC_DIR}/shared/utils/AV/AVError.cpp
)

endif()


