# importing various libraries
import numpy as np
from PyQt5 import QtWidgets
from pyqtgraph.Qt import QtCore
import pyqtgraph as pg
from pyqtgraph.dockarea import *
from .SharedVisualization import saveFigure, AbstractWindow

class DefaultPlot(pg.PlotItem):
    def __init__(self, data, title):
        super().__init__(title=title)
        self.rawData = data
        self.data = self.plot(data)

    def plotData(self):
        self.data.setData(self.rawData)

class DefaultWindow(AbstractWindow):
    def __init__(self, size, bciThread, acqThr):
        super().__init__(size, bciThread, acqThr)

    def initialize(self):
        area = DockArea()
        self.setCentralWidget(area)
        self.gridPlots = pg.GraphicsLayoutWidget()
        area.addDock(Dock("Demo", widget=self.gridPlots))
        self.resize(self.size[0], self.size[1])

    def setConfig(self):
        super().setConfig()
        self.gridPlots.clear()
        self.chPlot = list(range(self.acqThr.CHANNELS))

        for r in range(self.numRows):
            for c in range(self.numColumns):
                ch = r*self.numColumns+c
                if ch < self.acqThr.CHANNELS:
                    self.chPlot[ch] = DefaultPlot(data=self.acqThr.rawData[ch], 
                                                  title=self.acqThr.chNames[ch].decode('UTF-8'))
                    self.gridPlots.addItem(self.chPlot[ch])
            self.gridPlots.nextRow()

    def plot(self):
        for r in range(self.numRows):
            for c in range(self.numColumns):
                ch = r*self.numColumns+c
                if ch < self.acqThr.CHANNELS:
                    self.chPlot[ch].plotData()

    def giveUserInfo(self, text):
        labArgs = {'color': "#FFFFFF",
                   'size': "18pt",
                   'bold': True}
        self.gridPlots.clear()
        self.gridPlots.addLabel(text, **labArgs)