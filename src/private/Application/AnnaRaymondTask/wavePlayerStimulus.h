#ifndef WAVEPLAYERSTIMULUS_H
#define WAVEPLAYERSTIMULUS_H

#include "MyStimulus.h"
#include "WavePlayer.h"

class WavePlayerStimulus : public MyStimulus
{
public:
    WavePlayerStimulus();
    virtual ~WavePlayerStimulus();
    // A sound is a file name or a text to speak when single-quoted (enclosed with '').
    WavePlayerStimulus& SetSound(const std::string&);
    std::string Sound() const;
    WavePlayerStimulus& SetVolume(float);
    float Volume() const;
    int Error() const;

protected:
    virtual void OnPresent();
    virtual void OnConceal();
    virtual void OnRepeate();
    virtual bool OnIsPlaying();


private:
    WavePlayer* wvplayer;     
    void checkAudioPlayerErr();
};


#endif // WAVEPLAYERSTIMULUS_H