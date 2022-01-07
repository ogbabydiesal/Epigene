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
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor,                                                              public juce::Slider::Listener, public juce::Timer

{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider* slider) override;
    void timerCallback() override;
    void initialize (int size);
    void addToHistory (const juce::Point<float>& point); 
private:
    //juce::ImageComponent mImageComponent;
    juce::ImageButton fImageButton; //filterImage button
    juce::ImageButton pImageButton; //panImage button
    juce::ImageComponent greenBurst;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;
     //the fftSize
    juce::Slider midiVolume[256]; // [1]
    
    std::vector<juce::Point<float>> mouseHistory;
    int writePointer = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
