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

#ifndef __JUCER_HEADER_CONTROLASSIGNMENTDIALOGCOMPONENT_CONTROLASSIGNMENTDIALOGCOMPONENT_D934D364__
#define __JUCER_HEADER_CONTROLASSIGNMENTDIALOGCOMPONENT_CONTROLASSIGNMENTDIALOGCOMPONENT_D934D364__

//[Headers]     -- You can add your own extra header files here --
#include "juce.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ControlAssignmentDialogComponent  : public Component,
                                          public ComboBoxListener,
                                          public ButtonListener
{
public:
    //==============================================================================
    ControlAssignmentDialogComponent ();
    ~ControlAssignmentDialogComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void buttonClicked (Button* buttonThatWasClicked);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    TabbedComponent* tabbedComponent;
    ComboBox* filterSelectionComboBox;
    Label* label;
    ToggleButton* inputToggleButton;
    ToggleButton* outputToggleButton;
    TextButton* addButton;
    ComboBox* paramSelectionComboBox;
    Label* parameterLabel;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    ControlAssignmentDialogComponent (const ControlAssignmentDialogComponent&);
    const ControlAssignmentDialogComponent& operator= (const ControlAssignmentDialogComponent&);
};


#endif   // __JUCER_HEADER_CONTROLASSIGNMENTDIALOGCOMPONENT_CONTROLASSIGNMENTDIALOGCOMPONENT_D934D364__
