#include "../includes.h"
#include "ControlSurface.h"
#include "LoopControls.h"
#include "ControlActions.h"
#include "MainHostWindow.h"

class ControlAssignmentDialogComponent : public Component, public ButtonListener, public ComboBoxListener
{
	GraphDocumentComponent* graphDoc;

    ComboBox* filterSelectionComboBox;
    Label* label;
    ToggleButton* inputToggleButton;
    ToggleButton* outputToggleButton;
    TextButton* addButton;
    TabbedComponent* tabbedComponent;

	// Param tab controls
	Component* tabComponentParam;
	ComboBox* paramSelectionComboBox;
	Label* parameterLabel;

	// MIDI tab controls
	Component* tabComponentMidi;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    ControlAssignmentDialogComponent (const ControlAssignmentDialogComponent&);
    const ControlAssignmentDialogComponent& operator= (const ControlAssignmentDialogComponent&);

public:
	ControlAssignmentDialogComponent(GraphDocumentComponent* graphDoc)
		: graphDoc(graphDoc)
	{		
		addAndMakeVisible (filterSelectionComboBox = new ComboBox (T("Filter Selection")));
		filterSelectionComboBox->setEditableText (false);
		filterSelectionComboBox->setJustificationType (Justification::centredLeft);
		filterSelectionComboBox->setTextWhenNothingSelected (String::empty);
		filterSelectionComboBox->setTextWhenNoChoicesAvailable (T("(no choices)"));
		
		// first find the filter with the most parameters, so we can allocate our menu item
		// values to return the proper parameter
		int itemsPerFilter = 0;
		for (int i=graphDoc->graph.getNumFilters(); --i >= 0;)
		{
			itemsPerFilter = jmax(itemsPerFilter, graphDoc->graph.getNode(i)->processor->getNumParameters());
		}

		for (int i=0; i<graphDoc->graph.getNumFilters(); ++i)
		{
			/*if (graphDoc->graph.getNode(i)->processor->getNumParameters() > 0)
			{
				PopupMenu paramMenu;
				for (int j=0; j<graphDoc->graph.getNode(i)->processor->getNumParameters(); ++j)
				{
					juce::String paramName(graphDoc->graph.getNode(i)->processor->getParameterName(j));
					if (paramName.isEmpty())
						paramName = T("Unknown");
					paramMenu.addItem(i*itemsPerFilter + j, paramName);
				}
				filterMenu.addSubMenu(graphDoc->graph.getNode(i)->processor->getName(), paramMenu);
			}*/
			filterSelectionComboBox->addItem(graphDoc->graph.getNode(i)->processor->getName(), i+1);
		}
		filterSelectionComboBox->addListener (this);

		addAndMakeVisible (label = new Label (T("new label"),
											  T("Graph Node")));
		label->setFont (Font (15.0000f, Font::plain));
		label->setJustificationType (Justification::centredRight);
		label->setEditable (false, false, false);
		label->setColour (TextEditor::textColourId, Colours::black);
		label->setColour (TextEditor::backgroundColourId, Colour (0x0));

		addAndMakeVisible (inputToggleButton = new ToggleButton (T("inputToggleButton")));
		inputToggleButton->setButtonText (T("Follow input"));
		inputToggleButton->setConnectedEdges (Button::ConnectedOnRight);
		//inputToggleButton->addButtonListener (this);
		inputToggleButton->setToggleState (true, false);

		addAndMakeVisible (outputToggleButton = new ToggleButton (T("outputToggleButton")));
		outputToggleButton->setButtonText (T("Send output"));
		//outputToggleButton->addButtonListener (this);
		outputToggleButton->setToggleState (true, false);

		addAndMakeVisible (addButton = new TextButton (T("addButton")));
		addButton->setButtonText (T("Add Assignment"));
		addButton->addButtonListener (this);

		addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
		tabbedComponent->setTabBarDepth (30);
		tabbedComponent->addTab (T("Plugin Param"), Colours::lightgrey, 0, false);
		tabbedComponent->addTab (T("MIDI"), Colours::lightgrey, 0, false);
		tabbedComponent->addTab (T("OSC"), Colours::lightgrey, 0, false);
		tabbedComponent->setCurrentTabIndex (0);

		addAndMakeVisible (paramSelectionComboBox = new ComboBox (T("paramSelectionComboBox")));
		paramSelectionComboBox->setEditableText (false);
		paramSelectionComboBox->setJustificationType (Justification::centredLeft);
		paramSelectionComboBox->setTextWhenNothingSelected (String::empty);
		paramSelectionComboBox->setTextWhenNoChoicesAvailable (T("(no choices)"));
		paramSelectionComboBox->addListener (this);

		addAndMakeVisible (parameterLabel = new Label (T("parameterLabel"),
													   T("Parameter")));
		parameterLabel->setFont (Font (15.0000f, Font::plain));
		parameterLabel->setJustificationType (Justification::centredRight);
		parameterLabel->setEditable (false, false, false);
		parameterLabel->setColour (TextEditor::textColourId, Colours::black);
		parameterLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));


		setSize (480, 320);
	
	}

	~ControlAssignmentDialogComponent()
	{
		deleteAllChildren();
	}

	void resized()
	{
		filterSelectionComboBox->setBounds (224, 16, 150, 24);
		label->setBounds (112, 16, 104, 24);
		inputToggleButton->setBounds (128, 48, 120, 24);
		outputToggleButton->setBounds (256, 48, 120, 24);
		addButton->setBounds (168, 288, 150, 24);
		tabbedComponent->setBounds (8, 72, 464, 208);

		paramSelectionComboBox->setBounds (224, 120, 150, 24);
		parameterLabel->setBounds (64, 120, 150, 24);
	}

	void buttonClicked (Button* button)
	{
		if (button == addButton)
		{
			// close and add it
			
		}
	}

	void comboBoxChanged (ComboBox* comboBox)
	{
		if (comboBox == filterSelectionComboBox)
		{
			// repopulate the controls in the param combo box
			paramSelectionComboBox->clear();
			int nodeIdx = filterSelectionComboBox->getSelectedId() - 1;
			if (graphDoc->graph.getNode(nodeIdx)->processor->getNumParameters() > 0)
			{				
				for (int j=0; j<graphDoc->graph.getNode(nodeIdx)->processor->getNumParameters(); ++j)
				{
					String paramName(graphDoc->graph.getNode(nodeIdx)->processor->getParameterName(j));
					if (paramName.isEmpty())
						paramName = T("(Unknown)");
					paramSelectionComboBox->addItem(paramName, j+1);
				}
				//filterMenu.addSubMenu(graphDoc->graph.getNode(i)->processor->getName(), paramMenu);
			}
		}
	}

	ControlAction* getChosenAction()
	{		
		if (tabbedComponent->getCurrentTabIndex() == 0)
		{
			// PluginParam
			int filterIndex = filterSelectionComboBox->getSelectedId() - 1;
			int paramIndex = paramSelectionComboBox->getSelectedId() - 1;

			Logger::outputDebugPrintf(T("Selected filter %i and parameter %i"), filterIndex, paramIndex);
			if (filterIndex == -1 || paramIndex == -1)
				return 0;
			return new PluginParameterControlAction(graphDoc->graph.getNode(filterIndex), paramIndex);
		}
		else if (tabbedComponent->getCurrentTabIndex() == 1)
		{
			// MIDI
		}
		return 0;
	}
};

