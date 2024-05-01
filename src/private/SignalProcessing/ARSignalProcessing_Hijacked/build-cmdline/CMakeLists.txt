###########################################################################
## $Id: CMakeLists.txt 7465 2023-06-30 15:23:43Z mellinger $
## Authors: jezhill@gmail.com
## Description: 

BCI2000_ADD_CMDLINE_FILTER( AudioEscapeFilter FROM .. )
BCI2000_ADD_CMDLINE_FILTER( HijackFilter FROM ..
                            EXTRA_SOURCES ../Gaussian.cpp
                            EXTRA_HEADERS ../Gaussian.h
                          )
BCI2000_ADD_CMDLINE_FILTER( FilterFilter  FROM ..
                            EXTRA_SOURCES ../CustomIIR.cpp
                            EXTRA_HEADERS ../CustomIIR.h
                            INCLUDING "MATH"
                          )
BCI2000_ADD_CMDLINE_FILTER( LogFilter FROM .. )
BCI2000_ADD_CMDLINE_FILTER( PairwiseCouplingFilter FROM .. )
