////////////////////////////////////////////////////////////////////////////////
// $Id: WebcamRecorder.cpp 7882 2024-03-01 20:34:21Z mellinger $
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

#include "fourcc.h"
#include "VideoMediaType.h"
#include "AudioMediaType.h"
#include "TextRenderer.h"
#include "Exception.h"
#include "Debugging.h"
#include "Resources.h"
#include "SaveToFile.h"
#include "AVLog.h"
#include "AVError.h"
#include "Thread.h"
#include "Runnable.h"

#include <mutex>
#include <condition_variable>
#include <regex>
#include <limits>

extern "C" {
#include "libavdevice/avdevice.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace {
    time_t Now()
    {
        return ::time(0);
    }

    std::string TimeToString(time_t inTime)
    {
        struct ::tm* p = ::localtime(&inTime);
        if (!p)
            return "<n/a>";
        char buf[32];
        ::strftime(buf, sizeof(buf), "%d %b %y %T", p);
        return buf;
    }
}

struct WebcamThread::Private
{
    int mCameraIndex = -1;
    WebcamController::Parameters mConfig;
    WebcamClient* mpClient = nullptr;

    MemberCall<void(Private*)> mThreadFuncCall;
    Thread mThread;

    std::condition_variable mConditionVariable;
    std::mutex mMutex;
    enum { idle, initializing, running, terminating, cleanup, error } mState = idle;
    std::string mError, mTitle;

    std::atomic<int> mTimestampMode = 0;
    TextRenderer mTextRenderer;

    SaveToFile mSaveToFile;

    Private();
    void ThreadFunc();
    void AddTimestamp(int width, int height, uint8_t* data);

    struct Device { std::string driver, name, url, friendlyName; };
    static std::vector<Device> ListWebcams();
    static std::vector<VideoMediaType> ListVideoMediaTypes(const Device&);
    static VideoMediaType MatchVideoMediaType(const std::vector<VideoMediaType>&, const VideoMediaType&);

    static std::vector<VideoMediaType> ListVideoMediaTypesDshow(const AVInputFormat*, const std::string& url);
    static std::vector<VideoMediaType> ParseVideoMediaTypesDshow(const std::string& log);
};

WebcamThread::WebcamThread()
    : p(new Private)
{
    AVLog::AddClient();
}

WebcamThread::~WebcamThread()
{
    AVLog::RemoveClient();
    delete p;
}

const std::string& WebcamThread::Error() const
{
    std::lock_guard<std::mutex> lock(p->mMutex);
    return p->mError;
}

std::string WebcamThread::WindowTitle() const
{
    std::lock_guard<std::mutex> lock(p->mMutex);
    return p->mTitle;
}

bool WebcamThread::StartRecording(const std::string& file)
{
    return p->mSaveToFile.StartRecording(file);
}

bool WebcamThread::SwitchRecording(const std::string& file)
{
    return p->mSaveToFile.SwitchRecording(file);
}

bool WebcamThread::StopRecording()
{
    return p->mSaveToFile.StopRecording();
}

bool WebcamThread::SetDecimation(int d)
{
    if (d < 1)
        return false;
    p->mSaveToFile.SetDecimation(d);
    return true;
}

bool WebcamThread::SetTimestampMode(int d)
{
    if (d >= NumTimestampModes)
        return false;
    p->mTimestampMode = d;
    return true;
}

void WebcamThread::Stop()
{
    std::unique_lock<std::mutex> lock(p->mMutex);
    p->mState = Private::terminating;
    lock.unlock();
    p->mConditionVariable.notify_one();
    p->mThread.Terminate();
}

bool WebcamThread::Start(int cameraIndex, const WebcamController::Parameters& config, WebcamClient* pClient)
{
    if (p->mThread.Running())
        Stop();
    p->mCameraIndex = cameraIndex;
    p->mConfig = config;
    p->mpClient = pClient;
    p->mThread.Start();
    std::unique_lock<std::mutex> lock(p->mMutex);
    p->mConditionVariable.wait(lock, [this] {
        return p->mState == Private::running || p->mState == Private::error;
    });
    return p->mError.empty();
}

WebcamThread::Private::Private()
: mThreadFuncCall(&Private::ThreadFunc, this),
  mThread(&mThreadFuncCall, "WebcamThread")
{
}

