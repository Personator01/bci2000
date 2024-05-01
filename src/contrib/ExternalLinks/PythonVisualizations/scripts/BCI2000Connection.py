import sys
import os
import importlib
import threading

def BCI2000Instance(progPath):
    #set directory to where the python file was run in case path is relative
    os.chdir(os.path.dirname(sys.argv[0]))
    sys.path.append(progPath) #BCI2000 prog path
    BCI2000Remote = importlib.import_module('BCI2000Remote') #in prog folder
    return BCI2000Remote.BCI2000Remote() #init BCI2000 remote

class BCI2000Thread(threading.Thread):
    def __init__(self, bci, address, initBCI2000, bciLock):
        super(BCI2000Thread, self).__init__() #thread parent constructor
        self.bci = bci
        self.address = address
        self.initBCI2000 = initBCI2000
        self.bciLock = bciLock
    def run(self):
        print("Waiting to connect to BCI2000...")
        self.bci.Connect()
        self.bci.Execute('Wait for Idle|Suspended|Resting|Initialization 2')
        while (self.bci.Result == "false"):
            print(self.bci.GetSystemState())
            print("Waiting for the run to be stopped")
            self.bci.Execute('Wait for Idle|Suspended|Resting|Initialization 5')
        #print(self.bci.GetSystemState())
        if self.bci.GetSystemState() == "Idle":
            self.bci.WindowVisible = True
            self.initBCI2000(self.bci)
        else:
            print("BCI2000 IS ALREADY INITIALIZED, NOT LOADING NEW PARAMETERS")
        setRequiredVariables(self, self.bci, self.address) #required for pipeline
        self.bciLock.release() #bci2000 is initialized

def getDataFileName(bci):
    #gets absolute path from BCI2000 parameters and Operator Path
    d = bci.GetParameter('DataDirectory')
    s = bci.GetParameter('DataFile')
    pNames = s.split("${")
    for i in range(len(pNames)):
        if '}' in pNames[i]:
            #get parameter name
            names = pNames[i].split("}")
            value = bci.GetParameter(names[0])
            names[0] = value
            newP = ''.join(names)
            pNames[i] = newP
    newS = ''.join(pNames)
    absPath = bci.OperatorPath
    relPath = d + '/' + newS
    absPath = absPath.replace('Operator.exe', relPath)
    return absPath

def setSharingAddress(bci, address):
    addressString = address[0] + ":" + str(address[1])
    bci.Execute('Set Parameter SignalSharingDemoClientAddress ' + addressString)

def setRequiredVariables(bciThr, bci, address):
    setSharingAddress(bci, address)
    bciThr.savePath = getDataFileName(bci)
