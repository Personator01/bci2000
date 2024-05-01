###########################################################################
## $Id: CMakeLists.txt 7298 2023-04-04 15:06:36Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000Automation

BCI2000_INCLUDE( COMMODULE )

utils_set_appicon( puzzlepiece )
BCI2000_ADD_TARGET(
  WINDLL BCI2000Automation

  BCI2000Automation.cpp
  BCI2000Automation_i.c
  BCI2000Automation.rc
  CBCI2000Remote.cpp
  Resource.h
  ${BCI2000_SRC_DIR}/shared/utils/Lib/sockstream.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Lib/selfpipe.cpp
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp
  
  InstallAutomation.cmd
)
