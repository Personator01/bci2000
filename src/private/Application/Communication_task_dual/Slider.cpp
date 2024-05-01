#include "Slider.h"
#include <qpainter.h>
#include "BCIStream.h"
#include "qpoint.h"

Slider::Slider(GUI::GraphDisplay& display, int zOrder) : GraphObject(display, zOrder)
{
    
}

Slider::~Slider() {
    DestructorEntered();
}

//getter and setter
//Slider& Slider::SetLineWidth(float f) {
//    if (m_def.line_width.load() != f) {
//        m_def.line_width = f;
//        Change();
//    }
//    return *this;
//}
//
//float Slider::GetLineWidth() const {
//    return m_def.line_width;
//}

Slider& Slider::SetLineColor(RGBColor c) {
    if (m_def.line_color.load() != c) {
        m_def.line_color = c;
        Change();
    }
    return *this;
}

RGBColor Slider::LineColor() const {
    return m_def.line_color;
}

Slider& Slider::SetAxesColor(RGBColor c) {
    if (m_def.axes_color.load() != c) {
        m_def.axes_color = c;
        Change();
    }
    return *this;
}

RGBColor Slider::AxesColor() const {
    return m_def.axes_color;
}

Slider& Slider::SetAxesXPosition(float f) {
    if (f > 1.0f || f < 0.0f) 
        bcierr << "X position of axes must between 0 and 1!" << std::endl;
    if (m_def.axes_x_position.load() != f) {
        m_def.axes_x_position = f;
        Change();
    }
    return *this;
}

float Slider::AxesXPosition() const {
    return m_def.axes_x_position;
}

//Slider& Slider::SetText(std::string s1, std::string s2) {
//    if (m_def.text1.Const()->c_str() != s1) {
//        *(m_def.text1).Mutable() = s1;
//        Change();
//    }
//    if (m_def.text2.Const()->c_str() != s2) {
//        *(m_def.text2).Mutable() = s2;
//        Change();
//    }
//    return *this;
//}
//
//std::vector<std::string> Slider::GetText() const {
//    std::vector<std::string> result{ m_def.text1.Const()->c_str(), m_def.text2.Const()->c_str() };
//    return result;
//}
//
//Slider& Slider::SetTextColor(RGBColor c) {
//    if (m_def.text_color.load() != c) {
//        m_def.text_color = c;
//        Change();
//    }
//    return *this;
//}
//
//RGBColor Slider::GetTextColor() const {
//    return m_def.text_color;
//}

void Slider::OnPaint(const GUI::DrawContext& inDC)
{
    Draw(inDC, m_def);
}

void Slider::Draw(const GUI::DrawContext& inDC, const SliderDef& inDef) {   
    //color
    const RGBColor& line_color = inDef.line_color;
    const RGBColor& axes_color = inDef.axes_color;
    //const RGBColor& text_color = inDef.text_color;

    QColor line_color_q(line_color.R(), line_color.G(), line_color.B());
    QColor axes_color_q(axes_color.R(), axes_color.G(), axes_color.B());
    //QColor text_color_q(text_color.R(), text_color.G(), text_color.B());

    //define two rects
    float axes_width = inDC.rect.Width() / 90;
    float axes_height = inDC.rect.Height();
    float axes_left = inDC.rect.left + inDC.rect.Width() * inDef.axes_x_position;
    float axes_top = inDC.rect.top;

    float line_height = inDC.rect.Height() / 5;
    float line_top = inDC.rect.top + axes_height / 2 - line_height / 2;

    //correct axes_left
    if (inDef.axes_x_position == 0)
        axes_left = inDC.rect.left;
    else if (inDef.axes_x_position == 1)
        axes_left = std::ceil(inDC.rect.right - axes_width);

    QRect line_rect(inDC.rect.left, line_top, inDC.rect.Width(), line_height);
    QRect axes_rect(axes_left, axes_top, axes_width, axes_height);

    //define text points
    //QPoint text_p_1(corrected.left, corrected.top + corrected.Height() * 1.5);
    //QPoint text_p_2(corrected.left + corrected.Width(), corrected.top + corrected.Height() * 1.5);

    //prepare the painter
    QPainter* p = inDC.handle.dc;
     
    //prepare the brush
    QBrush fillBrush_q;
    fillBrush_q.setStyle(Qt::SolidPattern);
    fillBrush_q.setColor(line_color_q);

    // Prepare the pen
    //QPen outlinePen_q;
    //outlinePen_q.setStyle(Qt::SolidLine);
    //outlinePen_q.setColor(line_color_q);
    //outlinePen_q.setWidth(static_cast<int>(inDef.line_width));
    //p->setPen(outlinePen_q);

    //draw line
    p->fillRect(line_rect, fillBrush_q);
    
    //draw axes
    fillBrush_q.setColor(axes_color_q);
    //outlinePen_q.setColor(axes_color_q);
    //p->setPen(outlinePen_q);

    p->fillRect(axes_rect, fillBrush_q);

    //draw text
    //QFont font = p->font();
    //font.setPixelSize(corrected.Width()/20);
    //p->setFont(font);
    //outlinePen_q.setColor(text_color_q);
    //outlinePen_q.setWidth(static_cast<int>(inDef.line_width / 2));
    //p->setPen(outlinePen_q);
    //p->drawText(text_p_1, QString::fromStdString(inDef.text1.Const()->c_str()));
    //p->drawText(text_p_2, QString::fromStdString(inDef.text2.Const()->c_str()));


    //test
    //QRect back_rect(inDC.rect.left, inDC.rect.top, inDC.rect.Width(), inDC.rect.Height());
    //fillBrush_q.setStyle(Qt::Dense2Pattern);
    //fillBrush_q.setColor(Qt::yellow);
    //p->fillRect(back_rect, fillBrush_q);
}
