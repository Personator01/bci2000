###########################################################################
## $Id: FFMPEG.cmake 7785 2024-01-04 22:58:25Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including ffmpeg-LibAV in a project

set( INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/ffmpeg/include" )
set( LIBS_EXTLIB )
set( HDR_EXTLIB )
set( SRC_EXTLIB )

if(WIN32)

  file( GLOB libavfiles_ "${INC_EXTLIB}/../imports/*" )
  list( APPEND SRC_EXTLIB ${libavfiles_} )
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

  set( LIBS_EXTLIB -lavcodec -lavutil -lavformat -lswresample -lswscale )
  find_library( lib_ffmpeg avcodec )
  find_path( inc_ffmpeg avcodec.h PATH_SUFFIXES libavcodec )
  if( NOT lib_ffmpeg )
	  set( inc_ffmpeg "${PROJECT_SRC_DIR}/extlib/homebrew/include" )
	  set( lib_ffmpeg "${PROJECT_SRC_DIR}/extlib/homebrew/lib" )
	  message( "FFMPEG could not be found by CMake. Using the portable homebrew location. Please run install_dependencies, if you have not already." )
  else()
	  # use actual parent directory to avoid error 20 "not a directory" when loading library
          get_filename_component( lib_ffmpeg ${lib_ffmpeg} DIRECTORY )
          get_filename_component( inc_ffmpeg ${inc_ffmpeg} DIRECTORY )
  endif()
#  message( "Using ffmpeg lib file ${lib_ffmpeg}" )
#  message( "Using ffmpeg include files from ${inc_ffmpeg}" )

  set( LIBDIR_EXTLIB "${lib_ffmpeg}" )
  set( INC_EXTLIB "${inc_ffmpeg}" )
  set( EXTLIB_OK true )

endif()

