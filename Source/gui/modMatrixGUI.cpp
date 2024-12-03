//
// Created by blaze on 07.11.2024.
//

#include "modMatrixGUI.h"



typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
modMatrixGUI::modMatrixGUI(SuperWaveSynthAudioProcessor& p) : audioProcessor(p),intens1("Intensity",2,false,true),
intens2("Intensity",2,false,true),intens3("Intensity",2,false,true),
intens4("Intensity",2,false,true),intens5("Intensity",2,false,true)
{
    addAndMakeVisible(intens1);
    addAndMakeVisible(intens2);
    addAndMakeVisible(intens3);
    addAndMakeVisible(intens4);
    addAndMakeVisible(intens5);

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

    intens1Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity1",intens1.slider);
    intens2Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity2",intens2.slider);
    intens3Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity3",intens3.slider);
    intens4Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity4",intens4.slider);
    intens5Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"modIntensity5",intens5.slider);

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

    source1.setLookAndFeel(&comboLook);
    source2.setLookAndFeel(&comboLook);
    source3.setLookAndFeel(&comboLook);
    source4.setLookAndFeel(&comboLook);
    source5.setLookAndFeel(&comboLook);

    dest1.setLookAndFeel(&comboLook);
    dest2.setLookAndFeel(&comboLook);
    dest3.setLookAndFeel(&comboLook);
    dest4.setLookAndFeel(&comboLook);
    dest5.setLookAndFeel(&comboLook);

    setSize(300, 200);
}

modMatrixGUI::~modMatrixGUI()
{
    source1.setLookAndFeel(nullptr);
    source2.setLookAndFeel(nullptr);
    source3.setLookAndFeel(nullptr);
    source4.setLookAndFeel(nullptr);
    source5.setLookAndFeel(nullptr);

    dest1.setLookAndFeel(nullptr);
    dest2.setLookAndFeel(nullptr);
    dest3.setLookAndFeel(nullptr);
    dest4.setLookAndFeel(nullptr);
    dest5.setLookAndFeel(nullptr);
}

void modMatrixGUI::paint (juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff949FD6));
    g.fillRoundedRectangle(getLocalBounds().toFloat(),8);
    juce::Rectangle<int> totalArea = getLocalBounds();
}

void modMatrixGUI::resized()
{

    juce::Rectangle<int> area = getLocalBounds();

    juce::FlexBox layout;
    layout.flexDirection = juce::FlexBox::Direction::column;

    juce::FlexBox row1;
    setUpLayout(row1,source1,intens1,dest1);
    juce::FlexBox row2;
    setUpLayout(row2,source2,intens2,dest2);
    juce::FlexBox row3;
    setUpLayout(row3,source3,intens3,dest3);
    juce::FlexBox row4;
    setUpLayout(row4,source4,intens4,dest4);
    juce::FlexBox row5;
    setUpLayout(row5,source5,intens5,dest5);

    layout.items.add(juce::FlexItem(row1).withFlex(1.0f));
    layout.items.add(juce::FlexItem(row2).withFlex(1.0f));
    layout.items.add(juce::FlexItem(row3).withFlex(1.0f));
    layout.items.add(juce::FlexItem(row4).withFlex(1.0f));
    layout.items.add(juce::FlexItem(row5).withFlex(1.0f));

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

void modMatrixGUI::setUpLayout(juce::FlexBox& box, juce::ComboBox& item1, customSlider& item2, juce::ComboBox& item3)
{

    box.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(box,item1,itemWidth,itemHeight,margin);
    addItemToFlexBox(box,item2,itemWidth,itemHeight,margin);
    addItemToFlexBox(box,item3,itemWidth,itemHeight,margin);
}
