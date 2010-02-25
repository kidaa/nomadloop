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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../application/juce_Application.h"
#include "../juce_Component.h"
#include "../juce_ComponentDeletionWatcher.h"
#include "../juce_Desktop.h"
#include "../../../events/juce_MessageManager.h"
#include "../../../core/juce_Time.h"
#include "../../../core/juce_Random.h"
#include "../layout/juce_ComponentBoundsConstrainer.h"
#include "../mouse/juce_FileDragAndDropTarget.h"
#include "../mouse/juce_MouseInputSource.h"

//#define JUCE_ENABLE_REPAINT_DEBUGGING 1


//==============================================================================
static VoidArray heavyweightPeers;


//==============================================================================
ComponentPeer::ComponentPeer (Component* const component_,
                              const int styleFlags_) throw()
    : component (component_),
      styleFlags (styleFlags_),
      lastPaintTime (0),
      constrainer (0),
      lastFocusedComponent (0),
      lastDragAndDropCompUnderMouse (0),
      fakeMouseMessageSent (false),
      isWindowMinimised (false)
{
    heavyweightPeers.add (this);
}

ComponentPeer::~ComponentPeer()
{
    heavyweightPeers.removeValue (this);

    Desktop::getInstance().triggerFocusCallback();
}

//==============================================================================
int ComponentPeer::getNumPeers() throw()
{
    return heavyweightPeers.size();
}

ComponentPeer* ComponentPeer::getPeer (const int index) throw()
{
    return (ComponentPeer*) heavyweightPeers [index];
}

ComponentPeer* ComponentPeer::getPeerFor (const Component* const component) throw()
{
    for (int i = heavyweightPeers.size(); --i >= 0;)
    {
        ComponentPeer* const peer = (ComponentPeer*) heavyweightPeers.getUnchecked(i);

        if (peer->getComponent() == component)
            return peer;
    }

    return 0;
}

bool ComponentPeer::isValidPeer (const ComponentPeer* const peer) throw()
{
    return heavyweightPeers.contains (const_cast <ComponentPeer*> (peer));
}

void ComponentPeer::updateCurrentModifiers() throw()
{
    ModifierKeys::updateCurrentModifiers();
}

//==============================================================================
void ComponentPeer::handleMouseEvent (const Point<int>& positionWithinPeer, const ModifierKeys& newMods, const int64 time)
{
    Desktop::getInstance().getMainMouseSource().handleEvent (this, positionWithinPeer, time, newMods);
}

void ComponentPeer::handleMouseWheel (const Point<int>& positionWithinPeer, const int64 time, float x, float y)
{
    Desktop::getInstance().getMainMouseSource().handleWheel (this, positionWithinPeer, time, x, y);
}

//==============================================================================
void ComponentPeer::handlePaint (LowLevelGraphicsContext& contextToPaintTo)
{
    Graphics g (&contextToPaintTo);

#if JUCE_ENABLE_REPAINT_DEBUGGING
    g.saveState();
#endif

    JUCE_TRY
    {
        component->paintEntireComponent (g);
    }
    JUCE_CATCH_EXCEPTION

#if JUCE_ENABLE_REPAINT_DEBUGGING
    // enabling this code will fill all areas that get repainted with a colour overlay, to show
    // clearly when things are being repainted.
    {
        g.restoreState();

        g.fillAll (Colour ((uint8) Random::getSystemRandom().nextInt (255),
                           (uint8) Random::getSystemRandom().nextInt (255),
                           (uint8) Random::getSystemRandom().nextInt (255),
                           (uint8) 0x50));
    }
#endif
}

bool ComponentPeer::handleKeyPress (const int keyCode,
                                    const juce_wchar textCharacter)
{
    updateCurrentModifiers();

    Component* target = Component::currentlyFocusedComponent->isValidComponent()
                            ? Component::currentlyFocusedComponent
                            : component;

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        Component* const currentModalComp = Component::getCurrentlyModalComponent();

        if (currentModalComp != 0)
            target = currentModalComp;
    }

    const KeyPress keyInfo (keyCode,
                            ModifierKeys::getCurrentModifiers().getRawFlags()
                               & ModifierKeys::allKeyboardModifiers,
                            textCharacter);

    bool keyWasUsed = false;

    while (target != 0)
    {
        const ComponentDeletionWatcher deletionChecker (target);

        if (target->keyListeners_ != 0)
        {
            for (int i = target->keyListeners_->size(); --i >= 0;)
            {
                keyWasUsed = ((KeyListener*) target->keyListeners_->getUnchecked(i))->keyPressed (keyInfo, target);

                if (keyWasUsed || deletionChecker.hasBeenDeleted())
                    return keyWasUsed;

                i = jmin (i, target->keyListeners_->size());
            }
        }

        keyWasUsed = target->keyPressed (keyInfo);

        if (keyWasUsed || deletionChecker.hasBeenDeleted())
            break;

        if (keyInfo.isKeyCode (KeyPress::tabKey) && Component::getCurrentlyFocusedComponent() != 0)
        {
            Component* const currentlyFocused = Component::getCurrentlyFocusedComponent();
            currentlyFocused->moveKeyboardFocusToSibling (! keyInfo.getModifiers().isShiftDown());
            keyWasUsed = (currentlyFocused != Component::getCurrentlyFocusedComponent());
            break;
        }

        target = target->parentComponent_;
    }

    return keyWasUsed;
}

