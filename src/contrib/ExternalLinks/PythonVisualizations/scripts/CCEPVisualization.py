# importing various libraries
import numpy as np
from PyQt5 import QtWidgets
from pyqtgraph.Qt import QtCore
import pyqtgraph as pg
from pyqtgraph.dockarea import *
from .SharedVisualization import saveFigure, AbstractWindow

class CCEPWindow(AbstractWindow):
    def giveUserInfo(self, text):
        labArgs = {'color': "#FFFFFF",
                   'size': "18pt",
                   'bold': True}
        self.gridPlots.clear()
        self.gridNums.clear()
        self.gridPlots.addLabel(text, **labArgs)
        self.gridNums.addLabel(text, **labArgs)

    def plot(self):
        try:
            newVal = int(self.bciThread.bci.GetStateVariable("CCEPTriggered").value)
        except:
            newVal = 0
        
        if self.stopped == True and self.saveFigures == True:
            self.saveFigures()
            self.stopped = False
            self.clearFigures()

        if newVal != self.oldVal:
            print("plotting")
            for r in range(self.numRows):
                for c in range(self.numColumns):
                    ch = r*self.numColumns+c
                    if ch < self.acqThr.CHANNELS:
                        self.chPlot[ch].plotData()

            self.oldVal = newVal
    
    def setConfig(self):
        super().setConfig()

        self.gridNums.clear()
        self.gridPlots.clear()

        self.chPlot = list(range(self.acqThr.CHANNELS))
        self.regs = list(range(self.acqThr.CHANNELS))
        self.threshRegs = list(range(self.acqThr.CHANNELS))
        #init variables
        self.oldVal = 0
        self.baselineLength = self.getParameterValue("BaselineEpochLength") #for now, assume ms
        self.trigLatLength = self.getParameterValue("TriggerLatencyLength")
        self.ccepLength = self.getParameterValue("CCEPEpochLength")
        self.sr = self.getParameterValue("SamplingRate")
        self.highThresh = self.getParameterValue("ChannelThreshold")
        self.lowThresh = -self.highThresh
        self.baseSamples = self.msToSamples(self.baselineLength)
        self.trigSamples = self.msToSamples(self.trigLatLength)
        
        self.ccepRatio = (self.baselineLength + self.trigLatLength) / (self.ccepLength + self.baselineLength + self.trigLatLength)
        self.x = np.linspace(-self.baselineLength, self.trigLatLength + self.ccepLength, self.acqThr.ELEMENTS)

        for r in range(self.numRows):
            for c in range(self.numColumns):
                ch = r*self.numColumns+c
                if ch < self.acqThr.CHANNELS:
                    chName = self.acqThr.chNames[ch].decode('UTF-8')
                    sub1 = self.gridNums.addLayout()
                    sub1.addLabel("<b>%s"%(chName), size='20pt', bold=True)
                    sub1.nextRow()
                    lab = sub1.addLabel(ch, size='18pt', color='#FFFFFF')

                    self.chPlot[ch] = CCEPPlot(self, data=self.acqThr.rawData[ch], label=lab, title=chName)
                    if ch != 0:
                        self.chPlot[ch].setLink(self.chPlot[ch-1])
                    self.gridPlots.addItem(self.chPlot[ch])

            self.gridPlots.nextRow()
            self.gridNums.nextRow()
                    
        self.chPlot[0].setLink(self.chPlot[-1]) #connect last one to first

    def initialize(self):
        self.setWindowTitle("BCI2000 CCEPs")
        
        #FIGURE 1
        area = DockArea()
        self.setCentralWidget(area)
        #self.pens = [pg.mkPen(x) for x in np.linspace((0, 0, 0), (256, 256, 256), 256)] #create all the pens we could ever need
        self.pens = [pg.mkPen(x) for x in np.linspace(0, 1, 256)] #create all the pens we could ever need
        self.gridPlots = pg.GraphicsLayoutWidget(title="CCEPS")
        self.gridNums = pg.GraphicsLayoutWidget(title="CCEP Aggregate")

        button = QtWidgets.QPushButton('Clear Figures')
        button.clicked.connect(self.clearFigures)
        saveButton = QtWidgets.QPushButton('Save Figures')
        saveButton.clicked.connect(self.saveFigures)

        titleD = Dock("Configuration", size=(1,1))
        titleLab = QtWidgets.QLabel("CCEP Visualization")
        titleLab.setAlignment(QtCore.Qt.AlignHCenter)
        titleLab.setWordWrap(True)
        titleLab.setMinimumWidth(100)
        titleD.addWidget(titleLab, row=0, colspan=4)
        titleD.addWidget(button, row=1, col=0, colspan=2)
        titleD.addWidget(saveButton, row=1, col=2, colspan=2)
        titleD.hideTitleBar()
        d1 = Dock("CCEPs", widget=self.gridPlots)
        d2 = Dock("Total CCEPs", widget=self.gridNums)
        area.addDock(titleD, 'top')
        area.addDock(d1, 'bottom')
        area.addDock(d2, 'left', d1)
        self.resize(self.size[0], self.size[1])

        #style
        button.setStyleSheet(self.buttonStyle)
        saveButton.setStyleSheet(self.buttonStyle)
        titleD.setStyleSheet(self.titleDockStyle)
        titleLab.setStyleSheet(self.labelStyle)
        
    def msToSamples(self, lengthMs):
        return int(lengthMs * self.sr/1000.0)
    def __init__(self, size, bciThread, acqThr):
        super().__init__(size, bciThread, acqThr)
       
    def getParameterValue(self, pName):
        pString = self.bciThread.bci.GetParameter(pName)
        return float(''.join(c for c in pString if c.isdigit()))  

    def updateParameter(self, newLat):
        if newLat != self.trigLatLength:
            self.trigLatLength = newLat
            self.trigSamples = self.msToSamples(newLat)
            self.bciThread.bci.SetParameter("TriggerLatencyLength", str(round(newLat))+"ms")
    
    def clearFigures(self):
        for r in range(self.numRows):
            for c in range(self.numColumns):
                ch = r*self.numColumns+c
                if ch < self.acqThr.CHANNELS:
                    for child in self.chPlot[ch].listDataItems():
                        self.chPlot[ch].removeItem(child)

                    self.chPlot[ch].totalCCEPs = 0
                    self.chPlot[ch].totalChanged()

    def saveFigures(self):
        saveFigure(self.bciThread.savePath, self.gridPlots, '_CCEPs', '.svg')

