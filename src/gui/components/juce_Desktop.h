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

#ifndef __JUCE_DESKTOP_JUCEHEADER__
#define __JUCE_DESKTOP_JUCEHEADER__

#include "juce_Component.h"
#include "../../core/juce_Time.h"
#include "../../utilities/juce_DeletedAtShutdown.h"
#include "../../events/juce_Timer.h"
#include "../../events/juce_AsyncUpdater.h"
#include "../../containers/juce_OwnedArray.h"
#include "../graphics/geometry/juce_RectangleList.h"
class MouseInputSource;
class MouseInputSourceInternal;
class MouseListener;


//==============================================================================
/**
    Classes can implement this interface and register themselves with the Desktop class
    to receive callbacks when the currently focused component changes.

    @see Desktop::addFocusChangeListener, Desktop::removeFocusChangeListener
*/
class JUCE_API  FocusChangeListener
{
public:
    /** Destructor. */
    virtual ~FocusChangeListener()  {}

    /** Callback to indicate that the currently focused component has changed. */
    virtual void globalFocusChanged (Component* focusedComponent) = 0;
};


//==============================================================================
/**
    Describes and controls aspects of the computer's desktop.

*/
class JUCE_API  Desktop  : private DeletedAtShutdown,
                           private Timer,
                           private AsyncUpdater
{
public:
    //==============================================================================
    /** There's only one dektop object, and this method will return it.
    */
    static Desktop& JUCE_CALLTYPE getInstance();

    //==============================================================================
    /** Returns a list of the positions of all the monitors available.

        The first rectangle in the list will be the main monitor area.

        If clippedToWorkArea is true, it will exclude any areas like the taskbar on Windows,
        or the menu bar on Mac. If clippedToWorkArea is false, the entire monitor area is returned.
    */
    const RectangleList getAllMonitorDisplayAreas (bool clippedToWorkArea = true) const throw();

    /** Returns the position and size of the main monitor.

        If clippedToWorkArea is true, it will exclude any areas like the taskbar on Windows,
        or the menu bar on Mac. If clippedToWorkArea is false, the entire monitor area is returned.
    */
    const Rectangle<int> getMainMonitorArea (bool clippedToWorkArea = true) const throw();

    /** Returns the position and size of the monitor which contains this co-ordinate.

        If none of the monitors contains the point, this will just return the
        main monitor.

        If clippedToWorkArea is true, it will exclude any areas like the taskbar on Windows,
        or the menu bar on Mac. If clippedToWorkArea is false, the entire monitor area is returned.
    */
    const Rectangle<int> getMonitorAreaContaining (const Point<int>& position, bool clippedToWorkArea = true) const;


    //==============================================================================
    /** Returns the mouse position.

        The co-ordinates are relative to the top-left of the main monitor.
    */
    static const Point<int> getMousePosition();

    /** Makes the mouse pointer jump to a given location.

        The co-ordinates are relative to the top-left of the main monitor.
    */
    static void setMousePosition (const Point<int>& newPosition);

    /** Returns the last position at which a mouse button was pressed.
    */
    static const Point<int> getLastMouseDownPosition() throw();

    /** Returns the number of times the mouse button has been clicked since the
        app started.

        Each mouse-down event increments this number by 1.
    */
    static int getMouseButtonClickCounter() throw();

    //==============================================================================
    /** This lets you prevent the screensaver from becoming active.

        Handy if you're running some sort of presentation app where having a screensaver
        appear would be annoying.

        Pass false to disable the screensaver, and true to re-enable it. (Note that this
        won't enable a screensaver unless the user has actually set one up).

        The disablement will only happen while the Juce application is the foreground
        process - if another task is running in front of it, then the screensaver will
        be unaffected.

        @see isScreenSaverEnabled
    */
    static void setScreenSaverEnabled (bool isEnabled) throw();

    /** Returns true if the screensaver has not been turned off.

        This will return the last value passed into setScreenSaverEnabled(). Note that
        it won't tell you whether the user is actually using a screen saver, just
        whether this app is deliberately preventing one from running.

        @see setScreenSaverEnabled
    */
    static bool isScreenSaverEnabled() throw();

    //==============================================================================
    /** Registers a MouseListener that will receive all mouse events that occur on
        any component.

        @see removeGlobalMouseListener
    */
    void addGlobalMouseListener (MouseListener* listener);

    /** Unregisters a MouseListener that was added with the addGlobalMouseListener()
        method.

        @see addGlobalMouseListener
    */
    void removeGlobalMouseListener (MouseListener* listener);

    //==============================================================================
    /** Registers a MouseListener that will receive a callback whenever the focused
        component changes.
    */
    void addFocusChangeListener (FocusChangeListener* listener);

    /** Unregisters a listener that was added with addFocusChangeListener(). */
    void removeFocusChangeListener (FocusChangeListener* listener);

    //==============================================================================
    /** Takes a component and makes it full-screen, removing the taskbar, dock, etc.

        The component must already be on the desktop for this method to work. It will
        be resized to completely fill the screen and any extraneous taskbars, menu bars,
        etc will be hidden.

        To exit kiosk mode, just call setKioskModeComponent (0). When this is called,
        the component that's currently being used will be resized back to the size
        and position it was in before being put into this mode.

        If allowMenusAndBars is true, things like the menu and dock (on mac) are still
        allowed to pop up when the mouse moves onto them. If this is false, it'll try
        to hide as much on-screen paraphenalia as possible.
    */
    void setKioskModeComponent (Component* componentToUse,
                                bool allowMenusAndBars = true);

    /** Returns the component that is currently being used in kiosk-mode.

        This is the component that was last set by setKioskModeComponent(). If none
        has been set, this returns 0.
    */
    Component* getKioskModeComponent() const throw()                { return kioskModeComponent; }

    //==============================================================================
    /** Returns the number of components that are currently active as top-level
        desktop windows.

        @see getComponent, Component::addToDesktop
    */
    int getNumComponents() const throw();

    /** Returns one of the top-level desktop window components.

        The index is from 0 to getNumComponents() - 1. This could return 0 if the
        index is out-of-range.

        @see getNumComponents, Component::addToDesktop
    */
    Component* getComponent (int index) const throw();

    /** Finds the component at a given screen location.

        This will drill down into top-level windows to find the child component at
        the given position.

        Returns 0 if the co-ordinates are inside a non-Juce window.
    */
    Component* findComponentAt (const Point<int>& screenPosition) const;


    //==============================================================================
    /** Returns the number of MouseInputSource objects the system has at its disposal.
        In a traditional single-mouse system, there might be only one object. On a multi-touch
        system, there could be one input source per potential finger.
        To find out how many mouse events are currently happening, use getNumDraggingMouseSources().
        @see getMouseSource
    */
    int getNumMouseSources() const throw()                          { return mouseSources.size(); }

    /** Returns one of the system's MouseInputSource objects.
        The index should be from 0 to getNumMouseSources() - 1. Out-of-range indexes will return
        a null pointer.
        In a traditional single-mouse system, there might be only one object. On a multi-touch
        system, there could be one input source per potential finger.
    */
    MouseInputSource* getMouseSource (int index) const throw()      { return mouseSources [index]; }

    /** Returns the main mouse input device that the system is using.
        @see getNumMouseSources()
    */
    MouseInputSource& getMainMouseSource() const throw()            { return *mouseSources.getUnchecked(0); }

    /** Returns the number of mouse-sources that are currently being dragged.
        In a traditional single-mouse system, this will be 0 or 1, depending on whether a
        juce component has the button down on it. In a multi-touch system, this could
        be any number from 0 to the number of simultaneous touches that can be detected.
    */
    int getNumDraggingMouseSources() const throw();

    /** Returns one of the mouse sources that's currently being dragged.
        The index should be between 0 and getNumDraggingMouseSources() - 1. If the index is
        out of range, or if no mice or fingers are down, this will return a null pointer.
    */
    MouseInputSource* getDraggingMouseSource (int index) const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

    /** Tells this object to refresh its idea of what the screen resolution is.

        (Called internally by the native code).
    */
    void refreshMonitorSizes();

    /** True if the OS supports semitransparent windows */
    static bool canUseSemiTransparentWindows() throw();

private:
    //==============================================================================
    static Desktop* instance;

    friend class Component;
    friend class ComponentPeer;
    friend class MouseInputSource;
    friend class MouseInputSourceInternal;
    friend class DeletedAtShutdown;
    friend class TopLevelWindowManager;

    OwnedArray <MouseInputSource> mouseSources;
    void createMouseInputSources();

    ListenerList <MouseListener> mouseListeners;
    ListenerList <FocusChangeListener> focusListeners;

    Array <Component*> desktopComponents;
    Array <Rectangle<int> > monitorCoordsClipped, monitorCoordsUnclipped;

    Point<int> lastFakeMouseMove;
    void sendMouseMove();

    int mouseClickCounter;
    void incrementMouseClickCounter() throw();

    Component* kioskModeComponent;
    Rectangle<int> kioskComponentOriginalBounds;

    void timerCallback();
    void resetTimer();

    int getNumDisplayMonitors() const throw();
    const Rectangle<int> getDisplayMonitorCoordinates (int index, bool clippedToWorkArea) const throw();

    void addDesktopComponent (Component* c);
    void removeDesktopComponent (Component* c);
    void componentBroughtToFront (Component* c);

    void triggerFocusCallback();
    void handleAsyncUpdate();

    Desktop();
    ~Desktop();

    Desktop (const Desktop&);
    Desktop& operator= (const Desktop&);
};


#endif   // __JUCE_DESKTOP_JUCEHEADER__
