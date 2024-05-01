# importing various libraries
import pyqtgraph as pg
import pyqtgraph.exporters
import numpy as np
import os
import sys
from PyQt5 import QtWidgets, QtGui

#-----VISUALIZATION ABSTRACTION------#
class AbstractWindow(QtWidgets.QMainWindow):
    def __init__(self, size, bciThread, acqThr):
        super().__init__()
        self.size = size
        self.bciThread = bciThread
        self.acqThr = acqThr
        self.saveImages = False
        self.setStyles()
        iconPath = '../../doc/htmlhelp/rsrc/bci2000logo.svg'
        if os.path.isfile(iconPath):
            self.setWindowIcon(QtGui.QIcon(iconPath))
    
    def initialize(self):
        pass
    def setConfig(self):
        self.numRows = int(np.floor(np.sqrt(self.acqThr.CHANNELS)))
        self.numColumns = int(np.ceil(self.acqThr.CHANNELS / self.numRows))
        pass
    def plot(self):
        pass
    def giveUserInfo(self, text):
        pass
    def saveFigures(self):
        pass

    def closeEvent(self, event): #overrides QMainWindow closeEvent
        if self.saveImages:
            self.saveFigures()
        event.accept()
        self.quitAll()

    def quitAll(self):
        try:
            #stop everything
            self.timer.stop()
            self.acqThr.go = False
            self.acqThr.conn.close()
            self.acqThr.join()
            self.bciThread.join()
        except:
            pass
        finally:
            self.close()
            sys.exit()

    def setStyles(self):
        self.buttonStyle = """QPushButton {
                                color: white;
                                background-color: black; 
                                border-style: outset; 
                                border-width: 2px; 
                                border-radius: 10px; 
                                border-color: white; 
                                font: bold 18px; 
                                min-width: 5em; 
                                min-height: 1em;
                                padding: 10px;
                                margin: 20px;
                                }
                                QPushButton:hover {
                                    background-color: grey;
                                        margin: 5px;
                            }"""
        self.titleDockStyle = """
                            Dock > QWidget {
                                background-color: black;
                            }
                            """
        self.labelStyle = """
                            QLabel {
                                font: bold 24px; 
                                color: white;
                            }"""
        


#-----SAVING FIGURE------#
def saveFigure(path, graphicsLayoutObj, suffix, ext='.png', antialias=True):
    newPath = _getPath(path)
    print("Saving image at " + newPath + suffix + ext)
    if ext == '.svg':
        exporter = pg.exporters.SVGExporter(graphicsLayoutObj.ci)
    else:
        exporter = pg.exporters.ImageExporter(graphicsLayoutObj.ci)
        exporter.parameters()['antialias'] = antialias
    #double size to make image quality better
    #exporter.parameters()['width'] = exporter.parameters()['width'] * 2
    #exporter.parameters()['height'] = exporter.parameters()['height'] * 2
    exporter.export(_nonExistantFileName(newPath + suffix, ext) + ext)

########################
#### HELPER METHODS ####
########################
def _getPath(path):
    base = path
    #remove file type
    for i in range(len(path)-1, 0, -1):
        if base[i] == '.':
            base = base[:i]
            break
    #print(base)
    newPath = _nonExistantFileName(base, '.dat')
    #print(newPath)
    return newPath

#return file path with name, without an extension
def _nonExistantFileName(path, ext):
    #print(path)
    if path[-2:] == '00':
        path = path[:-1] + '1' #BCI2000 run numbers are never 00
    oldPath = path
    while os.path.isfile(path + ext):
        #print(path + ext + " is a file")
        #increment run number until we are not overwriting
        oldPath = path
        path = _changeName(path, -1)
    if ext == '.dat':
        path = oldPath #bc current dat file exists, but we want that number
    return path

def _changeName(path, index):
    #tries to match BCI2000 dat file name
    def getSuffix(path, index):
        if index+1 == 0:
            suf = ''
        else:
            suf = path[index+1:]
        return suf
    def replaceStr(path, index, v):
        newPath = path[:index] + str(v) + getSuffix(path, index)
        return newPath
    if path[index].isnumeric():
        if path[index] == '9':
            path = replaceStr(path, index, 0)
            path = _changeName(path, index - 1)
        else:
            path = replaceStr(path, index, int(path[index]) + 1)
    else:
        if index == -1:
            pre = path[::]
        else:
            pre = path[:index+1]
        path = pre + '1' + getSuffix(path, index) #add new digit
        #print(path)
    return path