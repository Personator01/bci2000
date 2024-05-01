////////////////////////////////////////////////////////////////////////////////
// Authors: Alex Belsten belsten@neurotechcenter.org
// Description: BB8ControlTask implementation
//				For use with Bluetooth sphero robot
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2023: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////

#include "BB8ControlTask.h"
#include "BCIStream.h"
#include "BluetoothWrapper_BLED112.h"
#include "ConnectorFilters.h"
#include <math.h>

using namespace std;

Filter( BB8ControlTask, 30);

BB8ControlTask::BB8ControlTask() :
	mrDisplay(Window()),
	device       (NULL),
	mDirection      (0),
	mMouseX         (0),
	mMouseY         (0),
	mSpeed          (0),
	mScreenX        (0),
	mScreenY        (0){
}

BB8ControlTask::~BB8ControlTask() {
	if (device !=  NULL) {
		try {
			device -> disconnect();
		}
		catch (Exception ex) {
			bcierr << ex.What() << std::endl;
			delete device;
			device = NULL;
		}
	}
	CloseComHandle();
	Halt();
}

void
BB8ControlTask::Publish() {
	// Define any parameters that the filter needs....

	BEGIN_PARAMETER_DEFINITIONS
	"Application:BB8ControlTask string BB8Name= D9:F6:1C:6C:DC:AB //  BB8 Bluetooth address",
	"Application:BB8ControlTask string ComPort= COM3 //  Bluetooth Device COM Port",
	"Application:BB8ControlTask int ScreenX= 1920 1 % // Screen size x dimension",
	"Application:BB8ControlTask int ScreenY= 1080 1 % // Screen size y dimension",
	"Application:BB8ControlTask int ControlType= 1 1 1 3 // Type of Control: 1 Mouse, 2 Keyboard, 3 Linear Classification (enumeration)",
	"Application:BB8ControlTask string BB8Color= 0x00FF00 0xFFFFFF 0x000000 0xFFFFFF // LED Color (color)",
	"Application:BB8ControlTask int RandomColors= 0 0 0 1 // Set LED to random colors? (boolean)",
	END_PARAMETER_DEFINITIONS

	BEGIN_STATE_DEFINITIONS
	"Feedback   1 0 0 0"
	END_STATE_DEFINITIONS
}

void
BB8ControlTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const {
	Parameter("BB8Name");
	Parameter("ComPort");
	Parameter("SampleBlockSize");
	Parameter("BB8Color");
	Parameter("RandomColors");

	if (Parameter("ScreenX") < 0)
		bcierr << "Screen size not valid" << endl;
	if (Parameter("ScreenY") < 0)
		bcierr << "Screen size not valid" << endl;

	State("Feedback");
	State("KeyDown");
	State("KeyUp");
	State("MousePosX");
	State("MousePosY");
	Output = Input;
}

void
BB8ControlTask::Initialize( const SignalProperties& Input, const SignalProperties& Output ) {
	// The user has pressed "Set Config" and all Preflight checks have passed.
	// The signal properties can no longer be modified, but the const limitation has gone, so
	// the BB8ControlTask instance itself can be modified. Allocate any memory you need, start any
	// threads, store any information you need in private member variables.
	mBlockSize = Parameter("SampleBlockSize");
	mScreenX   = Parameter("ScreenX");
	mScreenY   = Parameter("ScreenY");
	int num    = Parameter("BB8Color");
	mColor     = color( num );

	switch (int(Parameter( "ControlType" ))) {
    	case 1:			//MOUSE CONTROL
			bciout << "MOUSE CONTROL" << endl;
			break;
		case 2:
			bciout << "KEYBOARD CONTROL" << endl;
			break;
		case 3:
			bciout << "LINEAR CLASSIFICATION" << endl;
			break;
		default:
			bcierr << "NOT VALID CONTROL TYPE" << endl;
	}

	if (device != NULL) {
		try {
			device->disconnect();
		}
		catch (Exception ex) {
			bcierr << ex.What() << std::endl;
			delete device;
			device = NULL;
		}
		CloseComHandle();
	}

	std::string comport = Parameter("ComPort");
	if (!SetComHandle(comport.c_str())) {
		bcierr << "Could not connect to COM Port: " << comport << std::endl;
	}
	State("Feedback") = 0;

	device = new SpheroDevice(Parameter("BB8Name"));
	device -> connect();

	if (device -> state() != SpheroState_Connected) {
		bcierr << "Sphero not connected! State is: " << GetStateString() << std::endl;
		CloseComHandle();
		return;
	}
	device -> Initialize();
	device -> abortOrbBasicProgram();
	device -> SilentReceive();
	device -> setBackLEDOutput(100);
	mHeading = NoMovement; //stand still
	mDirection = 0;        // 0 degrees
	device -> setHeading(mDirection);
}

