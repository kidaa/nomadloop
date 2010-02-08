#include "ControlActions.h"
#include "ControlSurface.h"

ControlAction::ControlAction()
{
}

ControlAction::~ControlAction()
{
}


ControlSurfaceMappableComponent* ControlAction::getMappedComponent() const
{
	return owner;
}

void ControlAction::setMappedComponent(ControlSurfaceMappableComponent* c)
{
	owner = c;
}

const String ControlAction::getText() const
{
	return T("Test");
}

// ==================================================

void MidiRemoteControlDispatcher::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
{
	bool handled = false;
	for (int i=midiControlActions.size(); --i >= 0; )
		handled = midiControlActions.getUnchecked(i)->processIncomingMessage(message) || handled;
	if (!handled && passThrough != 0)
		passThrough->handleIncomingMidiMessage(source, message);
}

void MidiRemoteControlDispatcher::handlePartialSysexMessage(MidiInput* source, const juce::uint8 *messageData,
		const int numBytesSoFar, const double timestamp)
{
}

void MidiRemoteControlDispatcher::addMidiControlAction(MidiControlAction* midiControlAction)
{
	midiControlActions.addIfNotAlreadyThere(midiControlAction);
}

void MidiRemoteControlDispatcher::removeMidiControlAction(MidiControlAction* midiControlAction)
{
	midiControlActions.removeValue(midiControlAction);
}

juce_ImplementSingleton (MidiRemoteControlDispatcher);

MidiControlAction::MidiControlAction(/*AudioProcessorGraph::Node::Ptr node,*/ MidiInput* source, const MidiMessage& message)
: /*node(node),*/ source(source), messageTemplate(message), followInput(false), sendOutput(false), isLearning(false)
{
	MidiRemoteControlDispatcher::getInstance()->addMidiControlAction(this);
}

MidiControlAction::~MidiControlAction()
{
	MidiRemoteControlDispatcher::getInstance()->removeMidiControlAction(this);
}

void MidiControlAction::learn()
{
	isLearning = true;
}

bool MidiControlAction::processIncomingMessage(const MidiMessage& message)
{
	if (isLearning)
	{
		isLearning = false;
		messageTemplate = message;
		return true;
	}
	else if (message.isForChannel(messageTemplate.getChannel()))
	{
		if (messageTemplate.isNoteOnOrOff() && message.isNoteOnOrOff() && message.getNoteNumber() == messageTemplate.getNoteNumber())
		{
			// toggle!
			if (message.isNoteOn() && message.getNoteNumber() == messageTemplate.getNoteNumber())
			{
				getMappedComponent()->setValue(getMappedComponent()->getValue() >= 0.5f?0.f:1.f);
				getMappedComponent()->triggerAsyncUpdate();
			}
			return true;
		}
		else if (messageTemplate.isController() && message.isController() && messageTemplate.getControllerNumber() == message.getControllerNumber())
		{
			//setValue(message.getControllerValue() / 127.0f);
			getMappedComponent()->setValue(message.getControllerValue() / 127.0f);		
			getMappedComponent()->triggerAsyncUpdate();
			return true;
		}
	}
	return false;
}

void MidiControlAction::setValue(float value)
{
}


// ==================================================

PluginParameterControlAction::PluginParameterControlAction(AudioProcessorGraph::Node::Ptr node, int parameterIndex)
: boundNode(node), boundParameterIndex(parameterIndex)
{
	boundNode->processor->addListener(this);
}

PluginParameterControlAction::~PluginParameterControlAction()
{
	boundNode->processor->removeListener(this);
}

void PluginParameterControlAction::setValue(float value)
{
	//this->value = value;
	if (boundNode != 0)
	{			
		boundNode->processor->setParameter(boundParameterIndex, value);
	}
}

const String PluginParameterControlAction::getText() const
{
	return boundNode->processor->getParameterText(boundParameterIndex);
}

void PluginParameterControlAction::audioProcessorParameterChanged(AudioProcessor *processor, int parameterIndex, float value)
{
	Logger::outputDebugPrintf(T("got update: new value %f"), value);
	if (parameterIndex == boundParameterIndex)
	{
		getMappedComponent()->setValue(value);		
		getMappedComponent()->triggerAsyncUpdate();
	}
}

void PluginParameterControlAction::audioProcessorChanged(AudioProcessor *processor)
{
}
