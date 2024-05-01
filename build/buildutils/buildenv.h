#ifndef BUILDENV_H
#define BUILDENV_H

#define CMAKE_C_COMPILER "/usr/bin/clang"
#define CMAKE_CXX_COMPILER "/usr/bin/clang++"
#define CMAKE_C_FLAGS " -DNDEBUG=1 -include "/home/tytbu/bcistuff/bci2000//src/shared/config/gccprefix.h" -openmp -fPIC -fvisibility=hidden -D_FILE_OFFSET_BITS=64 -Wstrict-aliasing -Wno-endif-labels -Wno-multichar -Wno-enum-compare -Wno-narrowing"
#define CMAKE_CXX_FLAGS " -DNDEBUG=1 -include "/home/tytbu/bcistuff/bci2000//src/shared/config/gccprefix.h" -openmp -fPIC -fvisibility=hidden -D_FILE_OFFSET_BITS=64 -Wstrict-aliasing -Wno-endif-labels -Wno-multichar -Wno-enum-compare -Wno-narrowing -std=c++17"
#define CMAKE_C_COMPILER_ID "Clang"
#define CMAKE_CXX_COMPILER_ID "Clang"
#define CMAKE_SOURCE_DIR "/home/tytbu/bcistuff/bci2000/build"
#define CMAKE_BINARY_DIR "/home/tytbu/bcistuff/bci2000/build"

#endif // BUILDENV_H
