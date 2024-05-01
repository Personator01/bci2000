#! /usr/bin/env python3

#import libraries
import socket
from multiprocessing import shared_memory
import matplotlib.pyplot as plt
import numpy as np

def readBciLengthField(conn, fieldSize):
    """read a length field of specified size from a socket"""
    # read fieldSize bytes that make up a little-endian number
    n = 0
    for i in range(fieldSize):
        n += conn.recv(1) << n
    if n == (1 << (fieldSize * 8)) - 1:
        b = conn.recv(1)
        while b[-1] != 0:
            b.append(conn.recv(1))
        n = b.atoi()
    return n

def readBciMessage(conn):
    """read a full BCI2000 message from a socket"""
    descsupp = conn.recv(2) # get descriptor and descriptor supplement
    if descsupp.size() != 2:
        raise RuntimeError('Could not read descriptor fields')
    messageLength = readBciLengthField(conn, 2)
    data = bytes()
    while data.size() < messageLength:
        data.append(conn.recv(messageLength - data.size()))
    return descsupp, data

#user input
HOST, PORT = "localhost", 1879
print("Waiting for BCI2000 on %s at port %i" %(HOST, PORT))

#initialize variables
CHANNELS = -1
ELEMENTS = -1
SPACE = 32 #ascii code for space
EMPTY = b''
lastEl = SPACE
setProps = False
gotMemoryName = False
chNames = []
myBuffer = [] #add stream to this buffer, items separated by space
figure, ax = plt.subplots(figsize=(10, 8))
ax.set_xlim(-2,2)
ax.set_ylim(-2,2)
figure.set_facecolor((0,0,0,1))
ax.set_axis_off()
ax.set_frame_on(0)
figure.canvas.draw()
chEndIndex = 1

#listen for connection on specified port
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen(1)
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        try:
            while True: #go until we manually stop program
                while setProps is False: #get properties
                    #get stream
                    stream = conn.recv(128)
                    #split into names
                    names = stream.split(b' ')
                    #remove any empty characters
                    while(EMPTY in names):
                        names.remove(EMPTY)

                    #combine name if it is split up
                    if lastEl != SPACE and stream[0] != SPACE:
                        #last channel name was split apart
                        namePart = names.pop(0)
                        myBuffer[-1] += namePart
                    #save in case it was a space
                    lastEl = stream[-1]

                    myBuffer = np.append(myBuffer, names)
                    if len(myBuffer) > chEndIndex+1 and CHANNELS == -1:
                        # we have at least part of ch information
                        chID = myBuffer[1] # either start of channel names or number of channels
                        if chID==b'{': #start of channel names
                            while chEndIndex < len(myBuffer):
                                if myBuffer[chEndIndex] != b'}': #end of channel names
                                    chEndIndex += 1
                                else:
                                    chNames = myBuffer[2:chEndIndex]
                                    CHANNELS = len(chNames) #we have all the channels
                                    break
                        else:
                            CHANNELS = int(str(chID, encoding='utf-8'))
                            chNames = range(1,CHANNELS+1)
                            #chEndIndex -= 1

                    if CHANNELS == -1:
                        continue
                    #-----DONE WITH CHANNELS-----#

                    #element size is next
                    elIndex = chEndIndex+1
                    if elIndex+1 < len(myBuffer): #wait for extra element in case element number is split up
                        #got element number
                        el = myBuffer[elIndex]
                        elementString = str(el, encoding='utf-8')
                        ELEMENTS = int(elementString)

                    if CHANNELS == -1 or ELEMENTS == -1:
                        continue
                    #-----DONE WITH CHANNELS AND ELEMENTS-----#


                    #initialize variables once we have channels and elements
                    phi = np.zeros((CHANNELS, ELEMENTS))
                    bla = np.zeros((ELEMENTS,1))
                    lineArr = list(range(CHANNELS))
                    for i in range(0,CHANNELS):
                        lineArr[i], = ax.plot(bla, bla)

                    print("Properties: Channels: %i, Elements: %i" %(CHANNELS, ELEMENTS))
                    setProps= True

                if not gotMemoryName:
                    #print(stream)
                    stream = conn.recv(1)
                    for b in stream:
                        if b==66: #signal type 2, 6th bit flipped (+64) for shared memory
                            #next 8 bytes are number of channels
                            chs = conn.recv(2)
                            if (int.from_bytes(chs, byteorder='little', signed=False) != CHANNELS):
                                conn.recv(1) #to make sure we don't get on a cycle
                                break
                            #next 8 bytes are number of samples
                            els = conn.recv(2)
                            if (int.from_bytes(els, byteorder='little', signed=False) != ELEMENTS):
                                conn.recv(1) #to make sure we don't get on a cycle
                                break
                            #WE ARE CERTAIN WE HAVE MEMORY NAME
                            mem = conn.recv(128)
                            streamName = mem.split(b'/')[1] #mem name right after
                            #print(streamName)
                            byteName = streamName.split(b'\x00')[0]
                            mName = str(byteName, encoding='utf-8')
                            mem = shared_memory.SharedMemory(mName)
                            gotMemoryName = True
                            print("Connected to shared memory")
                            print("Visualizing data...")
                            break

                else:
                    #block until we get data
                    stream = conn.recv(128)
                    #update visualization with new data
                    data = np.ndarray((CHANNELS,ELEMENTS),dtype=np.double, buffer=mem.buf)
                    for ch in range(0,CHANNELS):
                        for el in range(0, ELEMENTS):
                            phi[ch, el] = el*2*np.pi/(ELEMENTS-1)

                        xdata = np.multiply(1+0.003*data[ch,:],np.cos(phi[ch,:]))
                        ydata = np.multiply(1+0.003*data[ch,:],np.sin(phi[ch,:]))
                        #update plots
                        lineArr[ch].set_xdata(xdata)
                        lineArr[ch].set_ydata(ydata)

                    #update figure
                    figure.canvas.draw()
                    plt.pause(0.01) #render update
        except Exception as inst:
             print('Exception caught: ', type(inst))
             print(inst)
        finally:
            print('disconnected')
            conn.close() #close connection to client
