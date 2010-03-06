#include "UtilityFilters.h"

GainCut::GainCut() : gain(0), gainToRampTo(0)
{
	setPlayConfigDetails (1, 1, 0, 0);
}

void GainCut::fillInPluginDescription(PluginDescription &desc) const
{
	desc.name = "Gain Cut";
	desc.pluginFormatName = "Internal";
	desc.category = "Mixing";
	desc.manufacturerName = "Monkey Fairness Productions";
	desc.version = "0.1";
	desc.fileOrIdentifier = "";
	desc.lastFileModTime = 0;
	desc.uid = 4;
	desc.isInstrument = false;
	desc.numInputChannels = 1;
	desc.numOutputChannels = 1;
}

const String GainCut::getName() const
{
	return T("Gain Cut");
}

void GainCut::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
}

void GainCut::releaseResources()
{
}

void GainCut::processBlock(AudioSampleBuffer &buffer, MidiBuffer &)
{
	buffer.applyGainRamp(0,0,buffer.getNumSamples(), gain, gainToRampTo);
	gain = gainToRampTo;
}

const String GainCut::getInputChannelName(const int) const
{
	return T("Input");
}

const String GainCut::getOutputChannelName(const int) const
{
	return T("Output");
}

bool GainCut::isInputChannelStereoPair(int) const
{
	return false;
}

bool GainCut::isOutputChannelStereoPair(int) const
{
	return false;
}

bool GainCut::acceptsMidi() const
{
	return false;
}

bool GainCut::producesMidi() const
{
	return false;
}

AudioProcessorEditor* GainCut::createEditor()
{
	return 0;
}

int GainCut::getNumParameters()
{
	return 1;
}

const String GainCut::getParameterName(int)
{
	return T("Gain");
}

float GainCut::getParameter(int)
{
	return gain;
}

const String GainCut::getParameterText(int)
{
	return String(gain);
}

void GainCut::setParameter(int, float newGain)
{
	gainToRampTo = newGain;
}

int GainCut::getNumPrograms()
{
	return 0;
}

int GainCut::getCurrentProgram()
{
	return 0;
}

void GainCut::setCurrentProgram(int)
{
}
const String GainCut::getProgramName(int)
{
	return String::empty;
}
void GainCut::changeProgramName(int, const String&)
{
}
void GainCut::getStateInformation(MemoryBlock&)
{
}
void GainCut::setStateInformation(const void *, int)
{
}