// ================================================

ControlSurfaceMappableComponent::ControlSurfaceMappableComponent(const String& componentName)
	: Component(componentName), value(0.f), text(T("Test"))
{
}

ControlSurfaceMappableComponent::~ControlSurfaceMappableComponent()
{		
}

void ControlSurfaceMappableComponent::showContextMenu()
{
	PopupMenu m;
	m.addItem (-1, "Move");
	m.addItem (-2, "Resize");
	m.addItem (-3, "Delete");
	m.addSeparator();
	m.addItem (-4, "Assign new ControlAction...");
	m.addItem (-5, "MIDI Learn");

	
	GraphDocumentComponent* graphDoc = dynamic_cast<MainHostWindow*>(getTopLevelComponent())->getGraphEditor();

	// first find the filter with the most parameters, so we can allocate our menu item
	// values to return the proper parameter
	int itemsPerFilter = 0;
	for (int i=graphDoc->graph.getNumFilters(); --i >= 0;)
	{
		itemsPerFilter = jmax(itemsPerFilter, graphDoc->graph.getNode(i)->processor->getNumParameters());
	}

	/*PopupMenu filterMenu;
	for (int i=0; i<graphDoc->graph.getNumFilters(); ++i)
	{
		if (graphDoc->graph.getNode(i)->processor->getNumParameters() > 0)
		{
			PopupMenu paramMenu;
			for (int j=0; j<graphDoc->graph.getNode(i)->processor->getNumParameters(); ++j)
			{
				juce::String paramName(graphDoc->graph.getNode(i)->processor->getParameterName(j));
				if (paramName.isEmpty())
					paramName = T("Unknown");
				paramMenu.addItem(i*itemsPerFilter + j, paramName);
			}
			filterMenu.addSubMenu(graphDoc->graph.getNode(i)->processor->getName(), paramMenu);
		}
	}
	m.addSubMenu(T("Assignment"), filterMenu);*/

	int choice = m.show();

	if (choice > 0)
	{
		// find plugin and parameter
		int filterIndex = choice/itemsPerFilter;
		int paramIndex = choice%itemsPerFilter;

		Logger::outputDebugPrintf(T("Selected filter %i and parameter %i"), filterIndex, paramIndex);
		PluginParameterControlAction* action = new PluginParameterControlAction(graphDoc->graph.getNode(filterIndex), paramIndex);
		action->setMappedComponent(this);
		controlActions.add(action);
		
		/*boundNode = graphDoc->graph.getNode(filterIndex);
		boundParameterIndex = paramIndex;
		boundNode->processor->addListener(this);*/

		refresh();
	}
	else if (choice == -1)
	{
		static_cast<SubviewComponent*>(this->getParentComponent())->setMovingComponent(this);
	}
	else if (choice == -2)
	{
		static_cast<SubviewComponent*>(this->getParentComponent())->setResizingComponent(this);
	}
	else if (choice == -3)
	{
		delete this;
	}
	else if (choice == -4)
	{
		ControlAssignmentDialogComponent c(graphDoc);
		DialogWindow::showModalDialog(T("Control Assignment"), &c, 0, Colours::azure, false);

		ControlAction* action = c.getChosenAction();

		// find plugin and parameter
		/*int filterIndex = choice/itemsPerFilter;
		int paramIndex = choice%itemsPerFilter;

		juce::Logger::outputDebugPrintf(T("Selected filter %i and parameter %i"), filterIndex, paramIndex);
		PluginParameterControlAction* action = new PluginParameterControlAction(graphDoc->graph.getNode(filterIndex), paramIndex);*/
		if (action != 0)
		{
			action->setMappedComponent(this);
			controlActions.add(action);
		}
		
		/*boundNode = graphDoc->graph.getNode(filterIndex);
		boundParameterIndex = paramIndex;
		boundNode->processor->addListener(this);*/

		refresh();
	}
	else if (choice == -5)
	{
		// MIDI learn
		MidiControlAction* midiAction = new MidiControlAction(0, MidiMessage(0xf0));
		midiAction->setMappedComponent(this);
		controlActions.add(midiAction);
		midiAction->learn();
	}
}


