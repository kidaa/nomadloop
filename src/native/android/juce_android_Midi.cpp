/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
const StringArray MidiOutput::getDevices()
{
    StringArray devices;

    return devices;
}

int MidiOutput::getDefaultDeviceIndex()
{
    return 0;
}

MidiOutput* MidiOutput::openDevice (int index)
{
    return 0;
}

MidiOutput::~MidiOutput()
{
}

void MidiOutput::reset()
{
}

bool MidiOutput::getVolume (float&, float&)
{
    return false;
}

void MidiOutput::setVolume (float, float)
{
}

void MidiOutput::sendMessageNow (const MidiMessage&)
{
}

//==============================================================================
MidiInput::MidiInput (const String& name_)
    : name (name_),
      internal (0)
{
}

MidiInput::~MidiInput()
{
}

void MidiInput::start()
{
}

void MidiInput::stop()
{
}

int MidiInput::getDefaultDeviceIndex()
{
    return 0;
}

const StringArray MidiInput::getDevices()
{
    StringArray devs;

    return devs;
}

MidiInput* MidiInput::openDevice (int index, MidiInputCallback* callback)
{
    return 0;
}


#endif
