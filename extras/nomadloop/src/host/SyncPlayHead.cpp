#include "SyncPlayHead.h"

SyncPlayHead::SyncPlayHead()
	: ppqn(24), /*bpm(90.f),*/ lastClockTime(0.f), ppqCycleCount(0)
{
	currentPositionInfo.bpm = 90.f;
	currentPositionInfo.editOriginTime = 0.f;
	currentPositionInfo.frameRate = AudioPlayHead::fpsUnknown;
	currentPositionInfo.isPlaying = true;
	currentPositionInfo.isRecording = true;
	currentPositionInfo.ppqPosition = 0.f;
	currentPositionInfo.ppqPositionOfLastBarStart = 0.f;
	currentPositionInfo.timeInSeconds = 0.f;
	currentPositionInfo.timeSigDenominator = 4;
	currentPositionInfo.timeSigNumerator = 4;
}

bool SyncPlayHead::getCurrentPosition(juce::AudioPlayHead::CurrentPositionInfo &result)
{
	LoopProcessor* masterLoop = LoopManager::getInstance()->getMasterLoop();
	if (masterLoop != 0)
	{
		// if a master loop is playing, use that to calculate measures, rather
		// than relying on the information received from any external sync

		// assume all is 4/4 for now

		result.bpm = 60.0/(masterLoop->getLengthInSeconds() / 4.0);
		result.editOriginTime = 0.f;
		result.frameRate = AudioPlayHead::fpsUnknown;
		result.isPlaying = true;
		result.isRecording = true;
		result.ppqPosition = /*ppqn*/ 4.0 * masterLoop->getScrubPositionInSamples() / static_cast<double>(masterLoop->getLengthInSamples());
		result.ppqPositionOfLastBarStart = 0.f;
		result.timeInSeconds = masterLoop->getScrubPositionInSeconds();
		result.timeSigDenominator = 4;
		result.timeSigNumerator = 4;
	}
	else
	{
		// use any information coming in from MIDI sync
		result = currentPositionInfo;
	}
	
	return true;
}

void SyncPlayHead::handleIncomingMidiMessage (MidiInput *source, const MidiMessage &message)
{
	//messageCollector.addMessageToQueue (message);
	
	if (message.isMidiClock())
	{
		//Logger::outputDebugPrintf(T("MIDI clock received at time: %f"), message.getTimeStamp());
		double elapsed = message.getTimeStamp() - lastClockTime;
		double estimatedBpm = 60.f/(elapsed*ppqn);

		// if it's this low, the clock has been freewheeling for some time...
		// wait till the next clock before actually resetting the tempo
		if (estimatedBpm < 20.f)
		{
			currentPositionInfo.ppqPosition = 0;
			currentPositionInfo.ppqPositionOfLastBarStart = 0;
			ppqCycleCount = 0;
		}
		//lastClockTime = message.getTimeStamp();
		else
		{
			currentPositionInfo.bpm = estimatedBpm;	
			currentPositionInfo.ppqPosition += 1.f/ppqn;

			// TODO: this assumes quarter notes in timeSigDenominator for now
			ppqCycleCount = (ppqCycleCount+1)%(ppqn*currentPositionInfo.timeSigNumerator);
			if (ppqCycleCount == 0)
			{
				currentPositionInfo.ppqPositionOfLastBarStart = currentPositionInfo.ppqPosition;
			}
		}
		currentPositionInfo.editOriginTime = currentPositionInfo.timeInSeconds = lastClockTime = message.getTimeStamp();		

		//Logger::outputDebugPrintf(T("Clock time: %f"), lastClockTime);
	}
}

void SyncPlayHead::handlePartialSysexMessage (MidiInput *source, const uint8 *messageData, const int numBytesSoFar, const double timestamp)
{
	Logger::outputDebugPrintf(T("Sysex from mobius!!"));
}


// CURRENTLY NOT USED -- idea is to quantize midi clocks to audio updates to give more stable timing,
// but unknown if that method yields superior results, and it's slightly more expensive
void SyncPlayHead::audioDeviceIOCallback(const float **inputChannelData, int numInputChannels,
		float **outputChannelData, int numOutputChannels, int numSamples)
{
	incomingMidi.clear();
    messageCollector.removeNextBlockOfMessages (incomingMidi, numSamples);

	MidiBuffer::Iterator itor(incomingMidi);
	MidiMessage message(0);
	int samplePos;
	int i=0;
	while (itor.getNextEvent(message, samplePos))
	{
		if (message.isMidiClock())
		{
			Logger::outputDebugPrintf(T("MIDI clock %i this frame at sample pos %i"), i, samplePos);
			i++;
		}
	}
}

void SyncPlayHead::audioDeviceAboutToStart(AudioIODevice *device)
{
	sampleRate = device->getCurrentSampleRate();
    blockSize = device->getCurrentBufferSizeSamples();

	messageCollector.reset(sampleRate);
}

void SyncPlayHead::audioDeviceStopped()
{
}

