////////////////////////////////////////////////////////////////////////////////
// $Id: BarGauge.h 7464 2023-06-30 15:04:08Z mellinger $
// Author: griffin.milsap@gmail.com
// Description: Displays one vertical bar for a single signal.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BAR_GAUGE_H
#define BAR_GAUGE_H

#include <string>
#include <vector>
#include "Gauge.h"
#include "Scale.h"
#include "Shapes.h"
#include "TextField.h"

class BarGauge : public Gauge
{
public:
  BarGauge( GUI::GraphDisplay &disp, int zOrder );
  ~BarGauge();

  // Virtual implementation of Gauge
  virtual void Update( float value1, float value2 );
  virtual void UpdateStatics();
  virtual void OnPaint( const GUI::DrawContext & );

private:
  RectangularShape* mpBar;
  RectangularShape* mpBorder;
  VerticalScale* mpScale;
  TextField* mpCaption;
};

#endif // BAR_GAUGE_H
