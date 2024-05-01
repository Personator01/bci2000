////////////////////////////////////////////////////////////////////////////////
// $Id: CircleObject.h 6171 2020-12-31 13:20:16Z mellinger $
// Authors: griffin.milsap@gmail.com
// Description: Defines a renderable circle
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef CIRCLE_OBJECT_H
#define CIRCLE_OBJECT_H

#include "RenderObject.h"

#define CIRCLE_RESOLUTION 16

class CircleObject : public RenderObject
{
public:
	explicit CircleObject();

	// Public Interface
	void SetPosition( float x, float y );
	void SetRadius( float Radius );

	// Gets
	void GetPosition( float &x, float &y ) { x = mX; y = mY; }
	float Radius() { return mRadius; }
	float X() { return mX; }
	float Y() { return mY; }

protected:
	// Virtual Interface
	virtual void Construct();
	virtual void Draw();

private:
	GLfloat mX, mY, mRadius; // Default to 0
};

#endif // CIRCLE_H