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

#include "includes.h"
#include "HarmScaleFilter.h"
#include "DemoEditorComponent.h"
#include <cmath>


//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HarmScaleFilter();
}

//==============================================================================
HarmScaleFilter::HarmScaleFilter()
{
    gain = 1.0f;
    lastUIWidth = 400;
    lastUIHeight = 140;

	// initialize a C major scale
	const bool Cmaj[12] = {true, false, true, false, true, true, false, true, false, true, false, true};
	for (int i=0; i<12; ++i)
		activeNoteInScale[i] = Cmaj[i];

	// start out with a major third
	transposeSemitones = 4;

	recalculateScaleTables();

    zeromem (&lastPosInfo, sizeof (lastPosInfo));
    lastPosInfo.timeSigNumerator = 4;
    lastPosInfo.timeSigDenominator = 4;
    lastPosInfo.bpm = 120;
}

HarmScaleFilter::~HarmScaleFilter()
{
}

//==============================================================================
const String HarmScaleFilter::getName() const
{
    return "HarmScalePlugin";
}

int HarmScaleFilter::getNumParameters()
{
    return 14;	
}

float HarmScaleFilter::getParameter (int index)
{
	if (index == 0)
		return gain;
	else if (index >= 1 && index <= 12)
		return activeNoteInScale[index-1]?1.f:0.f;
	else if (index == 13)
		return (transposeSemitones/24.f + 0.5f);

	return 0.f;
}

void HarmScaleFilter::setParameter (int index, float newValue)
{
    if (index == 0)
    {
        if (gain != newValue)
        {
            gain = newValue;

            // if this is changing the gain, broadcast a change message which
            // our editor will pick up.
            sendChangeMessage (this);
        }
    }
	else if (index >= 1 && index <= 12)
	{
		if (activeNoteInScale[index-1] ^ (newValue >= 0.5f))
		{
			activeNoteInScale[index-1] = (newValue >= 0.5f);
			recalculateScaleTables();

			/*if (newValue >= 0.5f)
			{
				activeNoteInScale[index-1] = true;
				recalculateScaleTables();
			}
			else
			{
				activeNoteInScale[index-1] = false;
				scaleFloorMap[index-1] = scaleFloorMap[(index-2)%12];				
			}*/
			sendChangeMessage (this);
		}
	}
	else if (index == 13)
	{
		newValue = newValue*24.f - 12.f;
		if (static_cast<int>(newValue) != transposeSemitones)
		{
			transposeSemitones = static_cast<int>(newValue);
			sendChangeMessage (this);
		}
	}
}

const String HarmScaleFilter::getParameterName (int index)
{
    if (index == 0)
        return T("Gain");
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
	else if (index == 13)
		return T("Semitone shift");

    return String::empty;
}

const String HarmScaleFilter::getParameterText (int index)
{
    if (index == 0)
        return String (gain, 2);
	else if (index >= 1 && index <= 12)
		return activeNoteInScale[index-1]?T("on"):T("off");
	else if (index == 13)
		return (transposeSemitones>0?T("+"):T("")) + String (transposeSemitones);

    return String::empty;
}

const String HarmScaleFilter::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String HarmScaleFilter::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool HarmScaleFilter::isInputChannelStereoPair (int index) const
{
    return false;
}

bool HarmScaleFilter::isOutputChannelStereoPair (int index) const
{
    return false;
}

bool HarmScaleFilter::acceptsMidi() const
{
    return true;
}

bool HarmScaleFilter::producesMidi() const
{
    return true;
}

//==============================================================================
void HarmScaleFilter::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // do your pre-playback setup stuff here..    
}

