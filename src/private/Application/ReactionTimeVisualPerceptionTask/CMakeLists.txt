###########################################################################
## Authors:
## Description: Build information for the ReactionTimeVisualPerceptionTask application module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( 3DAPI )
if( WIN32 )
bci2000_add_application_module(
  ReactionTimeVisualPerceptionTask
  ReactionTimeVisualPerceptionTask.cpp
)
endif( WIN32 )
