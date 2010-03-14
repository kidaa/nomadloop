#include "Arpeggiator.h"
#include <cmath>

Arpeggiator::Arpeggiator() : rate(0), outputIndex(0)
{
	setPlayConfigDetails (0, 0, 0, 0);
}

void Arpeggiator::fillInPluginDescription(PluginDescription &desc) const
{
	desc.name = "Arpeggiator";
	desc.pluginFormatName = "Internal";
	desc.category = "Midi Effects";
	desc.manufacturerName = "Monkey Fairness Productions";
	desc.version = "0.1";
	desc.fileOrIdentifier = "";
	desc.lastFileModTime = 0;
	desc.uid = 4;
	desc.isInstrument = false;
	desc.numInputChannels = 0;
	desc.numOutputChannels = 0;
}

const String Arpeggiator::getName() const
{
	return T("Arpeggiator");
}

void Arpeggiator::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
	this->sampleRate = sampleRate;
}

void Arpeggiator::releaseResources()
{
}

void Arpeggiator::processBlock(AudioSampleBuffer &sampleBuffer, MidiBuffer &midiBuffer)
{
	MidiBuffer::Iterator itor(midiBuffer);
	MidiMessage message(0x80, 1, 1);
	int samplePos;

	while (itor.getNextEvent(message, samplePos))
	{
		if (message.isNoteOn())
		{
			activeInputNotes.addIfNotAlreadyThere(message.getNoteNumber());
		}
		else if (message.isNoteOff())
		{
			activeInputNotes.removeValue(message.getNoteNumber());
		}
	}
	
	AudioPlayHead* playHead = getPlayHead();

	if (activeInputNotes.size() > 0 && playHead != 0)
	{
		midiBuffer.clear();
		AudioPlayHead::CurrentPositionInfo pos;

		playHead->getCurrentPosition(pos);

		int samplesPerArpIncrement = (sampleRate * 60) / (16*pos.bpm);

		double samplesLeftInBuffer = sampleBuffer.getNumSamples();

		// sixteenth notes
		double ppqPerArpNote = 1.0/4.0;

		// calculate the sample position for the first playing note at this edge
		double ppqOfNextArpNote = std::ceil(pos.ppqPosition / ppqPerArpNote) * ppqPerArpNote;
		int samplePosition = 0.5 + (sampleRate * 60/pos.bpm)*(ppqOfNextArpNote - pos.ppqPosition);

		if (activeOutputNoteCount > 0 && activeInputNotes.size() == 0)
		{
			// some notes were still playing last frame, so turn them off now			
			midiBuffer.addEvent(MidiMessage::allNotesOff(1), samplePosition);
		}
		else while (samplePosition < sampleBuffer.getNumSamples())
		{
			// turn off old notes			
			//midiBuffer.addEvent(MidiMessage::noteOff(1, activeInputNotes.getUnchecked(outputIndex)), samplePosition);
			midiBuffer.addEvent(MidiMessage::allNotesOff(1), samplePosition);

			outputIndex = (outputIndex + 1) % (activeInputNotes.size());

			// move to next piece in the pattern, turn on those notes
			midiBuffer.addEvent(MidiMessage::noteOn(1, activeInputNotes.getUnchecked(outputIndex), 90.0f), samplePosition);

			//NomadJuce::Logger::outputDebugPrintf(T("Played note %i at %i"), outputIndex, samplePosition);

			samplePosition += samplesPerArpIncrement;
		}

		//NomadJuce::Logger::outputDebugPrintf(T("step samples: %i\t\tbuffersize: %i\tovershot buffer by: %i"), samplesPerArpIncrement, sampleBuffer.getNumSamples(), (samplePosition - sampleBuffer.getNumSamples()));		
	}

	activeOutputNoteCount = activeInputNotes.size();

}

const String Arpeggiator::getInputChannelName(const int) const
{
	return T("Input");
}

const String Arpeggiator::getOutputChannelName(const int) const
{
	return T("Output");
}

bool Arpeggiator::isInputChannelStereoPair(int) const
{
	return false;
}

bool Arpeggiator::isOutputChannelStereoPair(int) const
{
	return false;
}

bool Arpeggiator::acceptsMidi() const
{
	return true;
}

bool Arpeggiator::producesMidi() const
{
	return true;
}

AudioProcessorEditor* Arpeggiator::createEditor()
{
	return 0;
}

int Arpeggiator::getNumParameters()
{
	return 1;
}

const String Arpeggiator::getParameterName(int)
{
	return T("Rate");
}

float Arpeggiator::getParameter(int)
{
	return rate;
}

const String Arpeggiator::getParameterText(int)
{
	if (rate < 0.25f)
		return T("1/4");
	else if (rate < 0.5f)
		return T("1/8");
	else if (rate < 0.75f)
		return T("1/16");
	else
		return T("1/32");
	
}

void Arpeggiator::setParameter(int index, float value)
{
	rate = value;
}

int Arpeggiator::getNumPrograms()
{
	return 0;
}

int Arpeggiator::getCurrentProgram()
{
	return 0;
}

void Arpeggiator::setCurrentProgram(int)
{
}
const String Arpeggiator::getProgramName(int)
{
	return String::empty;
}

void Arpeggiator::changeProgramName(int, const String&)
{
}

void Arpeggiator::getStateInformation(MemoryBlock&)
{
}

void Arpeggiator::setStateInformation(const void *, int)
{
}