void WebcamThread::Private::ThreadFunc()
{
  AVDictionary* pOptions = nullptr;
  AVFormatContext* pFormatContext = nullptr;
  AVCodecContext* pCodecContext = nullptr;
  AVFrame* pDecodedFrame = nullptr;
  AVFrame* pRgbFrame = nullptr;
  SwsContext* pSwsContext = nullptr;

  try {
    std::unique_lock<std::mutex> lock(mMutex);
    mState = initializing;
    mError.clear();
    lock.unlock();

    const auto& font = Resources::FreeMonoBold_ttf;
    mTextRenderer.LoadFont(font.data, font.length);
    mTextRenderer.SetTextColor(RGBColor::LtGreen);

    mSaveToFile.SetVideoProfile(mConfig.videoprofile);

#if 0
    AudioMediaType audioType;
    audioType.codec = FCC('sowt'); // 16bit pcm little-endian
    audioType.channels = 2;
    audioType.bitspersample = 16;
    audioType.samplerate = 44100;
#endif

    std::string url, displayName;
    const AVInputFormat* pInputFormat = nullptr;

    if (!mConfig.url.empty()) {
        url = mConfig.url;
        displayName = url;
        ::av_dict_set_int(&pOptions, "timeout", 1000000, 0); // microseconds
        ::av_dict_set_int(&pOptions, "buffer_size", 1024*1024, 0); // bytes
    }
    else {
        auto devices = ListWebcams();
        if (mCameraIndex < 0 || mCameraIndex >= devices.size()) {
            throw std_runtime_error << "No device found with index " + std::to_string(mCameraIndex);
        }
        auto& device = devices[mCameraIndex];
        url = device.url;
        displayName = device.friendlyName;
        auto videoMediaTypes = ListVideoMediaTypes(device);

        VideoMediaType videoType;
        videoType.codec = FCC('raw ');
        videoType.width = mConfig.width;
        videoType.height = mConfig.height;
        videoType.framerateN = mConfig.fps * 10000;
        videoType.framerateD = 10000;

        videoType = MatchVideoMediaType(videoMediaTypes, videoType);

        pInputFormat = ::av_find_input_format(device.driver.c_str());
        std::string video_size = std::to_string(videoType.width) + "x" + std::to_string(videoType.height);
        ::av_dict_set(&pOptions, "video_size", video_size.c_str(), 0);
        float framerate = videoType.framerateN * 1.0f / videoType.framerateD;
        ::av_dict_set_int(&pOptions, "framerate", framerate, 0);
    }

    int err =::avformat_open_input(&pFormatContext, url.c_str(), pInputFormat, &pOptions);
    if (err < 0) {
        throw std_runtime_error << "avformat_open_input(): " << AVError(err).Message();
    }
    ::av_dict_free(&pOptions);

    err = ::avformat_find_stream_info(pFormatContext, nullptr);
    if (err < 0) {
        throw std_runtime_error << "avformat_find_stream_info(): " << AVError(err).Message();
    }
    int videoStream = -1;
    for (int i = 0; i < pFormatContext->nb_streams; ++i) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream < 0) {
        throw std_runtime_error << "Could not identify video stream";
    }

    AVCodecID id = pFormatContext->streams[videoStream]->codecpar->codec_id;
    const AVCodec* pCodec = ::avcodec_find_decoder(id);
    if (!pCodec) {
        throw std_runtime_error << "Codec not found: " << id;
    }

    pCodecContext = ::avcodec_alloc_context3(pCodec);
    if (!pCodecContext) {
        throw std_runtime_error << "Could not allocate codec context";
    }

    err = ::avcodec_parameters_to_context(pCodecContext, pFormatContext->streams[videoStream]->codecpar);
    if (err < 0) {
        throw std_runtime_error << "avcodec_parameters_to_context(): " << AVError(err).Message();
    }
    
    err = ::avcodec_open2(pCodecContext, pCodec, nullptr);
    if (err < 0) {
        throw std_runtime_error << "avcodec_open2(): " << AVError(err).Message();
    }

    lock.lock();
    mTitle = "Camera " + std::to_string(mCameraIndex) + ": " + displayName;
    mState = running;
    lock.unlock();
    mConditionVariable.notify_one();

    AVPacket encodedPacket;
    ::av_init_packet(&encodedPacket);
    pDecodedFrame = ::av_frame_alloc();

    int height = pCodecContext->height, width = pCodecContext->width;
    std::vector<uint8_t> rgbData(height * width * 4, 0);

    pRgbFrame = ::av_frame_alloc();
    AVPixelFormat rgbFormat = AV_PIX_FMT_RGB32;
    int rgbAlign = 32; // Must be power of 2. If this is too small, sws_scale() will write out-of-bounds (?)
    int size = ::av_image_alloc(pRgbFrame->data, pRgbFrame->linesize, width, height, rgbFormat, rgbAlign);
    if (size <= 0) {
        throw std_runtime_error << "av_image_alloc(): " << AVError(size).Message();
    }

    pSwsContext = ::sws_getContext(width, height,
        pCodecContext->pix_fmt, width, height,
        rgbFormat, 0, nullptr, nullptr, nullptr);
    if (!pSwsContext) {
        throw std_runtime_error << "sws_getContext() failed";
    }

    AVRational rate = ::av_guess_frame_rate(pFormatContext, pFormatContext->streams[videoStream], nullptr);
    Rate fps(rate.num, Time::Interval::OneSecond() * rate.den);
    mSaveToFile.SetVideoProfile(mConfig.videoprofile);
    mSaveToFile.SetVideoProperties(width, height, fps);
    mSaveToFile.SetClient(mpClient);

    // about 20 characters in date/time
    mTextRenderer.SetTextSize(width / 24);

    AVLog::Client logclient; // enable logging of AV errors to bcierr

    while (mState != terminating) {

        err = ::av_read_frame(pFormatContext, &encodedPacket);
        if (err < 0) {
            throw std_runtime_error << "av_read_frame(): " << AVError(err).Message();
        }

        // Send packet to decoder
        if (encodedPacket.stream_index == videoStream) {
            err = ::avcodec_send_packet(pCodecContext, &encodedPacket);
            ::av_packet_unref(&encodedPacket);
            if (err < 0) {
                continue; // may fail intermittently
            }
        }
        else {
            ::av_packet_unref(&encodedPacket);
            continue;
        }

        // Receive uncompressed frame from decoder
        // For certain resolutions, the decoded frame is distorted by libavdevice.
        // This can be reproduced with the command line version of ffmpeg (libavdevice 60).
        // This has been observed for resolutions:
        //  176x144, 
        err = ::avcodec_receive_frame(pCodecContext, pDecodedFrame);
        if (err == AVERROR(EAGAIN)) {
            continue; // not every packet may produce a frame
        }
        else if (err < 0) {
            throw std_runtime_error << "avcodec_receive_frame(): " << AVError(err).Message();
        }
        ::sws_scale(pSwsContext,
            pDecodedFrame->data, pDecodedFrame->linesize, 0, pDecodedFrame->height,
            pRgbFrame->data, pRgbFrame->linesize
        );
        ::av_image_copy_to_buffer(rgbData.data(), rgbData.size(),
            pRgbFrame->data, pRgbFrame->linesize, AV_PIX_FMT_RGB32, 
            width, height, 1
        );
        AddTimestamp(width, height, rgbData.data());

        if (!mSaveToFile.HandleVideo(rgbData.data())) {
            throw std_runtime_error << mSaveToFile.Error();
        }
        if (mpClient) {
            mpClient->OnWebcamFrameData(width, height, rgbData.data());
        }
    }

    lock.lock();
    mConditionVariable.wait(lock, [this]{return mState == terminating;});
    mState = cleanup;
    lock.unlock();
  }
  catch (std::exception& e) {
    std::unique_lock<std::mutex> lock(mMutex);
    mError = e.what();
    mState = error;
    lock.unlock();
    if (mpClient) {
        mpClient->OnWebcamFrameData(0, 0, nullptr);
    }
    mConditionVariable.notify_one();
  }

  ::av_dict_free(&pOptions);
  ::avformat_close_input(&pFormatContext);
  ::avcodec_free_context(&pCodecContext);
  ::av_frame_free(&pDecodedFrame);
  ::av_frame_free(&pRgbFrame);
  ::sws_freeContext(pSwsContext);
}

