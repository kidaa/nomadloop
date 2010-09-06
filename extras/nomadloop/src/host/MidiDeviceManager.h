#ifndef ADLER_MIDIDEVICEMANAGER
#define ADLER_MIDIDEVICEMANAGER

#include "../includes.h"

// Simple global visitor pattern to allow MIDI input to get to all graph nodes which desire it
class MidiDeviceManager : public MidiInputCallback
{
	Array<MidiInputCallback*> callbacks;
	
public:
	MidiDeviceManager();
	~MidiDeviceManager();
	
	void handleIncomingMidiMessage(MidiInput *source, const MidiMessage& message);
	
	void addCallback(MidiInputCallback *callback);
	void removeCallback(MidiInputCallback *callback);
	
	juce_DeclareSingleton (MidiDeviceManager, true)
};

#endif