void HarmScaleFilter::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void HarmScaleFilter::processBlock (AudioSampleBuffer& buffer,
                                   MidiBuffer& midiMessages)
{
    // for each of our input channels, we'll attenuate its level by the
    // amount that our volume parameter is set to.
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        buffer.applyGain (channel, 0, buffer.getNumSamples(), gain);
    }

    // in case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }

    // have a go at getting the current time from the host, and if it's changed, tell
    // our UI to update itself.
    AudioPlayHead::CurrentPositionInfo pos;

    if (getPlayHead() != 0 && getPlayHead()->getCurrentPosition (pos))
    {
        if (memcmp (&pos, &lastPosInfo, sizeof (pos)) != 0)
        {
            lastPosInfo = pos;
            sendChangeMessage (this);
        }
    }
    else
    {
        zeromem (&lastPosInfo, sizeof (lastPosInfo));
        lastPosInfo.timeSigNumerator = 4;
        lastPosInfo.timeSigDenominator = 4;
        lastPosInfo.bpm = 120;
    }

	// here's the meat - modify the incoming MIDI stream
	MidiBuffer processedMidiMessages;
	MidiBuffer::Iterator midiItor(midiMessages);
	MidiMessage message(0x80, 60, 0);
	int samplePos;

	while (midiItor.getNextEvent(message, samplePos))
	{
		int channel = message.getChannel();	
		if (message.isNoteOn())
		{
			int pitch = channelData[channel].lastInputNote = message.getNoteNumber();

			// add semitones, then count down till a valid pitch is reached
			pitch += transposeSemitones;

			pitch = scaleFloorMap[pitch%12] + 12*(pitch/12);

			channelData[channel].lastOutputNote = pitch;

			// reset pitch bend			
			processedMidiMessages.addEvent(MidiMessage::pitchWheel(channel,
				calculatePitchbendForNote(channelData[channel].lastInputNote,
				channelData[channel].lastInputPitchbend,
				channelData[channel].lastOutputNote)), samplePos);			

			message.setNoteNumber(pitch);
		}
		else if (message.isNoteOff())
		{
			message.setNoteNumber(channelData[channel].lastOutputNote);			
		}
		else if (message.isPitchWheel())
		{		
			int pitchWheel = message.getPitchWheelValue();

			int newPitchWheel = calculatePitchbendForNote(channelData[channel].lastInputNote, pitchWheel, channelData[channel].lastOutputNote);
			channelData[channel].lastOutputPitchbend = newPitchWheel;
			message = MidiMessage::pitchWheel(channel, newPitchWheel);
		}
		processedMidiMessages.addEvent(message, samplePos);		
	}

	midiMessages.swap(processedMidiMessages);	
}

//==============================================================================
AudioProcessorEditor* HarmScaleFilter::createEditor()
{
    //return new DemoEditorComponent (this);
	return 0;
}

//==============================================================================
void HarmScaleFilter::getStateInformation (MemoryBlock& destData)
{
    // you can store your parameters as binary data if you want to or if you've got
    // a load of binary to put in there, but if you're not doing anything too heavy,
    // XML is a much cleaner way of doing it - here's an example of how to store your
    // params as XML..

    // create an outer XML element..
    XmlElement xmlState (T("MYPLUGINSETTINGS"));

    // add some attributes to it..
    xmlState.setAttribute (T("pluginVersion"), 1);
    xmlState.setAttribute (T("gainLevel"), gain);
    xmlState.setAttribute (T("uiWidth"), lastUIWidth);
    xmlState.setAttribute (T("uiHeight"), lastUIHeight);

    // you could also add as many child elements as you need to here..


    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xmlState, destData);
}

void HarmScaleFilter::setStateInformation (const void* data, int sizeInBytes)
{
    // use this helper function to get the XML from this binary blob..
    XmlElement* const xmlState = getXmlFromBinary (data, sizeInBytes);

    if (xmlState != 0)
    {
        // check that it's the right type of xml..
        if (xmlState->hasTagName (T("MYPLUGINSETTINGS")))
        {
            // ok, now pull out our parameters..
            gain = (float) xmlState->getDoubleAttribute (T("gainLevel"), gain);

            lastUIWidth = xmlState->getIntAttribute (T("uiWidth"), lastUIWidth);
            lastUIHeight = xmlState->getIntAttribute (T("uiHeight"), lastUIHeight);

            sendChangeMessage (this);
        }

        delete xmlState;
    }
}

