/*
  ==============================================================================

    customLookAndFeel.cpp
    Created: 10 Jul 2024 11:07:30am
    Author:  blaze

  ==============================================================================
*/

#include "customLook.h"

void customLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    auto outline = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
    auto fill = juce::Colour(0x80DFB8EC);


    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);

    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto radiuscircle = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 3.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmin(6.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius,
        arcRadius,
        0.0f,
        rotaryStartAngle,
        rotaryEndAngle,
        true);

    g.setColour(outline);
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (slider.isEnabled())
    {
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            toAngle,
            true);

        g.setColour(fill);
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    auto outerCircleRadius = radiuscircle*2;
    auto outerCircleDiameter = outerCircleRadius/2;
    auto innerCircleRadius = outerCircleRadius * 0.8;
    auto innerCircleDiameter = innerCircleRadius / 2;
    auto intermediateRadius = (outerCircleRadius - innerCircleRadius)*0.5;
    auto intermediateWidth = juce::jmax(6.0, intermediateRadius);
    auto intermediateOffset = intermediateWidth/2;

    auto outerShadowRadius = (outerCircleRadius - innerCircleRadius) * 0.2;
    auto outerShadowWidth = juce::jmax(1.0, outerShadowRadius);
    auto outerShadowOffset = intermediateWidth / 2;
    //outer circle
    //===================================
    g.setColour(juce::Colour(0xff1A1818));
    g.fillEllipse(bounds.getCentreX()-outerCircleDiameter, bounds.getCentreY()-outerCircleDiameter, outerCircleRadius, outerCircleRadius);

    //intermediate Ellipse
    //=============================================================
    //g.fillEllipse(bounds.getCentreX() - innerCircleDiameter-3, bounds.getCentreY() - innerCircleDiameter-3, innerCircleRadius+6, innerCircleRadius+6);
    juce::Path intermediateEllipse;
    intermediateEllipse.addEllipse(bounds.getCentreX() - innerCircleDiameter - intermediateOffset, bounds.getCentreY() - innerCircleDiameter - intermediateOffset, innerCircleRadius + intermediateRadius, innerCircleRadius + intermediateRadius);
    //==============================================================
    //innerCircle
    juce::Path innerCircle;
    innerCircle.addEllipse(bounds.getCentreX() - innerCircleDiameter, bounds.getCentreY() - innerCircleDiameter, innerCircleRadius, innerCircleRadius);
    //shadow=============================
    juce::Path outerShadow;
    outerShadow.addEllipse(bounds.getCentreX() - innerCircleDiameter - outerShadowOffset, bounds.getCentreY() - innerCircleDiameter - outerShadowOffset, innerCircleRadius + outerShadowWidth, innerCircleRadius + outerShadowWidth);
    juce::Point<int> shadowOffset;
    shadowOffset.setXY(3, 3);
    auto shadow = juce::DropShadow(juce::Colour(0x8a2D2D2D), 6, shadowOffset);
    shadow.drawForPath(g, outerShadow);
    //==================================

    //interMediate Circle fill
    g.setColour(juce::Colour(0x70232020)); //0x99
    g.fillPath(intermediateEllipse);
    //shadow inner
    shadow.drawForPath(g, innerCircle);
    //inner circle fill
    g.setColour(juce::Colour(0xff232020));
    g.fillPath(innerCircle);
    //inner circle stroke
    g.setColour(juce::Colour(0x26131212));
    juce::Path innerCircleStroke;
    innerCircleStroke.addEllipse(bounds.getCentreX() - innerCircleDiameter, bounds.getCentreY() - innerCircleDiameter, innerCircleRadius, innerCircleRadius);
    g.strokePath(innerCircleStroke, juce::PathStrokeType(2));
    //highlight arc
    juce::Path highlightArc;
    highlightArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        innerCircleDiameter,
        innerCircleDiameter,
        0.0f,
        juce::MathConstants<float>::pi*1.3f,
        juce::MathConstants<float>::pi*2.05f,
        true);
    auto startPoint = highlightArc.getPointAlongPath(0);
    auto endPoint = highlightArc.getPointAlongPath(highlightArc.getLength());
    auto middlePoint = highlightArc.getPointAlongPath(highlightArc.getLength() / 2);
    //g.setColour(juce::Colours::whitesmoke);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0x26131212), startPoint, juce::Colour(0x24CACACA), endPoint,true));
    //g.setGradientFill(juce::ColourGradient(juce::Colour(0xffCACACA), middlePoint, juce::Colour(0x26131212), endPoint, false));
    g.strokePath(highlightArc, juce::PathStrokeType(2, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));


    /*auto thumbWidth = lineW * 2.0f;
    juce::Point<float> thumbPoint(bounds.getCentreX() + innerCircleDiameter * std::cos(toAngle - juce::MathConstants<float>::halfPi),
        bounds.getCentreY() + innerCircleDiameter * std::sin(toAngle - juce::MathConstants<float>::halfPi));
    juce::Path highlightRoute;
    highlightRoute.addEllipse(juce::Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));


    juce::Point<int> highlightPoint(0, 0);
    auto highlight = juce::DropShadow(juce::Colour(0x336D5B5B), 1, highlightPoint);
    highlight.drawForPath(g, highlightRoute);*/

    //g.setColour(slider.findColour(juce::Slider::thumbColourId));
    //g.fillEllipse(juce::Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));
}
