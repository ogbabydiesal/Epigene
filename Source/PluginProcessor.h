/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void process (juce::dsp::ProcessContextReplacing<float>context);
    void updateParameters();
    float binAmps[256];

private:
    void bufferFiller(int channel, int bufferSize, int circBufferSize, float* channelData, int hopSize, juce::AudioBuffer<float>& buffer, int chunkTwoSize);
    void spectralShit(int channel, int bufferSize, int circBufferSize, int OwritePosition, juce::AudioBuffer<float>& OcircBuffer);
    void hopCounter(int channel, int bufferSize, int circBufferSize);
    juce::AudioBuffer<float> circBuffer; //input circular buffer
    juce::AudioBuffer<float> OcircBuffer; //output circular buffer
    juce::AudioBuffer<float> chunkTwo; //FFT processing container
    
    int writePosition {0}; //input buffer write position
    int OwritePosition {0}; //output buffer write position
    int OreadPosition {0}; //output buffer read position
    juce::dsp::FFT forwardFFT;
    juce::dsp::FFT inverseFFT;
    int fftSize = 256;
    int hopSize = fftSize / 2;
    int hopCount = 0;
    float chunkOne [256];
    float fftBuffer [512]; //twice fftSize to store mirror image
    //float binValues [256];
    juce::String fftSizeStr = "";
    juce::dsp::WindowingFunction<float> window;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
