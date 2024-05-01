###########################################################################
## $Id: CMakeLists.txt 7465 2023-06-30 15:23:43Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000Automation

BCI2000_INCLUDE( COMModule )

utils_set_appicon( puzzlepiece )
BCI2000_ADD_TARGET(
  WINDLL XLDataExportClient

  XLDataExportClient.cpp
  XLDataExportClient_i.c
  XLDataExportClient.rc
  XLDataExportReceiver.cpp
  Resource.h

  InstallXLDataExportClient.cmd
)
