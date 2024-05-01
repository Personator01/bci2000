#! ../prog/BCI2000Shell
@cls & ..\prog\BCI2000Shell %0 %* #! && exit /b 0 || exit /b 1\n


execute script FindPortablePython3.bat  # this is necessary so that BCI2000 can find Python


change directory $BCI2000LAUNCHDIR
show window; set title ${Extract file base $0}
reset system
startup system localhost

#start executable PythonSource          --local --PythonSrcClassFile= --PythonSrcShell=0 --PythonSrcLog=Srclogger.txt
Start executable DSISerial --local --DSISerialPort=COM9
#start executable PythonSignalProcessing --local --PythonSigClassFile= --PythonSigShell=0 --PythonSigLog=Siglogger.txt
#Start executable SignalGenerator --local 
#Start executable DummySignalProcessing --local
#Start executable FilePlayback --local --PlaybackFileName=BCI_004_Day2_BCITraining_Session1S001R01.dat
#Start executable DummySignalProcessing --local
Start executable HandVibrationFilter --local
#Start executable CursorTask --local
start executable PythonApplication      --local --PythonAppClassFile=VibrotactileCursorTaskWPrime.py --PythonAppShell=0 --PythonAppLog=Applogger.txt
#start DummyApplication --local

wait for connected 600
Load parameterfile "../parms/BCI008_VibrationCursorTask_Ramp.prm"
#Load parameterfile "../parms/HandVibration/Frequency_range_4-7.prm"
#Load parameterfile "../parms/NickTesting.prm"
#load parameterfile "../parms/PythonDemo1_Triangle.prm"

