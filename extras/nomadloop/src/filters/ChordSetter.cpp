#include "ChordSetter.h"
#include <cmath>

ChordSetter::ChordSetter()
{
	setPlayConfigDetails (1, 1, 0, 0);
}

void ChordSetter::fillInPluginDescription(PluginDescription &desc) const
{
	desc.name = "ChordSetter";
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

const String ChordSetter::getName() const
{
	return T("Chord Setter");
}

void ChordSetter::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
	//this->sampleRate = sampleRate;
	setPlayConfigDetails(0, 0, sampleRate, estimatedSamplesPerBlock);
}

void ChordSetter::releaseResources()
{
}

void ChordSetter::processBlock(AudioSampleBuffer &sampleBuffer, MidiBuffer &midiBuffer)
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
}

const String ChordSetter::getInputChannelName(const int) const
{
	return T("Input");
}

const String ChordSetter::getOutputChannelName(const int) const
{
	return T("Output");
}

bool ChordSetter::isInputChannelStereoPair(int) const
{
	return false;
}

bool ChordSetter::isOutputChannelStereoPair(int) const
{
	return false;
}

bool ChordSetter::acceptsMidi() const
{
	return true;
}

bool ChordSetter::producesMidi() const
{
	return false;
}

AudioProcessorEditor* ChordSetter::createEditor()
{
	return new ChordSetterEditor(this);
}

int ChordSetter::getNumParameters()
{
	return 0;
}

const String ChordSetter::getParameterName(int)
{
	return String::empty;
}

float ChordSetter::getParameter(int)
{
	return 0.f;
}

const String ChordSetter::getParameterText(int)
{
	return String::empty;	
}

void ChordSetter::setParameter(int index, float value)
{
}

int ChordSetter::getNumPrograms()
{
	return 1;
}

int ChordSetter::getCurrentProgram()
{
	return 0;
}

void ChordSetter::setCurrentProgram(int)
{
}
const String ChordSetter::getProgramName(int)
{
	return String::empty;
}

void ChordSetter::changeProgramName(int, const String&)
{
}

void ChordSetter::getStateInformation(MemoryBlock&)
{
}

void ChordSetter::setStateInformation(const void *, int)
{
}


ChordSetterEditor::ChordSetterEditor(ChordSetter* const owner)
: AudioProcessorEditor(owner)
{
	setSize(256, 128);

	this->startTimer(1000/45);
}

ChordSetterEditor::~ChordSetterEditor()
{
	// TODO: this call shouldn't be necessary... debug
	this->getAudioProcessor()->editorBeingDeleted(this);
}

 
void ChordSetterEditor::changeListenerCallback (void* source)
{
}

void ChordSetterEditor::sliderValueChanged (Slider*)
{
}

void ChordSetterEditor::timerCallback()
{
	repaint();
}

void ChordSetterEditor::paint (Graphics& g)
{
	g.fillAll(Colours::dodgerblue);

	ChordSetter* setter = getFilter();

	setter->getCallbackLock().enter();

	String notes(T("Notes: "));
	for (int i=0; i<setter->activeInputNotes.size(); ++i)
	{
		notes << T(",") << setter->activeInputNotes.getUnchecked(i);
	}
	g.setColour(Colours::white);
	g.drawText(notes, 0, 0, getWidth(), 16, Justification::centredLeft, true);
	
	setter->getCallbackLock().exit();
}

void ChordSetterEditor::resized()
{
}

