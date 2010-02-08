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
#include "GrooveGridFilter.h"
#include "GrooveGridEditorComponent.h"
#include <cmath>


//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrooveGridFilter();
}

//==============================================================================
GrooveGridFilter::GrooveGridFilter()
{
    gain = 1.0f;
    lastUIWidth = 400;
    lastUIHeight = 140;

	lastStep = 0;

	for (int i=0; i<16; ++i)
	for (int j=0; j<8; ++j)
	{
		grid[i][j] = 0;
	}

    zeromem (&lastPosInfo, sizeof (lastPosInfo));
    lastPosInfo.timeSigNumerator = 4;
    lastPosInfo.timeSigDenominator = 4;
    lastPosInfo.bpm = 120;
}

GrooveGridFilter::~GrooveGridFilter()
{
}

//==============================================================================
const String GrooveGridFilter::getName() const
{
    return "Groove Grid";
}

int GrooveGridFilter::getNumParameters()
{
    return 0;	
}

float GrooveGridFilter::getParameter (int index)
{
	if (index == 0)
		return gain;

	return 0.f;
}

void GrooveGridFilter::setParameter (int index, float newValue)
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
}

const String GrooveGridFilter::getParameterName (int index)
{
    if (index == 0)
        return T("Gain");	

    return String::empty;
}

const String GrooveGridFilter::getParameterText (int index)
{
    if (index == 0)
        return String (gain, 2);
	
    return String::empty;
}

const String GrooveGridFilter::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String GrooveGridFilter::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool GrooveGridFilter::isInputChannelStereoPair (int index) const
{
    return false;
}

bool GrooveGridFilter::isOutputChannelStereoPair (int index) const
{
    return false;
}

bool GrooveGridFilter::acceptsMidi() const
{
    return true;
}

bool GrooveGridFilter::producesMidi() const
{
    return true;
}

//==============================================================================
void GrooveGridFilter::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // do your pre-playback setup stuff here..    
}

void GrooveGridFilter::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void GrooveGridFilter::processBlock (AudioSampleBuffer& buffer,
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

	// TODO: extract any needed info from the MIDI stream before clearing the buffer
midiMessages.clear();

    // have a go at getting the current time from the host, and if it's changed, tell
    // our UI to update itself.
    AudioPlayHead::CurrentPositionInfo pos;

    if (getPlayHead() != 0 && getPlayHead()->getCurrentPosition (pos))
    {
        if (memcmp (&pos, &lastPosInfo, sizeof (pos)) != 0)
        {
            lastPosInfo = pos;
            sendChangeMessage (this);

			const int ppqPerBar = (pos.timeSigNumerator * 4 / pos.timeSigDenominator);
		    const double beats  = (fmod (pos.ppqPosition, ppqPerBar) / ppqPerBar) * pos.timeSigNumerator;

			//const int bar       = ((int) pos.ppqPosition) / ppqPerBar + 1;
			const int beat      = ((int) beats) + 1;
			const int sixteenths = ((int)beats)*4 + ((int) (fmod (beats, 1.0) * 4.0));
		    const int ticks     = ((int) (fmod (beats, 1.0) * 960.0));

			// if we've arrived at the next sixteenth note in the sequence yet
			if (sixteenths == (lastStep+1)%16)
			{
				// output the MIDI notes
				lastStep = sixteenths;				
				MidiBuffer buffer;

				buffer.addEvent(MidiMessage::allNotesOff(10),0);
				
				for (int i=0; i<8; ++i)
				{
					if (grid[sixteenths%16][i])
					{
						buffer.addEvent(MidiMessage::noteOn(10, 32+i, (uint8)90), 1);
					}
				}

				midiMessages.swap(buffer);
			}
        }
    }
    else
    {
        zeromem (&lastPosInfo, sizeof (lastPosInfo));
        lastPosInfo.timeSigNumerator = 4;
        lastPosInfo.timeSigDenominator = 4;
        lastPosInfo.bpm = 120;
    }
}

//==============================================================================
AudioProcessorEditor* GrooveGridFilter::createEditor()
{
    return new GrooveGridEditorComponent (this);	
}

//==============================================================================
void GrooveGridFilter::getStateInformation (MemoryBlock& destData)
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

void GrooveGridFilter::setStateInformation (const void* data, int sizeInBytes)
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
