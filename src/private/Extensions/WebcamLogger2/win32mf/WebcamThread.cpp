////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.com
// Description: A thread performing video input from a camera, and saving to
//  to file. Callbacks to a controller are done through a WebcamClient object
//  pointer.
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
#include "WebcamThread.h"

#include "WebcamPipeline.h"
#include "GrabberTransform.h"
#include "SaveToFileTransform.h"
#include "MFInit.h"

#include "Exception.h"
#include "ExceptionCatcher.h"
#include "BCIStream.h"

#include <thread>

struct WebcamThread::Private
{
    int mCameraIndex = -1;
    WebcamController::Parameters mConfig;
    WebcamClient* mpClient;
    std::thread* mpThread = nullptr;

    std::condition_variable mConditionVariable;
    std::mutex mMutex;
    enum { idle, initializing, running, terminating, cleanup, error } mState = idle;
    std::string mErrorString, mTitle;

    com::Ptr<SaveToFileTransform> mpSaveToFile;

    void ThreadFunc();
};

WebcamThread::WebcamThread()
    : p(new Private)
{
}

WebcamThread::~WebcamThread()
{
    delete p;
}

const std::string& WebcamThread::Error() const
{
    return p->mErrorString;
}

std::string WebcamThread::WindowTitle() const
{
    return p->mTitle;
}

bool WebcamThread::StartRecording(const std::string& file)
{
    p->mpSaveToFile->StartRecording(file);
    if (!p->mpSaveToFile->Error().empty()) {
        p->mErrorString = p->mpSaveToFile->Error();
        return false;
    }
    return true;
}

bool WebcamThread::SwitchRecording(const std::string& file)
{
    p->mpSaveToFile->SwitchRecording(file);
    if (!p->mpSaveToFile->Error().empty()) {
        p->mErrorString = p->mpSaveToFile->Error();
        return false;
    }
    return true;
}

bool WebcamThread::StopRecording()
{
    p->mpSaveToFile->StopRecording();
    if (!p->mpSaveToFile->Error().empty()) {
        p->mErrorString = p->mpSaveToFile->Error();
        return false;
    }
    return true;
}

bool WebcamThread::SetDecimation(int d)
{
    p->mpSaveToFile->SetDecimation(d);
    if (!p->mpSaveToFile->Error().empty()) {
        p->mErrorString = p->mpSaveToFile->Error();
        return false;
    }
    return true;
}

bool WebcamThread::SetTimestampMode(int d)
{
    p->mpSaveToFile->SetTimestampMode(d);
    if (!p->mpSaveToFile->Error().empty()) {
        p->mErrorString = p->mpSaveToFile->Error();
        return false;
    }
    return true;
}

void WebcamThread::Stop()
{
    std::unique_lock<std::mutex> lock(p->mMutex);
    p->mState = Private::terminating;
    lock.unlock();
    p->mConditionVariable.notify_one();
    if (p->mpThread && p->mpThread->joinable())
        p->mpThread->join();
    delete p->mpThread;
    p->mpThread = nullptr;
}

bool WebcamThread::Start(int cameraIndex, const WebcamController::Parameters& config, WebcamClient* pClient)
{
    if (p->mpThread)
        Stop();
    p->mCameraIndex = cameraIndex;
    p->mConfig = config;
    p->mpClient = pClient;
    p->mpThread = new std::thread([this](){ p->ThreadFunc(); });
    std::unique_lock<std::mutex> lock(p->mMutex);
    p->mConditionVariable.wait(lock, [this] {
        return p->mState == Private::running || p->mState == Private::error;
    });
    return p->mErrorString.empty();
}

void WebcamThread::Private::ThreadFunc()
{
  MFInit mfInit;

  std::unique_lock<std::mutex> lock(mMutex);
  mState = initializing;
  mErrorString.clear();
  lock.unlock();

  try {
    com::Ptr<WebcamPipeline> pPipeline = new WebcamPipeline;

    VideoMediaType videoType;
    videoType.codec = FCC('raw ');
    videoType.width = mConfig.width;
    videoType.height = mConfig.height;
    videoType.framerateN = mConfig.fps;
    videoType.framerateD = 1;
    pPipeline->SetDesiredVideoMediaType(videoType);

    AudioMediaType audioType;
    audioType.codec = FCC('sowt'); // 16bit pcm little-endian
    audioType.channels = 2;
    audioType.bitspersample = 16;
    audioType.samplerate = 44100;
    pPipeline->SetDesiredAudioMediaType(audioType);

    if (!mConfig.url.empty())
        pPipeline->ActivateCameraFromURL(mCameraIndex, mConfig.url, mConfig.bufferingtime, mConfig.maxbuffertimems);
    else
        pPipeline->ActivateCameraFromWebcamIndex(mCameraIndex);
    if (!mConfig.audiourl.empty())
        pPipeline->ActivateAudioFromURL(mConfig.audiourl, mConfig.audiobufferingtime, mConfig.audiomaxbuffertimems);
    std::string error = pPipeline->Error();
    if (!error.empty())
      throw std_runtime_error << error;

    com::Ptr<SaveToFileTransform> pSaveToFile = new SaveToFileTransform;
    pSaveToFile->SetClient(mpClient);
    pPipeline->SetTransform(static_cast<IMFTransform*>(pSaveToFile));
    error = pPipeline->Error();
    if (!error.empty())
      throw std_runtime_error << error;

    pPipeline->CreateTopology();
    error = pPipeline->Error();
    if (!error.empty())
      throw std_runtime_error << error;

    pSaveToFile->SetVideoProfile(mConfig.videoprofile);

    auto pType = pPipeline->ActualVideoMediaType().Strip();
    pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    pSaveToFile->AddInputAvailableType(SaveToFileTransform::VideoStreamID, pType);

    if (pPipeline->ActualAudioMediaType().mpType)
    {
      auto pType = pPipeline->ActualAudioMediaType().Strip();
      pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
      pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
      pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
      pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);

      pSaveToFile->AddInputAvailableType(SaveToFileTransform::AudioStreamID, pType);
    }

    pPipeline->ResolveTopology();
    error = pPipeline->Error();
    if (!error.empty())
      throw std_runtime_error << error;

    pPipeline->Start();
    error = pPipeline->Error();
    if (!error.empty())
      throw std_runtime_error << error;

    lock.lock();
    mpSaveToFile = pSaveToFile;
    mTitle = "Camera " + std::to_string(mCameraIndex) + ": " + pPipeline->CameraName();
    mState = running;
    lock.unlock();
    mConditionVariable.notify_one();

    mConditionVariable.wait(lock, [this]{return mState == terminating;});

    lock.lock();
    mState = cleanup;
    pSaveToFile = nullptr;
    lock.unlock();

    pPipeline->Stop();
    error = pPipeline->Error();
    if (!error.empty())
      throw std_runtime_error << error;
    pPipeline->Close();
    error = pPipeline->Error();
    if (!error.empty())
      throw std_runtime_error << error;
    pPipeline->Shutdown();
    error = pPipeline->Error();
    if (!error.empty())
      throw std_runtime_error << error;
  }
  catch(const std::exception& e)
  {
    std::unique_lock<std::mutex> lock(mMutex);
    mState = error;
    mErrorString = e.what();
    lock.unlock();
    mConditionVariable.notify_one();
  }
}
