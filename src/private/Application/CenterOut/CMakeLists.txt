###########################################################################
## $Id: CMakeLists.txt 7465 2023-06-30 15:23:43Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for CursorTask in Qt

bci2000_add_application_module(
  CenterOut
  CursorFeedbackTask.cpp
  FeedbackScene2D.cpp
  FeedbackScene3D.cpp
  FeedbackScene.h
  SUPPORT /prog/images/*
  SUPPORT /parms/examples/CursorTask*.prm
)
