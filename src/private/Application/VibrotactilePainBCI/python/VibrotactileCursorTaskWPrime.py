#   $Id: TriangleApplication.py 4144 2012-06-17 04:48:37Z jhill $
#   
#   This file is a BCPy2000 demo file, which illustrates the capabilities
#   of the BCPy2000 framework.
# 
#   Copyright (C) 2007-10  Jeremy Hill
#   
#   bcpy2000@bci2000.org
#   
#   This program is free software: you can redistribute it
#   and/or modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation, either version 3 of
#   the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import numpy
import random

from BCPy2000.GenericApplication import BciGenericApplication
#from BCPy2000 import PygameRenderer
from BCPy2000.AppTools.Boxes import Box
from BCPy2000.AppTools.Displays import fullscreen
from BCPy2000.AppTools.Shapes import PolygonTexture, Disc
from BCPy2000.AppTools.StateMonitors import addstatemonitor, addphasemonitor

import keyboard
import time

# from pygame import mixer; from pygame.mixer import Sound
# from WavTools.FakePyGame import mixer, Sound # would provide a workalike interface to the line above
                                               #
#import WavTools                                # but let's get into the habit of using WavTools.player
                                               # explicitly, since it is more flexible, and its timing
                                               # is now more reliable than that of pygame.mixer.Sound

#################################################################
#################################################################

