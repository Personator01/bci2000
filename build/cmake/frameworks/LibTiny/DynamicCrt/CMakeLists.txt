###########################################################################
## $Id: CMakeLists.txt 7465 2023-06-30 15:23:43Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Defines a static library containing LibTiny utils.

SET( NAME LibTiny_DynamicCrt )
set_crt_link_type( DYNAMIC )
# Include "usage header" to define include directories and extlib dependencies
UTILS_INCLUDE( frameworks/LibTiny )
BCI2000_ADD_TARGET( INFO "Tiny utility library" STATIC_LIBRARY ${NAME} ${SRC_LIBTINY} )
IF( FAILED )
  MESSAGE( ERROR "Could not satisfy dependencies of the LibTiny library." )
ELSE( FAILED )
  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} )
ENDIF( FAILED )
