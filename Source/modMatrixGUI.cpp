//
// Created by blaze on 07.11.2024.
//

#include "modMatrixGUI.h"
#include <JuceHeader.h>


typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
modMatrixGUI::modMatrixGUI(SimpleSynthAudioProcessor& p) : audioProcessor(p)
{
    makeSlider(intens1);
    makeSlider(intens2);
    makeSlider(intens3);
    makeSlider(intens4);
    makeSlider(intens5);
    
    setUpComboBox(source1,sources);
    setUpComboBox(source2,sources);
    setUpComboBox(source3,sources);
    setUpComboBox(source4,sources);
    setUpComboBox(source5,sources);

    setUpComboBox(dest1,destinations);
    setUpComboBox(dest2,destinations);
    setUpComboBox(dest3,destinations);
    setUpComboBox(dest4,destinations);
    setUpComboBox(dest5,destinations);

    intens1Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity1",intens1);
    intens2Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity2",intens2);
    intens3Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity3",intens3);
    intens4Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity4",intens4);
    intens5Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity5",intens5);

    source1Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modSource1",source1);
    source2Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modSource2",source2);
    source3Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modSource3",source3);
    source4Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modSource4",source4);
    source5Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modSource5",source5);

    dest1Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modDestination1",dest1);
    dest2Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modDestination2",dest2);
    dest3Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modDestination3",dest3);
    dest4Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modDestination4",dest4);
    dest5Attach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"modDestination5",dest5);



    setSize(300, 200);
}

modMatrixGUI::~modMatrixGUI()
{

}

void modMatrixGUI::paint (juce::Graphics& g)
{
    juce::Rectangle<int> area = getLocalBounds();
    g.setColour(juce::Colour(0xff312F2F));
    g.fillRoundedRectangle(area.toFloat(),6);
}

void modMatrixGUI::resized()
{

    juce::Rectangle<int> area = getLocalBounds().reduced(5);

    juce::FlexBox layout;
    layout.flexDirection = juce::FlexBox::Direction::column;

    juce::FlexBox row1;
    setUpLayout(row1,dest1,intens1,source1);
    //row1.performLayout(area);
    juce::FlexBox row2;
    setUpLayout(row2,dest2,intens2,source2);
    juce::FlexBox row3;
    setUpLayout(row3,dest3,intens3,source3);
    juce::FlexBox row4;
    setUpLayout(row4,dest4,intens4,source4);
    juce::FlexBox row5;
    setUpLayout(row5,dest5,intens5,source5);

    layout.items.add(juce::FlexItem(row1).withFlex(1.0f));
    layout.items.add(juce::FlexItem(row2).withFlex(1.0f));
    layout.items.add(juce::FlexItem(row3).withFlex(1.0f));
    layout.items.add(juce::FlexItem(row4).withFlex(1.0f));
    layout.items.add(juce::FlexItem(row5).withFlex(1.0f));

    // Perform the layout with the correct FlexItems
    layout.performLayout(area);

}

void modMatrixGUI::comboBoxChanged(juce::ComboBox* box)
{
}

void modMatrixGUI::sliderValueChanged(juce::Slider* slider)
{
}

void modMatrixGUI::buttonClicked(juce::Button* button)
{

}

void modMatrixGUI::makeKnob(juce::Slider& slider,juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(12.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void modMatrixGUI::makeSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
}

void modMatrixGUI::setUpComboBox(juce::ComboBox& box,juce::StringArray& s)
{
    addAndMakeVisible(box);
    box.addItemList(s,1);
    box.setSelectedId(0);
}

void modMatrixGUI::setUpLayout(juce::FlexBox& box, juce::ComboBox& item1, juce::Slider& item2, juce::ComboBox& item3)
{

    box.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(box,item1,itemWidth,itemHeight,margin);
    addItemToFlexBox(box,item2,itemWidth,itemHeight,margin);
    addItemToFlexBox(box,item3,itemWidth,itemHeight,margin);
}
