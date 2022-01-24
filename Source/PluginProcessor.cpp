/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), forwardFFT(8), inverseFFT(8), window (fftSize, juce::dsp::WindowingFunction<float>::hann, false)
#endif
{
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    auto circBufferSize = 4196;
    circBuffer.setSize (getTotalNumOutputChannels(), (int)circBufferSize);
    OcircBuffer.setSize (getTotalNumOutputChannels(), (int)circBufferSize);
    OwritePosition = hopSize*2;
    //testRead = 0; //test only
    std::fill(binAmps + 0, binAmps + fftSize, 1);
    
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    auto bufferSize = buffer.getNumSamples();
    auto circBufferSize = circBuffer.getNumSamples();
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        bufferFiller(channel, bufferSize, circBufferSize, channelData, hopSize, buffer);
        for (int sample = 0; sample < bufferSize; ++sample)
        {
            channelData[sample] = (OcircBuffer.getSample(channel, (OreadPosition + sample) % circBufferSize) * .5f);
            OcircBuffer.setSample(channel, (OreadPosition + sample) % circBufferSize, 0.0f);
            
            
        }
        OreadPosition = (OreadPosition + bufferSize) % circBufferSize;
        
    }
}

void NewProjectAudioProcessor::bufferFiller(int channel, int bufferSize, int circBufferSize, float* channelData, int hopSize, juce::AudioBuffer<float>& buffer)
{
    
    for (int x = 0; x < bufferSize; x ++)
    {
        circBuffer.copyFrom(channel, writePosition, channelData + x, 1);
        //DBG(circBuffer.getSample(channel, writePosition));
        writePosition = (writePosition + 1) % circBufferSize;
        hopCounter(channel, bufferSize, circBufferSize);
        
    }
    //writePosition = (writePosition + bufferSize) % circBufferSize;
    /*
    if (circBufferSize - writePosition >= fftSize)
    {
        circBuffer.copyFrom(channel, writePosition, channelData, bufferSize);
    }
    else
    {
        samplesToEnd = circBufferSize - writePosition;
        samplesatBeg = bufferSize - samplesToEnd;
        circBuffer.copyFrom(channel, writePosition, channelData, samplesToEnd);
        circBuffer.copyFrom(channel, 0, channelData + samplesToEnd, samplesatBeg);
    }
     
    writePosition = (writePosition + bufferSize) % circBufferSize;
    //what to do about hopCounter
    for (int x = 0; x < bufferSize; x++)
    {
        hopCounter(channel, bufferSize, circBufferSize);
    }
    */
    
    
}

void NewProjectAudioProcessor::hopCounter(int channel, int bufferSize, int circBufferSize)
{
    //DBG(hopCount);
    if(++hopCount >= hopSize)
        
    {
        
        //start the spectral processing
        spectralShit(channel, bufferSize, circBufferSize, OwritePosition, OcircBuffer);
        //after spectral processing increase output buffer write pointer one hop-size
        OwritePosition = (OwritePosition + hopSize) % circBufferSize;
        hopCount = 0;
    }
}
void NewProjectAudioProcessor::spectralShit(int channel, int bufferSize, int circBufferSize, int OwritePosition, juce::AudioBuffer<float>& OcircBuffer)
{
    //get most recent fftsize of samples using modulo indexing and store them in a buffer
    for (int x = 0; x < fftSize; x++)
    {
       
        chunkOne[x] = circBuffer.getSample(channel, (writePosition + x - fftSize + circBufferSize) % circBufferSize);
    }
    //window the buffer
    //window.multiplyWithWindowingTable(chunkOne, fftSize);
    
    for (int x = 0; x < fftSize; ++x)
    {
        //fftBuffer is 2x the fftSize, so we really only fill half the fftBuffer with this for-loop
        fftBuffer[x] = chunkOne[x];
        
    }
    //compute the fft of the time domain signals and store in that same buffer
    //forwardFFT.performRealOnlyForwardTransform(fftBuffer, true);
    
    //do processing in the frequency domain here
    
    for (int x = 0; x < fftSize; ++x)
    {
        //fftBuffer[x] *= binAmps[x]; //simple spectral filter;
        //DBG ("bin is " + std::to_string(x) + " and value is " + std::to_string(binAmps[x]));
    }
    //compute the ifft on that buffer
    //first half of the inverse fft is our reconstituted values
    //inverseFFT.performRealOnlyInverseTransform(fftBuffer);
    
    //unwrap and ADD this fftSize of samples into an output buffer
    
    for (int x = 0; x < fftSize; ++x)
    {
        //unwrap into output buffer use some modulo stuff
        
        OcircBuffer.addSample(channel, (OwritePosition + x) % circBufferSize, fftBuffer[x]);
    }
    /*
    if ( circBufferSize - OwritePosition >= fftSize)
    {
        OcircBuffer.addFrom(channel, OwritePosition, fftBuffer, fftSize);
    }
    else
    {
        samplesToEnd = circBufferSize - OwritePosition;
        samplesatBeg = fftSize - samplesToEnd;
        OcircBuffer.addFrom(channel, OwritePosition, fftBuffer, samplesToEnd);
        OcircBuffer.addFrom(channel, 0, fftBuffer + samplesToEnd, samplesatBeg);
    }
     */
}
//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
