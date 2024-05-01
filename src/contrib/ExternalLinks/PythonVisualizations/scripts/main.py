
from scripts.BCI2000Connection import *

from scripts.PACVisualization import PACWindow
from scripts.CCEPVisualization import CCEPWindow
from scripts.AcquireDataThread import AcquireDataThread
from scripts.DefaultVisualization import DefaultWindow
import pyqtgraph as pg
from enum import Enum
import time
from pyqtgraph.Qt import QtCore
#
# Helper file to combine data thread, BCI2000 connection, and visualization
#
def defaultBCI(bci):
    bci.WindowVisible = False
    bci.Execute('Wait for Connected')
def main(bciConfig = defaultBCI, path="../../prog", address = ("localhost", 1879), winSize = (1600, 1200), timerUpdate=30 ):
    bciInst = BCI2000Instance(path)

    bciLock = threading.Lock()
    bciThread = BCI2000Thread(bciInst, address, bciConfig, bciLock)

    bciLock.acquire()
    bciThread.start()

    bciLock.acquire()
    MainWindow(bciThread, winSize, timerUpdate) #start the rest of the visualization

#define all available python visualization types here
#name must be equivalent to the Filter name in the Signal Processing pipeline
class VisTypes(Enum):
    Error       = 0
    Default     = 1
    PACFilter   = 2
    CCEPFilter  = 3

class MainWindow():
    #find which filter we are visualizing
    def getVisualizationType(self, bciThr):
        visType = VisTypes.Default
        shared = False
        signalShared = "SignalSharingDemoFilter"

        try:
            #this fails when we access it too early for some reason
            p = bciThr.bci.GetMatrixParameter("SignalProcessingFilterChain" )
        except:
            time.sleep(1)
            return self.getVisualizationType(bciThr) #just try again
        for n in p:
            for t in VisTypes:
                if n[0] == t.name:
                    #found visualization filter
                    visType = t
            if n[0] == signalShared:
                shared = True
        if shared and visType != VisTypes.Error:
            print("\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
            print("Initializing %s Visualization" %(visType.name))
            print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
            return visType

        #at this point, something is wrong
        if shared == False:
            print("BCI2000 data is not available. SignalSharingDemoFilter is not in the pipeline")
            print("Please restart after adding SignalSharingDemoFilter")
        
        elif visType == VisTypes.Error:
            print("Cannot visualize data in Python. No available implementation for the SignalProcessing filters used")
            print("List of possible SignalProcessing filters:")
            for t in VisTypes:
                if t != VisTypes.Error:
                    print(t.name)
            print("Please restart after changing your SignalProcessing module")
        
        input("Press Enter to quit")
        return VisTypes.Error

    def updatePlots(self):
        try:
            bci2000State = self.bciThread.bci.GetSystemState()
            #if any(self.bci2000State == st for st in ["Idle", "Resting", "Suspended"]):

            if bci2000State != "Running":
                self.win.stopped = True
                self.initialized = False
                if bci2000State == "Resting":
                    #save figures before resetting
                    if self.win.saveImages:
                        self.win.saveFigures()
                    self.win.saveImages = False
                    self.win.giveUserInfo("Configuration set. Waiting for the run to be started...")
                return
            else:
                if not self.initialized:
                    #First time in the run after a SetConfig
                    self.win.setConfig()
                    self.initialized = True

        except:
            #self.win.giveUserInfo("Lost connection with BCI2000")
            self.win.quitAll()
        else:
            if self.initialized and self.acqThr.newData:
                self.acqThr.newData = False
                self.win.plot()
                self.win.saveImages = True #only save images if there is data to be shown

    def visualize(self, bciThread, acqThr, visType, winDimensions, timerUpdate):
        pg.mkQApp() #init app
        #change visualization based on our signal processing pipeline
        if visType == VisTypes.PACFilter:
            self.win = PACWindow(winDimensions, bciThread, acqThr)
        elif visType == VisTypes.CCEPFilter:
            self.win = CCEPWindow(winDimensions, bciThread, acqThr)
        else:
            self.win = DefaultWindow(winDimensions, bciThread, acqThr)
        self.win.initialize()
        self.win.giveUserInfo("Waiting for the configuration to be set...")
        self.win.show()

        self.initialized = False
        self.win.timer = QtCore.QTimer()
        self.win.timer.setInterval(timerUpdate)
        self.win.timer.timeout.connect(self.updatePlots)
        self.win.timer.start()

        sys.exit(pg.mkQApp().exec_())

    def __init__(self, bciThread, winSize, timerUpdate):
        visType = self.getVisualizationType(bciThread)
        if visType == VisTypes.Error:
            return

        #start to get data
        dataLock = threading.Lock()
        dataLock.acquire()
        acqThr = AcquireDataThread(bciThread, dataLock)
        acqThr.start()
        self.bciThread = bciThread
        self.acqThr = acqThr

        #wait until signal properties are acquired
        #bciThread.bci.SetConfig()
        #bciThread.bci.Start()
        #dataLock.acquire()
        self.visualize(bciThread, acqThr, visType, winSize, timerUpdate)
