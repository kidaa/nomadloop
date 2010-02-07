#ifndef ADLER_CONTROLSURFACE
#define ADLER_CONTROLSURFACE

#include "GraphEditorPanel.h"
#include "ControlActions.h"

class SubviewComponent : public Component
{
	Component* overlayComponent;
	Component* movingComp;
	Component* resizingComp;

public:
	SubviewComponent();
	virtual ~SubviewComponent();

	void paint(Graphics& g);
	void resized();

	void mouseDown(const juce::MouseEvent& e);
	void mouseMove(const juce::MouseEvent& e);

	void setMovingComponent(Component* c);
	void setResizingComponent(Component* c);
};

// Base class for a GUI widget for controlling arbitrary commands
// via touch
class ControlSurfaceComponent : public Component
{
	/*Component* overlayComponent;
	Component* movingComp;
	Component* resizingComp;*/

	SubviewComponent* view;

	XmlElement* createSubviewXml(SubviewComponent*) const;
	void restoreSubviewFromXml (SubviewComponent* container, const XmlElement& xml);

public:
	ControlSurfaceComponent();
	virtual ~ControlSurfaceComponent();

	void paint(Graphics& g);
	void resized();

	/*void mouseDown(const MouseEvent& e);
	void mouseMove(const MouseEvent& e);

	void setMovingComponent(Component* c);
	void setResizingComponent(Component* c);*/

	// ===========
	XmlElement* createXml() const;
	void restoreFromXml (const XmlElement& xml);
};

class ControlSurfaceMappableComponent : public Component, public AsyncUpdater
{
public:
	ControlSurfaceMappableComponent(const juce::String& componentName);
	virtual ~ControlSurfaceMappableComponent();

	void showContextMenu();

	virtual void handleAsyncUpdate();
	virtual void refresh() = 0;

	/*int getBoundNodeId() const
	{
		if (boundNode != 0)
			return boundNode->id;
		return 0;
	}

	int getBoundParameterIndex() const
	{
		return boundParameterIndex;
	}

	void setBoundParameter(GraphDocumentComponent* graphDoc, const uint32 nodeId, int parameterIdx)
	{
		if (nodeId != 0)
		{
			//GraphDocumentComponent* graphDoc = dynamic_cast<MainHostWindow*>(getTopLevelComponent())->getGraphEditor();
			
			boundNode = graphDoc->graph.getNodeForId(nodeId);
			if (boundNode != 0)
			{
				boundParameterIndex = parameterIdx;
				boundNode->processor->addListener(this);
				getUpdatedValue();
			}
		}
	}*/

	void setValue(float value);

	// safe to call from GUI thread
	float getValue();

	void setText(const String&);
	const String getText() const;

	void createControlActionsXml (XmlElement*) const;
	void restoreControlActionsFromXml (GraphDocumentComponent*, const XmlElement*);

protected:	
	OwnedArray<ControlAction> controlActions;
	/*AudioProcessorGraph::Node::Ptr boundNode;
	int boundParameterIndex;*/

	// do not call from GUI thread
	float getUpdatedValue();
	


private:
	float value;
	String text;
};

#endif
