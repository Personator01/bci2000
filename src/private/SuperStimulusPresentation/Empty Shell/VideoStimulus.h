////////////////////////////////////////////////////////////////////////////////
// $Id: VideoStimulus.h 4597 2013-12-03 14:04:06Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de, josh.goldberg.819@gmail.com
// Description: A stimulus consisting of a video.
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
#ifndef VIDEO_STIMULUS_H
#define VIDEO_STIMULUS_H

#include "BCIStream.h"
#include <string>

#include "GraphObject.h"
#include "VisualStimulus.h"

using namespace std;
using namespace GUI;

class VideoStimulus : public VisualStimulus, public GUI::GraphObject, public QWidget
{
 public:
  VideoStimulus( GUI::GraphDisplay& display );
  virtual ~VideoStimulus();
	
	// Property sets
	VideoStimulus& SetFile  ( const string& fileName );
	VideoStimulus& SetVolume( const qreal& volume    );
	VideoStimulus& SetRate  ( const qreal& rate      );
	VideoStimulus& SetSize  ( const GUI::Rect rect   );
	VideoStimulus& SetSize  ( const int& width,
														const int& height      );

 protected:
  // Stimulus(Presentation) event handlers
  virtual void OnPresent(                         );
  virtual void OnConceal(                         );
	// The following are never actually called
	virtual void OnChange (       GUI::DrawContext& ) { bciout << "Changing!"; }
	virtual void OnMove   (       GUI::DrawContext& ) { bciout << "Moving!"; }
	virtual void OnResize (       GUI::DrawContext& ) { bciout << "Resizing!";}
	virtual void OnRun    (                         ) { bciout << "Running!"; }
	// OnPaint has been declared for convenience, but is not used
	virtual void OnPaint  ( const GUI::DrawContext& );

 // Qt-style Handlers
 public slots:
  void mediaStateChanged(QMediaPlayer::State state) { bciout << "State " << state; }
	void positionChanged(qint64 position) { bciout << "Position " << position; }
	void durationChanged(qint64 duration) { bciout << "Duration " << duration; }
	void handleError() { bciwarn << "Error! " << mFileName; }
  
 public:
  // Error checking
	bool CheckMediaPlaylist          ( bool verbose, const string& fileName );
	bool CheckMediaPlayer            ( bool verbose );
	bool CheckMediaPlayerAvailability( bool verbose );
	bool CheckMediaPlayerError       ( bool verbose );
	bool CheckMediaPlayerStatus      ( bool verbose );
	bool CheckMediaPlayerBufferStatus( bool verbose );

 private:
	GraphDisplay* mpDisplay;

	// File information
	string mFileName;

};

#endif // VIDEO_STIMULUS_H