void HarmScaleFilter::recalculateScaleTables()
{
	// first make sure we have a scale...
	int scaleStops = 0;
	for (int i=0; i<12; ++i)
	{
		if (activeNoteInScale[i])
			++scaleStops;
	}

	if (scaleStops < 2)
	{
		// special case it... chromatic
		for (int i=0; i<12; ++i)
		{
			scaleFloorMap[i] = i;
			scaleStepRunLength[i] = 1;
		}
	}
	else
	{
		int floorPitchClass = 12;
		int i=0;

		for (i = 0; i<12; ++i)		
			scaleFloorMap[i] = 12;

		i=0;
		int runLength = 0;
		while (scaleFloorMap[i] == 12)
		{
			if (activeNoteInScale[i])
			{
				// set the run length of the last active pitch class, now that we know it's length
				if (floorPitchClass < 12)
					scaleStepRunLength[floorPitchClass] = runLength;

				runLength = 1;

				floorPitchClass = i;
			}
			else
			{
				scaleStepRunLength[i] = 0;
				runLength++;
			}
			scaleFloorMap[i] = floorPitchClass;
			i = (i+1)%12;
		}

		// set last runlength
		scaleStepRunLength[floorPitchClass] = runLength;

		// make it proper offset, so octaves work properly
		for (int i=0; i<12; ++i)
		{
			if (scaleFloorMap[i] > i)
				scaleFloorMap[i] -= 12;
		}
	}

	juce::Logger::outputDebugPrintf(T("New scale: %i %i %i %i %i %i %i %i %i %i %i %i"),
		scaleFloorMap[0], scaleFloorMap[1], scaleFloorMap[2], scaleFloorMap[3], scaleFloorMap[4], scaleFloorMap[5], 
		scaleFloorMap[6], scaleFloorMap[7], scaleFloorMap[8], scaleFloorMap[9], scaleFloorMap[10], scaleFloorMap[11]);

	juce::Logger::outputDebugPrintf(T("New runlengths: %i %i %i %i %i %i %i %i %i %i %i %i"),
		scaleStepRunLength[0], scaleStepRunLength[1], scaleStepRunLength[2], scaleStepRunLength[3], scaleStepRunLength[4], scaleStepRunLength[5], 
		scaleStepRunLength[6], scaleStepRunLength[7], scaleStepRunLength[8], scaleStepRunLength[9], scaleStepRunLength[10], scaleStepRunLength[11]);	
}

int HarmScaleFilter::calculatePitchbendForNote(int inputNote, int inputPitchWheel, int lastOutputNote)
{
	// remap value based on shift
	float semitoneModByPitchWheel = (inputPitchWheel-8192)/(8192.0f/12);
	float inputShiftedPitch = semitoneModByPitchWheel + inputNote;
	int intShiftedPitch = static_cast<int>(std::floor(inputShiftedPitch));

	// calculate percent of sweep through the pitch-run block
	float sweep = (inputShiftedPitch-(scaleFloorMap[intShiftedPitch % 12] + 12*(intShiftedPitch/12)))/scaleStepRunLength[scaleFloorMap[intShiftedPitch%12]%12];

	float harmonyScaledPitch = inputShiftedPitch + transposeSemitones;
	int scaledRoundedPitch = static_cast<int>(std::floor(harmonyScaledPitch/* + 0.5f*/));
	int pitchBin = scaleFloorMap[scaledRoundedPitch % 12];
	float targetPitch = pitchBin + 12*(scaledRoundedPitch/12) + sweep*0.5f*scaleStepRunLength[pitchBin];

	// recalculate pitch bend value to map last output note to targetPitch
	float semitoneOffsetFromOutputNote = targetPitch-lastOutputNote;
	int newPitchWheel = 8192+static_cast<int>(8192*semitoneOffsetFromOutputNote/12);

	//juce::Logger::outputDebugPrintf(T("Pitch bend in: %i, making input note a %i, swept %f, and out note: %i, and outwheel: %i"), pitchWheel, static_cast<int>(inputShiftedPitch+0.5f), sweep, 12*(scaledRoundedPitch/12) + pitchBin, newPitchWheel);
	juce::Logger::outputDebugPrintf(T("IN: %i/%i/%f, sweep: %f, OUT: %i/%i/%f"),
		inputNote, inputPitchWheel, inputShiftedPitch,
		sweep,
		lastOutputNote, newPitchWheel, targetPitch);

	return jmin(16383, jmax(0, newPitchWheel));
}
