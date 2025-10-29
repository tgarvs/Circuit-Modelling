/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RCThreeWaysAudioProcessor::RCThreeWaysAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", create_params())
#endif
{
}

RCThreeWaysAudioProcessor::~RCThreeWaysAudioProcessor()
{
}

//==============================================================================
const juce::String RCThreeWaysAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RCThreeWaysAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RCThreeWaysAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RCThreeWaysAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RCThreeWaysAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RCThreeWaysAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RCThreeWaysAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RCThreeWaysAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RCThreeWaysAudioProcessor::getProgramName (int index)
{
    return {};
}

void RCThreeWaysAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RCThreeWaysAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void RCThreeWaysAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RCThreeWaysAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RCThreeWaysAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    
    
    auto meth {apvts.getRawParameterValue("METHOD") -> load()};
    auto res {apvts.getRawParameterValue("RESISTOR") -> load()};
    auto cap {apvts.getRawParameterValue("CAPACITOR") -> load()};
//    std::cout << meth << std::endl;
    
    

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    
    
}

//==============================================================================
bool RCThreeWaysAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RCThreeWaysAudioProcessor::createEditor()
{
    return new RCThreeWaysAudioProcessorEditor (*this);
}

//==============================================================================
void RCThreeWaysAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RCThreeWaysAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RCThreeWaysAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout RCThreeWaysAudioProcessor::create_params(){
    
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterInt> (juce::ParameterID("METHOD", 1), "capacitor", 1, 3, 2));
    params.push_back(std::make_unique<juce::AudioParameterInt> (juce::ParameterID("RESISTOR", 2), "resistor", 0, 20000, 1000));
    params.push_back(std::make_unique<juce::AudioParameterInt> (juce::ParameterID("CAPACITOR", 2), "capacitor", 0, 20000, 1000));
    return {params.begin(), params.end()};
}
