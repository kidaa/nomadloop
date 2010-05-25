/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifdef ADD_TO_LIST
  ADD_TO_LIST (TextButtonHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class TextButtonHandler  : public ComponentTypeHelper<TextButton>
{
public:
    TextButtonHandler() : ComponentTypeHelper<TextButton> ("TextButton", "TextButton", "TEXTBUTTON", "textButton")
    {
        addEditableColour (TextButton::buttonColourId, "Background", "backgroundColour");
        addEditableColour (TextButton::textColourOffId, "Text Colour", "textColour");
    }

    ~TextButtonHandler()  {}

    Component* createComponent()                { return new TextButton(); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 150, 24); }

    void initialiseNew (ComponentTypeInstance& item)
    {
        item.set (Ids::text, "New Button");
        item.set (Ids::radioGroup, 0);
        item.set (Ids::connectedLeft, false);
        item.set (Ids::connectedRight, false);
        item.set (Ids::connectedTop, false);
        item.set (Ids::connectedBottom, false);
    }

    static int getConnectedFlags (ComponentTypeInstance& item)
    {
        int connected = 0;
        if (item [Ids::connectedLeft])    connected |= TextButton::ConnectedOnLeft;
        if (item [Ids::connectedRight])   connected |= TextButton::ConnectedOnRight;
        if (item [Ids::connectedTop])     connected |= TextButton::ConnectedOnTop;
        if (item [Ids::connectedBottom])  connected |= TextButton::ConnectedOnBottom;
        return connected;
    }

    void update (ComponentTypeInstance& item, TextButton* comp)
    {
        comp->setButtonText (item [Ids::text].toString());
        comp->setRadioGroupId (item [Ids::radioGroup]);
        comp->setConnectedEdges (getConnectedFlags (item));
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addTooltipProperty (props);
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (item.getValue (Ids::text), "Button Text", 1024, false));
        props.getLast()->setTooltip ("The button's text.");

        props.add (new TextPropertyComponent (Value (new NumericValueSource<int> (item.getValue (Ids::radioGroup))), "Radio Group", 8, false));
        props.getLast()->setTooltip ("The radio group that this button is a member of.");

        props.add (new BooleanPropertyComponent (item.getValue (Ids::connectedLeft), "Connected left", "Connected"));
        props.add (new BooleanPropertyComponent (item.getValue (Ids::connectedRight), "Connected right", "Connected"));
        props.add (new BooleanPropertyComponent (item.getValue (Ids::connectedTop), "Connected top", "Connected"));
        props.add (new BooleanPropertyComponent (item.getValue (Ids::connectedBottom), "Connected bottom", "Connected"));

        addEditableColourProperties (item, props);
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode
            << item.createConstructorStatement (String::empty)
            << item.getMemberName() << "->setButtonText (" << CodeHelpers::stringLiteral (item [Ids::text]) << ");" << newLine;

        if ((int) item [Ids::radioGroup] != 0)
            code.constructorCode << item.getMemberName() << "->setRadioGroupId (" << String ((int) item [Ids::radioGroup]) << ");" << newLine;

        if (getConnectedFlags (item) != 0)
            code.constructorCode << item.getMemberName() << "->setConnectedEdges (" << String (getConnectedFlags (item)) << ");" << newLine;
    }
};


#endif