void
BB8ControlTask::StartRun() {
	// The user has just pressed "Start" (or "Resume")
	bciout << "Hello World!" << endl;
	device -> setRGBLedOutput( mColor.R, mColor.G, mColor.B, 1 );
}

void
BB8ControlTask::Process( const GenericSignal& Input, GenericSignal& Output ) {
	//device->SilentReceive();
	device->ResetWatchdog();

	if (Parameter( "RandomColors" )) {
		device -> setRGBLedOutput( std::rand() % 255, std::rand() % 255, std::rand() % 255, 1);
	} else {
		device -> setRGBLedOutput( mColor.R, mColor.G, mColor.B, 1 );
	}

	switch (int(Parameter( "ControlType" ))) {
    	case 1:			//MOUSE CONTROL
			mMouseX =  (signed int)State("MousePosX") - (mScreenX/2) - 32768 ;
			mMouseY = ((signed int)State("MousePosY") - (mScreenY/2) - 32768 ) * (-1) ;
			if (mMouseX == 0) mMouseX = 1; 				//avoid divide by zero
			mDirection = (( atan( (float)mMouseY / (float)mMouseX ) * 180.0 / M_PI ) - 90 ) * (-1);
			if ( mMouseX < 0 ) mDirection += 180;
			mSpeed = 0.175 * (float)pow( pow( mMouseX, 2) + pow( mMouseY, 2), 0.5);

			device -> roll( mSpeed, mDirection, 1);
			// bciout << "X: "   << mMouseX
			// 	 << "\nY: "   << mMouseY
			// 	 << "\nDIR: " << mDirection << endl;
			break;
        case 2:			//KEYBOARD CONTROL
			for (int i = 0; i < mBlockSize; i++) {
				switch ( OptionalState("KeyDown")(i) ) {
					case 'W':
						mHeading = Forward;
						break;
					case 'S':
						mHeading = Backwards;
						break;
					case 'D':
						mHeading = Right;
						break;
					case 'A':
						mHeading = Left;
						break;
				}
				if (OptionalState("KeyUp")(i) != 0) {
					mHeading = NoMovement;
				}
			}
			switch (mHeading) {
				case Forward:
					mDirection = 0;
					device -> roll( 150, mDirection, 1);
					break;
				case Backwards:
					mDirection = 180; // (mDirection + 180) % 360;
					device -> roll( 150, mDirection, 1);
					break;
				case Left:
					mDirection = 270; // (mDirection + 270) % 360;
					device -> roll( 150, mDirection, 1);
					break;
				case Right:
					mDirection = 90; //(mDirection + 90) % 360;
					device -> roll( 150, mDirection, 1);
					break;
				default:
					device -> roll( 0, mDirection, 0);
			}
	        break;
			//keyboard control type switch end
		case 3:
			Output = Input;
			for (int el = 0; el < Input.Elements(); ++el) {
				if ( Input( 0, el ) < Input( 1, el) ) {
					mHeading = Forward;
				} else {
					mHeading = Backwards;
				}
			}
			switch (mHeading) {
				case Forward:
					mDirection = 0;
					device -> roll( 150, mDirection, 1);
					break;
				case Backwards:
					mDirection = 180; // (mDirection + 180) % 360;
					device -> roll( 150, mDirection, 1);
					break;
				case Left:
					mDirection = 270; // (mDirection + 270) % 360;
					device -> roll( 150, mDirection, 1);
					break;
				case Right:
					mDirection = 90; //(mDirection + 90) % 360;
					device -> roll( 150, mDirection, 1);
					break;
				default:
					device -> roll( 0, mDirection, 0);
			}
			break;
		default:
	  	    bcierr << "Unknown control value" << endl;
    } //control type switch end
}

void
BB8ControlTask::StopRun() {
	// The Running state has been set to 0, either because the user has pressed "Suspend",
	// or because the run has reached its natural end.
	bciwarn << "Goodbye World." << endl;
	// You know, you can delete methods if you're not using them.
	// Remove the corresponding declaration from BB8ControlTask.h too, if so.
}

void
BB8ControlTask::Halt() {
	// Stop any threads or other asynchronous activity.
	// Good practice is to write the Halt() method such that it is safe to call it even *before*
	// Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
	// have already been deleted:  set them to NULL after deletion).
}

std::string
BB8ControlTask::GetStateString() {
	switch (device -> state()) {
		case SpheroState_None:                       { return "SpheroRAW not initialized"; }
		case SpheroState_Error_BluetoothError:       { return "Error - Couldn't initialize Bluetooth stack"; }
		case SpheroState_Error_BluetoothUnavailable: { return "Error - No valid Bluetooth adapter found";}
		case SpheroState_Error_NotPaired:            { return "Error - Specified Sphero not Paired"; }
		case SpheroState_Error_ConnectionFailed:     { return "Error - Connecting failed" ; }
		case SpheroState_Disconnected:               { return "Sphero disconnected"; }
		case SpheroState_Connected:                  { return "Sphero connected"; }
	}
	return "";
}
