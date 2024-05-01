////////////////////////////////////////////////////////////////////////////////
// $Id: HistoryGauge.cpp 7464 2023-06-30 15:04:08Z mellinger $
// Author: griffin.milsap@gmail.com
// Description: The history gauge displays the value of a signal on the vertical
//   axis and previous values of said signal for different values in recent
//   history.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include <sstream>
#include "HistoryGauge.h"
#include "PrecisionTime.h"
#include "GraphDisplay.h"
#include <QPainter>

HistoryGauge::HistoryGauge( GUI::GraphDisplay &disp, int zOrder )
: Gauge( disp, zOrder )
{
  // Construct
  mHistoryLength = 10;
  mpVScale = NULL;
  mpHScale = NULL;
  mpTimeLabel = NULL;
  mpCaption = NULL;
  mpBounds = NULL;
}

HistoryGauge::~HistoryGauge()
{
  // Deconstruct the caption,
  if( mpCaption )
  {
    mpCaption->Hide();
    delete mpCaption;
    mpCaption = NULL;
  }

  // The Time label,
  if( mpTimeLabel )
  {
    mpTimeLabel->Hide();
    delete mpTimeLabel;
    mpTimeLabel = NULL;
  }

  // The vertical scale,
  if( mpVScale )
  {
    mpVScale->Hide();
    delete mpVScale;
    mpVScale = NULL;
  }

  // The horizontal scale,
  if( mpHScale )
  {
    mpHScale->Hide();
    delete mpHScale;
    mpHScale = NULL;
  }

  // The bounds
  if( mpBounds )
  {
    mpBounds->Hide();
    delete mpBounds;
    mpBounds = NULL;
  }
}

void
HistoryGauge::UpdateStatics()
{
  // Setup the vertical scale
  if( !mpVScale )
    mpVScale = new VerticalScale( *mpDisplay );
  GUI::Rect vrect;
  vrect.left = mX;
  vrect.top = mY;
  vrect.right = mX + ( mWidth * 0.1f );
  vrect.bottom = mY + ( mHeight * 0.7f );
  mpVScale->SetDisplayRect( vrect );
  mpVScale->SetMin( GetDimension( 0 ).Min() );
  mpVScale->SetMax( GetDimension( 0 ).Max() );
  mpVScale->SetNumDeliminations( mVScalePrecision );
  if( mShowScale ) mpVScale->Show(); else mpVScale->Hide();
  mpVScale->SetFormat( mVScaleFormat );

  // Setup the Horizontal Scale
  if( !mpHScale )
    mpHScale = new HorizontalScale( *mpDisplay );
  GUI::Rect hrect;
  hrect.left = mX + ( mWidth * 0.1f );
  hrect.top = mY + ( mHeight * 0.7f );
  hrect.right = mX + mWidth;
  hrect.bottom = mY + ( mHeight * 0.8f );
  mpHScale->SetDisplayRect( hrect );
  mpHScale->SetMin( mHistoryLength );
  mpHScale->SetMax( 0 );
  mpHScale->SetNumDeliminations( 0 );
  if( mShowScale ) mpHScale->Show(); else mpHScale->Hide();
  mpHScale->SetFormat( mHScaleFormat );

  // Make the time label
  if( !mpTimeLabel )
    mpTimeLabel = new TextField( *mpDisplay );
  GUI::Rect timeLabelRect = { mX + ( mWidth * 0.2f ), mY + ( mHeight * 0.7f ),
                              mX + ( mWidth * 0.9f ), mY + ( mHeight * 0.8f ) };
  mpTimeLabel->SetAspectRatioMode( GUI::AspectRatioModes::AdjustNone );
  mpTimeLabel->SetDisplayRect( timeLabelRect );
  mpTimeLabel->SetText( "Time" );
  mpTimeLabel->SetTextColor( RGBColor::Black );
  mpTimeLabel->SetTextHeight( mCaptionSize );

  // Make the Caption
  if( !mpCaption )
    mpCaption = new TextField( *mpDisplay );
  GUI::Rect captionRect = { mX + ( 0.1f * mWidth ), mY + ( mHeight * 0.8f ), mX + mWidth, mY + mHeight };
  mpCaption->SetAspectRatioMode( GUI::AspectRatioModes::AdjustNone );
  mpCaption->SetDisplayRect( captionRect );
  mpCaption->SetText( mCaption );
  mpCaption->SetTextColor( RGBColor::Black );
  mpCaption->SetTextHeight( mCaptionSize );

  // Make the graph bounds
  if( !mpBounds )
    mpBounds = new RectangularShape( *mpDisplay );
  GUI::Rect boundsRect = { mX + ( 0.1 * mWidth ), mY,
                           mX + mWidth, mY + ( mHeight * 0.7f ) };
  mpBounds->SetAspectRatioMode( GUI::AspectRatioModes::AdjustNone );
  mpBounds->SetColor( RGBColor::Black );
  mpBounds->SetLineWidth( 2 );
  mpBounds->SetDisplayRect( boundsRect );
}

