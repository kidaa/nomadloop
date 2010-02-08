/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  27 Nov 2009 3:33:57 pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "ControlAssignmentDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ControlAssignmentDialogComponent::ControlAssignmentDialogComponent ()
    : tabbedComponent (0),
      filterSelectionComboBox (0),
      label (0),
      inputToggleButton (0),
      outputToggleButton (0),
      addButton (0),
      paramSelectionComboBox (0),
      parameterLabel (0)
{
    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    tabbedComponent->setTabBarDepth (30);
    tabbedComponent->addTab (T("Plugin Param"), Colours::white, 0, false);
    tabbedComponent->addTab (T("MIDI"), Colours::lightgrey, 0, false);
    tabbedComponent->addTab (T("OSC"), Colours::lightgrey, 0, false);
    tabbedComponent->setCurrentTabIndex (0);

    addAndMakeVisible (filterSelectionComboBox = new ComboBox (T("Filter Selection")));
    filterSelectionComboBox->setEditableText (false);
    filterSelectionComboBox->setJustificationType (Justification::centredLeft);
    filterSelectionComboBox->setTextWhenNothingSelected (String::empty);
    filterSelectionComboBox->setTextWhenNoChoicesAvailable (T("(no choices)"));
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
    inputToggleButton->addButtonListener (this);
    inputToggleButton->setToggleState (true, false);

    addAndMakeVisible (outputToggleButton = new ToggleButton (T("outputToggleButton")));
    outputToggleButton->setButtonText (T("Send output"));
    outputToggleButton->addButtonListener (this);
    outputToggleButton->setToggleState (true, false);

    addAndMakeVisible (addButton = new TextButton (T("addButton")));
    addButton->setButtonText (T("Add Assignment"));
    addButton->addButtonListener (this);

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


    //[UserPreSize]
    //[/UserPreSize]

    setSize (480, 320);

    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ControlAssignmentDialogComponent::~ControlAssignmentDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (tabbedComponent);
    deleteAndZero (filterSelectionComboBox);
    deleteAndZero (label);
    deleteAndZero (inputToggleButton);
    deleteAndZero (outputToggleButton);
    deleteAndZero (addButton);
    deleteAndZero (paramSelectionComboBox);
    deleteAndZero (parameterLabel);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ControlAssignmentDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ControlAssignmentDialogComponent::resized()
{
    tabbedComponent->setBounds (8, 72, 464, 208);
    filterSelectionComboBox->setBounds (224, 16, 150, 24);
    label->setBounds (112, 16, 104, 24);
    inputToggleButton->setBounds (128, 48, 120, 24);
    outputToggleButton->setBounds (256, 48, 120, 24);
    addButton->setBounds (168, 288, 150, 24);
    paramSelectionComboBox->setBounds (224, 120, 150, 24);
    parameterLabel->setBounds (64, 120, 150, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ControlAssignmentDialogComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == filterSelectionComboBox)
    {
        //[UserComboBoxCode_filterSelectionComboBox] -- add your combo box handling code here..
        //[/UserComboBoxCode_filterSelectionComboBox]
    }
    else if (comboBoxThatHasChanged == paramSelectionComboBox)
    {
        //[UserComboBoxCode_paramSelectionComboBox] -- add your combo box handling code here..
        //[/UserComboBoxCode_paramSelectionComboBox]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void ControlAssignmentDialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == inputToggleButton)
    {
        //[UserButtonCode_inputToggleButton] -- add your button handler code here..
        //[/UserButtonCode_inputToggleButton]
    }
    else if (buttonThatWasClicked == outputToggleButton)
    {
        //[UserButtonCode_outputToggleButton] -- add your button handler code here..
        //[/UserButtonCode_outputToggleButton]
    }
    else if (buttonThatWasClicked == addButton)
    {
        //[UserButtonCode_addButton] -- add your button handler code here..
        //[/UserButtonCode_addButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ControlAssignmentDialogComponent"
                 componentName="" parentClasses="public Component" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330000013" fixedSize="1" initialWidth="480"
                 initialHeight="320">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TABBEDCOMPONENT name="" id="5de9ed93712266d8" memberName="tabbedComponent" virtualName=""
                   explicitFocusOrder="0" pos="8 72 464 208" orientation="top" tabBarDepth="30"
                   initialTab="0">
    <TAB name="Plugin Param" colour="ffffffff" useJucerComp="1" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
    <TAB name="MIDI" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
    <TAB name="OSC" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
  </TABBEDCOMPONENT>
  <COMBOBOX name="Filter Selection" id="bcf5201f87033f63" memberName="filterSelectionComboBox"
            virtualName="" explicitFocusOrder="0" pos="224 16 150 24" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="new label" id="8178ca90d846e09f" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="112 16 104 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Graph Node" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <TOGGLEBUTTON name="inputToggleButton" id="ac002aa656935184" memberName="inputToggleButton"
                virtualName="" explicitFocusOrder="0" pos="128 48 120 24" buttonText="Follow input"
                connectedEdges="2" needsCallback="1" radioGroupId="0" state="1"/>
  <TOGGLEBUTTON name="outputToggleButton" id="4cd893d247c29850" memberName="outputToggleButton"
                virtualName="" explicitFocusOrder="0" pos="256 48 120 24" buttonText="Send output"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="1"/>
  <TEXTBUTTON name="addButton" id="58798e04edef8535" memberName="addButton"
              virtualName="" explicitFocusOrder="0" pos="168 288 150 24" buttonText="Add Assignment"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <COMBOBOX name="paramSelectionComboBox" id="9ed528a120f1df1" memberName="paramSelectionComboBox"
            virtualName="" explicitFocusOrder="0" pos="224 120 150 24" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="parameterLabel" id="e899eb7f042813b4" memberName="parameterLabel"
         virtualName="" explicitFocusOrder="0" pos="64 120 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Parameter" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
