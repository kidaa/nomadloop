#include "UtilityFilters.h"
#include "../host/MidiDeviceManager.h"
#include "../host/MainHostWindow.h"

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
	desc.lastFileModTime = Time();
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

bool GainCut::hasEditor() const { return false; }

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

// ==============================================

SelectableMidiInputFilter::SelectableMidiInputFilter() : input(0), sampleRate(0)
{
	setPlayConfigDetails (0, 0, 0, 0);
}

void SelectableMidiInputFilter::fillInPluginDescription(PluginDescription &desc) const
{
	desc.name = "Selectable Midi Input";
	desc.pluginFormatName = "Internal";
	desc.category = "Mixing";
	desc.manufacturerName = "Monkey Fairness Productions";
	desc.version = "0.1";
	desc.fileOrIdentifier = "";
	desc.lastFileModTime = Time();
	desc.uid = 4;
	desc.isInstrument = false;
	desc.numInputChannels = 0;
	desc.numOutputChannels = 0;
}

const String SelectableMidiInputFilter::getName() const
{
	return T("Selectable Midi Input");
}

void SelectableMidiInputFilter::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
	this->sampleRate = sampleRate;
	//input = MidiOutput::openDevice(MidiInput);
	//input->startBackgroundThread();
	messageCollector.reset(sampleRate);
	Logger::outputDebugString(T("Prepare to play to default MIDI output: "));
	
	MidiDeviceManager::getInstance()->addCallback(this);
}

void SelectableMidiInputFilter::releaseResources()
{
	//output->stopBackgroundThread();
	//output->reset();
	//delete output;
	MidiDeviceManager::getInstance()->removeCallback(this);
}

void SelectableMidiInputFilter::processBlock(AudioSampleBuffer &audioBuffer, MidiBuffer &buffer)
{
	buffer.clear();
    messageCollector.removeNextBlockOfMessages (buffer, audioBuffer.getNumSamples());
}

const String SelectableMidiInputFilter::getInputChannelName(const int) const
{
	return String::empty;
}

const String SelectableMidiInputFilter::getOutputChannelName(const int) const
{
	return String::empty;
}

bool SelectableMidiInputFilter::isInputChannelStereoPair(int) const
{
	return false;
}

bool SelectableMidiInputFilter::isOutputChannelStereoPair(int) const
{
	return false;
}

bool SelectableMidiInputFilter::acceptsMidi() const
{
	return false;
}

bool SelectableMidiInputFilter::producesMidi() const
{
	return true;
}

bool SelectableMidiInputFilter::hasEditor() const
{
	return false;
}

AudioProcessorEditor* SelectableMidiInputFilter::createEditor()
{
	return 0;
}

int SelectableMidiInputFilter::getNumParameters()
{
	return 1;
}

const String SelectableMidiInputFilter::getParameterName(int)
{
	return T("Input");
}

float SelectableMidiInputFilter::getParameter(int)
{
	return 0.f;
}

const String SelectableMidiInputFilter::getParameterText(int)
{
	return selectedInputName;
}

void SelectableMidiInputFilter::setParameter(int index, float value)
{
	// the value selects one of the active MIDI devices
	StringArray midiInputDevices = MidiInput::getDevices();
	
	int selectedIndex = value*midiInputDevices.size();
	
	selectedInputName = midiInputDevices[selectedIndex];
	dirty = true;
}

int SelectableMidiInputFilter::getNumPrograms()
{
	return 0;
}

int SelectableMidiInputFilter::getCurrentProgram()
{
	return 0;
}

void SelectableMidiInputFilter::setCurrentProgram(int)
{
}
const String SelectableMidiInputFilter::getProgramName(int)
{
	return String::empty;
}
void SelectableMidiInputFilter::changeProgramName(int, const String&)
{
}
void SelectableMidiInputFilter::getStateInformation(MemoryBlock& memBlock)
{
	memBlock.ensureSize(selectedInputName.getNumBytesAsUTF8(), false);
	memBlock.copyFrom(selectedInputName.toUTF8(), 0, selectedInputName.getNumBytesAsUTF8());
}
void SelectableMidiInputFilter::setStateInformation(const void *block, int size)
{
	selectedInputName = String::fromUTF8((const char*)block, size);
	dirty = true;
	updateHostDisplay();
}

