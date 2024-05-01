#ifndef MYTEXTSTIMULUS_H
#define MYTEXTSTIMULUS_H

#include "MyStimulus.h"
#include <qlabel.h>
#include <qgridlayout.h>

class MyTextStimulus : public MyStimulus
{
public:
    MyTextStimulus(QWidget* window);
    MyTextStimulus(QWidget* window, const QString cssStyle);
    virtual ~MyTextStimulus();
    // A sound is a file name or a text to speak when single-quoted (enclosed with '').
    MyTextStimulus& SetText(const std::string& text);
    std::string Text() const;
    MyTextStimulus& SetSize(int width, int height);
    QSize Size() const;

protected:
    virtual void OnPresent();
    virtual void OnConceal();
    virtual void OnRepeate();
    virtual bool OnIsPlaying();


private:
    QLabel* textLabel;
};

#endif // MYTEXTSTIMULUS_H
