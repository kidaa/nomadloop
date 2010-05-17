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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ComponentDragger.h"
#include "../juce_Component.h"


//==============================================================================
ComponentDragger::ComponentDragger()
    : constrainer (0)
{
}

ComponentDragger::~ComponentDragger()
{
}

//==============================================================================
void ComponentDragger::startDraggingComponent (Component* const componentToDrag,
                                               ComponentBoundsConstrainer* const constrainer_)
{
    jassert (componentToDrag->isValidComponent());

    if (componentToDrag != 0)
    {
        constrainer = constrainer_;
        originalPos = componentToDrag->relativePositionToGlobal (Point<int>());
    }
}

void ComponentDragger::dragComponent (Component* const componentToDrag, const MouseEvent& e)
{
    jassert (componentToDrag->isValidComponent());
    jassert (e.mods.isAnyMouseButtonDown()); // (the event has to be a drag event..)

    if (componentToDrag != 0)
    {
        Rectangle<int> bounds (componentToDrag->getBounds().withPosition (originalPos));

        const Component* const parentComp = componentToDrag->getParentComponent();
        if (parentComp != 0)
            bounds.setPosition (parentComp->globalPositionToRelative (originalPos));

        bounds.setPosition (bounds.getPosition() + e.getOffsetFromDragStart());

        if (constrainer != 0)
            constrainer->setBoundsForComponent (componentToDrag, bounds, false, false, false, false);
        else
            componentToDrag->setBounds (bounds);
    }
}


END_JUCE_NAMESPACE
