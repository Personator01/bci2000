#ifndef MYIMAGESTIMULUS_H
#define MYIMAGESTIMULUS_H

#pragma once
#include "MyStimulus.h"
#include <qlabel.h>

class MyImageStimulus : public MyStimulus
{
public:
    MyImageStimulus(QWidget* window);
    virtual ~MyImageStimulus();
    MyImageStimulus& SetImage(const std::string& imagePath);
    MyImageStimulus& SetSize(int width, int height);
    QSize Size() const;

protected:
    virtual void OnPresent();
    virtual void OnConceal();
    virtual void OnRepeate();
    virtual bool OnIsPlaying();


private:
    QLabel* imageLabel;
};




#endif // MYIMAGESTIMULUS_H