/*virtual void audioProcessorParameterChanged(AudioProcessor *processor, int parameterIndex, float value)
{
	Logger::outputDebugPrintf(T("got update: new value %f"), value);
	if (parameterIndex == boundParameterIndex)
		triggerAsyncUpdate();
}

virtual void audioProcessorChanged(AudioProcessor *processor)
{
}*/

void ControlSurfaceMappableComponent::handleAsyncUpdate()
{
	refresh();
}

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

void ControlSurfaceMappableComponent::setValue(float value)
{
	/*this->value = value;
	if (boundNode != 0)
	{			
		boundNode->processor->setParameter(boundParameterIndex, value);
	}*/

	this->value = value;
	for (int i=controlActions.size(); --i >= 0; )
	{
		controlActions.getUnchecked(i)->setValue(value);
	}

}

void ControlSurfaceMappableComponent::setText(const String& newText)
{
	text = newText;
}

const String ControlSurfaceMappableComponent::getText() const
{
	if (controlActions.size() > 0)
	{
		return controlActions.getUnchecked(0)->getText();
	}
	return text;
}

void ControlSurfaceMappableComponent::createControlActionsXml (XmlElement* controlSurfaceElement) const
{
	for (int i=0; i<controlActions.size(); ++i)
	{
		XmlElement* controlActionElement = 0;
		const std::type_info& type = typeid(*controlActions.getUnchecked(i));

		if (type == typeid(MidiControlAction))
		{
			controlActionElement = new XmlElement(T("MidiControlAction"));
			controlActionElement->setAttribute(T("channel"), 1);
		}
		else if (type == typeid(PluginParameterControlAction))
		{
			PluginParameterControlAction* ppca = dynamic_cast<PluginParameterControlAction*>(controlActions.getUnchecked(i));
			controlActionElement = new XmlElement(T("PluginParameterControlAction"));
			controlActionElement->setAttribute (T("nodeId"), ppca->getBoundNodeId());
			controlActionElement->setAttribute (T("nodeParam"), ppca->getBoundParameterIndex());
		}
	
		controlSurfaceElement->addChildElement(controlActionElement);
	}
}

void ControlSurfaceMappableComponent::restoreControlActionsFromXml (GraphDocumentComponent* graphDoc, const XmlElement* controlElement)
{
	controlActions.clear();

	forEachXmlChildElement (*controlElement, controlActionElement)
	{
		if (controlActionElement->hasTagName(T("MidiControlAction")))
		{
		}
		else if (controlActionElement->hasTagName(T("PluginParameterControlAction")))
		{
			controlActions.add(new PluginParameterControlAction(
				graphDoc->graph.getNodeForId(controlActionElement->getIntAttribute(T("nodeId"))),
				controlActionElement->getIntAttribute(T("nodeParam"))));
		}
	}
}

