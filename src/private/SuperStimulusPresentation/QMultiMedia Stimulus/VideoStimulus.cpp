////////////////////////////////////////////////////////////////////////////////
// $Id: VideoStimulus.cpp 4597 2013-12-03 14:04:06Z mellinger $
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
#ifndef VIDEO_STIMULUS_CPP
#define VIDEO_STIMULUS_CPP

#include "PCHIncludes.h"
#pragma hdrstop

#include "VideoStimulus.h"

#include "FileUtils.h"
#include "BCIError.h"
#include "BCIStream.h"
#include "NumericConstants.h"
#include "GraphObject.h"
#include "GraphDisplay.h"

#define USE_QT_VIDEO_PLAYLIST 0
#define USE_QT_VIDEO_PLAYLIST_FILE 1
#define USE_QT_VIDEO_QBUFFER  0
#define USE_QT_VIDEO_QBUFFER_PLAYLIST 0

VideoStimulus::VideoStimulus( GraphDisplay& display )
: GraphObject( display, ImageStimulusZOrder ) // Put a VideoStimulusZOrder in GraphObject.h?
{
	mpMediaPlayer = new QMediaPlayer(0, QMediaPlayer::StreamPlayback);
	mpMediaPlaylist = new QMediaPlaylist;
	if(USE_QT_VIDEO_PLAYLIST)
		mpMediaPlayer->setPlaylist(mpMediaPlaylist);

	mpDisplay = &display;
	
	// QWidget*   GraphDisplay::mpWidget  is private...
	// QGLWidget* GraphDisplay::Context() is const
	// mpVideoWidget = new QVideoWidget(display.mpWidget); // This may be null!
	// mpVideoWidget = new QVideoWidget((QWidget*)display.Context().handle.glContext);
	mpVideoWidget = new QVideoWidget;
	
	// This will be [0,0,0,0] in Initialize
	SetSize(display.Context().rect);

	mpMediaPlayer->setVideoOutput(mpVideoWidget);
}

// I'm not doing any memory management now, as pointer usage is still in flux
VideoStimulus::~VideoStimulus()
{
	delete mpMediaPlayer;
}

VideoStimulus&
VideoStimulus::SetFile( const string& fileName )
{
	QString qFileName( fileName.c_str() ); 
	QUrl qFileUrl( QUrl::fromLocalFile( qFileName ) );
	mFileNameQLocal = qFileUrl.toLocalFile();
	qFileUrl = QUrl::fromLocalFile(mFileNameQLocal);

	// Test for file existence
	mpQFile = new QFile( mFileNameQLocal );
	if( !mpQFile->exists() )
	{
		bcierr << "Could not find video file " << fileName << ".";
		delete mpQFile;
		return *this;
	}
	
	/**//**/
	// Using the playlist
	if ( USE_QT_VIDEO_PLAYLIST )
	{
		mpMediaPlaylist->clear();
		mpMediaPlaylist->addMedia(qFileUrl);
		mpMediaPlaylist->setCurrentIndex(1);
		mpMediaPlaylist->load(qFileUrl);
		mMediaContent = mpMediaPlayer->currentMedia();
		CheckMediaPlaylist( true, fileName );
	}
	/**//**/

	/**//**/
	// Using the playlist with a file
	if( USE_QT_VIDEO_PLAYLIST_FILE )
	{
		mpMediaPlaylist->clear();
		// mpMediaPlaylist->addMedia( mpQFile );
	}
	/**//**/

	/**//*
	// Using a QFile pointer
	mpQFile->open( QIODevice::ReadOnly );
	mpMediaContent = new QMediaContent( mFileNameQLocal );
	mpMediaPlayer->setMedia(*mpMediaContent, mpQFile);
	/**//**/
	
	/**//**/
	// Copy the file data into a QByteArray
	if( !USE_QT_VIDEO_PLAYLIST )
	{
		mpQFile->open( QIODevice::ReadOnly );
		int filesize = mpQFile->size();
		char* fdata = new char[filesize];
		qint64 num_read = mpQFile->read( fdata, filesize );
		if( !num_read )
		{
			bcierr << "No data for " << fileName << " could be read from memory.";
			return *this;
		}
		mByteArray = QByteArray( (const char *)fdata, filesize );
		mpQFile->close();
	}
	/**//**/

	/**//**/
	// Using a buffer
	if( USE_QT_VIDEO_QBUFFER )
	{
		mpBuffer = new QBuffer( &mByteArray, 0 );
		// mpBuffer->setBuffer( &mByteArray );
		mpBuffer->open( QIODevice::ReadOnly );
		mpBuffer->seek(0);
		//	mpBuffer->reset();
		mMediaContent = QMediaContent( qFileUrl );

		if( USE_QT_VIDEO_QBUFFER_PLAYLIST )
		{
			bciwarn << "Hard loading " << fileName << "!";
			mpMediaPlaylist->clear();
			mpMediaPlaylist->addMedia( mMediaContent );
			mpMediaPlaylist->setCurrentIndex( 1 );
			mpMediaPlaylist->load( mpBuffer );
		}
		else 
		{
			mpMediaPlayer->setMedia( mMediaContent, mpBuffer );
		}

		mpBuffer->close();
	}
	/**//**/
	
	/**//*
	// Using a data stream
	mpDataStream = new QDataStream( mByteArray );
	mpMediaContent = new QMediaContent( qFileUrl );
	mpMediaPlayer->setMedia( *mpMediaContent, (QIODevice*)mpDataStream );
	/**//**/

	// Tests for the media content (always return "" / 0 for everything)

	if(USE_QT_VIDEO_QBUFFER) {
		QMediaResource qResource1 = mMediaContent.canonicalResource();
		int    audioBitRate1 = qResource1.audioBitRate();
		string audioCodec1   = qResource1.audioCodec().toLatin1();
		string mimeType1     = qResource1.mimeType().toLatin1();
		string url1          = qResource1.url().toString().toLatin1();

		QMediaResource qResource2 = QMediaResource( qFileUrl );
		int    audioBitRate2 = qResource2.audioBitRate();
		string audioCodec2   = qResource2.audioCodec().toLatin1();
		string mimeType2     = qResource2.mimeType().toLatin1();
		string url2          = qResource2.url().toString().toLatin1();

		QMediaResource qResource3 = QMediaResource( QNetworkRequest( qFileUrl ) );
		int    audioBitRate3 = qResource3.audioBitRate();
		string audioCodec3   = qResource3.audioCodec().toLatin1();
		string mimeType3     = qResource3.mimeType().toLatin1();
		string url3          = qResource3.url().toString().toLatin1();
	}

	/**//**/

	// Test for file validity
	bciout << "Set " << fileName;
	CheckMediaPlayer( true );

	return *this;
}

