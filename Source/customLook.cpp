/*
  ==============================================================================

    customLookAndFeel.cpp
    Created: 10 Jul 2024 11:07:30am
    Author:  blaze

  ==============================================================================
*/

#include "customLook.h"

#include <juce_gui_basics/detail/juce_LookAndFeelHelpers.h>

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

void customLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    using namespace juce;
    if (slider.isBar())
    {
        g.setColour (slider.findColour (Slider::trackColourId));
        g.fillRect (slider.isHorizontal() ? Rectangle<float> (static_cast<float> (x), (float) y + 0.5f, sliderPos - (float) x, (float) height - 1.0f)
                                          : Rectangle<float> ((float) x + 0.5f, sliderPos, (float) width - 1.0f, (float) y + ((float) height - sliderPos)));

        drawLinearSliderOutline (g, x, y, width, height, style, slider);
    }
    else
    {
        auto isTwoVal   = (style == Slider::SliderStyle::TwoValueVertical   || style == Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

        auto trackWidth = jmin (6.0f, slider.isHorizontal() ? (float) height * 0.25f : (float) width * 0.25f);

        Point<float> startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                                 slider.isHorizontal() ? (float) y + (float) height * 0.5f : (float) (height + y));

        Point<float> endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                               slider.isHorizontal() ? startPoint.y : (float) y);

        Path backgroundTrack;
        backgroundTrack.startNewSubPath (startPoint);
        backgroundTrack.lineTo (endPoint);
        g.setColour (slider.findColour (Slider::backgroundColourId));
        g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        Path valueTrack;
        Point<float> minPoint, maxPoint, thumbPoint;

        if (isTwoVal || isThreeVal)
        {
            minPoint = { slider.isHorizontal() ? minSliderPos : (float) width * 0.5f,
                         slider.isHorizontal() ? (float) height * 0.5f : minSliderPos };

            if (isThreeVal)
                thumbPoint = { slider.isHorizontal() ? sliderPos : (float) width * 0.5f,
                               slider.isHorizontal() ? (float) height * 0.5f : sliderPos };

            maxPoint = { slider.isHorizontal() ? maxSliderPos : (float) width * 0.5f,
                         slider.isHorizontal() ? (float) height * 0.5f : maxSliderPos };
        }
        else
        {
            auto kx = slider.isHorizontal() ? sliderPos : ((float) x + (float) width * 0.5f);
            auto ky = slider.isHorizontal() ? ((float) y + (float) height * 0.5f) : sliderPos;

            minPoint = startPoint;
            maxPoint = { kx, ky };
        }

        auto thumbWidth = getSliderThumbRadius (slider);

        //This is how a valueTrack path start is determined

        //valueTrack.startNewSubPath (minPoint);
        //valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
        Point<float> midPoint;
        if(slider.isHorizontal())
            midPoint = {maxSliderPos,height/2.f};
        else
            midPoint = {width*0.5f,maxSliderPos};

        valueTrack.startNewSubPath(midPoint);
        valueTrack.lineTo(maxPoint);
        g.setColour (juce::Colour(0xffDFB8EC));
        g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        //Slider's thumb color
        if (! isTwoVal)
        {
            g.setColour (juce::Colours::pink);
            g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));
        }

        if (isTwoVal || isThreeVal)
        {
            auto sr = jmin (trackWidth, (slider.isHorizontal() ? (float) height : (float) width) * 0.4f);
            auto pointerColour = juce::Colours::pink;

            if (slider.isHorizontal())
            {
                drawPointer (g, minSliderPos - sr,
                             jmax (0.0f, (float) y + (float) height * 0.5f - trackWidth * 2.0f),
                             trackWidth * 2.0f, pointerColour, 2);

                drawPointer (g, maxSliderPos - trackWidth,
                             jmin ((float) (y + height) - trackWidth * 2.0f, (float) y + (float) height * 0.5f),
                             trackWidth * 2.0f, pointerColour, 4);
            }
            else
            {
                drawPointer (g, jmax (0.0f, (float) x + (float) width * 0.5f - trackWidth * 2.0f),
                             minSliderPos - trackWidth,
                             trackWidth * 2.0f, pointerColour, 1);

                drawPointer (g, jmin ((float) (x + width) - trackWidth * 2.0f, (float) x + (float) width * 0.5f), maxSliderPos - sr,
                             trackWidth * 2.0f, pointerColour, 3);
            }
        }

        if (slider.isBar())
            drawLinearSliderOutline (g, x, y, width, height, style, slider);
    }
}

void customLookAndFeel::drawLinearSliderOutline(juce::Graphics& g, int x, int y, int width, int height,
    juce::Slider::SliderStyle slider_style, juce::Slider& slider)
{
    using namespace juce;
    if (slider.getTextBoxPosition() == Slider::NoTextBox)
    {
        g.setColour (slider.findColour (Slider::textBoxOutlineColourId));
        g.drawRect (0, 0, slider.getWidth(), slider.getHeight(), 1);
    }
}

