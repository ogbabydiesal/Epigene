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
    setSize (640, 320);
    juce::Timer::startTimerHz(60.0f);
    initialize(10);
    //reference to normal button image
    auto buttonImage = juce::ImageCache::getFromMemory (BinaryData::button_png, BinaryData::button_pngSize);
    auto burstImg = juce::ImageCache::getFromMemory (BinaryData::greenThing_png, BinaryData::greenThing_pngSize);
    //random size for green gradient
    auto randomSize = juce::Random::getSystemRandom().nextInt (juce::Range<int> (90, 190));
    //rando placement x and y for green gradient
    auto randomX = juce::Random::getSystemRandom().nextInt (juce::Range<int> (45, 600));
    auto randomY = juce::Random::getSystemRandom().nextInt (juce::Range<int> (45, 280));
    //Filter Focus Button
    //normal, over down
    greenBurst.setImage(burstImg);
    greenBurst.toFront(true);
    //greenBurst.setSize(45 + randomSize, 45 + randomSize);
    greenBurst.setBounds(randomX, randomY, 45 + randomSize, 45 + randomSize);
    addAndMakeVisible(greenBurst);
    fImageButton.setImages(true, true, true, buttonImage, 1, juce::Colour::fromFloatRGBA(0,0,0,0), buttonImage, 1, juce::Colour::fromFloatRGBA(0,0,0,0), buttonImage, 1, juce::Colour::fromFloatRGBA(0,0,0,0));
    fImageButton.setSize(65, 65);
    addAndMakeVisible(fImageButton);
    
    //Pan Focus Button
    pImageButton.setImages(true, true, true, buttonImage, 1, juce::Colour::fromFloatRGBA(0,0,0,0), buttonImage, 1, juce::Colour::fromFloatRGBA(0,0,0,0), buttonImage, 1, juce::Colour::fromFloatRGBA(0,0,0,0));
    pImageButton.setSize(65, 65);
    addAndMakeVisible(pImageButton);
}
NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

//================ DRAW UI & UPDATE FILTER VALUES ===============================
void NewProjectAudioProcessorEditor::initialize (int size)
{
    mouseHistory.resize(size);
    std::fill(mouseHistory.begin(), mouseHistory.end() - 1, juce::Point<float> (0,0));
}

void NewProjectAudioProcessorEditor::addToHistory(const juce::Point<float>& point)
{
    mouseHistory[writePointer] = point;
    writePointer = (writePointer + 1) % mouseHistory.size();
}

void NewProjectAudioProcessorEditor::uiToFilter(juce::Point<float> mousePosition)
{
    auto OldValue = mousePosition.getX();
    auto OldValueY = mousePosition.getY();
    auto OldRange = (width - widthMin);
    auto NewRange = (NewMax - NewMin);
    NewValue = (((OldValue - widthMin) * NewRange) / OldRange) + NewMin;
    if (OldValueY < 0)
    {
        OldValueY = 0;
    }
    else if (OldValueY > 320)
    {
        OldValueY = 320;
    }
    if (NewValue < 0)
    {
        NewValue = 0;
    }
    else if (NewValue > NewMax - 1)
    {
        NewValue = NewMax -1;
    }
    
    float binVal = (OldValueY) / (320);
    
    if (NewValue >= 0 && NewValue < NewMax)
    {
        audioProcessor.binAmps[NewValue] = (1.0 - binVal);
        
    }
}

//==============================================================================

void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::transparentBlack);
    //g.setColour (juce::Colours::palegreen);
    g.setColour(juce::Colours::rebeccapurple);
    g.setFont (15.0f);
    g.drawFittedText ("haha", 0, 0, getWidth(), 50, juce::Justification::topRight, 1);
    
    auto position = getMouseXYRelative();
    
    //change mouse position int to float
    juce::Point<float> mousePosition { static_cast<float>(position.getX()), static_cast<float>(position.getY())};
    
    uiToFilter(mousePosition);
    g.drawFittedText (std::to_string(NewValue), 0, 0, getWidth(), 30, juce::Justification::bottomRight, 1);
    addToHistory(mousePosition);
    //color of line and path object
   
    juce::Path myPath;
    
    auto oldestPosition = writePointer;
    
    myPath.startNewSubPath (mouseHistory[oldestPosition]);
   
    for (int i =0; i < mouseHistory.size(); i++)
    {
        auto currentPosition = (writePointer + i) % mouseHistory.size();
        myPath.lineTo(mouseHistory[currentPosition]);
    }
    
    g.strokePath (myPath, juce::PathStrokeType (5.0, juce::PathStrokeType::JointStyle::curved, juce::PathStrokeType::EndCapStyle::rounded ));
}

void NewProjectAudioProcessorEditor::timerCallback()
{
    repaint();
}

void NewProjectAudioProcessorEditor::resized()
{
    fImageButton.setBounds(10, 10, 0, 0);
    pImageButton.setBounds(20, 45, 0, 0);
    
}