VideoStimulus&
VideoStimulus::SetVolume( const qreal& volume )
{
  // QMediaPlayer volume is in [0,100] (100 by default)
	mpMediaPlayer->setVolume(volume);
	return *this;
}

VideoStimulus&
VideoStimulus::SetRate( const qreal& rate)
{
	// MediaPlayer rate is in [0,Inf] (1 by default; max is limited by computer speed) 
	mpMediaPlayer->setPlaybackRate(rate);
	return *this;
}

VideoStimulus&
VideoStimulus::SetSize( const int& width, const int& height )
{
	// Store the size in various BCI2000 and Qt forms
	mWidth = float(width);
	mHeight = float(height);
	mSize = QSize(mWidth, mHeight);
	mQRect = QRect(0, 0, mWidth, mHeight);

	// The video widget needs to know to make itself that size
	mpVideoWidget->setMinimumSize(mSize);
	mpVideoWidget->setMaximumSize(mSize);

	return *this;
}
VideoStimulus&
VideoStimulus::SetSize( const GUI::Rect rect)
{
	return SetSize(rect.right - rect.left, rect.bottom - rect.top);
}


void
VideoStimulus::OnPresent()
{
	// Position the video widget to fill the display's widget
	// mpVideoWidget->setParent(mpDisplay->mpWidget);
	CheckMediaPlayer( true );
	mpVideoWidget->move(0, 0); 
	mpVideoWidget->show();

	mpMediaPlayer->play();
}

void
VideoStimulus::OnConceal()
{
	mpVideoWidget->hide();
	mpMediaPlayer->stop();
	mpMediaPlayer->setPosition(0);
}


void
VideoStimulus::OnPaint( const GUI::DrawContext& inDC )
{
	mpPixelMap = new QPixmap(mpVideoWidget->grab());
	if(mpPixelMap != NULL)
	{
		if( mWidth == mpPixelMap->width() && mHeight == mpPixelMap->height() )
			inDC.handle.painter->drawPixmap(
			::Floor( inDC.rect.left ),
			::Floor( inDC.rect.top ),
			*mpPixelMap );
		else
			inDC.handle.painter->drawPixmap(
			::Floor( inDC.rect.left ),
			::Floor( inDC.rect.top ),
			int(mWidth), int(mHeight),
			*mpPixelMap );
	}
}

bool
VideoStimulus::CheckMediaPlaylist( bool verbose, const string& fileName )
{
	switch( mpMediaPlaylist->error() )
	{
		// Only NoError should be allowed
		case QMediaPlaylist::Error::NoError: return true;
		// Many systems will report a 'format error' despite there being none
		case QMediaPlaylist::Error::FormatError: return true;
			// verbose && bcierr << "The video file '" + fileName + "' has a format error.";
		// break;
		// Any other error is bad
		case QMediaPlaylist::Error::FormatNotSupportedError:
			verbose && bcierr << "The video file '" + fileName + "' format is not supported on this machine.";
		break;
		case QMediaPlaylist::Error::NetworkError:
			verbose && bcierr << "Network error; cannot load video file '" + fileName + "'.";
		break;
		case QMediaPlaylist::Error::AccessDeniedError:
			verbose && bcierr << "Access to video file '" + fileName + "'was denied.";
		break;
	}
	QString qerror = mpMediaPlaylist->errorString();
	string error = string(qerror.toLatin1());
	bciout << "The error was '" << error << "'.";
	return false;
}

