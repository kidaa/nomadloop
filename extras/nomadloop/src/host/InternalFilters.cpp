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

#include "../includes.h"
#include "InternalFilters.h"
#include "FilterGraph.h"
#include "Looper.h"
#include "../filters/UtilityFilters.h"

//==============================================================================
InternalPluginFormat::InternalPluginFormat()
{
    {
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
        p.fillInPluginDescription (audioOutDesc);
    }

    {
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
        p.fillInPluginDescription (audioInDesc);
    }

    {
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
        p.fillInPluginDescription (midiInDesc);
    }

	{
		AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode);
		p.fillInPluginDescription (midiOutDesc);
	}

	{
		AudioLoopProcessor p;
		p.fillInPluginDescription (looperDesc);
	}

	{
		MidiLoopProcessor p;
		p.fillInPluginDescription (midiLooperDesc);
	}

	{
		GainCut p;
		p.fillInPluginDescription (gainCutDesc);
	}

	{
		Arpeggiator p;
		p.fillInPluginDescription (arpeggiatorDesc);
	}
}

AudioPluginInstance* InternalPluginFormat::createInstanceFromDescription (const PluginDescription& desc)
{
    if (desc.name == audioOutDesc.name)
    {
        return new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
    }
    else if (desc.name == audioInDesc.name)
    {
        return new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
    }
    else if (desc.name == midiInDesc.name)
    {
        return new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
    }
	else if (desc.name == midiOutDesc.name)
	{
		return new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode);
	}
	else if (desc.name == looperDesc.name)
	{
		return new AudioLoopProcessor();
	}
	else if (desc.name == midiLooperDesc.name)
	{
		return new MidiLoopProcessor();
	}
	else if (desc.name == gainCutDesc.name)
	{
		return new GainCut();
	}
	else if (desc.name == arpeggiatorDesc.name)
	{
		return new Arpeggiator();
	}

    return 0;
}

const PluginDescription* InternalPluginFormat::getDescriptionFor (const InternalFilterType type)
{
    switch (type)
    {
    case audioInputFilter:
        return &audioInDesc;
    case audioOutputFilter:
        return &audioOutDesc;
    case midiInputFilter:
        return &midiInDesc;
	case midiOutputFilter:
		return &midiOutDesc;
	case looperFilter:
		return &looperDesc;
	case midiLooperFilter:
		return &midiLooperDesc;
	case gainCutFilter:
		return &gainCutDesc;
	case arpeggiatorFilter:
		return &arpeggiatorDesc;
    }

    return 0;
}

void InternalPluginFormat::getAllTypes (OwnedArray <PluginDescription>& results)
{
    for (int i = 0; i < (int) endOfFilterTypes; ++i)
        results.add (new PluginDescription (*getDescriptionFor ((InternalFilterType) i)));
}