bool ComponentPeer::handleKeyUpOrDown (const bool isKeyDown)
{
    updateCurrentModifiers();

    Component* target = Component::currentlyFocusedComponent->isValidComponent()
                            ? Component::currentlyFocusedComponent
                            : component;

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        Component* const currentModalComp = Component::getCurrentlyModalComponent();

        if (currentModalComp != 0)
            target = currentModalComp;
    }

    bool keyWasUsed = false;

    while (target != 0)
    {
        const ComponentDeletionWatcher deletionChecker (target);

        keyWasUsed = target->keyStateChanged (isKeyDown);

        if (keyWasUsed || deletionChecker.hasBeenDeleted())
            break;

        if (target->keyListeners_ != 0)
        {
            for (int i = target->keyListeners_->size(); --i >= 0;)
            {
                keyWasUsed = ((KeyListener*) target->keyListeners_->getUnchecked(i))->keyStateChanged (isKeyDown, target);

                if (keyWasUsed || deletionChecker.hasBeenDeleted())
                    return keyWasUsed;

                i = jmin (i, target->keyListeners_->size());
            }
        }

        target = target->parentComponent_;
    }

    return keyWasUsed;
}

void ComponentPeer::handleModifierKeysChange()
{
    updateCurrentModifiers();

    Component* target = Component::getComponentUnderMouse();

    if (target == 0)
        target = Component::getCurrentlyFocusedComponent();

    if (target == 0)
        target = component;

    if (target->isValidComponent())
        target->internalModifierKeysChanged();
}

TextInputTarget* ComponentPeer::findCurrentTextInputTarget()
{
    Component* const c = Component::getCurrentlyFocusedComponent();
    if (component->isParentOf (c))
        return dynamic_cast <TextInputTarget*> (c);

    return 0;
}

//==============================================================================
void ComponentPeer::handleBroughtToFront()
{
    updateCurrentModifiers();

    if (component != 0)
        component->internalBroughtToFront();
}

void ComponentPeer::setConstrainer (ComponentBoundsConstrainer* const newConstrainer) throw()
{
    constrainer = newConstrainer;
}

void ComponentPeer::handleMovedOrResized()
{
    jassert (component->isValidComponent());
    updateCurrentModifiers();

    const bool nowMinimised = isMinimised();

    if (component->flags.hasHeavyweightPeerFlag && ! nowMinimised)
    {
        const ComponentDeletionWatcher deletionChecker (component);

        const Rectangle<int> newBounds (getBounds());
        const bool wasMoved   = (component->getPosition() != newBounds.getPosition());
        const bool wasResized = (component->getWidth() != newBounds.getWidth() || component->getHeight() != newBounds.getHeight());

        if (wasMoved || wasResized)
        {
            component->bounds_ = newBounds;

            if (wasResized)
                component->repaint();

            component->sendMovedResizedMessages (wasMoved, wasResized);

            if (deletionChecker.hasBeenDeleted())
                return;
        }
    }

    if (isWindowMinimised != nowMinimised)
    {
        isWindowMinimised = nowMinimised;
        component->minimisationStateChanged (nowMinimised);
        component->sendVisibilityChangeMessage();
    }

    if (! isFullScreen())
        lastNonFullscreenBounds = component->getBounds();
}

void ComponentPeer::handleFocusGain()
{
    updateCurrentModifiers();

    if (component->isParentOf (lastFocusedComponent))
    {
        Component::currentlyFocusedComponent = lastFocusedComponent;
        Desktop::getInstance().triggerFocusCallback();
        lastFocusedComponent->internalFocusGain (Component::focusChangedDirectly);
    }
    else
    {
        if (! component->isCurrentlyBlockedByAnotherModalComponent())
            component->grabKeyboardFocus();
        else
            Component::bringModalComponentToFront();
    }
}

void ComponentPeer::handleFocusLoss()
{
    updateCurrentModifiers();

    if (component->hasKeyboardFocus (true))
    {
        lastFocusedComponent = Component::currentlyFocusedComponent;

        if (lastFocusedComponent != 0)
        {
            Component::currentlyFocusedComponent = 0;
            Desktop::getInstance().triggerFocusCallback();
            lastFocusedComponent->internalFocusLoss (Component::focusChangedByMouseClick);
        }
    }
}