// do not call from GUI thread
float ControlSurfaceMappableComponent::getUpdatedValue()
{
	/*if (boundNode != 0)
	{
		
		return value = boundNode->processor->getParameter(boundParameterIndex);
	}
	return 0;*/
	return value;
}

// safe to call from GUI thread
float ControlSurfaceMappableComponent::getValue()
{
	return value;
}

class TouchVerticalSlider : public ControlSurfaceMappableComponent
{
public:
	TouchVerticalSlider(const String& componentName)
		: ControlSurfaceMappableComponent(componentName), draggingKnob(false)
	{
	}

	~TouchVerticalSlider()
	{
		deleteAllChildren();
	}

	void refresh()
	{
		getUpdatedValue();
		repaint();
	}

	void paint(Graphics &g)
	{
		Colour colour(Colours::azure);

		g.setColour(colour);
		g.drawRoundedRectangle(2.f, 2.f, getWidth()-4.f, getHeight()-4.f, 16.0f, 4.0f);

		this->getLookAndFeel().drawGlassLozenge(g, 0.f, (1.f-getValue())*(getHeight()-getWidth()), static_cast<float>(getWidth()), static_cast<float>(getWidth()), colour, 1.0f, 16.0f, false, false, false, false);
		g.setColour(Colours::black);
		
		g.setFont(Font(T("BoomBox 2"), 16.0f, 0));
		g.drawFittedText(getText(), 0, static_cast<int>((1.f-getValue())*(getHeight()-getWidth())), getWidth(), getWidth(), Justification(Justification::centred), 1);		
	}

	void mouseDown(const MouseEvent &e)
	{
		if (e.mods.isPopupMenu())
		{
			showContextMenu();
		}
		else
		{			
			int knobTop = static_cast<int>((1.f-getValue())*(getHeight()-getWidth()));
			int knobBottom = knobTop + getWidth();

			if (e.y >= knobTop && e.y <= knobBottom)
			{
				// begin dragging the knob
				draggingKnob = true;
				knobClickOffsetY = e.y - knobTop;
				Logger::outputDebugPrintf(T("beginning drag"));
			}

		}
	}

	void mouseDrag(const MouseEvent &e)
	{
		if (draggingKnob)
		{
			//Logger::outputDebugPrintf(T("drag Y to %i, new value %f"), e.y, 1.f - (e.y - knobClickOffsetY)/static_cast<float>(getHeight()-getWidth()));
			setValue(jmin(1.f,jmax(0.f, 1.f - (e.y - knobClickOffsetY)/static_cast<float>(getHeight()-getWidth()))));
			repaint();
		}
	}

	void mouseUp(const MouseEvent &e)
	{		
		draggingKnob = false;
	}

private:
	//float value;
	bool draggingKnob;
	int knobClickOffsetY;
};

class TouchHorizontalSlider : public ControlSurfaceMappableComponent
{
public:
	TouchHorizontalSlider(const String& componentName)
		: ControlSurfaceMappableComponent(componentName), draggingKnob(false)
	{
	}

	~TouchHorizontalSlider()
	{
		deleteAllChildren();
	}

	void refresh()
	{
		getUpdatedValue();
		repaint();
	}

	void paint(Graphics &g)
	{
		Colour colour(Colours::azure);

		g.setColour(colour);
		g.drawRoundedRectangle(2.f, 2.f, getWidth()-4.f, getHeight()-4.f, 16.0f, 4.0f);

		this->getLookAndFeel().drawGlassLozenge(g, getValue()*(getWidth()-getHeight()), 0.f, static_cast<float>(getHeight()), static_cast<float>(getHeight()), colour, 1.0f, 16.0f, false, false, false, false);
		g.setColour(Colours::black);
		
		g.setFont(Font(T("BoomBox 2"), 16.0f, 0));
		g.drawFittedText(getText(), static_cast<int>(getValue()*(getWidth()-getHeight())), 0, getHeight(), getHeight(), Justification(Justification::centred), 1);
	}

	void mouseDown(const MouseEvent &e)
	{
		if (e.mods.isPopupMenu())
		{
			showContextMenu();
		}
		else
		{			
			int knobLeft = static_cast<int>(getValue()*(getWidth()-getHeight()));
			int knobRight = knobLeft + getWidth();

			if (e.x >= knobLeft && e.x <= knobRight)
			{
				// begin dragging the knob
				draggingKnob = true;
				knobClickOffsetX = e.x - knobLeft;
				Logger::outputDebugPrintf(T("beginning drag"));
			}
		}
	}

