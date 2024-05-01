import threading
import socket
import numpy as np
from multiprocessing import shared_memory
#
# Data acquisition thread from BCI2000's shared memory
#
class AcquireDataThread(threading.Thread):
    def __init__(self, bciThread, lock):
        super(AcquireDataThread, self).__init__()
        self.bciThread = bciThread
        #self.host = address[0]
        #self.port = address[1]
        self.go = True
        self.chNames = []
        self.lock = lock
        self.newData = False

        self.CHANNELS = -1
        self.ELEMENTS = -1

    def run(self):
        while self.go:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.settimeout(1) #timeout = 5 seconds
                print("Attempting to connect to BCI2000")
                s.bind((self.bciThread.address[0], self.bciThread.address[1]))
                s.listen(1)
                try:
                    self.conn, addr = s.accept()
                    with self.conn:
                        #self.conn.settimeout(5) #timeout = 5 seconds
                        print('Connected by', addr)
                        setProps = False #flag if we have gotten to signal properties or not
                        gotMemoryName = False
                        #init property variables
                        propsBuffer = [] #add stream to this buffer, items separated by space
                        SPACE = 32 #ascii code for space
                        EMPTY = b''
                        lastEl = SPACE
                        chEndIndex = 1
                        self.newData = False
                        
                        #continually run to get data
                        try:
                            while self.go:
                                while setProps is False: #get properties
                                    #get stream
                                    stream = self.conn.recv(128)
                                    #if stream is EMPTY:
                                    #    print("breaking")
                                    #    raise Exception('stream is empty, try new connection')
                                    #print(stream)
                                    #split into names
                                    names = stream.split(b' ')
                                    #remove any empty characters
                                    while(EMPTY in names):
                                        names.remove(EMPTY)

                                    #combine name if it is split up
                                    if lastEl != SPACE and stream[0] != SPACE:
                                        #last channel name was split apart
                                        namePart = names.pop(0)
                                        propsBuffer[-1] += namePart
                                    #save in case it was a space
                                    lastEl = stream[-1] 

                                    propsBuffer = np.append(propsBuffer, names)
                                    #print(propsBuffer)
                                    if len(propsBuffer) > chEndIndex+1 and self.CHANNELS == -1:
                                        # we have at least part of ch information
                                        chID = propsBuffer[1] # either start of channel names or number of channels
                                        if chID==b'{': #start of channel names
                                            while chEndIndex < len(propsBuffer):
                                                if propsBuffer[chEndIndex] != b'}': #end of channel names
                                                    chEndIndex += 1
                                                else:
                                                    self.chNames = propsBuffer[2:chEndIndex]
                                                    self.CHANNELS = len(self.chNames) #we have all the channels
                                                    break
                                        else:
                                            self.CHANNELS = int(str(chID, encoding='utf-8'))
                                            self.chNames = range(1,self.CHANNELS+1)
                                            #chEndIndex -= 1
                                    
                                    if self.CHANNELS == -1:
                                        continue 
                                    #-----DONE WITH CHANNELS-----#

                                    #element size is next
                                    elIndex = chEndIndex+1                      
                                    if elIndex+1 < len(propsBuffer): #wait for extra element in case element number is split up
                                        #got element number
                                        el = propsBuffer[elIndex]
                                        elementString = str(el, encoding='utf-8')
                                        self.inputELEMENTS = int(elementString)
                                        self.ELEMENTS = self.inputELEMENTS

                                    if self.CHANNELS == -1 or self.ELEMENTS == -1:
                                        continue
                                    #-----DONE WITH CHANNELS AND ELEMENTS-----#

                                    #self.rawData = [np.linspace(0.01,1, self.ELEMENTS) for i in range(self.CHANNELS)]
                                    print("Properties: Channels: %i, Elements: %i" %(self.CHANNELS, self.ELEMENTS))
                                    self.rawData = np.full((self.CHANNELS,self.ELEMENTS), 1, dtype=np.double)
                                    setProps= True
                                    #if self.lock.locked():
                                    #    print("Releasing data lock")
                                    #    self.lock.release()

                                if not gotMemoryName:
                                    #print(stream)
                                    stream = self.conn.recv(1)
                                    for b in stream:
                                        if b==66: #signal type 2, 6th bit flipped (+64) for shared memory
                                            #next 8 bytes are number of channels
                                            chs = self.conn.recv(2)
                                            if (int.from_bytes(chs, byteorder='little', signed=False) != self.CHANNELS):
                                                self.conn.recv(1) #to make sure we don't get on a cycle
                                                break
                                            #next 8 bytes are number of samples
                                            els = self.conn.recv(2)
                                            if (int.from_bytes(els, byteorder='little', signed=False) != self.inputELEMENTS):
                                                self.conn.recv(1) #to make sure we don't get on a cycle
                                                break
                                            #WE ARE CERTAIN WE HAVE MEMORY NAME
                                            mem = self.conn.recv(128)
                                            streamName = mem.split(b'/')[1] #mem name right after
                                            #print(streamName)
                                            byteName = streamName.split(b'\x00')[0]
                                            mName = str(byteName, encoding='utf-8')
                                            mem = shared_memory.SharedMemory(mName)
                                            gotMemoryName = True
                                            print("Visualizing data...")
                                            break
                                else:
                                    try:
                                        stream = self.conn.recv(128)
                                        #print(stream)
                                        if not stream:
                                            break
                                    except:
                                        #print("Timeout, no data")
                                        continue
                                    else:
                                        #update visualization with new data
                                        #print("DATA", flush=True)
                                        data = np.ndarray((self.CHANNELS,self.inputELEMENTS),dtype=np.double, buffer=mem.buf)
                                        #print(data)
                                        for i in range(0,self.CHANNELS):
                                            self.rawData[i] = data[i,0:self.ELEMENTS]

                                        self.newData = True
                        except:
                            #Print('exception')
                            self.conn.close() #close connection to client
                        finally:
                            #print('disconnected')
                            self.conn.close() #close connection to client
                    self.CHANNELS = -1
                    self.ELEMENTS = -1
                except:
                    continue
        print("Stopping data acquisition")