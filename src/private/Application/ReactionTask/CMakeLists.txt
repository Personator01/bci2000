###########################################################################
## $Id: CMakeLists.txt 7465 2023-06-30 15:23:43Z mellinger $
## Authors:
## Description: Build information for the ReactionTask application module
##              
##              
###########################################################################
if( WIN32 )
BCI2000_ADD_APPLICATION_MODULE( ReactionTask
  ReactionTaskFilter.cpp
  ${BCI2000_SRC_DIR}/contrib/Extensions/StimBoxFilter/lib/gSTIMbox.imports.cpp
)
endif()
