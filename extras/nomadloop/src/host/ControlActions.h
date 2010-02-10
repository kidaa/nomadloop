#ifndef ADLER_CONTROLACTIONS
#define ADLER_CONTROLACTIONS

#include "../includes.h"

class ControlSurfaceMappableComponent;

// Base class for an assignment that can be assigned to a control as an output,
// such as mapping MIDI control into a filter, the VST parameter
// of a filter, or OSC messages
class ControlAction
{
	ControlSurfaceMappableComponent* owner;
public:
	ControlAction();
	virtual ~ControlAction() = 0;

	ControlSurfaceMappableComponent* getMappedComponent() const;
	void setMappedComponent(ControlSurfaceMappableComponent*);

	// values passed in should range from [0,1]
	virtual void setValue(float value) = 0;

	virtual const String getText() const;
};

class MidiControlAction : public ControlAction
{
	//AudioProcessorGraph::Node::Ptr node;
	const MidiInput* source;
	MidiMessage messageTemplate;
	/*int channel;
	int status;
	int value;*/

	bool followInput;
	bool sendOutput;

	bool isLearning;

public:	
	MidiControlAction(/*AudioProcessorGraph::Node::Ptr node,*/ MidiInput* source, const MidiMessage& message);	
	~MidiControlAction();

	// instructs the control action to latch onto the next received remote message
	void learn();

	// returns true if the message was handled, false if not a match
	bool processIncomingMessage(const MidiMessage& message);

	void setValue(float value);
};

class MidiRemoteControlDispatcher : public MidiInputCallback
{
	Array<MidiControlAction*> midiControlActions;

	// chain to the graphPlayer
	MidiInputCallback* passThrough;

	virtual void handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message);
	virtual void handlePartialSysexMessage(MidiInput* source, const JUCE_NAMESPACE::uint8 *messageData,
		const int numBytesSoFar, const double timestamp);
public:
	inline MidiRemoteControlDispatcher() : passThrough(0)
	{
	}

	inline void setPassThrough(MidiInputCallback* p)
	{
		passThrough = p;
	}

	void addMidiControlAction(MidiControlAction*);
	void removeMidiControlAction(MidiControlAction*);

	juce_DeclareSingleton (MidiRemoteControlDispatcher, true)
};

class PluginParameterControlAction : public ControlAction, public AudioProcessorListener
{
	AudioProcessorGraph::Node::Ptr boundNode;
	int boundParameterIndex;

public:
	PluginParameterControlAction(AudioProcessorGraph::Node::Ptr node, int parameterIndex);
	~PluginParameterControlAction();

	void setValue(float value);
	const String getText() const;

	inline int getBoundNodeId() const
	{
		return boundNode->id;
	}

	inline int getBoundParameterIndex() const
	{
		return boundParameterIndex;
	}

	virtual void audioProcessorParameterChanged(AudioProcessor *processor, int parameterIndex, float value);
	virtual void audioProcessorChanged(AudioProcessor *processor);

};

#endif
