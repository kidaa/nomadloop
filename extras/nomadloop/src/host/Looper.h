#ifndef ADLER_LOOPER
#define ADLER_LOOPER

#include "../includes.h"
#include "Analysis.h"
#include <vector>
#include <deque>

enum LoopState
{
	Paused = 0,
	Playing,
	Recording,
	Overdubbing
};

class LoopProcessor : public AudioPluginInstance
{
protected:
	LoopProcessor();
public:
	virtual ~LoopProcessor() = 0;

	virtual int getLengthInSamples() const = 0;
	virtual double getLengthInSeconds() const = 0;

	virtual int getScrubPositionInSamples() const = 0;
	virtual double getScrubPositionInSeconds() const = 0;

	virtual void drawContent(Graphics&, int width, int height) const = 0;
};

class MidiLoopProcessor : public LoopProcessor
{
	// Contains a series of MIDI events
	// corresponding to the beginning and end
	// of a voice containing a start and end pitch,
	// which may or may not be the same, and the
	// envelope between them
	struct VoiceFragment
	{
		int StartPitch;
		int EndPitch;
		
	};

	bool recordingCued;
	bool recording;
	int sampleLength;
	int sampleScrub;
	MidiBuffer sequence;

	bool activePitchClass[12];

	// sequence converted to a certain subset of notes
	MidiBuffer alteredSequence;
	void regenerateAlteredSequence();

	Key estimatedKey;

public:
	MidiLoopProcessor();

	void fillInPluginDescription(PluginDescription &looperDesc) const;

	const String getName() const;
	void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void processBlock(AudioSampleBuffer &, MidiBuffer &);
	const String getInputChannelName(const int) const;
	const String getOutputChannelName(const int) const;	
	bool isInputChannelStereoPair(int) const;
	bool isOutputChannelStereoPair(int) const;
	bool acceptsMidi() const;
	bool producesMidi() const;
	AudioProcessorEditor* createEditor();
	int getNumParameters();
	const String getParameterName(int);
	float getParameter(int);
	const String getParameterText(int);
	void setParameter(int, float);
	int getNumPrograms();
	int getCurrentProgram();
	void setCurrentProgram(int);
	const String getProgramName(int);
	void changeProgramName(int, const String&);
	void getStateInformation(MemoryBlock&);
	void setStateInformation(const void *, int);

	int getLengthInSamples() const;
	double getLengthInSeconds() const;

	int getScrubPositionInSamples() const;
	double getScrubPositionInSeconds() const;

	void drawContent(Graphics& g, int width, int height) const;

	inline const Key& getEstimatedKey() const { return estimatedKey; }
};

// A graph filter hooking into the looping engine
class AudioLoopProcessor : public LoopProcessor
{
	//bool recordingCued;
	//bool recording;

	LoopState cuedState;
	LoopState state;

	//std::deque<float> sampleData;
	std::vector<float> sampleData;
	int sampleScrub;

public:
	AudioLoopProcessor();

	void fillInPluginDescription(PluginDescription &looperDesc) const;

	const String getName() const;
	void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void processBlock(AudioSampleBuffer &,MidiBuffer &);
	const String getInputChannelName(const int) const;
	const String getOutputChannelName(const int) const;
	bool isInputChannelStereoPair(int) const;
	bool isOutputChannelStereoPair(int) const;
	bool acceptsMidi() const;
	bool producesMidi() const;
	AudioProcessorEditor* createEditor();
	int getNumParameters();
	const String getParameterName(int);
	float getParameter(int);
	const String getParameterText(int);
	void setParameter(int, float);
	int getNumPrograms();
	int getCurrentProgram();
	void setCurrentProgram(int);
	const String getProgramName(int);
	void changeProgramName(int, const String&);
	void getStateInformation(MemoryBlock&);
	void setStateInformation(const void *, int);

	int getLengthInSamples() const;
	double getLengthInSeconds() const;

	int getScrubPositionInSamples() const;
	double getScrubPositionInSeconds() const;

	void drawContent(Graphics& g, int width, int height) const;
};

// Holder of the global loop state, used to sync loops together, i.e. keep track of master loop
class LoopManager
{
	LoopProcessor* masterLoop;
public:
	LoopManager();
	~LoopManager();

	LoopProcessor* getMasterLoop();
	void setMasterLoop(LoopProcessor* newMasterLoop);

	juce_DeclareSingleton (LoopManager, true)
};

#endif
