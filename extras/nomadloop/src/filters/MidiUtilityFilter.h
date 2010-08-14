/*
 *  MidiUtilityFilter.h
 *  PluginHost
 *
 *  Created by Noah Adler on 8/1/10.
 *
 */

#ifndef ADLER_MIDIUTILITYFILTER
#define ADLER_MIDIUTILITYFILTER

#include "../includes.h"

class MidiUtilityFilter : public AudioPluginInstance
	{
		float pitchBendRange;		
		int octaveShift;
		int currentProgram;
		bool triggerSend;
		
	public:
		MidiUtilityFilter();
		
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
