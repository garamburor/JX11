/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class JX11AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    JX11AudioProcessorEditor (JX11AudioProcessor&);
    ~JX11AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JX11AudioProcessor& audioProcessor;

    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;

    juce::Slider outputLevelKnob;
    SliderAttachment outputLevelAttachment{ audioProcessor.apvts,
                ParameterID::outputLevel.getParamID(), outputLevelKnob };

    juce::Slider filterResoKnob;
    SliderAttachment filterResoAttachment{ audioProcessor.apvts,
              ParameterID::filterReso.getParamID(), filterResoKnob };

    using ButtonAttachment = APVTS::ButtonAttachment;

    juce::TextButton polyModeButton;
    ButtonAttachment polyModeAttachment{ audioProcessor.apvts,
              ParameterID::polyMode.getParamID(), polyModeButton };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JX11AudioProcessorEditor)
};
