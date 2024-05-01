###########################################################################
## $Id: CMakeLists.txt 7043 2022-12-04 19:27:36Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000 and CMake

IF( WIN32 )

# Set this directory name
SET( DIR_NAME "${DIR_NAME}BCPy2000/" )

OPTION( BUILD_BCPY2000 "Build BCPy2000 modules" OFF )

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( foundation )
file(COPY demo/batch/FindPortablePython.bat DESTINATION ${PROJECT_BUILD_ROOT}/batch/)
file(COPY demo/batch/PythonDemo_Template.bat DESTINATION ${PROJECT_BUILD_ROOT}/batch/)
file(COPY demo/batch/PythonDemo_Triangle.bat DESTINATION ${PROJECT_BUILD_ROOT}/batch/)
file(COPY demo/parms/PythonDemo1_Triangle.prm DESTINATION ${PROJECT_BUILD_ROOT}/parms/)
file(COPY demo/parms/PythonDemo2_Playback.prm DESTINATION ${PROJECT_BUILD_ROOT}/parms/)

file(MAKE_DIRECTORY ${PROJECT_BUILD_ROOT}/python/)
file(COPY demo/python/BciApplication.py DESTINATION ${PROJECT_BUILD_ROOT}/python/)
file(COPY demo/python/BciSignalProcessing.py DESTINATION ${PROJECT_BUILD_ROOT}/python/)
file(COPY demo/python/BciSource.py DESTINATION ${PROJECT_BUILD_ROOT}/python/)
file(COPY demo/python/feedback-drums.WAV DESTINATION ${PROJECT_BUILD_ROOT}/python/)
file(COPY demo/python/feedback-piano.WAV DESTINATION ${PROJECT_BUILD_ROOT}/python/)
file(COPY demo/python/feedback-strings.WAV DESTINATION ${PROJECT_BUILD_ROOT}/python/)
file(COPY demo/python/TrefoilSource.py DESTINATION ${PROJECT_BUILD_ROOT}/python/)
file(COPY demo/python/TriangleApplication.py DESTINATION ${PROJECT_BUILD_ROOT}/python/)

ENDIF( WIN32 )
