#include "MidiDeviceManager.h"

MidiDeviceManager::MidiDeviceManager()
{
}

MidiDeviceManager::~MidiDeviceManager()
{
}

void MidiDeviceManager::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
{
	for (int i=callbacks.size(); --i >= 0; )
		callbacks.getUnchecked(i)->handleIncomingMidiMessage(source, message);
}

void MidiDeviceManager::addCallback(MidiInputCallback* callback)
{
	callbacks.addIfNotAlreadyThere(callback);
}

void MidiDeviceManager::removeCallback(MidiInputCallback* callback)
{
	callbacks.removeValue(callback);
}

juce_ImplementSingleton(MidiDeviceManager);
