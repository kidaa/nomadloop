#include "Looper.h"

LoopProcessor::LoopProcessor()
{
}

LoopProcessor::~LoopProcessor()
{
}

// ==========================

MidiLoopProcessor::MidiLoopProcessor()
: recordingCued(false), recording(false), sampleLength(0), sampleScrub(0)
{
	setPlayConfigDetails (1, 1, 0, 0);
}

void MidiLoopProcessor::fillInPluginDescription(PluginDescription &looperDesc) const
{
	looperDesc.name = "Midi Looper";
	looperDesc.pluginFormatName = "Internal";
	looperDesc.category = "Arrangement";
	looperDesc.manufacturerName = "Nomad Productions";
	looperDesc.version = "0.1";
	looperDesc.fileOrIdentifier = "";
	looperDesc.lastFileModTime = Time();
	looperDesc.uid = 4;
	looperDesc.isInstrument = false;
	looperDesc.numInputChannels = 0;
	looperDesc.numOutputChannels = 0;
}

// TODO: this is disgustingly inefficient, and furthermore only works for one-note-per-channel
// sequences, a la MIDI guitar.  FIX IT!
void MidiLoopProcessor::drawContent(Graphics &g, int width, int height) const
{
	MidiBuffer::Iterator itor(sequence);
	MidiMessage message(0x80, 1, 1);
	int samplePosition = 0;

	// for now, we limit ourselves to one note per channel... guitar input!
	bool channelsOn[16] = {0};
	int channelsNote[16] = {0};
	int channelsBend[16] = {0};
	double lastDrawTime = 0.0;

	Colour colours[6] = {Colours::white, Colours::blue, Colours::purple, Colours::orange, Colours::yellow, Colours::lightgreen};

	while (itor.getNextEvent(message, samplePosition))
	{
		if (message.isNoteOn())
		{
			channelsOn[message.getChannel()-1] = true;
			channelsNote[message.getChannel()-1] = message.getNoteNumber();
		}
		else if (message.isNoteOff())
		{			
			for (int c=0; c<16; ++c)
			{
				if (channelsOn[c])
				{
					g.setColour(colours[c%6]);
					g.drawLine(
						(width*lastDrawTime)/getLengthInSamples(),
						height*(1.0f-(channelsNote[c] + (channelsBend[c])/(8192.0f/12))/128.0f),
						(width*message.getTimeStamp())/getLengthInSamples(),
						height*(1.0f-(channelsNote[c] + (channelsBend[c])/(8192.0f/12))/128.0f)
						);
				}
			}
			channelsOn[message.getChannel()-1] = false;
		}
		else if (message.isPitchWheel())
		{
			for (int c=0; c<16; ++c)
			{
				if (channelsOn[c])
				{
					g.setColour(colours[c%6]);
					g.drawLine(
						(width*lastDrawTime)/getLengthInSamples(),
						height*(1.0f-(channelsNote[c] + (channelsBend[c])/(8192.0f/12))/128.0f),
						(width*message.getTimeStamp())/getLengthInSamples(),
						height*(1.0f-(channelsNote[c] + (channelsBend[c])/(8192.0f/12))/128.0f)
						);
				}
			}
			channelsBend[message.getChannel()-1] = message.getPitchWheelValue();			
		}
		lastDrawTime = message.getTimeStamp();
	}
}

const String MidiLoopProcessor::getName() const
{
	return T("Midi Looper");
}

void MidiLoopProcessor::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
	setPlayConfigDetails (0, 0, sampleRate, estimatedSamplesPerBlock);
}

void MidiLoopProcessor::releaseResources()
{
}

