###########################################################################
## $Id: SDL2.cmake 7785 2024-01-04 22:58:25Z mellinger $
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including PortAudio in a project

set( INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/SDL2/include" )
#set( INC_EXTLIB "/opt/homebrew/include/SDL2" )
set( LIBS_EXTLIB )
set( HDR_EXTLIB )
set( SRC_EXTLIB )

if(WIN32)

  file( GLOB importfiles_ "${INC_EXTLIB}/../imports/*" )
  list( APPEND SRC_EXTLIB ${importfiles_} )
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( libdir_ "${INC_EXTLIB}/../win32-amd64/dynamic" )
  else()
    set( libdir_ "${INC_EXTLIB}/../win32-x86/dynamic" )
  endif()
  file( GLOB dlls_ "${libdir_}/*.dll" )
  foreach( _R0 ${dlls_} )
    list( APPEND SRC_EXTLIB COMPANION ${_R0} )
  endforeach()
  set( EXTLIB_OK TRUE )

else()

  set( LIBS_EXTLIB -lSDL2 )
  find_library( lib_sdl2 SDL2 )
  find_path( inc_sdl2 SDL.h PATH_SUFFIXES SDL2 )
  if( NOT lib_sdl2 )
	  set( inc_sdl2 "${PROJECT_SRC_DIR}/extlib/homebrew/include/SDL" )
	  set( lib_sdl2 "${PROJECT_SRC_DIR}/extlib/homebrew/lib/libSDL2.a" )
	  message( "SDL2 could not be found by CMake. Using the portable homebrew location. Please run install_dependencies, if you have not already." )
  else()
	  # use actual parent directory to avoid error 20 "not a directory" when loading library
          get_filename_component( lib_sdl2 ${lib_sdl2} DIRECTORY )
          get_filename_component( inc_sdl2 ${inc_sdl2} DIRECTORY )
  endif()
  #message( "Using sdl2 lib file ${lib_sdl2}" )
  #message( "Using sdl2 include files from ${inc_sdl2}" )

  set( LIBDIR_EXTLIB "${lib_sdl2}" )
  set( INC_EXTLIB "${inc_sdl2}" )

  set( EXTLIB_OK true )

endif()

