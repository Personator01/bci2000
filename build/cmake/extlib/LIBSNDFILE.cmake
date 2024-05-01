###########################################################################
## $Id: LIBSNDFILE.cmake 7886 2024-03-02 20:23:15Z mellinger $
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including libsndfile in a project

if(WIN32)

  set( INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/libsndfile/include" )
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

  find_library( lib_sndfile sndfile )
  mark_as_advanced(lib_sndfile)
  find_path( inc_sndfile sndfile.h PATH_SUFFIXES sndfile )
  mark_as_advanced(inc_sndfile)
  set( INC_EXTLIB ${inc_sndfile} )
  set( LIBS_EXTLIB ${lib_sndfile} )
  set( EXTLIB_OK true )

endif()

