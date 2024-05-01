////////////////////////////////////////////////////////////////////////////////
// $Id: Brick.h 7464 2023-06-30 15:04:08Z mellinger $
// Authors: jezhill@gmail.com
// Description: Defines a brick object for breakout
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BRICK_H
#define BRICK_H

#include "../RectangleObject.h"
#include "Ball.h"
#include <string>
#include <vector>

class Brick : public RectangleObject
{
public:
	Brick();
	virtual ~Brick();

	virtual void Update();
	virtual int  Collide(Ball & ball);

	virtual void Init( double hitPoints );

	virtual bool Dead() { return mHitPoints <= 0.0; }
	virtual void SaveHitPoints() { mSaved = mHitPoints; }
	virtual void RestoreHitPoints() { mHitPoints = mSaved; }

protected:
	double mHitPoints;
	double mSaved;
	bool mColliding;
	std::vector<RectangleObject *> mRects;
};


#endif // BRICK_H
