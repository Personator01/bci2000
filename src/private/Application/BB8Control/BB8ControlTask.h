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


#ifndef INCLUDED_BB8ControlTask_H  // makes sure this header is not included more than once
#define INCLUDED_BB8ControlTask_H

#include "ApplicationBase.h"
#include "SpheroDevice.h"
#include "SockStream.h"
#include "Sockets.h"
#include "Streambuf.h"

struct color {
	color() { color( 0xFFFFFF ); }
	color( int HEX ) :
		Hex (HEX),
		R ( (HEX & 0xff0000) >> 16 ),
		G ( (HEX & 0x00ff00) >>  8 ),
		B ( (HEX & 0x0000ff) >>  0 ) {
	}
	int Hex;
	int R;
	int G;
	int B;
};

class BB8ControlTask : public ApplicationBase {
	enum SpheroDirection {
		NoMovement =  0,
		Forward    =  1,
		Backwards  = -1,
		Left       = -2,
		Right      =  2
	};

public:
	BB8ControlTask();
	~BB8ControlTask();
	void Publish() override;
	void Preflight(  const SignalProperties& Input, SignalProperties& Output ) const override;
	void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
	void StartRun() override;
	void Process( const GenericSignal& Input, GenericSignal& Output ) override;
	void StopRun() override;
	void Halt() override;

private:
	std::string GetStateString();
	void FetchAndPrintResponses();

	ApplicationWindow& mrDisplay;
	SpheroDevice* device;
	color mColor;

	SpheroDirection mHeading;
	signed int mMouseX;
	signed int mMouseY;
	int mDirection;
	int mSpeed;
	int mScreenX;
	int mScreenY;
	int mBlockSize;
};

#endif // INCLUDED_$ucname$_H