void WebcamThread::Private::AddTimestamp(int width, int height, uint8_t* pData)
{
    std::string text;
    if (mTimestampMode == FrameCounter)
        text = std::to_string(mSaveToFile.CurrentFrameCount());
    else
        text = TimeToString(Now());

    auto size = mTextRenderer.MeasureText(text);
    const int margin = 10;
    int left = margin, top = margin;
    switch (mTimestampMode) {
    case TimestampTopLeft:
        break;
    case TimestampTopRight:
    case FrameCounter:
        left = width - margin - size.width;
        break;
    case TimestampBottomLeft:
        top = height - margin - size.height;
        break;
    case TimestampBottomRight:
        left = width - margin - size.width;
        top = height - margin - size.height;
        break;
    }
    union { uint8_t* u8; uint32_t* u32; } ptr = { pData };
    mTextRenderer.AttachToImage(width, height, ptr.u32);
    mTextRenderer.RenderText(left, top, text);
}

std::vector<WebcamThread::Private::Device> WebcamThread::Private::ListWebcams()
{
    std::vector<Device> devices;
    ::avdevice_register_all();
    const AVInputFormat* pFormat = nullptr;
    while (pFormat = ::av_input_video_device_next(pFormat)) {
        AVDeviceInfoList* pSources = nullptr;
        if (::avdevice_list_input_sources(pFormat, nullptr, nullptr, &pSources) >= 0) {
            for (int i = 0; i < pSources->nb_devices; ++i) {
                bool hasVideo = false;
                for (int j = 0; j < pSources->devices[i]->nb_media_types; ++j) {
                    hasVideo = hasVideo || pSources->devices[i]->media_types[j] == AVMEDIA_TYPE_VIDEO;
                }
                if (!hasVideo) {
                    continue;
                }

                Device device = {
                    pFormat->name,
                    pSources->devices[i]->device_description,
                    "",
                    pSources->devices[i]->device_name,
                };
                // some drivers have special device naming schemes
                if (device.driver == "dshow") {
                    device.url = "video=" + device.name;
                }
                else {
                    device.url = device.name;
                }
                devices.push_back(device);
            }
            ::avdevice_free_list_devices(&pSources);
        }
    }
    return devices;
}

