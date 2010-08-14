#include "MidiUtilityFilter.h"
#include <cmath>

MidiUtilityFilter::MidiUtilityFilter()
: pitchBendRange(1), octaveShift(0), triggerSend(true)
{
	setPlayConfigDetails (1, 1, 0, 0);
}

void MidiUtilityFilter::fillInPluginDescription(PluginDescription &desc) const
{
	desc.name = "Midi Utility";
	desc.pluginFormatName = "Internal";
	desc.category = "Midi Effects";
	desc.manufacturerName = "Zark";
	desc.version = "0.1";
	desc.fileOrIdentifier = "";
	desc.lastFileModTime = 0;
	desc.uid = 4;
	desc.isInstrument = false;
	desc.numInputChannels = 0;
	desc.numOutputChannels = 0;
}

const String MidiUtilityFilter::getName() const
{
	return T("Midi Utility");
}

void MidiUtilityFilter::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
	//this->sampleRate = sampleRate;
	setPlayConfigDetails(0, 0, sampleRate, estimatedSamplesPerBlock);
}

void MidiUtilityFilter::releaseResources()
{
}

void MidiUtilityFilter::processBlock(AudioSampleBuffer &sampleBuffer, MidiBuffer &midiBuffer)
{
	MidiBuffer::Iterator itor(midiBuffer);
	MidiMessage message(0x80, 1, 1);
	int samplePos;
	
	MidiBuffer outputBuffer;
	outputBuffer.ensureSize(midiBuffer.getNumEvents());

	while (itor.getNextEvent(message, samplePos))
	{
		if (message.isNoteOnOrOff())
		{
			message.setNoteNumber(message.getNoteNumber() + 12*octaveShift);
		}
		outputBuffer.addEvent(message, samplePos);
	}
	
	midiBuffer.swapWith(outputBuffer);
	
	if (triggerSend)
	{
		for (int c=0; c<15; ++c)
		{
			// program change
			midiBuffer.addEvent(MidiMessage::programChange(c, currentProgram), 0);
			
			// Update pitch bend range
			midiBuffer.addEvent(MidiMessage::controllerEvent(c, 101, 0), 0);
			midiBuffer.addEvent(MidiMessage::controllerEvent(c, 100, 0), 0);
			midiBuffer.addEvent(MidiMessage::controllerEvent(c, 6, (int)(pitchBendRange*12.5)), 0);
			midiBuffer.addEvent(MidiMessage::controllerEvent(c, 38, 0), 0);
			midiBuffer.addEvent(MidiMessage::controllerEvent(c, 101, 127), 0);
			midiBuffer.addEvent(MidiMessage::controllerEvent(c, 100, 127), 0);
			
			// turn off any older notes
			midiBuffer.addEvent(MidiMessage::allNotesOff(c), 0);
		}
		
		triggerSend = false;
	}
}

const String MidiUtilityFilter::getInputChannelName(const int) const
{
	return T("Input");
}

const String MidiUtilityFilter::getOutputChannelName(const int) const
{
	return T("Output");
}

bool MidiUtilityFilter::isInputChannelStereoPair(int) const
{
	return false;
}

bool MidiUtilityFilter::isOutputChannelStereoPair(int) const
{
	return false;
}

bool MidiUtilityFilter::acceptsMidi() const
{
	return true;
}

bool MidiUtilityFilter::producesMidi() const
{
	return true;
}

AudioProcessorEditor* MidiUtilityFilter::createEditor()
{
	return 0;
}

int MidiUtilityFilter::getNumParameters()
{
	return 3;
}

const String MidiUtilityFilter::getParameterName(int i)
{
	if (i == 0)
		return T("Pitch Bend Range");
	else if (i == 1)
		return T("Octave shift");
	else if (i == 2)
		return T("Immediate Program Change");
	return String::empty;
}

float MidiUtilityFilter::getParameter(int i)
{
	if (i == 0)
		return pitchBendRange;
	if (i == 1)
		return (octaveShift+2.5f)/5.0f;
	if (i == 2)
		return currentProgram/127.0f;
	return 0.f;
}

const String MidiUtilityFilter::getParameterText(int i)
{
	if (i == 0)
		return (T("+/-") + String((int)(pitchBendRange * 12.5)));
	else if (i == 1)
		return (String(octaveShift));
	else if (i == 2)
		return (String(currentProgram));
	return String::empty;
}

void MidiUtilityFilter::setParameter(int index, float value)
{
	if (index == 0)
	{
		pitchBendRange = value;
		triggerSend = true;
	}
	else if (index == 1)
	{
		octaveShift = static_cast<int>(value*5.0f-2.5f);
		triggerSend = true;
	}
	else if (index == 2)
	{
		currentProgram = static_cast<int>(value * 127.0f);
		triggerSend = true;
	}
}

int MidiUtilityFilter::getNumPrograms()
{
	return 1;
}

int MidiUtilityFilter::getCurrentProgram()
{
	return 0;
}

void MidiUtilityFilter::setCurrentProgram(int)
{
}
const String MidiUtilityFilter::getProgramName(int)
{
	return String::empty;
}

void MidiUtilityFilter::changeProgramName(int, const String&)
{
}

void MidiUtilityFilter::getStateInformation(MemoryBlock&)
{
}

void MidiUtilityFilter::setStateInformation(const void *, int)
{
}

