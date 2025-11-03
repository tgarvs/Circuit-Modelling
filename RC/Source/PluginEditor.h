
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class RCThreeWaysAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RCThreeWaysAudioProcessorEditor (RCThreeWaysAudioProcessor&);
    ~RCThreeWaysAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RCThreeWaysAudioProcessor& audioProcessor;
    
    juce::Slider methodKnob;
    juce::Label methodLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> methodAttachment;
    
    juce::Slider rKnob;
    juce::Label rLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resistorAttachment;

    
    juce::Slider cKnob;
    juce::Label cLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> capacitorAttachment;

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RCThreeWaysAudioProcessorEditor)
};
