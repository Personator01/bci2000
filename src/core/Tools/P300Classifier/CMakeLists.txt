###########################################################################
## $Id: CMakeLists.txt 6616 2022-03-10 21:37:56Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for P300Classifier

BCI2000_INCLUDE( SWLDA )
BCI2000_ADD_TOOLS_GUIAPP( 
  P300Classifier
  main.cpp 
  CARfilter.cpp
  CDataFile.cpp
  CmdLine.cpp
  configdialog.cpp
  configdialog.qrc
  downsampling.cpp
  filter.cpp
  GetP3Responses.cpp
  GetScore.cpp
  load_bcidat.cpp
  P300_Classify.cpp
  pages.cpp
  ReadIniParameters.cpp
  StimulusPresentation_Classify.cpp
  SWLDA.cpp
  WritePrm.cpp
)