class CCEPPlot(pg.PlotItem):
    def __init__(self, parent, data, label, title):
        super().__init__(title=title)
        self.p = parent
        self.rawData = data
        self.totLab = label
        axView = self.getViewBox()
        axView.disableAutoRange()
        axView.setMouseEnabled(x=False, y=True)
        xLim = -self.p.baselineLength
        yLim = self.p.trigLatLength + self.p.ccepLength
        axView.setXRange(xLim, yLim, padding=0)
        axView.setYRange(-1000, 1000)

        self.threshReg = pg.LinearRegionItem(orientation='horizontal', values=(self.p.lowThresh, self.p.highThresh), movable=True, brush=(253, 248, 121, 10),
                                        pen=pg.mkPen(color=(253, 248, 121, 100), width=1, style=QtCore.Qt.DotLine))
        self.latReg = pg.LinearRegionItem(values=(0, self.p.trigLatLength), movable=True, brush=(9, 24, 80, 100), 
                                          pen=pg.mkPen(color=(9, 24, 80), width=1, style=QtCore.Qt.DotLine), bounds=[xLim, yLim])
        self.latReg.setZValue(1000) #make sure its in front of all plots
        #callbacks
        self.latReg.sigRegionChanged.connect(self.regionChanged)
        self.threshReg.sigRegionChanged.connect(self.threshRegionChanged)

        self.addItem(self.latReg)
        self.addItem(self.threshReg)

        self.threshHigh = self.p.highThresh
        self.threshLow = self.p.lowThresh
        self.latHigh = self.p.trigLatLength
        self.latLow = 0

        self.totalCCEPs = 0

    def getActiveData(self, data):
        lowThreshSamples = self.p.msToSamples(self.latLow)
        p1 = data[:self.p.baseSamples+lowThreshSamples]
        p2 = data[self.p.baseSamples+self.p.trigSamples:]
        d = np.concatenate((p1, p2))
        #print(np.shape(d))
        return d
        return data[self.p.baseSamples+self.p.trigSamples:]
        lowThreshSamples = self.p.msToSamples(self.threshLow)
        nonBaseData = data[self.p.baseSamples:]
        minX = np.max([0, lowThreshSamples])
        print(self.p.trigSamples)
        return nonBaseData[minX+self.p.trigSamples:]


    def updateTotalCCEPs(self):
        self.totalCCEPs = 0
        for p in self.listDataItems():
            d = p.getData()[1]
            ccepData = self.getActiveData(d)
            #ccepData = d[self.p.baseSamples+self.p.trigSamples:]
            if np.max(ccepData) > self.threshHigh or np.min(ccepData) < self.threshLow:
                #print("yeah")
                self.totalCCEPs = self.totalCCEPs + 1
        self.totalChanged()

    def totalChanged(self):
        self.totLab.setText(self.totalCCEPs)
        #print("text changed")

    def setLink(self, plt):
        self.friend = plt #each plot gets one friend they affect
        self.getViewBox().setYLink(self.friend.getViewBox())

    def threshRegionChanged(self, reg):
        newReg = reg.getRegion()
        self.threshHigh = newReg[1]
        self.threshLow = newReg[0]
        if self.threshHigh != self.friend.threshHigh or self.threshLow != self.friend.threshLow:
            #print("CHANGED: " + str(self.threshHigh) + ", " + self.titleLabel.text)
            #print("me: (%d, %d)\nfriend: (%d, %d)" %(self.threshHigh, self.threshLow, self.friend.threshHigh, self.friend.threshLow))
            self.friend.threshReg.setRegion(newReg)
        self.updateCCEPcolors()

    def regionChanged(self, reg):
        newReg = reg.getRegion()
        self.latHigh = newReg[1]
        self.latLow = newReg[0]
        if self.latHigh != self.friend.latHigh or self.latLow != self.friend.latLow:
            self.p.updateParameter(self.latHigh) #just use high limit to keep official low limit as 0
            self.friend.latReg.setRegion(reg.getRegion())
            #self.p.updateParameter(self.latHigh - np.max(self.threshLow, 0)) #just use high limit to keep official low limit as 0

        self.updateCCEPcolors()

    def updateCCEPcolors(self):
        if len(self.listDataItems()) == 0:
            return
        lastPlot = self.listDataItems()[-1]
        data = lastPlot.getData()[1]
        ccepData = self.getActiveData(data)
        #ccepData = data[self.p.baseSamples+self.p.trigSamples:]
        if np.max(ccepData) > self.threshHigh or np.min(ccepData) < self.threshLow:
            p = pg.mkPen('y') #ccep!
        else:
            p = pg.mkPen('w')
        if lastPlot.opts['pen'] != p:
            lastPlot.setPen(p)
            self.totalChanged()
        #update total counter (should move this eventually, gets called more than necessary)
        self.updateTotalCCEPs()

    def plotData(self):
        #update colors for old plots
        children = self.listDataItems()
        #should map greyscale to about 0-125, on a logarithmic scale
        expColors = [(255) * (1 - 2**(-x)) for x in np.linspace(0+1/(len(children)+1), 1-1/(len(children)+1), len(children))]
        for child, c in zip(children, expColors):
            child.setPen(self.p.pens[int(c)])
        
        #new data, normalize amplitude with baseline data
        if self.p.baseSamples == 0:
            data = self.rawData
        else:
            avBase = np.mean(self.rawData[:self.p.baseSamples])
            #avBase = np.mean(self.acqThr.rawData[ch]) #just testing this out
            data = np.subtract(self.rawData, avBase)

        #detect ccep
        ccepData = self.getActiveData(data)
        #ccepData = data[self.p.baseSamples+self.p.trigSamples:]
        if np.max(ccepData) > self.threshHigh or np.min(ccepData) < self.threshLow:
            p = pg.mkPen('y') #ccep!
            self.totalCCEPs = self.totalCCEPs + 1
        else:
            p = pg.mkPen('w')
        
        #plot new data
        self.plot(x=self.p.x, y=data, useCache=True, pen=p)
        self.totalChanged() #only update, don't need to recalculate every graph