void customLookAndFeel::drawLinearSliderThumb(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style,
    juce::Slider& slider)
{
    using namespace juce;
     auto sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

    /*auto knobColour = detail::LookAndFeelHelpers::createBaseColour (slider.findColour (Slider::thumbColourId),
                                                                    slider.hasKeyboardFocus (false) && slider.isEnabled(),
                                                                    slider.isMouseOverOrDragging() && slider.isEnabled(),
                                                                    slider.isMouseButtonDown() && slider.isEnabled());*/
    auto knobColour = juce::Colours::pink;
    const float outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

    if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
    {
        float kx, ky;

        if (style == Slider::LinearVertical)
        {
            kx = (float) x + (float) width * 0.5f;
            ky = sliderPos;
        }
        else
        {
            kx = sliderPos;
            ky = (float) y + (float) height * 0.5f;
        }

        drawGlassSphere (g,
                         kx - sliderRadius,
                         ky - sliderRadius,
                         sliderRadius * 2.0f,
                         knobColour, outlineThickness);
    }
    else
    {
        if (style == Slider::ThreeValueVertical)
        {
            drawGlassSphere (g, (float) x + (float) width * 0.5f - sliderRadius,
                             sliderPos - sliderRadius,
                             sliderRadius * 2.0f,
                             knobColour, outlineThickness);
        }
        else if (style == Slider::ThreeValueHorizontal)
        {
            drawGlassSphere (g,sliderPos - sliderRadius,
                             (float) y + (float) height * 0.5f - sliderRadius,
                             sliderRadius * 2.0f,
                             knobColour, outlineThickness);
        }

        if (style == Slider::TwoValueVertical || style == Slider::ThreeValueVertical)
        {
            auto sr = jmin (sliderRadius, (float) width * 0.4f);

            drawGlassPointer (g, jmax (0.0f, (float) x + (float) width * 0.5f - sliderRadius * 2.0f),
                              minSliderPos - sliderRadius,
                              sliderRadius * 2.0f, knobColour, outlineThickness, 1);

            drawGlassPointer (g,
                              jmin ((float) x + (float) width - sliderRadius * 2.0f,
                                    (float) x + (float) width * 0.5f),
                              maxSliderPos - sr,
                              sliderRadius * 2.0f,
                              knobColour,
                              outlineThickness,
                              3);
        }
        else if (style == Slider::TwoValueHorizontal || style == Slider::ThreeValueHorizontal)
        {
            auto sr = jmin (sliderRadius, (float) height * 0.4f);

            drawGlassPointer (g, minSliderPos - sr,
                              jmax (0.0f, (float) y + (float) height * 0.5f - sliderRadius * 2.0f),
                              sliderRadius * 2.0f, knobColour, outlineThickness, 2);

            drawGlassPointer (g,
                              maxSliderPos - sliderRadius,
                              jmin ((float) y + (float) height - sliderRadius * 2.0f,
                                    (float) y + (float) height * 0.5f),
                              sliderRadius * 2.0f,
                              knobColour,
                              outlineThickness,
                              4);
        }
    }
}

void customLookAndFeel::drawLinearSliderBackground(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style,
    juce::Slider& slider)
{
    using namespace juce;
    const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

    const Colour trackColour (slider.findColour (Slider::trackColourId));
    const Colour gradCol1 (trackColour.overlaidWith (Colour (slider.isEnabled() ? 0x13000000 : 0x09000000)));
    const Colour gradCol2 (trackColour.overlaidWith (Colour (0x06000000)));
    Path indent;

    if (slider.isHorizontal())
    {
        auto iy = (float) y + (float) height * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill (ColourGradient::vertical (gradCol1, iy, gradCol2, iy + sliderRadius));

        indent.addRoundedRectangle ((float) x - sliderRadius * 0.5f, iy, (float) width + sliderRadius, sliderRadius, 5.0f);
    }
    else
    {
        auto ix = (float) x + (float) width * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill (ColourGradient::horizontal (gradCol1, ix, gradCol2, ix + sliderRadius));

        indent.addRoundedRectangle (ix, (float) y - sliderRadius * 0.5f, sliderRadius, (float) height + sliderRadius, 5.0f);
    }

    g.fillPath (indent);

    g.setColour (trackColour.contrasting (0.5f));
    g.strokePath (indent, PathStrokeType (0.5f));
}

//======================================================================================================================
//filter emu button
void notToggleButtonLook::drawButtonBackground(juce::Graphics& g, juce::Button& button,
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

void notToggleButtonLook::drawButtonText(juce::Graphics& g, juce::TextButton& button,
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
