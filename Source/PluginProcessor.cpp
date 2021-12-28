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
                       ), forwardFFT(8), inverseFFT(8), window (fftSize, juce::dsp::WindowingFunction<float>::hann)
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
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    auto circBufferSize = 1024;
    circBuffer.setSize (getTotalNumOutputChannels(), (int)circBufferSize);
    
    //creates a buffer not sure if we need this yet...
    auto chunkTwoSize = fftSize;
    chunkTwo.setSize (getTotalNumOutputChannels(), (int)fftSize);
    
    OwritePosition = hopSize;
    OcircBuffer.setSize (getTotalNumOutputChannels(), (int)samplesPerBlock);
    //chunkOne.setSize(getTotalNumOutputChannels(), (int)fftSize);
    //chunkTwo.setSize(getTotalNumOutputChannels(), (int)fftSize);
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
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
    auto chunkTwoSize = chunkTwo.getNumSamples();
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        //writes sample blocks into a circular buffer from The Audio Programmer Tutorial 15
        bufferFiller(channel, bufferSize, circBufferSize, channelData, hopSize, buffer, chunkTwoSize);
        //process the fft on an arbitrary block size (i.e. not necessarily process block size)
        //spectralShit(channel, bufferSize, circBufferSize, chunkTwoSize);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = channelData[sample] * binAmps[0];
            //channelData[sample] = fftBuffer[sample] ;
        }
        //send samples out to output buffer
        juce::dsp::AudioBlock<float> block (buffer);
        
    }
}

void NewProjectAudioProcessor::bufferFiller(int channel, int bufferSize, int circBufferSize, float* channelData, int hopSize, juce::AudioBuffer<float>& buffer, int chunkTwoSize)
{
    // Check to see if main buffer copies to circ buffer without needing to wrap...
    for (int x = 0; x < bufferSize; ++x) {
        circBuffer.copyFromWithRamp (channel, writePosition, channelData + x, 1, 0.1f, 0.1f);
        //hopCounter(channel, bufferSize, circBufferSize, chunkTwoSize);
        if (++writePosition >= circBufferSize)
        {
            writePosition = 0;
        }
    }
}

void NewProjectAudioProcessor::hopCounter(int channel, int bufferSize, int circBufferSize, int chunkTwoSize)
{
    if(++hopCount > hopSize)
    {
        hopCount = 0;
        //DBG ("i triggered");
        //spectralShit(channel, bufferSize, circBufferSize, chunkTwoSize, OwritePosition, OcircBuffer);
        
        OwritePosition = (OwritePosition + hopSize) % bufferSize;
        //DBG (writePosition);
    }
    //DBG (hopCount);
    //DBG (writePosition);
    //DBG ("circbuffersize = " + std::to_string(circBufferSize));
}
void NewProjectAudioProcessor::spectralShit(int channel, int bufferSize, int circBufferSize, int chunkTwoSize, int OwritePosition, juce::AudioBuffer<float>& OcircBuffer)
{
    
    //get most recent fftsize of samples and store them in a windowed buffer
    //if we can access the most recent (fftSize) num of samples from the circbuffer without having to wrap around to the end of the circbuffer
    for (int x = 0; x < fftSize; x++)
    {
        //chunkTwo.copyFromWithRamp(channel, 0, (circBuffer - fftSize)  , <#int numSamples#>, <#float startGain#>, <#float endGain#>)
    }
    if (writePosition - fftSize >= 0) //can this also just be if(writePosition > fftSize)?
    //if (chunkTwoSize > bufferSize + writePosition)
    {
        for (int n = 0; n < fftSize; ++n)
        {
            //syntax if chunkOne is an Array not a Buffer <-- i think this will work better for windowing and the fft stuff
            chunkOne[n] = circBuffer.getSample(channel, (writePosition - fftSize) + n);
            /*chunkOne.setSample(channel, n, circBuffer.getSample(channel, (writePosition - fftSize) + n));
             */
        }
        
    }
    else
    {
        //find out how we need to break it up
        int sampsAtEnd = fftSize - writePosition;
        //create two for loops that fill up the buffer
            //first the samps at the end of the circbuffer (before the wrap - as those are earliest)
        
        for (int n = 0; n < sampsAtEnd; n++)
        {
            chunkOne[n] = circBuffer.getSample(channel, (circBufferSize - sampsAtEnd) + n);
            /*chunkOne.setSample(channel, n, circBuffer.getSample(channel, (circBufferSize - sampsAtEnd) + n));
            */
        }
            //then the values at the beginning of the buffer, as these have come later
        for (int n = 0; n < fftSize - sampsAtEnd; n++)
        {
            chunkOne[n + sampsAtEnd] = circBuffer.getSample(channel, n);
            /*chunkOne.setSample(channel, n + sampsAtEnd, circBuffer.getSample(channel, n));
             */
        }
        //window the chunk
        
        window.multiplyWithWindowingTable(chunkOne, fftSize);
    }
    //copy the windowed chunk into the fftArray
    for (int x = 0; x < fftSize; ++x)
    {
        fftBuffer[x] = chunkOne[x];
        //std::cout << fftBuffer[x] << std::endl;
    }
    //compute the fft on that buffer
    forwardFFT.performRealOnlyForwardTransform(fftBuffer, true);
    //computer the ifft on that buffer
    //first half of the inverse are our reconstituted values
    inverseFFT.performRealOnlyInverseTransform(fftBuffer);
    for (int x = 0; x < 512; x ++) {
        
    }
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
