////////////////////////////////////////////////////////////////////////////////
// $Id: XYGauge.h 7464 2023-06-30 15:04:08Z mellinger $
// Author: griffin.milsap@gmail.com
// Description: This gauge displays two signals, one on the vertical axis and
//   one on the horizontal axis.  A cursor denotes current value of both signals
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef XY_GAUGE_H
#define XY_GAUGE_H

#include <string>
#include <vector>
#include "Gauge.h"
#include "Scale.h"
#include "Shapes.h"
#include "TextField.h"

class XYGauge : public Gauge
{
public:
  XYGauge( GUI::GraphDisplay &disp, int zOrder );
  ~XYGauge();

  // Virtual implementation of Gauge
  virtual void Update( );
  virtual void UpdateStatics();
  virtual void OnPaint( const GUI::DrawContext & );

private:
  TextField* mpCaption;
  VerticalScale* mpVScale;
  HorizontalScale* mpHScale;
  RectangularShape* mpBounds;
  std::vector<EllipticShape*> mCursors;
};

#endif // XY_GAUGE_H