	void mouseDrag(const MouseEvent &e)
	{
		if (draggingKnob)
		{
			//Logger::outputDebugPrintf(T("drag Y to %i, new value %f"), e.y, 1.f - (e.y - knobClickOffsetY)/static_cast<float>(getHeight()-getWidth()));
			setValue(jmin(1.f,jmax(0.f, (e.x - knobClickOffsetX)/static_cast<float>(getWidth()-getHeight()))));
			repaint();
		}
	}

	void mouseUp(const MouseEvent &e)
	{		
		draggingKnob = false;
	}

private:
	//float value;
	bool draggingKnob;
	int knobClickOffsetX;
};

class XYDragger : public ControlSurfaceMappableComponent
{
	ComponentDragger dragger;

public:

	XYDragger(const String& name)
		: ControlSurfaceMappableComponent(name)
	{
	}

	void refresh()
	{
	}

	void paint(Graphics& g)
	{
		Colour colour(Colours::papayawhip);

		this->getLookAndFeel().drawGlassLozenge(g, 0.f, (1.f-getValue())*(getHeight()-getWidth()), static_cast<float>(getWidth()), static_cast<float>(getWidth()), colour, 1.0f, getWidth()/2.0f, false, false, false, false);
		g.setColour(Colours::black);
		
	}

	void mouseDown(const MouseEvent& e)
	{
		if (e.mods.isPopupMenu())
		{
			showContextMenu();
		}
		else
		{			
			// TODO: add constraints so we can't drag it outside the parent area
			dragger.startDraggingComponent(this, 0);
		}
	}

	void mouseDrag(const MouseEvent& e)
	{
		dragger.dragComponent(this, e);
		setValue(this->getX() / static_cast<float>(this->getParentComponent()->getWidth() - this->getWidth()));
		// TODO: update
	}

};

class RotaryKnob : public ControlSurfaceMappableComponent, public SliderListener
{
	Slider* pot;
public:
	RotaryKnob(const String& componentName)
		: ControlSurfaceMappableComponent(componentName)
	{
		pot = new Slider(componentName);
		pot->setSliderStyle(Slider::Rotary);
		pot->setColour(Slider::rotarySliderFillColourId, Colours::aquamarine);
		pot->setColour(Slider::rotarySliderOutlineColourId, Colours::aquamarine.darker());
		pot->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
		pot->setRange(0.f, 1.f);
		pot->addListener(this);
		pot->addMouseListener(this, true);
		
		addAndMakeVisible(pot);
	}

	~RotaryKnob()
	{
		pot->removeMouseListener(this);
		pot->removeListener(this);
		deleteAllChildren();
	}

	void resized()
	{
		pot->setBounds(0, 0, getWidth(), getHeight());
	}

	void refresh()
	{
		pot->setValue(getUpdatedValue());
		repaint();
	}

	void mouseDown(const MouseEvent &e)
	{
		if (e.mods.isPopupMenu())
		{
			showContextMenu();
		}
	}

	void sliderValueChanged(Slider *slider)
	{
		setValue(slider->getValue());
	}

	void sliderDragStarted(Slider *slider)
	{
	}

	void sliderDragEnded(Slider *slider)
	{
	}

};


// A touchable button
class TouchButton : public ControlSurfaceMappableComponent
{
public:
	TouchButton(const String& componentName)
		: ControlSurfaceMappableComponent(componentName)
	{
	}

	~TouchButton()
	{
		deleteAllChildren();
	}

	void refresh()
	{
		getValue();
		repaint();
	}

	void paint(Graphics &g)
	{
		Colour colour(Colours::red);

		if (isOn())
		{
			this->getLookAndFeel().drawGlassLozenge(g, 0.f, 0.f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), colour, 1.0f, 16.0f, false, false, false, false);
			g.setColour(Colours::black);
		}		
		else
		{
			g.setColour(colour);
			g.drawRoundedRectangle(2.f, 2.f, getWidth()-4.f, getHeight()-4.f, 16.0f, 4.0f);
		}
		g.setFont(Font(T("BoomBox 2"), 16.0f, 0));
		g.drawFittedText(getText(), 0, 0, getWidth(), getHeight(), Justification(Justification::centred), 1);
	}

	void mouseDown(const MouseEvent &e)
	{
		if (e.mods.isPopupMenu())
		{
			showContextMenu();
		}
		else
		{			
			setValue(isOn()?0.f:1.f);		
			repaint();
		}
	}

	bool isOn()
	{
		return getValue() >= 0.5f;
	}
};
///////////////////


SubviewComponent::SubviewComponent()
: movingComp(0), resizingComp(0)
{
	addAndMakeVisible(overlayComponent = new Component());
	overlayComponent->addMouseListener(this, false);
	overlayComponent->setTopLeftPosition(0, 0);
	overlayComponent->toBack();
}

