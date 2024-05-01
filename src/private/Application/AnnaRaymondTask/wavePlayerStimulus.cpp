#include "WavePlayerStimulus.h"
#include "BCIStream.h"

WavePlayerStimulus::WavePlayerStimulus()
{   
    wvplayer = new WavePlayer();
    wvplayer->SetVolume(100);
    
}

WavePlayerStimulus::~WavePlayerStimulus()
{
    if (wvplayer->IsPlaying())
        wvplayer->Stop();
    delete wvplayer;
}

WavePlayerStimulus& WavePlayerStimulus::SetSound(const std::string& inSound)
{
    if (!inSound.empty())
        wvplayer->SetFile(inSound);
    else
        bcierr("audio path is empty");
    return *this;
}

std::string WavePlayerStimulus::Sound() const
{
    return wvplayer->File();
}

WavePlayerStimulus& WavePlayerStimulus::SetVolume(float inVolume)
{
    wvplayer->SetVolume(inVolume);
    return *this;
}

float WavePlayerStimulus::Volume() const
{
    return wvplayer->Volume();
}


int WavePlayerStimulus::Error() const
{
    return wvplayer->ErrorState();
}

void WavePlayerStimulus::OnPresent()
{
    if (wvplayer->IsPlaying())
        wvplayer->Stop();
    wvplayer->Play();
    checkAudioPlayerErr();
    bciout << "------waveplayStimulus: " << wvplayer->File() << std::endl;
}

void WavePlayerStimulus::OnConceal()
{
    wvplayer->Stop();
}

void WavePlayerStimulus::OnRepeate() {
    OnPresent();
}

bool WavePlayerStimulus::OnIsPlaying() {
    return wvplayer->IsPlaying();
}

void WavePlayerStimulus::checkAudioPlayerErr() {
    switch (wvplayer->ErrorState())
    {
    case WavePlayer::noError:
        //bciout << "wavepalyer no error";
        break;
    case WavePlayer::fileOpeningError:
        bcierr << "waveplayer:the File property was set to a non-existent file, or a file that could not be loaded as an audio file.";
        break;
    case WavePlayer::featureNotSupported:
        bcierr << "waveplayer:a feature provided by one of the properties (such as panning) is not supported by the current implementation.";
        break;
    case WavePlayer::invalidParams:
        bcierr << "waveplayer:a property is out of range.";
        break;
    case WavePlayer::initError:
        bcierr << "waveplayer:there was an error when initializing an underlying library (such as DirectX Audio under Windows).";
        break;
    case WavePlayer::genError:
        bcierr << "waveplayer:an error occurred that does not fit into any of the remaining categories.";
        break;
    default:
        bcierr << "waveplayer:undefined error state";
        break;
    }
}