void MidiLoopProcessor::processBlock(AudioSampleBuffer& sampleBuffer, MidiBuffer& midiBuffer)
{
	LoopProcessor* masterLoop = LoopManager::getInstance()->getMasterLoop();

	if (recording != recordingCued)
	{
		if (masterLoop == 0)
		{			
			recording = recordingCued;
			if (!recording)
			{
				// take over as master, since no current master
				LoopManager::getInstance()->setMasterLoop(this);

				KeyAnalyzer a;
				a.analyze(sequence);
				estimatedKey = a.getKey();
			}
		}
		else if (masterLoop == this || masterLoop->getScrubPositionInSamples() == 0)
			recording = recordingCued;
		
	}

	if (recording)
	{
		// TODO: stick midi buffer data into the loop buffer here
		this->sequence.addEvents(midiBuffer, 0, sampleBuffer.getNumSamples(), this->sampleLength);
		sampleLength += sampleBuffer.getNumSamples();
	}
	else if (sampleLength > 0)
	{
		midiBuffer.clear();

		midiBuffer.addEvents(sequence, sampleScrub, sampleBuffer.getNumSamples(), -sampleScrub);
		sampleScrub = (sampleScrub + sampleBuffer.getNumSamples()) % sampleLength;
	}
}

const String MidiLoopProcessor::getInputChannelName(const int) const
{
	return T("Input");
}

const String MidiLoopProcessor::getOutputChannelName(const int) const
{
	return T("Output");
}

bool MidiLoopProcessor::isInputChannelStereoPair(int) const
{
	return true;
}

bool MidiLoopProcessor::isOutputChannelStereoPair(int) const
{
	return true;
}

bool MidiLoopProcessor::acceptsMidi() const
{
	return true;
}

bool MidiLoopProcessor::producesMidi() const
{
	return true;
}

bool MidiLoopProcessor::hasEditor() const { return false; }

AudioProcessorEditor* MidiLoopProcessor::createEditor()
{
	return 0;
}

int MidiLoopProcessor::getNumParameters()
{
	return 13;
}

const String MidiLoopProcessor::getParameterName(int index)
{
	if (index == 0)
		return T("Recording State");
	else if (index == 1)
                return T("C");
        else if (index == 2)
                return T("C#");
        else if (index == 3)
                return T("D");
        else if (index == 4)
                return T("D#");
        else if (index == 5)
                return T("E");
        else if (index == 6)
                return T("F");
        else if (index == 7)
                return T("F#");
        else if (index == 8)
                return T("G");
        else if (index == 9)
                return T("G#");
        else if (index == 10)
                return T("A");
        else if (index == 11)
                return T("A#");
        else if (index == 12)
		return T("B");
	return String::empty;
}

float MidiLoopProcessor::getParameter(int index)
{
	if (recordingCued)
		return 1.f;
	else if (index >= 1 && index <= 12)
		return activePitchClass[index-1]?1.f:0.f;
	return 0.f;
}

const String MidiLoopProcessor::getParameterText(int index)
{
	if (index == 0)
		return recordingCued?T("Recording"):T("Playing");
	return String::empty;
}

void MidiLoopProcessor::setParameter(int index, float value)
{
	if (index == 0)
	{
		if (value >= 0.5f && !recordingCued)
		{			
			// clear loop
			sampleLength = 0;
			sampleScrub = 0;
			sequence.clear();
		}
		recordingCued = (value >= 0.5f);
	}
	else if (index >= 1 && index <= 12)
	{
		if (activePitchClass[index-1] ^ (value >= 0.5f))
		{
			activePitchClass[index-1] = (value >= 0.5f);
			regenerateAlteredSequence();
		}
	}
}

int MidiLoopProcessor::getNumPrograms()
{
	return 1;
}

int MidiLoopProcessor::getCurrentProgram()
{
	return 0;
}

void MidiLoopProcessor::setCurrentProgram(int)
{
}

const String MidiLoopProcessor::getProgramName(int)
{
	return String::empty;
}

void MidiLoopProcessor::changeProgramName(int, const String&)
{
}

void MidiLoopProcessor::getStateInformation(MemoryBlock&)
{
}

