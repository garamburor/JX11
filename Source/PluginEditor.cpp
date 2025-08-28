/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JX11AudioProcessorEditor::JX11AudioProcessorEditor (JX11AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    outputLevelKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    outputLevelKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(outputLevelKnob);

    filterResoKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    filterResoKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(filterResoKnob);

    polyModeButton.setButtonText("Poly");
    polyModeButton.setClickingTogglesState(true);
    addAndMakeVisible(polyModeButton);

    setSize (600, 400);
}

JX11AudioProcessorEditor::~JX11AudioProcessorEditor()
{
}

//==============================================================================
void JX11AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void JX11AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    juce::Rectangle r(20, 20, 100, 120);
    outputLevelKnob.setBounds(r);

    r = r.withX(r.getRight() + 20);
    filterResoKnob.setBounds(r);
    
    r = r.withY(r.getBottom());
    polyModeButton.setSize(80, 30);
    polyModeButton.setCentrePosition({ r.getX(), r.getCentreY() });
}
