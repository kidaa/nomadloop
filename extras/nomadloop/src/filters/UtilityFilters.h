#ifndef ADLER_UTILITYFILTERS
#define ADLER_UTILITYFILTERS

#include "../includes.h"

class GainCut : public AudioPluginInstance
{
	float gain;
	float gainToRampTo;
public:
	GainCut();

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
	bool hasEditor() const;
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

class SelectableMidiInputFilter : public AudioPluginInstance, public MidiInputCallback
{
	MidiInput* input;
	String selectedInputName;
	MidiMessageCollector messageCollector;
	double sampleRate;
	bool dirty;
	
	inline void handleIncomingMidiMessage(MidiInput* input, const MidiMessage& message)
	{
		if (dirty)
		{
			if (input->getName() == selectedInputName)
			{
				this->input = input;
				dirty = false;
			}
			else
				return;
		}
		
		if (input == this->input)
		{
			messageCollector.addMessageToQueue(message);
		}
	}
	
public:
	SelectableMidiInputFilter();
	
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
	bool hasEditor() const;
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

class DefaultMidiOutputFilter : public AudioPluginInstance
{
	MidiOutput* output;
	double sampleRate;
public:
	DefaultMidiOutputFilter();

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
	bool hasEditor() const;
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