void MidiLoopProcessor::setStateInformation(const void *, int)
{
}

int MidiLoopProcessor::getLengthInSamples() const
{
	return sampleLength;
}

double MidiLoopProcessor::getLengthInSeconds() const
{
	return sampleLength / getSampleRate();
}

int MidiLoopProcessor::getScrubPositionInSamples() const
{
	return sampleScrub;
}

double MidiLoopProcessor::getScrubPositionInSeconds() const
{
	return sampleScrub / getSampleRate();
}

struct MidiEngineInfo
{
	// for now, we limit ourselves to one note per channel... guitar input!
	bool channelsOn[16];
	int channelsNote[16];
	int channelsBend[16];


	MidiEngineInfo()
	{
		zeromem(&channelsOn, sizeof(channelsOn));
		zeromem(&channelsNote, sizeof(channelsNote));
		zeromem(&channelsBend, sizeof(channelsBend));
	}

		// same contract as MidiBuffer::Iterator::getNextEvent, but
	// stores some stateful information
	bool readNextMidiMessage(MidiBuffer::Iterator& itor, MidiMessage& msg, int& samplePos)
	{
		bool ret = itor.getNextEvent(msg, samplePos);
		if (ret)
		{
			if (msg.isNoteOn())
			{
				channelsNote[msg.getChannel()-1] = msg.getNoteNumber();
				channelsOn[msg.getChannel()-1] = true;
			}
			else if (msg.isNoteOff())
			{
				channelsOn[msg.getChannel()-1] = false;
			}
			else if (msg.isPitchWheel())
			{
				channelsBend[msg.getChannel()-1] = msg.getPitchWheelValue();
			}
		}

		return ret;
	}

	int getBentPitch(MidiMessage& msg)
	{
		return msg.getNoteNumber() + channelsBend[msg.getChannel()-1]*(8192.0/12);
	}
};

// Generates a processed copy of the recorded buffer,
// making use of only the notes enabled in the keymap
void MidiLoopProcessor::regenerateAlteredSequence()
{
	MidiBuffer::Iterator itor(sequence);
	MidiMessage message(0x80, 1, 1);
	int samplePosition = 0;

	MidiEngineInfo info;
	double lastDrawTime = 0.0;

	// we'll overwrite this with processed copies from the recorded
	// buffer
	alteredSequence.ensureSize (sequence.getNumEvents());

	do
	{
		// Find first MIDI note-on
		while (info.readNextMidiMessage(itor, message, samplePosition) && !message.isNoteOn());

		// A MIDI fragment consists of an area between consecutive
		// pitches which potentially needs to be scaled.  For instance,
		// a glissando between two notes in the input should yield a
		// similarly distributed glissando between the output notes
		int startPitch = info.getBentPitch(message);

		// Find the spot when the note becomes a different pitch
		// based on bending
		while (info.readNextMidiMessage(itor, message, samplePosition) && !message.isNoteOff() && info.getBentPitch(message) == startPitch);

		// Now find the spot where either another pitch is reached
		// or the bending tapers off (i.e., dPitchBend = 0)
		
	} while (info.readNextMidiMessage(itor, message, samplePosition));

}



// ==========================

AudioLoopProcessor::AudioLoopProcessor()
: /*recordingCued(false), recording(false),*/ cuedState(Paused), state(Paused), sampleScrub(0)
{
	setPlayConfigDetails (1, 1, 0, 0);
}

void AudioLoopProcessor::fillInPluginDescription(PluginDescription &looperDesc) const
{
	looperDesc.name = "Audio Looper";
	looperDesc.pluginFormatName = "Internal";
	looperDesc.category = "Arrangement";
	looperDesc.manufacturerName = "Nomad Productions";
	looperDesc.version = "0.1";
	looperDesc.fileOrIdentifier = "";
	looperDesc.lastFileModTime = Time();
	looperDesc.uid = 4;
	looperDesc.isInstrument = false;
	looperDesc.numInputChannels = 1;
	looperDesc.numOutputChannels = 1;
}

