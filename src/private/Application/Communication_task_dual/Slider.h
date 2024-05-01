#pragma once
#ifndef SLIDER_H
#define SLIDER_H

#include "GraphObject.h"
#include "SynchronizedObject.h"
#include "Color.h"

class Slider : public GUI::GraphObject
{
public:
	Slider(GUI::GraphDisplay& display, int zOrder = 0);
	virtual ~Slider();
	//properties
	//Slider& SetLineWidth(float);
	//float GetLineWidth() const;
	Slider& SetLineColor(RGBColor);
	RGBColor LineColor() const;
	Slider& SetAxesColor(RGBColor);
	RGBColor AxesColor() const;
	//movement
	Slider& SetAxesXPosition(float);
	float AxesXPosition() const;
	//text, split text from slider is a better idea
	//Slider& SetText(std::string s1, std::string s2);
	//std::vector<std::string> GetText() const;
	//Slider& SetTextColor(RGBColor);
	//RGBColor GetTextColor() const;
	
protected:
	struct SliderDef
	{
		std::atomic<RGBColor> line_color, axes_color/*, text_color*/;
		std::atomic<float> /*line_width,*/ axes_x_position;
		//SynchronizedObject<std::string> text1, text2; //need to modify

		SliderDef()
			: line_color(RGBColor::Black), axes_color(RGBColor::Red),/* text_color(RGBColor::Black), */
			/*line_width(1),*/ axes_x_position(0.5)/*, text1("Indoors"), text2("Outdoors")*/
		{
		}
	};

	SliderDef m_def;

	// GraphObject event handlers
	void OnPaint(const GUI::DrawContext&) override;
	static void Draw(const GUI::DrawContext&, const SliderDef&);
};


#endif // SLIDER_H