class BciApplication(BciGenericApplication):
	
	#############################################################
	
	def Description(self):
		return "Serial Subtraction with prime number"
	
	#############################################################
	
	def Construct(self):
		self.define_param(
			"PythonApp:Design   int    ShowFixation=        0     0     0   1  // show a fixation point in the center (boolean)",
			"PythonApp:Screen   int    ScreenId=           -1    -1     %   %  // on which screen should the stimulus window be opened - use -1 for last",
			"PythonApp:Screen   float  WindowSize=         0.8   1.0   0.0 1.0 // size of the stimulus window, proportional to the screen",
			"PythonApp:Stimuli  float  RectangleSize=       1.0   1.0   0.0 10.0",
			"PythonApp:Stimuli   int    CursorMovement=      1     1     %   %  // Enables Cursor movement; use 1 for movement, 0 for stationary",
			"PythonApp:Stimuli   int    NumberDisplayed=      1     1     %   %  // 1 for displaying the number from start to stop, 0 for no number ",
			# "PythonApp:Stimuli  int    CursorMovement       1     1     %   % // Enable CursorMovement",
			# "PythonApp:Stimuli  int    Prime				1     1     %   % // To enable prime numbers use 1, use 0 for arndom numbers",

		)
		self.define_state(
			# StimulusCode
			# "HandVibration			   1 0 0 0",
			# "StimulusCode			   8 0 0 0",
			"TargetCode			   1 0 0 0",
			"ResultCode			   1 0 0 0",
			"BaselineOn   		   1 0 0 0",
			# "StartCueOn  		   1 0 0 0",
			"StopCueOn   		   1 0 0 0",  
			"TargetClass 		   2 0 0 0",  
			"FeedbackOn  		   1 0 0 0",  
			"zScore 			   32 0 0 0",
			"finalZscore		   32 0 0 0",
			"currentState          4 0 0 0",
			"Feedback 			   1 0 0 0",
			"StartCueOn			   1 0 0 0",
			"TargetReached		   1 0 0 0",
			"RandomIntegerArrayIndex    32 0 0 0",
			"TrialNumber 				8 0 0 0",
			"Frequency					4 0 0 0",
			"Amplitude 					16 0 0 0",
			"CursorPosition				16 0 0 0",
			"CursorNegative				1 0 0 0",
		)
	
	#############################################################
	
	def Preflight(self, sigprops):
		
		self.nclasses = 2

		siz = float(self.params['WindowSize'])
		screenid = int(self.params['ScreenId'])  # ScreenId 0 is the first screen, 1 the second, -1 the last
		fullscreen(scale=siz, id=screenid, frameless_window=(siz==1))
		# self.CursorMovementOn = self.params['CursorMovement']
		# only use a borderless window if the window is set to fill the whole screen
		
	#############################################################
	
	def Initialize(self, indim, outdim):
		import psychopy.visual as psypy
		import BCPy2000.PsychoPyRenderer as renderer
		# compute how big everything should be
		self.CursorMovementOn = self.params['CursorMovement']
		print(self.CursorMovementOn)
		scrw,scrh = self.screen.size
		scrsiz = min(scrw,scrh)
		RectSize = scrsiz*.5*float(self.params['RectangleSize'])
		TargetinPixels = (.5*scrh) - (.1*RectSize)
		BottomTarget = -(.5*scrh)
		# print(.1*RectSize)

		rectangle = psypy.ShapeStim(self.screen.screen, lineColor=(0,0,0), fillColor=(-1,0,-1), opacity = 1, vertices=((-1, 1), (1,1), (1,.9), (-1,.9)),size=scrsiz*.5*float(self.params['RectangleSize']), pos=(0,0))
		self.yValue = 0
		self.Target = TargetinPixels - 50
		self.BottomBoundary = BottomTarget + 50
		# let's have a wwhite background
		self.screen.color = (0,0,0)
		self.states['TrialNumber'] = 0
		self.RandomIntegerArray = [7, 11, 13, 17, 19]
		self.states['RandomIntegerArrayIndex'] = 1
		instructions = ('''
			Thank you for agreeing to participate!\n
			For this task you will be trying to move the cursor from the bottom\n
			of the screen to the top of the screen.\n
			If you are able to do this successfully, the hand vibration device will\n
			turn on, and provide you with gentle vibrations.\n
			The investigator will now explain the instructions to you.\n
						   ''')
		self.z = 0
		# OK, now register all those stimuli, plus a few more, with the framework
		self.stimulus('rectangle',z=1,   stim=rectangle)
		self.stimulus('instructions', z=2, stim=psypy.TextBox(self.screen.screen,text=instructions, font_color = (1,1,1), font_size=18, size=[1000,1000],pos=(100,-300),units='pix'),on=False )
		self.stimulus('cursor1',  z=3,   stim=psypy.Circle(self.screen.screen,pos=(0,self.yValue), radius=50, fillColor=(1,1,-1), lineWidth=2, lineColor=(1,1,-1)),on=False)
		self.stimulus('cue',      z=5,   stim=psypy.TextBox(self.screen.screen,text='?', size=[200,100],pos=(60,-20),units='pix', font_color = (1,-1,-1)),on=False)
		self.stimulus('RandomInteger',z=6, stim=psypy.TextBox(self.screen.screen,text='?', size=[300,100],pos=(125, ((.5*scrh)-45)),units='pix', font_color = (-1,1,1)),on=False)
		# set up the strings that are going to be presented in the 'cue' stimulus
		self.feedbackText = ['Start','Stop']
		self.pos = 0 
		# finally, some fancy stuff from AppTools.StateMonitors, for the developer to check
		# that everything's working OK
		if int(self.params['ShowSignalTime']):
			# turn on state monitors iff the packet clock is also turned on
			addstatemonitor(self, 'Running', showtime=True)
			addstatemonitor(self, 'TargetCode')
			addphasemonitor(self, 'phase', showtime=True)
			addphasemonitor(self, 'TrialNumber')
			m = addstatemonitor(self, 'fs_reg')
			m.func = lambda x: '% 6.1fHz' % x._regfs.get('SamplesPerSecond', 0)
			m.pargs = (self,)
			m = addstatemonitor(self, 'fs_avg')
			m.func = lambda x: '% 6.1fHz' % x.estimated.get('SamplesPerSecond',{}).get('global', 0)
			m.pargs = (self,)
			m = addstatemonitor(self, 'fs_run')
			m.func = lambda x: '% 6.1fHz' % x.estimated.get('SamplesPerSecond',{}).get('running', 0)
			m.pargs = (self,)
			m = addstatemonitor(self, 'fr_run')
			m.func = lambda x: '% 6.1fHz' % x.estimated.get('FramesPerSecond',{}).get('running', 0)
			m.pargs = (self,)
		self.finalZscore = 0

	#############################################################
	
	def StartRun(self):
		self.stimuli['instructions'].on = False
		self.stimuli['cursor1'].on = True
		self.stimuli['rectangle'].on = True
		self.yValue = 0
		self.stimuli['cursor1'].pos = [0,self.yValue]
		self.finalZscore =0
		self.stimuli['cue'].on =True
		if int(self.params['NumberDisplayed']):
			self.stimuli['RandomInteger'].on = True

		
	#############################################################
	
	def Phases(self):
		
		self.phase(next='startcue',        duration=       10000,   name='baseline')
		self.phase(next='taskPerformance', duration=        3000,   name='startcue')
		self.phase(next='stopcue',         duration= 	   20000,   name='taskPerformance')
		self.phase(next='buffer',        duration=        5000,   name='stopcue')
		self.phase(next='baseline',        duration=        7000,   name='buffer')
		self.design(start='baseline', new_trial='StartRun`', interblock='idle')
	#############################################################
	
	def Transition(self, phase):
		
		# record what's going 
		# record what's going 
		self.states['BaselineOn'] = int(phase in ['baseline'])
		self.states['StartCueOn'] = int(phase in ['startcue'])
		self.states['StopCueOn']  = int(phase in ['stopcue'])
		# self.states['StartCueOn'] = int(phase in ['startcue'])
		self.states['FeedbackOn'] = int(phase in ['taskPerformance'])
		if phase == 'baseline':
			self.states['currentState'] = 0
			self.states['TargetCode'] = 0 
			self.states['TargetReached'] = 0
			# self.states['ResultCode'] = 0
			# if(self.states['RandomIntegerArrayIndex'] > 98):
			# 	self.states['RandomIntegerArrayIndex'] = 0
			# self.states['HandVibration'] = 0
			# self.states['StimulusCode'] = 0
			self.states['Frequency'] = 0
			self.states['RandomIntegerArrayIndex'] = numpy.random.randint(0,4)
			self.states['TrialNumber'] = self.states['TrialNumber'] +1
			

		if phase == 'startcue':
			# print(self.RandomIntegerArray[self.states['RandomIntegerArrayIndex']])
			# self.states['HandVibration'] = 0
			# self.states['StimulusCode'] = 1
			# self.states['HandVibration'] = 1
			self.stimuli['cue'].AddDispatch(lambda: self.stimuli['cue'].setText(self.feedbackText[0]))
			if int(self.params['NumberDisplayed']):
				self.stimuli['RandomInteger'].AddDispatch(lambda: self.stimuli['RandomInteger'].setText(str(self.RandomIntegerArray[self.states['RandomIntegerArrayIndex']])))
			self.states['currentState'] = 1
			self.states['TargetCode'] = 1
			self.states['Feedback'] = 1	
			self.states['Frequency'] = 0
			
		if phase == 'taskPerformance':
			# self.states['HandVibration'] = 0
			# self.states['StimulusCode'] = 2
			# self.states['HandVibration'] = 1
			self.states['currentState'] = 2
			self.states['TargetCode'] = 1
			self.states['Frequency'] = numpy.random.randint(1,4)
			# self.states['Feedback'] = 1
		if phase == 'stopcue':
			if not int(self.states['TargetReached']):
				self.states['Amplitude'] = 0 
			self.stimuli['cue'].AddDispatch(lambda: self.stimuli['cue'].setText(self.feedbackText[1]))
			# self.states['HandVibration'] = 0
			# self.states['StimulusCode'] = 3
			# self.states['HandVibration'] = 1
			self.states['currentState'] = 3
			self.states['TargetCode'] = 0
			# if int(self.CursorMovementOn) == 0:
			# 	self.states['Amplitude'] =14
			# self.states['Frequency'] = 0
			# self.states['Feedback'] = 1
		if phase == 'buffer':
			self.states['Amplitude'] = 0
			# self.states['HandVibration'] = 0
			# self.states['StimulusCode'] = 4
			# self.states['HandVibration'] = 1
			# self.states['ResultCode'] = 4
			self.states['Frequency'] = 0
			self.states['TargetReached'] = 0
			self.states['currentState'] = 4
			self.states['Feedback'] = 0
		# self.stimuli['rectangle'].on = (phase in ['startcue', 'taskPerformance'])
		# self.stimuli['cursor1'].on = (phase in ['startcue','taskPerformance'])
		if int(self.params['NumberDisplayed']):
			self.stimuli['RandomInteger'].on = (phase in ['startcue','taskPerformance'])
		self.stimuli['cue'].on = (phase in ['startcue','stopcue'])
	#############################################################
	
	def Process(self, sig):
		x =(sig.mean(axis=1))
		self.z = self.z + 1
		print(self.z)
		# print("\n")
		# x = numpy.maximum(x, 0.0)
		# self.states['HandVibration'] = 1
		q = numpy.asarray(x)
		self.zScore = float(q[1])
		if int(self.CursorMovementOn):
			if int(self.states['StartCueOn']):
				self.pos=self.yValue
				self.stimuli['cursor1'].pos = [0,self.pos]
			if int(self.states['FeedbackOn']):
				if self.stimuli['cursor1'].pos[1] > self.Target:
					self.states['TargetReached'] = 1
					self.change_phase()
					self.states['Amplitude'] =30
					self.states['ResultCode'] = 1
					self.finalZscore = 0
					self.pos=self.yValue
				if self.stimuli['cursor1'].pos[1] < self.BottomBoundary:
					self.zScore = numpy.maximum(self.zScore, 0.0)
				self.finalZscore += self.zScore
				self.pos = self.yValue + self.finalZscore
				self.stimuli['cursor1'].pos = [0,self.pos]
			
		if int(self.states['FeedbackOn']):
			if self.states['TargetReached'] != 1:
				if self.zScore <= 0: self.states['Amplitude'] = 0
				elif 0 < self.zScore <= .4615: self.states['Amplitude'] = 17
				# bucket 2
				elif .4616 <= self.zScore <= .9231: self.states['Amplitude'] = 18
				# bucket 3
				elif 0.9232 <= self.zScore <= 1.3846: self.states['Amplitude'] = 19
				# bucket 4
				elif 1.3847 <= self.zScore <= 1.8462: self.states['Amplitude'] = 20
				# bucket 5
				elif 1.8463 <= self.zScore <= 2.3077: self.states['Amplitude'] = 21
				# bucket 6
				if 2.3078 <= self.zScore <= 2.7692: self.states['Amplitude'] = 22
				# bucket 7
				elif 2.7693 <= self.zScore <= 3.2308: self.states['Amplitude'] = 23
				# bucket 8
				elif 3.2309 <= self.zScore <= 3.6923: self.states['Amplitude'] = 24
				# bucket 9
				elif 3.6924 <= self.zScore <= 4.1538: self.states['Amplitude'] = 25
				# bucket 10
				elif 4.1539 <= self.zScore <= 4.6154: self.states['Amplitude'] = 26
				# bucket 11
				elif 4.6155 <= self.zScore <= 5.0769: self.states['Amplitude'] = 27
				# bucket 1
				elif 5.0770 <= self.zScore <= 5.5385: self.states['Amplitude'] = 28
				# if 0 < self.zScore <= 1: self.states['Amplitude'] = 1
				# # bucket 2
				elif 5.5386 <= self.zScore <= 6: self.states['Amplitude'] = 29

				elif 6 < self.zScore: self.states['Amplitude'] = 30
				# if 1 < self.zScore <= 2: self.states['Amplitude'] = 3
				# # bucket 3
				# if 2 < self.zScore <= 3: self.states['Amplitude'] = 5
				# # bucket 4
				# if 3 < self.zScore <= 4: self.states['Amplitude'] = 7
				# # bucket 5
				# if 4 < self.zScore <= 5: self.states['Amplitude'] = 9
				# # bucket 6
				# if 5 < self.zScore <= 6: self.states['Amplitude'] = 11
				# bucket 7
				# if 2.7693 <= self.zScore <= 3.2308: self.states['Amplitude'] = 7
				# # bucket 8
				# if 3.2309 <= self.zScore <= 3.6923: self.states['Amplitude'] = 8
				# # bucket 9
				# if 3.6924 <= self.zScore <= 4.1538: self.states['Amplitude'] = 9
				# # bucket 10
				# if 4.1539 <= self.zScore <= 4.6154: self.states['Amplitude'] = 10
				# # bucket 11
				# if 4.6155 <= self.zScore <= 5.0769: self.states['Amplitude'] = 11
				# # bucket 12
				# if 5.0769 <= self.zScore <= 5.5385: self.states['Amplitude'] = 12
				# bucket 13
				
		if int(self.states['StopCueOn']):
			self.pos=self.yValue
			self.finalZscore = 0
			self.stimuli['cursor1'].pos = [0,self.pos]
		# print(self.zScore)
		# bucket 1
			# if (self.z%20==0):
		self.states['CursorPosition'] = abs(self.pos)
		if self.pos < 0: self.states['CursorNegative'] = 1
		if self.pos > 0: self.states['CursorNegative'] = 0
		# else: self.states['Amplitude'] =14
		# if int(self.states['StopCueOn']) and int(self.states['TargetReached']):
		# 	self.states['Amplitude'] = 30
		# if int(self.states['StopCueOn']) and (self.states['TargetReached'] == 0):
		# 	self.states['Amplitude'] = 0


		
	#############################################################
	
	def StopRun(self):
		import BCPy2000.PsychoPyRenderer as renderer
		self.states['FeedbackOn'] = 0
		self.FeedbackOn = 0
		# self.stimuli['cue'].on = False
		# self.states['Frequency'] = 0
		self.Frequency = 0
		# self.states['Amplitude'] = 0
		self.Amplitude = 0
		self.stimuli['rectangle'].on = False
		self.stimuli['cursor1'].on = False
		# self.stimuli['RandomInteger'].on = False
		self.stimuli['cue'].on =False
		self.stimuli['RandomInteger'].on = False

		
#################################################################
#################################################################
