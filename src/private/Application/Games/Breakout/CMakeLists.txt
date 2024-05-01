###########################################################################
## $Id: CMakeLists.txt 6415 2021-11-02 17:09:34Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for Breakout in Qt

if(FALSE) # qgl not supported by Qt any more

# Set the executable name
SET( EXECUTABLE_NAME GameBreakout )

# Set the project specific sources
SET( SRC_PROJECT
  BreakoutGame.cpp
  BreakoutWidget.cpp
  ../Game.cpp
  ../GameWidget.cpp
  ../ThemeManager.cpp
  ../RenderObject.cpp
  ../RectangleObject.cpp
  ../CircleObject.cpp
  Ball.cpp
  BlockArray.cpp
  Brick.cpp
  Paddle.cpp
)
SET( HDR_PROJECT
  BreakoutGame.h
  BreakoutWidget.h
  ../Game.h
  ../GameWidget.h
  ../ThemeManager.h
  ../RenderObject.h
  ../RectangleObject.h
  ../CircleObject.h
  Ball.h
  BlockArray.h
  Brick.h
  Paddle.h
)

# Be sure to compile against EXTLIB dependencies
BCI2000_INCLUDE( "DSOUND" )
BCI2000_INCLUDE( "3DAPI" )

# Create the signal processing module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

endif(FALSE) # qgl not supported any more

