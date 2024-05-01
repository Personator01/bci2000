#include "MyImageStimulus.h"
#include "BCIStream.h"

MyImageStimulus::MyImageStimulus(QWidget* window)
{
    imageLabel = new QLabel(window);
    imageLabel->setAlignment(Qt::AlignCenter);    
}


MyImageStimulus::~MyImageStimulus()
{
    delete imageLabel;
}

MyImageStimulus& MyImageStimulus::SetImage(const std::string& imagePath)
{
    QString s = QString::fromStdString(imagePath);
    QPixmap pix;
    if (pix.load(s)) {
        pix = pix.scaled(imageLabel->size(), Qt::KeepAspectRatio);
        imageLabel->setPixmap(pix);
    }
    else {
        bcierr("MyImageStimulus: load Image error!please check the image file path");
    }
    
    return *this;
}


MyImageStimulus& MyImageStimulus::SetSize(int width, int height)
{
    imageLabel->resize(width, height);
    return *this;
}

QSize MyImageStimulus::Size() const
{
    return imageLabel->size();
}

void MyImageStimulus::OnPresent()
{
    imageLabel->show();
}

void MyImageStimulus::OnConceal()
{
    imageLabel->hide();
}

void MyImageStimulus::OnRepeate() {
    OnPresent();
}

bool MyImageStimulus::OnIsPlaying() {
    return !(imageLabel->isHidden());
}


