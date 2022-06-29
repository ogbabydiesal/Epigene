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
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer

{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //void sliderValueChanged (juce::Slider* slider) override;
    void timerCallback() override;
    void initialize (int size);
    void addToHistory (const juce::Point<float>& point);
    void uiToFilter(juce::Point<float> mousePosition);
    void MouseDown(const juce::MouseEvent &event);
    void MouseEnter(const juce::MouseEvent &event);
    void MouseExit(const juce::MouseEvent &event);
    
    bool isEntered {false};
private:
    //juce::ImageComponent mImageComponent;
    juce::ImageButton fImageButton; //filterImage button
    juce::ImageButton pImageButton; //panImage button
    juce::ImageComponent greenBurst;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;
    
    
    std::vector<juce::Point<float>> mouseHistory;
    int writePointer = 0;
    int width = 640;
    //OldMin
    int widthMin = 0;
    int NewMax = audioProcessor.fftSize;
    //int NewMax = 256;
    int NewMin = 0;
    int NewValue = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
