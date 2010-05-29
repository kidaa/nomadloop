#ifndef ADLER_ARPEGGIATOR
#define ADLER_ARPEGGIATOR

#include "../includes.h"

class Arpeggiator : public AudioPluginInstance
{
	float lastPpqPosition;

	double sampleRate;

	float rate;
	
	Array<int> activeInputNotes;
	int activeOutputNoteCount;
	int outputIndex;

public:
	Arpeggiator();
	
	void fillInPluginDescription(PluginDescription &desc) const;

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
};

#endif
