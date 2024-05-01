###########################################################################
## $Id: CMakeLists.txt 8061 2024-04-23 17:45:48Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}SignalSource/" )
#ADD_SUBDIRECTORY( PrivateSourceModule )
#ADD_SUBDIRECTORY( g.USBamp_BufferedADC )
ADD_SUBDIRECTORY( NeuralynxNetComADC)
ADD_SUBDIRECTORY( NihonKohdenSource )
ADD_SUBDIRECTORY( NatusNeuroworks )
ADD_SUBDIRECTORY( NatusSignalSource )
ADD_SUBDIRECTORY( BioPacADC )
ADD_SUBDIRECTORY( DelsysTrignoAvanti )
ADD_SUBDIRECTORY( Cortec )