// ==============================================

DefaultMidiOutputFilter::DefaultMidiOutputFilter() : output(0), sampleRate(0)
{
	setPlayConfigDetails (0, 0, 0, 0);
}

void DefaultMidiOutputFilter::fillInPluginDescription(PluginDescription &desc) const
{
	desc.name = "Default Midi Output";
	desc.pluginFormatName = "Internal";
	desc.category = "Mixing";
	desc.manufacturerName = "Monkey Fairness Productions";
	desc.version = "0.1";
	desc.fileOrIdentifier = "";
	desc.lastFileModTime = Time();
	desc.uid = 4;
	desc.isInstrument = false;
	desc.numInputChannels = 0;
	desc.numOutputChannels = 0;
}

const String DefaultMidiOutputFilter::getName() const
{
	return T("Default Midi Output");
}

void DefaultMidiOutputFilter::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
	this->sampleRate = sampleRate;
	output = MidiOutput::openDevice(MidiOutput::getDefaultDeviceIndex());

	if (output != 0)
		output->startBackgroundThread();
	Logger::outputDebugString(T("Prepare to play to default MIDI output: "));
}

void DefaultMidiOutputFilter::releaseResources()
{
	if (output != 0)
	{
		output->stopBackgroundThread();
		output->clearAllPendingMessages();
		delete output;
	}
}

void DefaultMidiOutputFilter::processBlock(AudioSampleBuffer &, MidiBuffer &buffer)
{
	if (output != 0)
	{
		MidiMessage msg(0x80,60,0);
		int sample;
		MidiBuffer::Iterator itor(buffer);

		while (itor.getNextEvent(msg, sample))
		{
			output->sendMessageNow(msg);
		}
		//output->sendBlockOfMessages(buffer, 1, sampleRate);
	}
}

const String DefaultMidiOutputFilter::getInputChannelName(const int) const
{
	return String::empty;
}

const String DefaultMidiOutputFilter::getOutputChannelName(const int) const
{
	return String::empty;
}

bool DefaultMidiOutputFilter::isInputChannelStereoPair(int) const
{
	return false;
}

bool DefaultMidiOutputFilter::isOutputChannelStereoPair(int) const
{
	return false;
}

bool DefaultMidiOutputFilter::acceptsMidi() const
{
	return true;
}

bool DefaultMidiOutputFilter::producesMidi() const
{
	return false;
}

bool DefaultMidiOutputFilter::hasEditor() const
{
	return false;
}

AudioProcessorEditor* DefaultMidiOutputFilter::createEditor()
{
	return 0;
}

int DefaultMidiOutputFilter::getNumParameters()
{
	return 0;
}

const String DefaultMidiOutputFilter::getParameterName(int)
{
	return String::empty;
}

float DefaultMidiOutputFilter::getParameter(int)
{
	return 0.f;
}

const String DefaultMidiOutputFilter::getParameterText(int)
{
	return String::empty;
}

void DefaultMidiOutputFilter::setParameter(int, float newGain)
{
}

int DefaultMidiOutputFilter::getNumPrograms()
{
	return 0;
}

int DefaultMidiOutputFilter::getCurrentProgram()
{
	return 0;
}

void DefaultMidiOutputFilter::setCurrentProgram(int)
{
}
const String DefaultMidiOutputFilter::getProgramName(int)
{
	return String::empty;
}
void DefaultMidiOutputFilter::changeProgramName(int, const String&)
{
}
void DefaultMidiOutputFilter::getStateInformation(MemoryBlock&)
{
}
void DefaultMidiOutputFilter::setStateInformation(const void *, int)
{
}