void
HistoryGauge::OnPaint( const GUI::DrawContext &inDC )
{
  // Setup the history line
  int length = mHistoryLength * 1000;
  for( unsigned int iDim = 0; iDim < mDimensions.size(); iDim++ )
  {
    Dimension& dimension = GetDimension( iDim );
    EntryList& history   = GetHistory( iDim ); 
    QPointF* histLine = new QPointF[ history.size() ];
    int i = 0;
    for( EntryList::iterator itr = history.begin(); itr != history.end(); itr++ )
    {
      float y = mY + ( ( 1 - dimension.Normalize( itr->value ) ) * ( 0.7f * mHeight ) );
      int dt = PrecisionTime::UnsignedDiff( itr->timestamp, history.begin()->timestamp );
      float val = ( float )dt / ( float )length;
      float x = mX + ( ( val ) * ( 0.9 * mWidth ) ) + ( 0.1f * mWidth );
      GUI::Rect normalizedCoords = { x, y, 0, 0 };
      GUI::Rect pixelCoords = mpDisplay->NormalizedToPixelCoords( normalizedCoords );
      histLine[i].setX( pixelCoords.left );
      histLine[i].setY( pixelCoords.top );
      i++;
    }

    inDC.handle.dc->setRenderHint( QPainter::Antialiasing );
    inDC.handle.dc->setPen( QPen( QBrush ( QColor( dimension.Color() ), Qt::SolidPattern ), 1.0, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin) );
    inDC.handle.dc->drawPolyline( histLine, history.size() - 2 );

    // Cleanup
    delete [] histLine;
  }
}

void
HistoryGauge::Update()
{
  int length = mHistoryLength * 1000;
  // Sample Filtering for optimization
  // We only need one sample per pixel - or thereabouts...
  // Get the number of pixels we have to work with.
  GUI::Rect updateRegion;
  updateRegion.top = mY;
  updateRegion.bottom = mY + ( 0.7 * mHeight );
  updateRegion.left = mX + ( 0.1 * mWidth );
  updateRegion.right = mX + ( mWidth );
  GUI::Rect pixelRegion = mpDisplay->NormalizedToPixelCoords( updateRegion );
  int numPixels = pixelRegion.right - pixelRegion.left;
  int stepSize = length / numPixels;

  for( unsigned int iDim = 0; iDim < mDimensions.size(); iDim++ )
  {
    Dimension& dimension = GetDimension( iDim );
    EntryList& history   = GetHistory( iDim );

    if( history.size() && PrecisionTime::UnsignedDiff( mNow, history.begin()->timestamp ) > length )
      history.clear();
  
    // Prepare a new value
    Entry ent;
    ent.value = dimension.RawValue();
    ent.timestamp = mNow;

    int dt = history.size() ? PrecisionTime::UnsignedDiff( ent.timestamp, history.begin()->timestamp ) : (stepSize+1);
    if( dt > stepSize )
    {
      history.push_back( ent );
      Change();
    }
  }
}

bool
HistoryGauge::OnClick( const GUI::Point & )
{
  ClearHistory();
  return true;
}

void
HistoryGauge::ClearHistory()
{
  mHistories.clear();
}
