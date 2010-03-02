/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_MOUSELISTENER_JUCEHEADER__
#define __JUCE_MOUSELISTENER_JUCEHEADER__

class MouseEvent;

//==============================================================================
/**
    A MouseListener can be registered with a component to receive callbacks
    about mouse events that happen to that component.

    @see Component::addMouseListener, Component::removeMouseListener
*/
class JUCE_API  MouseListener
{
public:
    /** Destructor. */
    virtual ~MouseListener()  {}

    /** Called when the mouse moves inside a component.

        If the mouse button isn't pressed and the mouse moves over a component,
        this will be called to let the component react to this.

        A component will always get a mouseEnter callback before a mouseMove.

        @param e    details about the position and status of the mouse event, including
                    the source component in which it occurred
        @see mouseEnter, mouseExit, mouseDrag, contains
    */
    virtual void mouseMove          (const MouseEvent& e);

    /** Called when the mouse first enters a component.

        If the mouse button isn't pressed and the mouse moves into a component,
        this will be called to let the component react to this.

        When the mouse button is pressed and held down while being moved in
        or out of a component, no mouseEnter or mouseExit callbacks are made - only
        mouseDrag messages are sent to the component that the mouse was originally
        clicked on, until the button is released.

        @param e    details about the position and status of the mouse event, including
                    the source component in which it occurred
        @see mouseExit, mouseDrag, mouseMove, contains
    */
    virtual void mouseEnter         (const MouseEvent& e);

    /** Called when the mouse moves out of a component.

        This will be called when the mouse moves off the edge of this
        component.

        If the mouse button was pressed, and it was then dragged off the
        edge of the component and released, then this callback will happen
        when the button is released, after the mouseUp callback.

        @param e    details about the position and status of the mouse event, including
                    the source component in which it occurred
        @see mouseEnter, mouseDrag, mouseMove, contains
    */
    virtual void mouseExit          (const MouseEvent& e);

    /** Called when a mouse button is pressed.

        The MouseEvent object passed in contains lots of methods for finding out
        which button was pressed, as well as which modifier keys (e.g. shift, ctrl)
        were held down at the time.

        Once a button is held down, the mouseDrag method will be called when the
        mouse moves, until the button is released.

        @param e    details about the position and status of the mouse event, including
                    the source component in which it occurred
        @see mouseUp, mouseDrag, mouseDoubleClick, contains
    */
    virtual void mouseDown          (const MouseEvent& e);

    /** Called when the mouse is moved while a button is held down.

        When a mouse button is pressed inside a component, that component
        receives mouseDrag callbacks each time the mouse moves, even if the
        mouse strays outside the component's bounds.

        @param e    details about the position and status of the mouse event, including
                    the source component in which it occurred
        @see mouseDown, mouseUp, mouseMove, contains, setDragRepeatInterval
    */
    virtual void mouseDrag          (const MouseEvent& e);

    /** Called when a mouse button is released.

        A mouseUp callback is sent to the component in which a button was pressed
        even if the mouse is actually over a different component when the
        button is released.

        The MouseEvent object passed in contains lots of methods for finding out
        which buttons were down just before they were released.

        @param e    details about the position and status of the mouse event, including
                    the source component in which it occurred
        @see mouseDown, mouseDrag, mouseDoubleClick, contains
    */
    virtual void mouseUp            (const MouseEvent& e);

    /** Called when a mouse button has been double-clicked on a component.

        The MouseEvent object passed in contains lots of methods for finding out
        which button was pressed, as well as which modifier keys (e.g. shift, ctrl)
        were held down at the time.

        @param e    details about the position and status of the mouse event, including
                    the source component in which it occurred
        @see mouseDown, mouseUp
    */
    virtual void mouseDoubleClick   (const MouseEvent& e);

    /** Called when the mouse-wheel is moved.

        This callback is sent to the component that the mouse is over when the
        wheel is moved.

        If not overridden, the component will forward this message to its parent, so
        that parent components can collect mouse-wheel messages that happen to
        child components which aren't interested in them.

        @param e    details about the position and status of the mouse event, including
                    the source component in which it occurred
        @param wheelIncrementX   the speed and direction of the horizontal scroll-wheel - a positive
                                 value means the wheel has been pushed to the right, negative means it
                                 was pushed to the left
        @param wheelIncrementY   the speed and direction of the vertical scroll-wheel - a positive
                                 value means the wheel has been pushed upwards, negative means it
                                 was pushed downwards
    */
    virtual void mouseWheelMove     (const MouseEvent& e,
                                     float wheelIncrementX,
                                     float wheelIncrementY);
};


#endif   // __JUCE_MOUSELISTENER_JUCEHEADER__
