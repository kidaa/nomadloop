#ifndef ADLER_SYNCPLAYHEAD
#define ADLER_SYNCPLAYHEAD

#include "../includes.h"
#include "Looper.h"

class SyncPlayHead : public juce::AudioPlayHead, public juce::MidiInputCallback, public juce::AudioIODeviceCallback
{
	int ppqn;
	//double bpm;
	double lastClockTime;	
	int ppqCycleCount;

	double lastMasterLoopRepeatPosition;

	CurrentPositionInfo currentPositionInfo;

	double sampleRate;
	int blockSize;

	MidiBuffer incomingMidi;
	MidiMessageCollector messageCollector;

	// simply wrap, and pass through to the underlying AudioProcessorPlayer for the processor
	// graph, but we want this so that we can get sample accurate MIDI clocks	
	void audioDeviceIOCallback(const float **inputChannelData, int numInputChannels,
		float **outputChannelData, int numOutputChannels, int numSamples);
	void audioDeviceAboutToStart(AudioIODevice *device);
	void audioDeviceStopped();

public:
	SyncPlayHead();

	bool getCurrentPosition(CurrentPositionInfo &result);

	void handleIncomingMidiMessage (MidiInput *source, const MidiMessage &message);
	void handlePartialSysexMessage (MidiInput *source, const uint8 *messageData, const int numBytesSoFar, const double timestamp);

};

#endif
