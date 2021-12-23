/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (640, 300);
    for (int x = 0; x < 64; x++)
    {
        // these define the parameters of our slider object
        midiVolume[x].setSliderStyle (juce::Slider::LinearBarVertical);
        midiVolume[x].setRange (0.0, 1, .01);
        midiVolume[x].setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
        //midiVolume[x].setPopupDisplayEnabled (true, false, this);
        //midiVolume[x].setTextValueSuffix (std::to_string(x) + " Volume");
        midiVolume[x].setValue(1.0);
        midiVolume[x].addListener (this);
        addAndMakeVisible (&midiVolume[x]);
    }
    
}

void NewProjectAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    for (int x = 0; x < 64; x++)
    {
        audioProcessor.binAmps[x] = midiVolume[x].getValue();
    }
    
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::blueviolet);
    
    g.setColour (juce::Colours::beige);
    g.setFont (15.0f);
    g.drawFittedText ("Spectral Filter", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
/*
    
*/
    
    
    
    
}

void NewProjectAudioProcessorEditor::resized()
{
    
    for (int x = 0; x < 64; x++)
    {
        midiVolume[x].setBounds ((x * 10), 30, 8, getHeight() - 60);
        // This is generally where you'll want to lay out the positions of any
        // subcomponents in your editor..
    }
    
    
    //midiVolume.setBounds (0 + 2, 30, 10.5, getHeight() - 60);
}
