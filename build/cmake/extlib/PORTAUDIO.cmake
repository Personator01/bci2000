###########################################################################
## $Id: PORTAUDIO.cmake 7663 2023-10-14 19:27:57Z mellinger $
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including PortAudio in a project

if(WIN32)

  set( INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/portaudio/include" )
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

  find_library( lib_portaudio portaudio )
  mark_as_advanced(lib_portaudio)

  find_path( inc_portaudio portaudio.h )
  mark_as_advanced(inc_portaudio)

  set( LIBS_EXTLIB ${lib_portaudio} )
  set( INC_EXTLIB ${inc_portaudio} )

  set( EXTLIB_OK true )

endif()
