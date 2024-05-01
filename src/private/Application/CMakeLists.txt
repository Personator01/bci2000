###########################################################################
## $Id: CMakeLists.txt 7760 2023-12-12 18:06:12Z lingling $
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}Application/" )

# Add this to the CMAKELists.txt in this directory
ADD_SUBDIRECTORY( Games )
ADD_SUBDIRECTORY( AttentionTask )
ADD_SUBDIRECTORY( CursorTaskWithGauges )
ADD_SUBDIRECTORY( GaugesOnly )
ADD_SUBDIRECTORY( ReactionTask )
ADD_SUBDIRECTORY( NeurofeedbackTask )
ADD_SUBDIRECTORY( ReactionAuditoryCalibrationTask )
add_subdirectory( ReactionAuditoryTask )
add_subdirectory( ReactionVisualCalibrationTask )
add_subdirectory( ReactionTimeVisualPerceptionTask )
add_subdirectory( BB8Control )
ADD_SUBDIRECTORY( KastnerCueingTask )
ADD_SUBDIRECTORY( AnnaRaymondTask )
#ADD_SUBDIRECTORY( SpaceInvaderApp )
#ADD_SUBDIRECTORY( KeypressHelper )
add_subdirectory( Communication_task_dual )
ADD_SUBDIRECTORY( SpatialWorkingMemoryGT )
ADD_SUBDIRECTORY( VibrationHabituation )
add_subdirectory( EglyDriverTask )
