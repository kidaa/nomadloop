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
	looperDesc.lastFileModTime = 0;
	looperDesc.uid = 4;
	looperDesc.isInstrument = false;
	looperDesc.numInputChannels = 0;
	looperDesc.numOutputChannels = 0;
}

// TODO: this is disgustingly inefficient, and furthermore only works for one-note-per-channel
// sequences, a la MIDI guitar.  FIX IT!
void MidiLoopProcessor::drawMidiBuffer(Graphics &g, int width, int height) const
{
	MidiBuffer::Iterator itor(sequence);
	MidiMessage message(0x80, 1, 1);
	int samplePosition = 0;

	// for now, we limit ourselves to one note per channel... guitar input!
	bool channelsOn[16] = {0};
	int channelsNote[16] = {0};
	int channelsBend[16] = {0};
	double lastDrawTime = 0.0;

	while (itor.getNextEvent(message, samplePosition))
	{
		if (message.isNoteOn())
		{
			channelsOn[message.getChannel()] = true;
			channelsNote[message.getChannel()] = message.getNoteNumber();
		}
		else if (message.isNoteOff())
		{			
			for (int c=0; c<16; ++c)
			{
				if (channelsOn[c])
				{
					g.drawLine(
						(width*lastDrawTime)/getLengthInSamples(),
						height*(1.0f-(channelsNote[c] + (channelsBend[c])/(8192.0f/12))/128.0f),
						(width*message.getTimeStamp())/getLengthInSamples(),
						height*(1.0f-(channelsNote[c] + (channelsBend[c])/(8192.0f/12))/128.0f)
						);
				}
			}
			channelsOn[message.getChannel()] = false;
		}
		else if (message.isPitchWheel())
		{
			for (int c=0; c<16; ++c)
			{
				if (channelsOn[c])
				{
					g.drawLine(
						(width*lastDrawTime)/getLengthInSamples(),
						height*(1.0f-(channelsNote[c] + (channelsBend[c])/(8192.0f/12))/128.0f),
						(width*message.getTimeStamp())/getLengthInSamples(),
						height*(1.0f-(channelsNote[c] + (channelsBend[c])/(8192.0f/12))/128.0f)
						);
				}
			}
			channelsBend[message.getChannel()] = message.getPitchWheelValue();			
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

AudioProcessorEditor* MidiLoopProcessor::createEditor()
{
	return 0;
}

int MidiLoopProcessor::getNumParameters()
{
	return 1;
}

const String MidiLoopProcessor::getParameterName(int index)
{
	if (index == 0)
		return T("Recording State");
	return String::empty;
}

float MidiLoopProcessor::getParameter(int)
{
	if (recordingCued)
		return 1.f;
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

// ==========================

AudioLoopProcessor::AudioLoopProcessor()
: recordingCued(false), recording(false), sampleScrub(0)
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
	looperDesc.lastFileModTime = 0;
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

	if (recording != recordingCued)
	{
		if (masterLoop == 0)
		{			
			recording = recordingCued;
			if (!recording)
			{
				// take over as master, since no current master
				LoopManager::getInstance()->setMasterLoop(this);
			}
		}
		else if (masterLoop == this || masterLoop->getScrubPositionInSamples() == 0)
			recording = recordingCued;
	}

	if (recording)
	{
		for (int i=0; i<sampleBuffer.getNumSamples(); ++i)
		{
			sampleData.push_back(sampleBuffer.getSampleData(0)[i]);
		}
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
				sampleBuffer.addFrom(0, destStartSample, &sampleData[sampleScrub], samplesToCopy);
				sampleScrub += samplesToCopy;
				samplesToCopy = 0;
			}
			else
			{
				sampleBuffer.addFrom(0, destStartSample, &sampleData[sampleScrub], samplesLeftInLoop);
				sampleScrub = 0;  // reset to beginning of loop
				destStartSample += samplesLeftInLoop;
			}

			sampleScrub = sampleScrub % sampleData.size();
		}
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

AudioProcessorEditor* AudioLoopProcessor::createEditor()
{
	return 0;
}

int AudioLoopProcessor::getNumParameters()
{
	return 1;
}

const String AudioLoopProcessor::getParameterName(int index)
{
	if (index == 0)
		return T("Recording State");
	return String::empty;
}

float AudioLoopProcessor::getParameter(int)
{
	if (recordingCued)
		return 1.f;
	return 0.f;
}

const String AudioLoopProcessor::getParameterText(int index)
{
	if (index == 0)
		return recordingCued?T("Recording"):T("Playing");
	return String::empty;
}

void AudioLoopProcessor::setParameter(int index, float value)
{
	if (index == 0)
	{
		if (value >= 0.5f && !recordingCued)
		{			
			// clear loop
			sampleData.clear();
			sampleScrub = 0;
		}
		recordingCued = (value >= 0.5f);
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