SubviewComponent::~SubviewComponent()
{
	overlayComponent->removeMouseListener(this);
	deleteAllChildren();
}

void SubviewComponent::paint(Graphics &g)
{
	/*if (dynamic_cast<SubviewComponent*>(getParentComponent()) == 0)
		g.fillAll(Colours::black);
	else
		g.fillAll(Colours::black);*/

	g.fillAll(Colours::black);
	g.setColour(Colours::cyan);	
	g.drawRoundedRectangle(2.f, 2.f, getWidth()-4.f, getHeight()-4.f, 16.0f, 4.0f);
}

void SubviewComponent::resized()
{
	overlayComponent->setSize(getWidth(), getHeight());
}

void SubviewComponent::mouseDown(const MouseEvent &e)
{
	if (movingComp != 0)
	{
		// end the moving operation on mouse down
		movingComp = 0;
		overlayComponent->toBack();
		return;
	}
	else if (resizingComp != 0)
	{
		resizingComp = 0;
		overlayComponent->toBack();
		return;
	}

	if (e.mods.isPopupMenu())
	{
		PopupMenu m;

		if (dynamic_cast<SubviewComponent*>(getParentComponent()) != 0)
		{
			m.addItem(-1, T("Move"));
			m.addItem(-2, T("Resize"));
			m.addItem(-3, T("Delete"));
			m.addSeparator();
		}
		m.addItem(1, T("Button"));
		m.addItem(2, T("Vertical fader"));
		m.addItem(3, T("Horizontal fader"));
		m.addItem(4, T("Rotary knob"));
		m.addItem(5, T("2D Dragger"));
		m.addSeparator();
		m.addItem(6, T("Add subview"));
		m.addItem(5, T("Add PresetSaver"));
		m.addItem(7, T("Add Plugin editor"));
		m.addSeparator();
		m.addItem(8, T("Loop view"));
		m.addItem(9, T("Arranger"));

		int choice = m.show();

		if (choice == -1)
		{
			static_cast<SubviewComponent*>(this->getParentComponent())->setMovingComponent(this);
		}
		else if (choice == -2)
		{
			static_cast<SubviewComponent*>(this->getParentComponent())->setResizingComponent(this);
		}
		else if (choice == -3)
		{
			delete this;
		}
		else if (choice == 1)
		{
			TouchButton* b = new TouchButton(T("Button"));
			b->setBounds(e.x, e.y, 96, 64);			
			addAndMakeVisible(b);
		}
		else if (choice == 2)
		{
			TouchVerticalSlider* s = new TouchVerticalSlider(T("Slider"));			
			s->setBounds(e.x, e.y, 64, 256);			
			addAndMakeVisible(s);
		}
		else if (choice == 3)
		{
			TouchHorizontalSlider* s = new TouchHorizontalSlider(T("Slider"));			
			s->setBounds(e.x, e.y, 256, 64);
			addAndMakeVisible(s);
		}
		else if (choice == 4)
		{
			RotaryKnob* r = new RotaryKnob(T("RotaryKnob"));
			r->setBounds(e.x, e.y, 64, 64);
			addAndMakeVisible(r);
		}
		else if (choice == 5)
		{
			XYDragger* d = new XYDragger(T("Dragger"));
			d->setBounds(e.x, e.y, 64, 64);
			addAndMakeVisible(d);
		}
		else if (choice == 6)
		{
			SubviewComponent* sc = new SubviewComponent();
			sc->setBounds(e.x, e.y, getWidth() / 2, getHeight() / 2);
			addAndMakeVisible(sc);
		}
		else if (choice == 7)
		{
			GraphDocumentComponent* graphDoc = dynamic_cast<MainHostWindow*>(getTopLevelComponent())->getGraphEditor();
			PopupMenu filterMenu;
			for (int i=0; i<graphDoc->graph.getNumFilters(); ++i)
			{
				filterMenu.addItem(i+1, graphDoc->graph.getNode(i)->processor->getName());				
			}
			int filterIndex = filterMenu.show();
			if (filterIndex > 0)
			{
				AudioProcessorEditor* ui = graphDoc->graph.getNode(filterIndex-1)->processor->createEditorIfNeeded();
				if (ui == 0)
				{
					ui = new GenericAudioProcessorEditor (graphDoc->graph.getNode(filterIndex-1)->processor);
				}
				ui->setTopLeftPosition(e.x, e.y);
				addAndMakeVisible(ui);
			}
		}
		else if (choice == 8)
		{
			LoopComponent* lc = new LoopComponent();
			lc->setBounds(e.x, e.y, 192, 96);
			addAndMakeVisible(lc);
		}
	}
}