std::vector<VideoMediaType> WebcamThread::Private::ListVideoMediaTypes(const Device& inDevice)
{
    const AVInputFormat* pFormat = ::av_find_input_format(inDevice.driver.c_str());
    if (pFormat && inDevice.driver == "dshow") {
        return ListVideoMediaTypesDshow(pFormat, inDevice.url);
    }
    return std::vector<VideoMediaType>();
}

VideoMediaType WebcamThread::Private::MatchVideoMediaType(const std::vector<VideoMediaType>& types, const VideoMediaType& desired)
{
    int idx = -1;
    double minDist = std::numeric_limits<double>::infinity();
    for (int i = 0; i < types.size(); ++i) {
        double sqDist = desired.sqDist(types[i]);
        if (sqDist < minDist) {
            minDist = sqDist;
            idx = i;
        }
    }
    if (idx < 0)
        return desired;
    return types[idx];
}

std::vector<VideoMediaType> WebcamThread::Private::ListVideoMediaTypesDshow(const AVInputFormat* inpFormat, const std::string& url)
{
    AVDictionary* pOptions = nullptr;
    AVFormatContext* pFormatContext = ::avformat_alloc_context(); // allocate it here so we can refer to it in the log.
    // Opening the input device with the "list_options" option will dump a list of video formats to
    // the log.
    std::string logdata;
    ::av_dict_set_int(&pOptions, "list_options", 1, 0);
    {
        AVLogFilter filter(AV_LOG_INFO, pFormatContext);
        int err = ::avformat_open_input(&pFormatContext, url.c_str(), inpFormat, &pOptions);
        if (err < 0 && err != AVERROR_EXIT) {
            throw std_runtime_error << "avformat_open_input(): " << AVError(err).Message();
        }
        logdata = filter.LogData();
        ::av_dict_free(&pOptions);
        ::avformat_close_input(&pFormatContext);
    }
    return ParseVideoMediaTypesDshow(logdata);
}

std::vector<VideoMediaType> WebcamThread::Private::ParseVideoMediaTypesDshow(const std::string& log)
{
    std::vector<VideoMediaType> types;
    const char* pPattern = "  pixel_format=(.+)"
                           "  min s=([0-9]+)x([0-9]+) fps=([0-9\\.]+)"
                           " max s=([0-9]+)x([0-9]+) fps=([0-9\\.]+)";
    std::regex pattern(pPattern);
    std::istringstream iss(log);
    std::string line;
    while (std::getline(iss, line)) {
        std::smatch match;
        if (std::regex_match(line, match, pattern)) {
            Assert(match.size() == 8);
            std::string pixelFormat = match[1].str(),
                        minWidth = match[2].str(),
                        minHeight = match[3].str(),
                        minFps = match[4].str(),
                        maxWidth = match[5].str(),
                        maxHeight = match[6].str(),
                        maxFps = match[7].str();

            const float expandBy = 10000;

            VideoMediaType minType;
            minType.width = ::atoi(minWidth.c_str());
            minType.height = ::atoi(minHeight.c_str());
            minType.framerateN = ::atof(minFps.c_str()) * expandBy;
            minType.framerateD = expandBy;
            types.push_back(minType);

            VideoMediaType maxType;
            maxType.width = ::atoi(maxWidth.c_str());
            maxType.height = ::atoi(maxHeight.c_str());
            maxType.framerateN = ::atof(maxFps.c_str()) * expandBy;
            maxType.framerateD = expandBy;
            types.push_back(maxType);
        }
    }
    return types;
}
