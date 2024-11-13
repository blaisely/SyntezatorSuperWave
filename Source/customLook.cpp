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
    auto fill = juce::Colour(0xCCDFB8EC);


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

void customLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    using namespace juce;
    auto cornerSize = 6.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    auto buttonColor = juce::Colour(0xffFCF1F1);
    auto buttonDownColor = juce::Colour(0xff5F5959);
    auto baseColour = buttonColor;
    if(button.getToggleState())
        baseColour = buttonDownColor;
    /*if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = buttonDownColor.contrasting (shouldDrawButtonAsDown ? 0.05f : 0.01f);*/


    g.setColour (baseColour);

    auto flatOnLeft   = button.isConnectedOnLeft();
    auto flatOnRight  = button.isConnectedOnRight();
    auto flatOnTop    = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), bounds.getHeight(),
                                  cornerSize, cornerSize,
                                  ! (flatOnLeft  || flatOnTop),
                                  ! (flatOnRight || flatOnTop),
                                  ! (flatOnLeft  || flatOnBottom),
                                  ! (flatOnRight || flatOnBottom));

        g.fillPath (path);

        g.setColour (baseColour);
        g.strokePath (path, PathStrokeType (1.0f));
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);

        g.setColour (baseColour);
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }
}

void customLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    using namespace juce;
    Font font (getTextButtonFont (button, button.getHeight()));
    Font customFont("Montserrat",12,0);
    auto fontColourOn = juce::Colour(0xff312F2F);
    auto fontColourOff = juce::Colours::whitesmoke;
    g.setFont (customFont);
    g.setColour (button.getToggleState() ? fontColourOff
                                                            : fontColourOn
                       .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt (font.getHeight() * 0.6f);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          Justification::centred, 2);
}

void customLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    using namespace juce;
    juce::Rectangle<int> area = label.getLocalBounds().reduced(0,5);
    g.setColour(juce::Colour(0xffD1CAF1));
    g.fillRoundedRectangle(area.toFloat(),8);
    auto fontColor = juce::Colours::white;

    if (! label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));

        g.setColour (fontColor);
        g.setFont (font);

        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());

        g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour (label.findColour (Label::outlineColourId));
    }

    g.drawRect (label.getLocalBounds());
}

void customLookAndFeel::drawLinearSlider(juce::Graphics& graphics, int x, int y, int width, int height, float sliderPos,
    float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle slider_style, juce::Slider& slider)
{
    LookAndFeel_V4::drawLinearSlider(graphics, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, slider_style,
                                     slider);
}

//======================================================================================================================
//filter emu button
void filterEmuLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    using namespace juce;
    auto cornerSize = 6.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    auto buttonColor = juce::Colour(0xffFCF1F1);
    auto buttonDownColor = juce::Colour(0xff5F5959);
    auto baseColour = buttonColor;

    g.setColour (baseColour);

    auto flatOnLeft   = button.isConnectedOnLeft();
    auto flatOnRight  = button.isConnectedOnRight();
    auto flatOnTop    = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), bounds.getHeight(),
                                  cornerSize, cornerSize,
                                  ! (flatOnLeft  || flatOnTop),
                                  ! (flatOnRight || flatOnTop),
                                  ! (flatOnLeft  || flatOnBottom),
                                  ! (flatOnRight || flatOnBottom));

        g.fillPath (path);

        g.setColour (baseColour);
        g.strokePath (path, PathStrokeType (1.0f));
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);

        g.setColour (baseColour);
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }
}

void filterEmuLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    using namespace juce;
    Font font (getTextButtonFont (button, button.getHeight()));
    Font customFont("Montserrat",12,0);
    auto fontColourOn = juce::Colour(0xff312F2F);
    auto fontColourOff = juce::Colours::whitesmoke;
    g.setFont (customFont);
    g.setColour (button.getToggleState() ? fontColourOn
                                                            : fontColourOn
                       .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt (font.getHeight() * 0.6f);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          Justification::centred, 2);
}

//======================================================================================================================
//Filter Label
void filterLabel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    using namespace juce;
    juce::Rectangle<int> area = label.getLocalBounds().reduced(0,5);
    g.setColour(juce::Colour(0xff949FD6));
    g.fillRoundedRectangle(area.toFloat(),8);
    auto fontColor = juce::Colours::white;

    if (! label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));

        g.setColour (fontColor);
        g.setFont (font);

        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());

        g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour (label.findColour (Label::outlineColourId));
    }

    g.drawRect (label.getLocalBounds());
}