void SubviewComponent::mouseMove(const MouseEvent &e)
{
	if (movingComp != 0)
	{
		movingComp->setTopLeftPosition((e.x/16)*16, (e.y/16)*16);
	}
	else if (resizingComp != 0)
	{
		int x = resizingComp->getX(), y = resizingComp->getY();
		resizingComp->setBounds(x, y, jmax(e.x - x, 32)/16*16, jmax(e.y - y, 32)/16*16);
	}
}

void SubviewComponent::setMovingComponent(Component *c)
{
	movingComp = c;
	overlayComponent->toFront(true);
}

void SubviewComponent::setResizingComponent(Component *c)
{
	resizingComp = c;
	overlayComponent->toFront(true);
}



///////////////////

ControlSurfaceComponent::ControlSurfaceComponent()
{
	/*addAndMakeVisible(overlayComponent = new Component());
	overlayComponent->addMouseListener(this, false);
	overlayComponent->setTopLeftPosition(0, 0);
	overlayComponent->toBack();*/
	addAndMakeVisible(dashboard = new DashboardComponent());
	addAndMakeVisible(view = new SubviewComponent());
}

ControlSurfaceComponent::~ControlSurfaceComponent()
{
	//overlayComponent->removeMouseListener(this);
	deleteAllChildren();
}

void ControlSurfaceComponent::paint(Graphics &g)
{
	g.fillAll(Colours::black);
}

void ControlSurfaceComponent::resized()
{
	dashboard->setBounds(0,0, getWidth(), 96);
	view->setBounds(0,96, getWidth(), getHeight()-96);
	//overlayComponent->setSize(getWidth(), getHeight());
}

/*void ControlSurfaceComponent::mouseDown(const MouseEvent &e)
{
	if (movingComp != 0)
	{
		// end the moving operation on mouse down
		movingComp = 0;
		overlayComponent->toBack();
		return;
	}
	else if (resizingComp != 0)
	{
		resizingComp = 0;
		overlayComponent->toBack();
		return;
	}

	if (e.mods.isPopupMenu())
	{
		PopupMenu m;

		m.addItem(1, T("Button"));
		m.addItem(2, T("Vertical fader"));
		m.addItem(3, T("Horizontal fader"));
		m.addItem(4, T("Rotary knob"));
		m.addItem(5, T("2D Dragger"));
		m.addSeparator();
		m.addItem(6, T("Add subview"));
		m.addItem(5, T("Add PresetSaver"));

		int choice = m.show();

		if (choice == 1)
		{
			TouchButton* b = new TouchButton(T("Button"));
			b->setBounds(e.x, e.y, 96, 64);			
			addAndMakeVisible(b);
		}
		else if (choice == 2)
		{
			TouchVerticalSlider* s = new TouchVerticalSlider(T("Slider"));			
			s->setBounds(e.x, e.y, 64, 256);			
			addAndMakeVisible(s);
		}
		else if (choice == 3)
		{
			TouchHorizontalSlider* s = new TouchHorizontalSlider(T("Slider"));			
			s->setBounds(e.x, e.y, 256, 64);
			addAndMakeVisible(s);
		}
		else if (choice == 4)
		{
			RotaryKnob* r = new RotaryKnob(T("RotaryKnob"));
			r->setBounds(e.x, e.y, 64, 64);
			addAndMakeVisible(r);
		}
		else if (choice == 6)
		{
			// subview

		}
	}
}

void ControlSurfaceComponent::mouseMove(const juce::MouseEvent &e)
{
	if (movingComp != 0)
	{
		movingComp->setTopLeftPosition((e.x/16)*16, (e.y/16)*16);
	}
	else if (resizingComp != 0)
	{
		int x = resizingComp->getX(), y = resizingComp->getY();
		resizingComp->setBounds(x, y, jmax(e.x - x, 32)/16*16, jmax(e.y - y, 32)/16*16);
	}
}

void ControlSurfaceComponent::setMovingComponent(juce::Component *c)
{
	movingComp = c;
	overlayComponent->toFront(true);
}

void ControlSurfaceComponent::setResizingComponent(juce::Component *c)
{
	resizingComp = c;
	overlayComponent->toFront(true);
}*/

XmlElement* ControlSurfaceComponent::createXml() const
{
	XmlElement* xml = new XmlElement(T("CONTROLSURFACE"));
	xml->addChildElement (createSubviewXml(view));
	return xml;
}

void ControlSurfaceComponent::restoreFromXml(const XmlElement& xml)
{
	deleteAllChildren();

	// recreate the component mover layer
	addAndMakeVisible(view = new SubviewComponent());
	view->setBounds(0, 0, getWidth(), getHeight());
	/*addAndMakeVisible(overlayComponent = new Component());
	overlayComponent->addMouseListener(this, false);
	overlayComponent->setBounds(0, 0, getWidth(), getHeight());
	overlayComponent->toBack();*/

	SubviewComponent* currentView = view;
	restoreSubviewFromXml (currentView, xml);
}