const String AudioLoopProcessor::getName() const
{
	return T("Looper");
}

void AudioLoopProcessor::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
	setPlayConfigDetails (1, 1, sampleRate, estimatedSamplesPerBlock);
}

void AudioLoopProcessor::releaseResources()
{
}

void AudioLoopProcessor::processBlock(AudioSampleBuffer& sampleBuffer, MidiBuffer& midiBuffer)
{
	LoopProcessor* masterLoop = LoopManager::getInstance()->getMasterLoop();

	if (state != cuedState)
	{
		if (masterLoop == 0)
		{			
			//recording = recordingCued;
			state = cuedState;
			if (state != Recording)
			{
				// take over as master, since no current master
				LoopManager::getInstance()->setMasterLoop(this);
			}
		}
		else if (masterLoop == this || masterLoop->getScrubPositionInSamples() == 0)
			state = cuedState;
	}

	if (state == Recording)
	{
		for (int i=0; i<sampleBuffer.getNumSamples(); ++i)
		{
			sampleData.push_back(sampleBuffer.getSampleData(0)[i]);
		}
		sampleBuffer.clear();
	}
	else if (!sampleData.empty())
	{
		// playing back
		int samplesToCopy = sampleBuffer.getNumSamples();
		int destStartSample = 0;

		while (samplesToCopy > 0)
		{	
			int samplesLeftInLoop = sampleData.size() - sampleScrub;
			if (samplesToCopy <= samplesLeftInLoop)
			{
				if (state == Overdubbing)
				{
					// first, sum the input to the buffer
					AudioSampleBuffer sumBuffer(sampleBuffer);
					sumBuffer.addFrom(0, destStartSample, &sampleData[sampleScrub], samplesToCopy);
					sampleBuffer.copyFrom(0, destStartSample, &sampleData[sampleScrub], samplesToCopy);
					float* ds = &sampleData[sampleScrub];
					AudioSampleBuffer dataStoreBuffer(&ds, 1, samplesToCopy);
					dataStoreBuffer.copyFrom(0, 0, sumBuffer, 0, 0, samplesToCopy);
				}
				else
				{
					sampleBuffer.copyFrom(0, destStartSample, &sampleData[sampleScrub], samplesToCopy);
				}
				sampleScrub += samplesToCopy;
				samplesToCopy = 0;
			}
			else
			{
				if (state == Overdubbing)
				{
					AudioSampleBuffer sumBuffer(sampleBuffer);
					sumBuffer.addFrom(0, destStartSample, &sampleData[sampleScrub], samplesLeftInLoop);
					sampleBuffer.copyFrom(0, destStartSample, &sampleData[sampleScrub], samplesLeftInLoop);
					float* ds = &sampleData[sampleScrub];
					AudioSampleBuffer dataStoreBuffer(&ds, 1, samplesLeftInLoop);
					dataStoreBuffer.copyFrom(0, 0, sumBuffer, 0, 0, samplesLeftInLoop);
				}
				else
				{
					sampleBuffer.copyFrom(0, destStartSample, &sampleData[sampleScrub], samplesLeftInLoop);				
				}
				sampleScrub = 0;  // reset to beginning of loop
				destStartSample += samplesLeftInLoop;
			}

			sampleScrub = sampleScrub % sampleData.size();
		}
	}
	else
	{
		sampleBuffer.clear();
	}
}

const String AudioLoopProcessor::getInputChannelName(const int) const
{
	return T("Input");
}

const String AudioLoopProcessor::getOutputChannelName(const int) const
{
	return T("Output");
}

bool AudioLoopProcessor::isInputChannelStereoPair(int) const
{
	return true;
}

bool AudioLoopProcessor::isOutputChannelStereoPair(int) const
{
	return true;
}

