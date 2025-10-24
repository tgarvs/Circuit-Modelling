/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RCThreeWaysAudioProcessorEditor::RCThreeWaysAudioProcessorEditor (RCThreeWaysAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 300);
    
    
    //Method Knob
    methodKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    methodKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(methodKnob);
    
    methodLabel.setText("Method", juce::NotificationType::dontSendNotification);
    methodLabel.setJustificationType(juce::Justification::centred);
    methodLabel.attachToComponent(&methodKnob, false);
    addAndMakeVisible(methodLabel);
    
    //Resistor Knob
    rKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    rKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(rKnob);
    
    rLabel.setText("Resistor", juce::NotificationType::dontSendNotification);
    rLabel.setJustificationType(juce::Justification::centred);
    rLabel.attachToComponent(&rKnob, false);
    addAndMakeVisible(rLabel);
    
    //Capacitor Knob
    cKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    cKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(cKnob);
    
    cLabel.setText("Capacitor", juce::NotificationType::dontSendNotification);
    cLabel.setJustificationType(juce::Justification::centred);
    cLabel.attachToComponent(&cKnob, false);
    addAndMakeVisible(cLabel);
    
    
}

RCThreeWaysAudioProcessorEditor::~RCThreeWaysAudioProcessorEditor()
{
}

//==============================================================================
void RCThreeWaysAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void RCThreeWaysAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    
    auto knobW = getWidth() * 0.33;
    auto knobH = getHeight() * 0.7;
    auto knobY = (getHeight() * 0.5) - knobH/2;
    
    methodKnob.setBounds((getWidth()*0.20)-knobW/2, knobY, knobW, knobH);
    rKnob.setBounds((getWidth()*0.5)-knobW/2, knobY, knobW, knobH);
    cKnob.setBounds((getWidth()*0.80)-knobW/2, knobY, knobW, knobH);
}