XmlElement* ControlSurfaceComponent::createSubviewXml(SubviewComponent* subviewComp) const
{
	XmlElement* subviewCompElement = new XmlElement(T("VIEW"));	
	subviewCompElement->setAttribute (T("x"), subviewComp->getX());
	subviewCompElement->setAttribute (T("y"), subviewComp->getY());
	subviewCompElement->setAttribute (T("width"), subviewComp->getWidth());
	subviewCompElement->setAttribute (T("height"), subviewComp->getHeight());

	for (int i=0; i < subviewComp->getNumChildComponents(); ++i)
	{
		ControlSurfaceMappableComponent* touchComp = dynamic_cast<ControlSurfaceMappableComponent*>(subviewComp->getChildComponent(i));
		if (touchComp != 0)
		{
			String touchCompName;

			const std::type_info& type = typeid(*touchComp);

			if (type == typeid(TouchButton))
				touchCompName = T("BUTTON");
			else if (type == typeid(TouchVerticalSlider))
				touchCompName = T("VSLIDER");
			else if (type == typeid(TouchHorizontalSlider))
				touchCompName = T("HSLIDER");
			else if (type == typeid(RotaryKnob))
				touchCompName = T("ROTARYKNOB");
			else
				touchCompName = type.name();

			XmlElement* touchCompElement = new XmlElement(touchCompName);
			touchCompElement->setAttribute (T("x"), touchComp->getX());
			touchCompElement->setAttribute (T("y"), touchComp->getY());
			touchCompElement->setAttribute (T("width"), touchComp->getWidth());
			touchCompElement->setAttribute (T("height"), touchComp->getHeight());

			/*touchCompElement->setAttribute (T("nodeId"), touchComp->getBoundNodeId());
			touchCompElement->setAttribute (T("nodeParam"), touchComp->getBoundParameterIndex());*/

			touchComp->createControlActionsXml(touchCompElement);

			subviewCompElement->addChildElement (touchCompElement);
		}
		else if (dynamic_cast<SubviewComponent*>(subviewComp->getChildComponent(i)) != 0)
		{
			XmlElement* subSubview = createSubviewXml(static_cast<SubviewComponent*>(subviewComp->getChildComponent(i)));
			
			subviewCompElement->addChildElement (subSubview);
		}
	}

	return subviewCompElement;
}

void ControlSurfaceComponent::restoreSubviewFromXml (SubviewComponent* container, const XmlElement& xml)
{
	forEachXmlChildElement (xml, el)
	{
		if (el->hasTagName(T("VIEW")))
		{
			// TODO: recursively restore view
			SubviewComponent* nestedView = new SubviewComponent();
			container->addAndMakeVisible(nestedView);
			nestedView->setBounds(el->getIntAttribute(T("x")),
					el->getIntAttribute(T("y")),
					el->getIntAttribute(T("width")),
					el->getIntAttribute(T("height")));
			restoreSubviewFromXml(nestedView, *el);
		}
		else
		{
			ControlSurfaceMappableComponent* touchComp = 0;

			if (el->hasTagName(T("BUTTON")))
			{
				touchComp = new TouchButton(T("Button"));
			}
			else if (el->hasTagName(T("VSLIDER")))
			{
				touchComp = new TouchVerticalSlider(T("Slider"));
			}
			else if (el->hasTagName(T("HSLIDER")))
			{
				touchComp = new TouchHorizontalSlider(T("Slider"));
			}
			else if (el->hasTagName(T("ROTARYKNOB")))
			{
				touchComp = new RotaryKnob(T("RotaryKnob"));
			}
			
			if (touchComp != 0)
			{
				touchComp->setBounds(el->getIntAttribute(T("x")),
					el->getIntAttribute(T("y")),
					el->getIntAttribute(T("width")),
					el->getIntAttribute(T("height")));
				/*int boundId = el->getIntAttribute(T("nodeId"));
				if (boundId != 0)
				{
					GraphDocumentComponent* graphDoc = dynamic_cast<MainHostWindow*>(getTopLevelComponent())->getGraphEditor();
					touchComp->setBoundParameter(graphDoc, boundId, el->getIntAttribute(T("nodeParam")));					
				}*/
				GraphDocumentComponent* graphDoc = dynamic_cast<MainHostWindow*>(getTopLevelComponent())->getGraphEditor();
				touchComp->restoreControlActionsFromXml(graphDoc, el);
				container->addAndMakeVisible(touchComp);
			}
		}
	}
}