Component* ComponentPeer::getLastFocusedSubcomponent() const throw()
{
    return (component->isParentOf (lastFocusedComponent) && lastFocusedComponent->isShowing())
                ? lastFocusedComponent
                : component;
}

void ComponentPeer::handleScreenSizeChange()
{
    updateCurrentModifiers();

    component->parentSizeChanged();
    handleMovedOrResized();
}

void ComponentPeer::setNonFullScreenBounds (const Rectangle<int>& newBounds) throw()
{
    lastNonFullscreenBounds = newBounds;
}

const Rectangle<int>& ComponentPeer::getNonFullScreenBounds() const throw()
{
    return lastNonFullscreenBounds;
}

//==============================================================================
static FileDragAndDropTarget* findDragAndDropTarget (Component* c,
                                                     const StringArray& files,
                                                     FileDragAndDropTarget* const lastOne)
{
    while (c != 0)
    {
        FileDragAndDropTarget* const t = dynamic_cast <FileDragAndDropTarget*> (c);

        if (t != 0 && (t == lastOne || t->isInterestedInFileDrag (files)))
            return t;

        c = c->getParentComponent();
    }

    return 0;
}

void ComponentPeer::handleFileDragMove (const StringArray& files, const Point<int>& position)
{
    updateCurrentModifiers();

    FileDragAndDropTarget* lastTarget = 0;

    if (dragAndDropTargetComponent != 0 && ! dragAndDropTargetComponent->hasBeenDeleted())
        lastTarget = const_cast <FileDragAndDropTarget*> (dynamic_cast <const FileDragAndDropTarget*> (dragAndDropTargetComponent->getComponent()));

    FileDragAndDropTarget* newTarget = 0;

    Component* const compUnderMouse = component->getComponentAt (position);

    if (compUnderMouse != lastDragAndDropCompUnderMouse)
    {
        lastDragAndDropCompUnderMouse = compUnderMouse;
        newTarget = findDragAndDropTarget (compUnderMouse, files, lastTarget);

        if (newTarget != lastTarget)
        {
            if (lastTarget != 0)
                lastTarget->fileDragExit (files);

            dragAndDropTargetComponent = 0;

            if (newTarget != 0)
            {
                Component* const targetComp = dynamic_cast <Component*> (newTarget);
                const Point<int> pos (component->relativePositionToOtherComponent (targetComp, position));

                dragAndDropTargetComponent = new ComponentDeletionWatcher (dynamic_cast <Component*> (newTarget));
                newTarget->fileDragEnter (files, pos.getX(), pos.getY());
            }
        }
    }
    else
    {
        newTarget = lastTarget;
    }

    if (newTarget != 0)
    {
        Component* const targetComp = dynamic_cast <Component*> (newTarget);
        const Point<int> pos (component->relativePositionToOtherComponent (targetComp, position));

        newTarget->fileDragMove (files, pos.getX(), pos.getY());
    }
}

void ComponentPeer::handleFileDragExit (const StringArray& files)
{
    handleFileDragMove (files, Point<int> (-1, -1));

    jassert (dragAndDropTargetComponent == 0);
    lastDragAndDropCompUnderMouse = 0;
}

void ComponentPeer::handleFileDragDrop (const StringArray& files, const Point<int>& position)
{
    handleFileDragMove (files, position);

    if (dragAndDropTargetComponent != 0 && ! dragAndDropTargetComponent->hasBeenDeleted())
    {
        FileDragAndDropTarget* const target = const_cast <FileDragAndDropTarget*> (dynamic_cast <const FileDragAndDropTarget*> (dragAndDropTargetComponent->getComponent()));

        dragAndDropTargetComponent = 0;
        lastDragAndDropCompUnderMouse = 0;

        if (target != 0)
        {
            Component* const targetComp = dynamic_cast <Component*> (target);

            if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
            {
                targetComp->internalModalInputAttempt();

                if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
                    return;
            }

            const Point<int> pos (component->relativePositionToOtherComponent (targetComp, position));
            target->filesDropped (files, pos.getX(), pos.getY());
        }
    }
}

//==============================================================================
void ComponentPeer::handleUserClosingWindow()
{
    updateCurrentModifiers();

    component->userTriedToCloseWindow();
}

//==============================================================================
void ComponentPeer::bringModalComponentToFront()
{
    Component::bringModalComponentToFront();
}

//==============================================================================
void ComponentPeer::clearMaskedRegion() throw()
{
    maskedRegion.clear();
}

void ComponentPeer::addMaskedRegion (int x, int y, int w, int h) throw()
{
    maskedRegion.add (x, y, w, h);
}

//==============================================================================
const StringArray ComponentPeer::getAvailableRenderingEngines() throw()
{
    StringArray s;
    s.add ("Software Renderer");
    return s;
}

int ComponentPeer::getCurrentRenderingEngine() throw()
{
    return 0;
}

void ComponentPeer::setCurrentRenderingEngine (int /*index*/) throw()
{
}

END_JUCE_NAMESPACE
