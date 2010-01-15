/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef DEMOJUCEPLUGINFILTER_H
#define DEMOJUCEPLUGINFILTER_H


//==============================================================================
/**
    A simple plugin filter that just applies a gain change to the audio
    passing through it.

*/
class HarmScaleFilter  : public AudioProcessor,
                        public ChangeBroadcaster
{
public:
    //==============================================================================
    HarmScaleFilter();
    ~HarmScaleFilter();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

	void processBlock (AudioSampleBuffer& buffer,
                       MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();

    //==============================================================================
    const String getName() const;

    int getNumParameters();

    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (const int channelIndex) const;
    const String getOutputChannelName (const int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;

    //==============================================================================
    int getNumPrograms()                                        { return 0; }
    int getCurrentProgram()                                     { return 0; }
    void setCurrentProgram (int index)                          { }
    const String getProgramName (int index)                     { return String::empty; }
    void changeProgramName (int index, const String& newName)   { }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    //==============================================================================
    // These properties are public so that our editor component can access them
    //  - a bit of a hacky way to do it, but it's only a demo!

    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    AudioPlayHead::CurrentPositionInfo lastPosInfo;

    // these are used to persist the UI's size - the values are stored along with the
    // filter's other parameters, and the UI component will update them when it gets
    // resized.
    int lastUIWidth, lastUIHeight;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    // this is our gain - the UI and the host can access this by getting/setting
    // parameter 0.
    float gain;

	// which pitch-classes are active stops in the scale
	bool activeNoteInScale[12];

	// offset in semitones, ranging +/- 12
	// for now, only positive shifts supported
	int transposeSemitones;

	// discrete mapping of pitch-classes to their nearest lower active pitch-class
	int scaleFloorMap[12];

	// for active pitch-classes, the length of semitones up till the next active pitch-class
	int scaleStepRunLength[12];
	void recalculateScaleTables();

	int calculatePitchbendForNote(int inputNote, int inputPitchWheel, int lastOutputNote);

	struct PerChannelData
	{
		PerChannelData()
			: lastInputNote(0), lastInputPitchbend(8192), lastOutputNote(0)
		{
		}

		int lastInputNote;
		int lastInputPitchbend;
		int lastOutputNote;
		int lastOutputPitchbend;
	};
	
	PerChannelData channelData[16];
};


#endif