bool
VideoStimulus::CheckMediaPlayer( bool verbose = false )
{
  return CheckMediaPlayerAvailability( verbose )
      && CheckMediaPlayerError( verbose )
      && CheckMediaPlayerStatus( verbose )
	    && CheckMediaPlayerBufferStatus( verbose );
}

bool
VideoStimulus::CheckMediaPlayerAvailability( bool verbose = false )
{
  switch( mpMediaPlayer->availability() )
  {
    // Being available is the only way to return true
    case QMultimedia::AvailabilityStatus::Available: return true;
    // Any other status is bad
    case QMultimedia::AvailabilityStatus::ServiceMissing: verbose && bcierr << "There is no service available to provide the requested video functionality."; break;
    case QMultimedia::AvailabilityStatus::ResourceError: verbose && bcierr << "The service could not allocate resource required to play video correctly."; break;
    case QMultimedia::AvailabilityStatus::Busy: verbose && bcierr << "The service must wait for access to necessary video resources."; break;
  }
	return false;
}

bool
VideoStimulus::CheckMediaPlayerError( bool verbose = false )
{
	switch( mpMediaPlayer->error() )
  {
			// Not having an error is the only way to return true
			case QMediaPlayer::Error::NoError: return true;
		  // Any other error, including unknown, is bad
			case QMediaPlayer::Error::ResourceError	      : verbose && bcierr << "A media resource couldn't be resolved." << " buffer: " << mpMediaPlayer->bufferStatus(); break;
      case QMediaPlayer::Error::FormatError	        : verbose && bcierr << "The format of a media resource isn't (fully) supported. Playback may still be possible, but without an audio or video component."; break;
      case QMediaPlayer::Error::NetworkError	      : verbose && bcierr << "A network error occurred."; break;
      case QMediaPlayer::Error::AccessDeniedError   : verbose && bcierr << "There are not the appropriate permissions to play a media resource."; break;
      case QMediaPlayer::Error::ServiceMissingError : verbose && bcierr << "A valid playback service was not found, playback cannot proceed."; break;
			default                                       : verbose && bcierr << "An unknown media player error occured."; break;
  }
	return false;
}

bool
VideoStimulus::CheckMediaPlayerStatus( bool verbose = false )
{
	switch( mpMediaPlayer->mediaStatus() ) {
			// Acceptable states (loaded, buffering, buffered, finished)
      case QMediaPlayer::LoadedMedia: 
        verbose && bciout << "The current media has been loaded. The player is in the�StoppedState.";
				return true;
      case QMediaPlayer::BufferingMedia:
        verbose && bciout << "The player is buffering data but has enough data buffered for playback to continue for the immediate future. The player is in the�PlayingState�or PausedState.";
				return true;
       case QMediaPlayer::BufferedMedia:
        verbose && bciout << "The player has fully buffered the current media. The player is in the PlayingState or PausedState.";
        return true;
      case QMediaPlayer::EndOfMedia:
        verbose && bciout << "Playback has reached the end of the current media. The player is in the StoppedState.";
				return true;
			// Unacceptable states (unknown or no media, loading, stalled, invalid)
      case QMediaPlayer::UnknownMediaStatus: 
        verbose && bciwarn << "The status of the media cannot be determined. Does the file exist?";
				return false;
      case QMediaPlayer::NoMedia: 
        verbose && bciwarn << "The is no current media. The player is in the�StoppedState.";
				return false;
      case QMediaPlayer::LoadingMedia: 
        verbose && bciwarn << "The current media is being loaded. The player may be in any state.";
				return false;
      case QMediaPlayer::StalledMedia:
        verbose && bciwarn << "Playback of the current media has stalled due to insufficient buffering or some other temporary interruption. The player is in the�PlayingState�or PausedState.";
				return false;
      case QMediaPlayer::InvalidMedia:
        verbose && bciwarn << "The current media (is invalid and) cannot be played. The player is in the�StoppedState.";
				return false;
    }
	return false;
}

bool
VideoStimulus::CheckMediaPlayerBufferStatus( bool verbose = false )
{
	// Complain if the file isn't done buffering
	if( int(mpMediaPlayer->bufferStatus()) < 100 )
	{
		verbose && bcierr << "The file is not done buffering (" << mpMediaPlayer->bufferStatus() << "/100)";
		return false;
	}
	return true;
}

#endif // VIDEO_STIMULUS_CPP