bool AudioLoopProcessor::acceptsMidi() const
{
	return false;
}

bool AudioLoopProcessor::producesMidi() const
{
	return false;
}

bool AudioLoopProcessor::hasEditor() const { return false; }

AudioProcessorEditor* AudioLoopProcessor::createEditor()
{
	return 0;
}

int AudioLoopProcessor::getNumParameters()
{
	return 2;
}

const String AudioLoopProcessor::getParameterName(int index)
{
	if (index == 0)
		return T("Recording");
	else if (index == 1)
		return T("Overdub");
	return String::empty;
}

float AudioLoopProcessor::getParameter(int p)
{
	if (cuedState == Recording && p==0)
		return 1.f;
	else if (cuedState == Overdubbing && p==1)
		return 1.f;
	return 0.f;
}

const String AudioLoopProcessor::getParameterText(int index)
{
	if (index == 0)
		return (cuedState==Recording)?T("On"):T("Off");
	else if (index == 1)
		return (cuedState==Overdubbing)?T("On"):T("Off");
	return String::empty;
}

void AudioLoopProcessor::setParameter(int index, float value)
{
	LoopState currentCuedState = cuedState;

	if (index == 0)
	{
		if (value >= 0.5f && cuedState != Recording)
		{
			// clear loop
			sampleData.clear();
			sampleScrub = 0;
		}
		cuedState = (value >= 0.5f)?Recording:Playing;
	}
	else if (index == 1)
	{
		if (value >= 0.5f)
			cuedState = Overdubbing;
		else
			cuedState = Playing;
	}

	if (currentCuedState != cuedState)
	{
		/*sendParamChangeMessageToListeners(0, cuedState==Recording?1:0);
		sendParamChangeMessageToListeners(1, cuedState==Overdubbing?1:0);*/
	}

}

int AudioLoopProcessor::getNumPrograms()
{
	return 1;
}

int AudioLoopProcessor::getCurrentProgram()
{
	return 0;
}

void AudioLoopProcessor::setCurrentProgram(int)
{
}

const String AudioLoopProcessor::getProgramName(int)
{
	return String::empty;
}

void AudioLoopProcessor::changeProgramName(int, const String&)
{
}

void AudioLoopProcessor::getStateInformation(MemoryBlock&)
{
}

void AudioLoopProcessor::setStateInformation(const void *, int)
{
}

int AudioLoopProcessor::getLengthInSamples() const
{
	return sampleData.size();
}

double AudioLoopProcessor::getLengthInSeconds() const
{
	return sampleData.size() / getSampleRate();
}

int AudioLoopProcessor::getScrubPositionInSamples() const
{
	return sampleScrub;
}

double AudioLoopProcessor::getScrubPositionInSeconds() const
{
	return sampleScrub / getSampleRate();
}

void AudioLoopProcessor::drawContent(Graphics& g, int width, int height) const
{
	if (!sampleData.empty())
	{
		float lastY = 0.f;
		for (int i=1; i<width; ++i)
		{
			g.drawLine(i-1, height*0.5f*(1 + sampleData[(i-1)*(sampleData.size()-1)/width]), i, height*0.5f*(1 + sampleData[i*(sampleData.size()-1)/width]));
		}
	}

	g.setColour(Colours::white);
	g.drawText(T("state"), 4, 4, width, height, Justification::topLeft, false);
	if (state == Overdubbing)
		g.drawText(T("OVR"), 4, 4, width, height, Justification::topLeft, false);
}

juce_ImplementSingleton (LoopManager);

LoopManager::LoopManager()
: masterLoop(0)
{
}

LoopManager::~LoopManager()
{
	clearSingletonInstance();
}

LoopProcessor* LoopManager::getMasterLoop()
{
	return masterLoop;
}

void LoopManager::setMasterLoop(LoopProcessor* newMasterLoop)
{
	masterLoop = newMasterLoop;
}
