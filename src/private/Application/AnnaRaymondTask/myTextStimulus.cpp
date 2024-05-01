#include "MyTextStimulus.h"
#include "BCIStream.h"

MyTextStimulus::MyTextStimulus(QWidget* window)
{
    textLabel = new QLabel(window);
    QFont font;
    font.setPixelSize(window->height() / 10);
    font.fromString("Arial");
    textLabel->setFont(font);

    textLabel->setAlignment(Qt::AlignCenter);

    QPalette palette = textLabel->palette();
    palette.setColor(textLabel->backgroundRole(), Qt::white);
    palette.setColor(textLabel->foregroundRole(), Qt::yellow);
    textLabel->setPalette(palette);
}

MyTextStimulus::MyTextStimulus(QWidget* window, const QString cssStyle)
{
    textLabel = new QLabel(window);
    QFont font;
    font.setPixelSize(window->height() / 10);
    font.fromString("Arial");
    textLabel->setFont(font);

    textLabel->setAlignment(Qt::AlignCenter);

    textLabel->setStyleSheet("QLabel{background-color:white; color:black; }");

    textLabel->setStyleSheet(cssStyle);
}

MyTextStimulus::~MyTextStimulus()
{
    delete textLabel;
}

MyTextStimulus& MyTextStimulus::SetText(const std::string& text)
{
    QString s = QString::fromStdString(text);
    textLabel->setText(s);
    return *this;
}

std::string MyTextStimulus::Text() const
{
    return (textLabel->text()).toStdString();
}

MyTextStimulus& MyTextStimulus::SetSize(int width, int height)
{
    textLabel->resize(width,height);
    return *this;
}

QSize MyTextStimulus::Size() const
{
    return textLabel->size();
}

void MyTextStimulus::OnPresent()
{
    textLabel->show();
}

void MyTextStimulus::OnConceal()
{
    textLabel->hide();
}

void MyTextStimulus::OnRepeate() {
    OnPresent();
}

bool MyTextStimulus::OnIsPlaying() {
    return !(textLabel->isHidden());
}


