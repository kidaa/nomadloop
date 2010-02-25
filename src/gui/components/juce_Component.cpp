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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Component.h"
#include "juce_ComponentDeletionWatcher.h"
#include "juce_Desktop.h"
#include "keyboard/juce_KeyListener.h"
#include "lookandfeel/juce_LookAndFeel.h"
#include "../../application/juce_Application.h"
#include "../graphics/geometry/juce_RectangleList.h"
#include "../graphics/imaging/juce_Image.h"
#include "../graphics/contexts/juce_LowLevelGraphicsContext.h"
#include "../../events/juce_MessageManager.h"
#include "../../events/juce_Timer.h"
#include "../../core/juce_Time.h"
#include "../../core/juce_Singleton.h"
#include "../../core/juce_PlatformUtilities.h"
#include "mouse/juce_MouseInputSource.h"

//==============================================================================
Component* Component::componentUnderMouse = 0;
Component* Component::currentlyFocusedComponent = 0;

static Array <Component*> modalComponentStack, modalComponentReturnValueKeys;
static Array <int> modalReturnValues;

static const int customCommandMessage   = 0x7fff0001;
static const int exitModalStateMessage  = 0x7fff0002;

//==============================================================================
static Point<int> unboundedMouseOffset;
static bool isUnboundedMouseModeOn = false;
static bool isCursorVisibleUntilOffscreen;

//==============================================================================
#define checkMessageManagerIsLocked     jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

static uint32 nextComponentUID = 0;


//==============================================================================
Component::Component() throw()
  : parentComponent_ (0),
    componentUID (++nextComponentUID),
    numDeepMouseListeners (0),
    lookAndFeel_ (0),
    effect_ (0),
    bufferedImage_ (0),
    mouseListeners_ (0),
    keyListeners_ (0),
    componentListeners_ (0),
    componentFlags_ (0)
{
}

Component::Component (const String& name) throw()
  : componentName_ (name),
    parentComponent_ (0),
    componentUID (++nextComponentUID),
    numDeepMouseListeners (0),
    lookAndFeel_ (0),
    effect_ (0),
    bufferedImage_ (0),
    mouseListeners_ (0),
    keyListeners_ (0),
    componentListeners_ (0),
    componentFlags_ (0)
{
}

Component::~Component()
{
    if (parentComponent_ != 0)
    {
        parentComponent_->removeChildComponent (this);
    }
    else if ((currentlyFocusedComponent == this)
              || isParentOf (currentlyFocusedComponent))
    {
        giveAwayFocus();
    }

    if (componentUnderMouse == this)
        componentUnderMouse = 0;

    if (flags.hasHeavyweightPeerFlag)
        removeFromDesktop();

    modalComponentStack.removeValue (this);

    for (int i = childComponentList_.size(); --i >= 0;)
        childComponentList_.getUnchecked(i)->parentComponent_ = 0;

    delete bufferedImage_;
    delete mouseListeners_;
    delete keyListeners_;
    delete componentListeners_;
}

//==============================================================================
void Component::setName (const String& name)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (componentName_ != name)
    {
        componentName_ = name;

        if (flags.hasHeavyweightPeerFlag)
        {
            ComponentPeer* const peer = getPeer();

            jassert (peer != 0);
            if (peer != 0)
                peer->setTitle (name);
        }

        if (componentListeners_ != 0)
        {
            const ComponentDeletionWatcher deletionChecker (this);

            for (int i = componentListeners_->size(); --i >= 0;)
            {
                ((ComponentListener*) componentListeners_->getUnchecked (i))
                    ->componentNameChanged (*this);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, componentListeners_->size());
            }
        }

    }
}

void Component::setVisible (bool shouldBeVisible)
{
    if (flags.visibleFlag != shouldBeVisible)
    {
        // if component methods are being called from threads other than the message
        // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
        checkMessageManagerIsLocked

        const ComponentDeletionWatcher deletionChecker (this);

        flags.visibleFlag = shouldBeVisible;

        internalRepaint (0, 0, getWidth(), getHeight());

        sendFakeMouseMove();

        if (! shouldBeVisible)
        {
            if (currentlyFocusedComponent == this
                || isParentOf (currentlyFocusedComponent))
            {
                if (parentComponent_ != 0)
                    parentComponent_->grabKeyboardFocus();
                else
                    giveAwayFocus();
            }
        }

        sendVisibilityChangeMessage();

        if ((! deletionChecker.hasBeenDeleted()) && flags.hasHeavyweightPeerFlag)
        {
            ComponentPeer* const peer = getPeer();

            jassert (peer != 0);
            if (peer != 0)
            {
                peer->setVisible (shouldBeVisible);
                internalHierarchyChanged();
            }
        }
    }
}

void Component::visibilityChanged()
{
}

void Component::sendVisibilityChangeMessage()
{
    const ComponentDeletionWatcher deletionChecker (this);

    visibilityChanged();

    if ((! deletionChecker.hasBeenDeleted()) && componentListeners_ != 0)
    {
        for (int i = componentListeners_->size(); --i >= 0;)
        {
            ((ComponentListener*) componentListeners_->getUnchecked (i))
                ->componentVisibilityChanged (*this);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, componentListeners_->size());
        }
    }
}

bool Component::isShowing() const throw()
{
    if (flags.visibleFlag)
    {
        if (parentComponent_ != 0)
        {
            return parentComponent_->isShowing();
        }
        else
        {
            const ComponentPeer* const peer = getPeer();

            return peer != 0 && ! peer->isMinimised();
        }
    }

    return false;
}

//==============================================================================
class FadeOutProxyComponent  : public Component,
                               public Timer
{
public:
    FadeOutProxyComponent (Component* comp,
                           const int fadeLengthMs,
                           const int deltaXToMove,
                           const int deltaYToMove,
                           const float scaleFactorAtEnd)
       : lastTime (0),
         alpha (1.0f),
         scale (1.0f)
    {
        image = comp->createComponentSnapshot (Rectangle<int> (0, 0, comp->getWidth(), comp->getHeight()));
        setBounds (comp->getBounds());
        comp->getParentComponent()->addAndMakeVisible (this);
        toBehind (comp);

        alphaChangePerMs = -1.0f / (float)fadeLengthMs;

        centreX = comp->getX() + comp->getWidth() * 0.5f;
        xChangePerMs = deltaXToMove / (float)fadeLengthMs;

        centreY = comp->getY() + comp->getHeight() * 0.5f;
        yChangePerMs = deltaYToMove / (float)fadeLengthMs;

        scaleChangePerMs = (scaleFactorAtEnd - 1.0f) / (float)fadeLengthMs;

        setInterceptsMouseClicks (false, false);

        // 30 fps is enough for a fade, but we need a higher rate if it's moving as well..
        startTimer (1000 / ((deltaXToMove == 0 && deltaYToMove == 0) ? 30 : 50));
    }

    ~FadeOutProxyComponent()
    {
        delete image;
    }

    void paint (Graphics& g)
    {
        g.setOpacity (alpha);

        g.drawImage (image,
                     0, 0, getWidth(), getHeight(),
                     0, 0, image->getWidth(), image->getHeight());
    }

    void timerCallback()
    {
        const uint32 now = Time::getMillisecondCounter();

        if (lastTime == 0)
            lastTime = now;

        const int msPassed = (now > lastTime) ? now - lastTime : 0;
        lastTime = now;

        alpha += alphaChangePerMs * msPassed;

        if (alpha > 0)
        {
            if (xChangePerMs != 0.0f || yChangePerMs != 0.0f || scaleChangePerMs != 0.0f)
            {
                centreX += xChangePerMs * msPassed;
                centreY += yChangePerMs * msPassed;
                scale += scaleChangePerMs * msPassed;

                const int w = roundToInt (image->getWidth() * scale);
                const int h = roundToInt (image->getHeight() * scale);

                setBounds (roundToInt (centreX) - w / 2,
                           roundToInt (centreY) - h / 2,
                           w, h);
            }

            repaint();
        }
        else
        {
            delete this;
        }
    }

    juce_UseDebuggingNewOperator

private:
    Image* image;
    uint32 lastTime;
    float alpha, alphaChangePerMs;
    float centreX, xChangePerMs;
    float centreY, yChangePerMs;
    float scale, scaleChangePerMs;

    FadeOutProxyComponent (const FadeOutProxyComponent&);
    FadeOutProxyComponent& operator= (const FadeOutProxyComponent&);
};

void Component::fadeOutComponent (const int millisecondsToFade,
                                  const int deltaXToMove,
                                  const int deltaYToMove,
                                  const float scaleFactorAtEnd)
{
    //xxx won't work for comps without parents
    if (isShowing() && millisecondsToFade > 0)
        new FadeOutProxyComponent (this, millisecondsToFade,
                                   deltaXToMove, deltaYToMove, scaleFactorAtEnd);

    setVisible (false);
}


//==============================================================================
bool Component::isValidComponent() const throw()
{
    return (this != 0) && isValidMessageListener();
}

void* Component::getWindowHandle() const throw()
{
    const ComponentPeer* const peer = getPeer();

    if (peer != 0)
        return peer->getNativeHandle();

    return 0;
}

//==============================================================================
void Component::addToDesktop (int styleWanted, void* nativeWindowToAttachTo)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (isOpaque())
        styleWanted &= ~ComponentPeer::windowIsSemiTransparent;
    else
        styleWanted |= ComponentPeer::windowIsSemiTransparent;

    int currentStyleFlags = 0;

    // don't use getPeer(), so that we only get the peer that's specifically
    // for this comp, and not for one of its parents.
    ComponentPeer* peer = ComponentPeer::getPeerFor (this);

    if (peer != 0)
        currentStyleFlags = peer->getStyleFlags();

    if (styleWanted != currentStyleFlags || ! flags.hasHeavyweightPeerFlag)
    {
        const ComponentDeletionWatcher deletionChecker (this);

#if JUCE_LINUX
        // it's wise to give the component a non-zero size before
        // putting it on the desktop, as X windows get confused by this, and
        // a (1, 1) minimum size is enforced here.
        setSize (jmax (1, getWidth()),
                 jmax (1, getHeight()));
#endif

        const Point<int> topLeft (relativePositionToGlobal (Point<int> (0, 0)));

        bool wasFullscreen = false;
        bool wasMinimised = false;
        ComponentBoundsConstrainer* currentConstainer = 0;
        Rectangle<int> oldNonFullScreenBounds;

        if (peer != 0)
        {
            wasFullscreen = peer->isFullScreen();
            wasMinimised = peer->isMinimised();
            currentConstainer = peer->getConstrainer();
            oldNonFullScreenBounds = peer->getNonFullScreenBounds();

            removeFromDesktop();

            setTopLeftPosition (topLeft.getX(), topLeft.getY());
        }

        if (parentComponent_ != 0)
            parentComponent_->removeChildComponent (this);

        if (! deletionChecker.hasBeenDeleted())
        {
            flags.hasHeavyweightPeerFlag = true;

            peer = createNewPeer (styleWanted, nativeWindowToAttachTo);

            Desktop::getInstance().addDesktopComponent (this);

            bounds_.setPosition (topLeft);
            peer->setBounds (topLeft.getX(), topLeft.getY(), getWidth(), getHeight(), false);

            peer->setVisible (isVisible());

            if (wasFullscreen)
            {
                peer->setFullScreen (true);
                peer->setNonFullScreenBounds (oldNonFullScreenBounds);
            }

            if (wasMinimised)
                peer->setMinimised (true);

            if (isAlwaysOnTop())
                peer->setAlwaysOnTop (true);

            peer->setConstrainer (currentConstainer);

            repaint();
        }

        internalHierarchyChanged();
    }
}

void Component::removeFromDesktop()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (flags.hasHeavyweightPeerFlag)
    {
        ComponentPeer* const peer = ComponentPeer::getPeerFor (this);

        flags.hasHeavyweightPeerFlag = false;

        jassert (peer != 0);
        delete peer;

        Desktop::getInstance().removeDesktopComponent (this);
    }
}

bool Component::isOnDesktop() const throw()
{
    return flags.hasHeavyweightPeerFlag;
}

void Component::userTriedToCloseWindow()
{
    /* This means that the user's trying to get rid of your window with the 'close window' system
       menu option (on windows) or possibly the task manager - you should really handle this
       and delete or hide your component in an appropriate way.

       If you want to ignore the event and don't want to trigger this assertion, just override
       this method and do nothing.
    */
    jassertfalse
}

void Component::minimisationStateChanged (bool)
{
}

//==============================================================================
void Component::setOpaque (const bool shouldBeOpaque) throw()
{
    if (shouldBeOpaque != flags.opaqueFlag)
    {
        flags.opaqueFlag = shouldBeOpaque;

        if (flags.hasHeavyweightPeerFlag)
        {
            const ComponentPeer* const peer = ComponentPeer::getPeerFor (this);

            if (peer != 0)
            {
                // to make it recreate the heavyweight window
                addToDesktop (peer->getStyleFlags());
            }
        }

        repaint();
    }
}

bool Component::isOpaque() const throw()
{
    return flags.opaqueFlag;
}

//==============================================================================
void Component::setBufferedToImage (const bool shouldBeBuffered) throw()
{
    if (shouldBeBuffered != flags.bufferToImageFlag)
    {
        deleteAndZero (bufferedImage_);
        flags.bufferToImageFlag = shouldBeBuffered;
    }
}

//==============================================================================
void Component::toFront (const bool setAsForeground)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (flags.hasHeavyweightPeerFlag)
    {
        ComponentPeer* const peer = getPeer();

        if (peer != 0)
        {
            peer->toFront (setAsForeground);

            if (setAsForeground && ! hasKeyboardFocus (true))
                grabKeyboardFocus();
        }
    }
    else if (parentComponent_ != 0)
    {
        if (parentComponent_->childComponentList_.getLast() != this)
        {
            const int index = parentComponent_->childComponentList_.indexOf (this);

            if (index >= 0)
            {
                int insertIndex = -1;

                if (! flags.alwaysOnTopFlag)
                {
                    insertIndex = parentComponent_->childComponentList_.size() - 1;

                    while (insertIndex > 0
                            && parentComponent_->childComponentList_.getUnchecked (insertIndex)->isAlwaysOnTop())
                    {
                        --insertIndex;
                    }
                }

                if (index != insertIndex)
                {
                    parentComponent_->childComponentList_.move (index, insertIndex);
                    sendFakeMouseMove();

                    repaintParent();
                }
            }
        }

        if (setAsForeground)
        {
            internalBroughtToFront();
            grabKeyboardFocus();
        }
    }
}

void Component::toBehind (Component* const other)
{
    if (other != 0)
    {
        // the two components must belong to the same parent..
        jassert (parentComponent_ == other->parentComponent_);

        if (parentComponent_ != 0)
        {
            const int index = parentComponent_->childComponentList_.indexOf (this);
            int otherIndex  = parentComponent_->childComponentList_.indexOf (other);

            if (index >= 0
                 && otherIndex >= 0
                 && index != otherIndex - 1
                 && other != this)
            {
                if (index < otherIndex)
                    --otherIndex;

                parentComponent_->childComponentList_.move (index, otherIndex);

                sendFakeMouseMove();
                repaintParent();
            }
        }
        else if (isOnDesktop())
        {
            jassert (other->isOnDesktop());

            if (other->isOnDesktop())
            {
                ComponentPeer* const us = getPeer();
                ComponentPeer* const them = other->getPeer();

                jassert (us != 0 && them != 0);
                if (us != 0 && them != 0)
                    us->toBehind (them);
            }
        }
    }
}

void Component::toBack()
{
    if (isOnDesktop())
    {
        jassertfalse //xxx need to add this to native window
    }
    else if (parentComponent_ != 0
         && parentComponent_->childComponentList_.getFirst() != this)
    {
        const int index = parentComponent_->childComponentList_.indexOf (this);

        if (index > 0)
        {
            int insertIndex = 0;

            if (flags.alwaysOnTopFlag)
            {
                while (insertIndex < parentComponent_->childComponentList_.size()
                        && ! parentComponent_->childComponentList_.getUnchecked (insertIndex)->isAlwaysOnTop())
                {
                    ++insertIndex;
                }
            }

            if (index != insertIndex)
            {
                parentComponent_->childComponentList_.move (index, insertIndex);

                sendFakeMouseMove();
                repaintParent();
            }
        }
    }
}

void Component::setAlwaysOnTop (const bool shouldStayOnTop)
{
    if (shouldStayOnTop != flags.alwaysOnTopFlag)
    {
        flags.alwaysOnTopFlag = shouldStayOnTop;

        if (isOnDesktop())
        {
            ComponentPeer* const peer = getPeer();

            jassert (peer != 0);
            if (peer != 0)
            {
                if (! peer->setAlwaysOnTop (shouldStayOnTop))
                {
                    // some kinds of peer can't change their always-on-top status, so
                    // for these, we'll need to create a new window
                    const int oldFlags = peer->getStyleFlags();
                    removeFromDesktop();
                    addToDesktop (oldFlags);
                }
            }
        }

        if (shouldStayOnTop)
            toFront (false);

        internalHierarchyChanged();
    }
}

bool Component::isAlwaysOnTop() const throw()
{
    return flags.alwaysOnTopFlag;
}

//==============================================================================
int Component::proportionOfWidth (const float proportion) const throw()
{
    return roundToInt (proportion * bounds_.getWidth());
}

int Component::proportionOfHeight (const float proportion) const throw()
{
    return roundToInt (proportion * bounds_.getHeight());
}

int Component::getParentWidth() const throw()
{
    return (parentComponent_ != 0) ? parentComponent_->getWidth()
                                   : getParentMonitorArea().getWidth();
}

int Component::getParentHeight() const throw()
{
    return (parentComponent_ != 0) ? parentComponent_->getHeight()
                                   : getParentMonitorArea().getHeight();
}

int Component::getScreenX() const
{
    return getScreenPosition().getX();
}

int Component::getScreenY() const
{
    return getScreenPosition().getY();
}

const Point<int> Component::getScreenPosition() const
{
    return (parentComponent_ != 0) ? parentComponent_->getScreenPosition() + getPosition()
                                   : (flags.hasHeavyweightPeerFlag ? getPeer()->getScreenPosition()
                                                                   : getPosition());
}

const Rectangle<int> Component::getScreenBounds() const
{
    return bounds_.withPosition (getScreenPosition());
}

const Point<int> Component::relativePositionToGlobal (const Point<int>& relativePosition) const
{
    const Component* c = this;
    Point<int> p (relativePosition);

    do
    {
        if (c->flags.hasHeavyweightPeerFlag)
            return c->getPeer()->relativePositionToGlobal (p);

        p += c->getPosition();
        c = c->parentComponent_;
    }
    while (c != 0);

    return p;
}

const Point<int> Component::globalPositionToRelative (const Point<int>& screenPosition) const
{
    if (flags.hasHeavyweightPeerFlag)
    {
        return getPeer()->globalPositionToRelative (screenPosition);
    }
    else
    {
        if (parentComponent_ != 0)
            return parentComponent_->globalPositionToRelative (screenPosition) - getPosition();

        return screenPosition - getPosition();
    }
}

const Point<int> Component::relativePositionToOtherComponent (const Component* const targetComponent, const Point<int>& positionRelativeToThis) const
{
    Point<int> p (positionRelativeToThis);

    if (targetComponent != 0)
    {
        const Component* c = this;

        do
        {
            if (c == targetComponent)
                return p;

            if (c->flags.hasHeavyweightPeerFlag)
            {
                p = c->getPeer()->relativePositionToGlobal (p);
                break;
            }

            p += c->getPosition();
            c = c->parentComponent_;
        }
        while (c != 0);

        p = targetComponent->globalPositionToRelative (p);
    }

    return p;
}

//==============================================================================
void Component::setBounds (int x, int y, int w, int h)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (w < 0) w = 0;
    if (h < 0) h = 0;

    const bool wasResized  = (getWidth() != w || getHeight() != h);
    const bool wasMoved    = (getX() != x || getY() != y);

#ifdef JUCE_DEBUG
    // It's a very bad idea to try to resize a window during its paint() method!
    jassert (! (flags.isInsidePaintCall && wasResized && isOnDesktop()));
#endif

    if (wasMoved || wasResized)
    {
        if (flags.visibleFlag)
        {
            // send a fake mouse move to trigger enter/exit messages if needed..
            sendFakeMouseMove();

            if (! flags.hasHeavyweightPeerFlag)
                repaintParent();
        }

        bounds_.setBounds (x, y, w, h);

        if (wasResized)
            repaint();
        else if (! flags.hasHeavyweightPeerFlag)
            repaintParent();

        if (flags.hasHeavyweightPeerFlag)
        {
            ComponentPeer* const peer = getPeer();

            if (peer != 0)
            {
                if (wasMoved && wasResized)
                    peer->setBounds (getX(), getY(), getWidth(), getHeight(), false);
                else if (wasMoved)
                    peer->setPosition (getX(), getY());
                else if (wasResized)
                    peer->setSize (getWidth(), getHeight());
            }
        }

        sendMovedResizedMessages (wasMoved, wasResized);
    }
}

void Component::sendMovedResizedMessages (const bool wasMoved, const bool wasResized)
{
    JUCE_TRY
    {
        if (wasMoved)
            moved();

        if (wasResized)
        {
            resized();

            for (int i = childComponentList_.size(); --i >= 0;)
            {
                childComponentList_.getUnchecked(i)->parentSizeChanged();

                i = jmin (i, childComponentList_.size());
            }
        }

        if (parentComponent_ != 0)
            parentComponent_->childBoundsChanged (this);

        if (componentListeners_ != 0)
        {
            const ComponentDeletionWatcher deletionChecker (this);

            for (int i = componentListeners_->size(); --i >= 0;)
            {
                ((ComponentListener*) componentListeners_->getUnchecked (i))
                    ->componentMovedOrResized (*this, wasMoved, wasResized);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, componentListeners_->size());
            }
        }
    }
    JUCE_CATCH_EXCEPTION
}

void Component::setSize (const int w, const int h)
{
    setBounds (getX(), getY(), w, h);
}

void Component::setTopLeftPosition (const int x, const int y)
{
    setBounds (x, y, getWidth(), getHeight());
}

void Component::setTopRightPosition (const int x, const int y)
{
    setTopLeftPosition (x - getWidth(), y);
}

void Component::setBounds (const Rectangle<int>& r)
{
    setBounds (r.getX(),
               r.getY(),
               r.getWidth(),
               r.getHeight());
}

void Component::setBoundsRelative (const float x, const float y,
                                   const float w, const float h)
{
    const int pw = getParentWidth();
    const int ph = getParentHeight();

    setBounds (roundToInt (x * pw),
               roundToInt (y * ph),
               roundToInt (w * pw),
               roundToInt (h * ph));
}

void Component::setCentrePosition (const int x, const int y)
{
    setTopLeftPosition (x - getWidth() / 2,
                        y - getHeight() / 2);
}

void Component::setCentreRelative (const float x, const float y)
{
    setCentrePosition (roundToInt (getParentWidth() * x),
                       roundToInt (getParentHeight() * y));
}

void Component::centreWithSize (const int width, const int height)
{
    setBounds ((getParentWidth() - width) / 2,
               (getParentHeight() - height) / 2,
               width,
               height);
}

void Component::setBoundsInset (const BorderSize& borders)
{
    setBounds (borders.getLeft(),
               borders.getTop(),
               getParentWidth() - (borders.getLeftAndRight()),
               getParentHeight() - (borders.getTopAndBottom()));
}

void Component::setBoundsToFit (int x, int y, int width, int height,
                                const Justification& justification,
                                const bool onlyReduceInSize)
{
    // it's no good calling this method unless both the component and
    // target rectangle have a finite size.
    jassert (getWidth() > 0 && getHeight() > 0 && width > 0 && height > 0);

    if (getWidth() > 0 && getHeight() > 0
         && width > 0 && height > 0)
    {
        int newW, newH;

        if (onlyReduceInSize && getWidth() <= width && getHeight() <= height)
        {
            newW = getWidth();
            newH = getHeight();
        }
        else
        {
            const double imageRatio = getHeight() / (double) getWidth();
            const double targetRatio = height / (double) width;

            if (imageRatio <= targetRatio)
            {
                newW = width;
                newH = jmin (height, roundToInt (newW * imageRatio));
            }
            else
            {
                newH = height;
                newW = jmin (width, roundToInt (newH / imageRatio));
            }
        }

        if (newW > 0 && newH > 0)
        {
            int newX, newY;
            justification.applyToRectangle (newX, newY, newW, newH,
                                            x, y, width, height);

            setBounds (newX, newY, newW, newH);
        }
    }
}

//==============================================================================
bool Component::hitTest (int x, int y)
{
    if (! flags.ignoresMouseClicksFlag)
        return true;

    if (flags.allowChildMouseClicksFlag)
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            Component* const c = getChildComponent (i);

            if (c->isVisible()
                && c->bounds_.contains (x, y)
                && c->hitTest (x - c->getX(),
                               y - c->getY()))
            {
                return true;
            }
        }
    }

    return false;
}

void Component::setInterceptsMouseClicks (const bool allowClicks,
                                          const bool allowClicksOnChildComponents) throw()
{
    flags.ignoresMouseClicksFlag = ! allowClicks;
    flags.allowChildMouseClicksFlag = allowClicksOnChildComponents;
}

void Component::getInterceptsMouseClicks (bool& allowsClicksOnThisComponent,
                                          bool& allowsClicksOnChildComponents) const throw()
{
    allowsClicksOnThisComponent = ! flags.ignoresMouseClicksFlag;
    allowsClicksOnChildComponents = flags.allowChildMouseClicksFlag;
}

bool Component::contains (const int x, const int y)
{
    if (((unsigned int) x) < (unsigned int) getWidth()
        && ((unsigned int) y) < (unsigned int) getHeight()
         && hitTest (x, y))
    {
        if (parentComponent_ != 0)
        {
            return parentComponent_->contains (x + getX(),
                                               y + getY());
        }
        else if (flags.hasHeavyweightPeerFlag)
        {
            const ComponentPeer* const peer = getPeer();

            if (peer != 0)
                return peer->contains (Point<int> (x, y), true);
        }
    }

    return false;
}

bool Component::reallyContains (int x, int y, const bool returnTrueIfWithinAChild)
{
    if (! contains (x, y))
        return false;

    Component* p = this;

    while (p->parentComponent_ != 0)
    {
        x += p->getX();
        y += p->getY();

        p = p->parentComponent_;
    }

    const Component* const c = p->getComponentAt (x, y);

    return (c == this) || (returnTrueIfWithinAChild && isParentOf (c));
}

Component* Component::getComponentAt (const Point<int>& position)
{
    return getComponentAt (position.getX(), position.getY());
}

Component* Component::getComponentAt (const int x, const int y)
{
    if (flags.visibleFlag
         && ((unsigned int) x) < (unsigned int) getWidth()
         && ((unsigned int) y) < (unsigned int) getHeight()
         && hitTest (x, y))
    {
        for (int i = childComponentList_.size(); --i >= 0;)
        {
            Component* const child = childComponentList_.getUnchecked(i);

            Component* const c = child->getComponentAt (x - child->getX(),
                                                        y - child->getY());

            if (c != 0)
                return c;
        }

        return this;
    }

    return 0;
}

//==============================================================================
void Component::addChildComponent (Component* const child, int zOrder)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (child != 0 && child->parentComponent_ != this)
    {
        if (child->parentComponent_ != 0)
            child->parentComponent_->removeChildComponent (child);
        else
            child->removeFromDesktop();

        child->parentComponent_ = this;

        if (child->isVisible())
            child->repaintParent();

        if (! child->isAlwaysOnTop())
        {
            if (zOrder < 0 || zOrder > childComponentList_.size())
                zOrder = childComponentList_.size();

            while (zOrder > 0)
            {
                if (! childComponentList_.getUnchecked (zOrder - 1)->isAlwaysOnTop())
                    break;

                --zOrder;
            }
        }

        childComponentList_.insert (zOrder, child);

        child->internalHierarchyChanged();
        internalChildrenChanged();
    }
}

void Component::addAndMakeVisible (Component* const child, int zOrder)
{
    if (child != 0)
    {
        child->setVisible (true);
        addChildComponent (child, zOrder);
    }
}

void Component::removeChildComponent (Component* const child)
{
    removeChildComponent (childComponentList_.indexOf (child));
}

Component* Component::removeChildComponent (const int index)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    Component* const child = childComponentList_ [index];

    if (child != 0)
    {
        sendFakeMouseMove();
        child->repaintParent();

        childComponentList_.remove (index);
        child->parentComponent_ = 0;

        JUCE_TRY
        {
            if ((currentlyFocusedComponent == child)
                || child->isParentOf (currentlyFocusedComponent))
            {
                // get rid first to force the grabKeyboardFocus to change to us.
                giveAwayFocus();
                grabKeyboardFocus();
            }
        }
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS
        catch (const std::exception& e)
        {
            currentlyFocusedComponent = 0;
            Desktop::getInstance().triggerFocusCallback();
            JUCEApplication::sendUnhandledException (&e, __FILE__, __LINE__);
        }
        catch (...)
        {
            currentlyFocusedComponent = 0;
            Desktop::getInstance().triggerFocusCallback();
            JUCEApplication::sendUnhandledException (0, __FILE__, __LINE__);
        }
#endif

        child->internalHierarchyChanged();
        internalChildrenChanged();
    }

    return child;
}

//==============================================================================
void Component::removeAllChildren()
{
    for (int i = childComponentList_.size(); --i >= 0;)
        removeChildComponent (i);
}

void Component::deleteAllChildren()
{
    for (int i = childComponentList_.size(); --i >= 0;)
        delete (removeChildComponent (i));
}

//==============================================================================
int Component::getNumChildComponents() const throw()
{
    return childComponentList_.size();
}

Component* Component::getChildComponent (const int index) const throw()
{
    return childComponentList_ [index];
}

int Component::getIndexOfChildComponent (const Component* const child) const throw()
{
    return childComponentList_.indexOf (const_cast <Component*> (child));
}

Component* Component::getTopLevelComponent() const throw()
{
    const Component* comp = this;

    while (comp->parentComponent_ != 0)
        comp = comp->parentComponent_;

    return (Component*) comp;
}

bool Component::isParentOf (const Component* possibleChild) const throw()
{
    while (possibleChild->isValidComponent())
    {
        possibleChild = possibleChild->parentComponent_;

        if (possibleChild == this)
            return true;
    }

    return false;
}

//==============================================================================
void Component::parentHierarchyChanged()
{
}

void Component::childrenChanged()
{
}

void Component::internalChildrenChanged()
{
    const ComponentDeletionWatcher deletionChecker (this);
    const bool hasListeners = componentListeners_ != 0;

    childrenChanged();

    if (hasListeners)
    {
        if (deletionChecker.hasBeenDeleted())
            return;

        for (int i = componentListeners_->size(); --i >= 0;)
        {
            ((ComponentListener*) componentListeners_->getUnchecked (i))
                ->componentChildrenChanged (*this);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, componentListeners_->size());
        }
    }
}

void Component::internalHierarchyChanged()
{
    parentHierarchyChanged();

    const ComponentDeletionWatcher deletionChecker (this);

    if (componentListeners_ != 0)
    {
        for (int i = componentListeners_->size(); --i >= 0;)
        {
            ((ComponentListener*) componentListeners_->getUnchecked (i))
                ->componentParentHierarchyChanged (*this);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, componentListeners_->size());
        }
    }

    for (int i = childComponentList_.size(); --i >= 0;)
    {
        childComponentList_.getUnchecked (i)->internalHierarchyChanged();

        // you really shouldn't delete the parent component during a callback telling you
        // that it's changed..
        jassert (! deletionChecker.hasBeenDeleted());
        if (deletionChecker.hasBeenDeleted())
            return;

        i = jmin (i, childComponentList_.size());
    }
}

//==============================================================================
void* Component::runModalLoopCallback (void* userData)
{
    return (void*) (pointer_sized_int) ((Component*) userData)->runModalLoop();
}

int Component::runModalLoop()
{
    if (! MessageManager::getInstance()->isThisTheMessageThread())
    {
        // use a callback so this can be called from non-gui threads
        return (int) (pointer_sized_int)
                    MessageManager::getInstance()
                       ->callFunctionOnMessageThread (&runModalLoopCallback, (void*) this);
    }

    Component* const prevFocused = getCurrentlyFocusedComponent();

    ScopedPointer <ComponentDeletionWatcher> deletionChecker;
    if (prevFocused != 0)
        deletionChecker = new ComponentDeletionWatcher (prevFocused);

    if (! isCurrentlyModal())
        enterModalState();

    JUCE_TRY
    {
        while (flags.currentlyModalFlag && flags.visibleFlag)
        {
            if  (! MessageManager::getInstance()->runDispatchLoopUntil (20))
                break;

            // check whether this component was deleted during the last message
            if (! isValidMessageListener())
                break;
        }
    }
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS
    catch (const std::exception& e)
    {
        JUCEApplication::sendUnhandledException (&e, __FILE__, __LINE__);
        return 0;
    }
    catch (...)
    {
        JUCEApplication::sendUnhandledException (0, __FILE__, __LINE__);
        return 0;
    }
#endif

    const int modalIndex = modalComponentReturnValueKeys.indexOf (this);
    int returnValue = 0;

    if (modalIndex >= 0)
    {
        modalComponentReturnValueKeys.remove (modalIndex);
        returnValue = modalReturnValues.remove (modalIndex);
    }

    modalComponentStack.removeValue (this);

    if (deletionChecker != 0 && ! deletionChecker->hasBeenDeleted())
        prevFocused->grabKeyboardFocus();

    return returnValue;
}

void Component::enterModalState (const bool takeKeyboardFocus_)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    // Check for an attempt to make a component modal when it already is!
    // This can cause nasty problems..
    jassert (! flags.currentlyModalFlag);

    if (! isCurrentlyModal())
    {
        modalComponentStack.add (this);
        modalComponentReturnValueKeys.add (this);
        modalReturnValues.add (0);

        flags.currentlyModalFlag = true;
        setVisible (true);

        if (takeKeyboardFocus_)
            grabKeyboardFocus();
    }
}

void Component::exitModalState (const int returnValue)
{
    if (isCurrentlyModal())
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            const int modalIndex = modalComponentReturnValueKeys.indexOf (this);

            if (modalIndex >= 0)
            {
                modalReturnValues.set (modalIndex, returnValue);
            }
            else
            {
                modalComponentReturnValueKeys.add (this);
                modalReturnValues.add (returnValue);
            }

            modalComponentStack.removeValue (this);

            flags.currentlyModalFlag = false;

            bringModalComponentToFront();
        }
        else
        {
            postMessage (new Message (exitModalStateMessage, returnValue, 0, 0));
        }
    }
}

bool Component::isCurrentlyModal() const throw()
{
    return flags.currentlyModalFlag
            && getCurrentlyModalComponent() == this;
}

bool Component::isCurrentlyBlockedByAnotherModalComponent() const throw()
{
    Component* const mc = getCurrentlyModalComponent();

    return mc != 0
            && mc != this
            && (! mc->isParentOf (this))
            && ! mc->canModalEventBeSentToComponent (this);
}

int JUCE_CALLTYPE Component::getNumCurrentlyModalComponents() throw()
{
    return modalComponentStack.size();
}

Component* JUCE_CALLTYPE Component::getCurrentlyModalComponent (int index) throw()
{
    Component* const c = (Component*) (modalComponentStack [modalComponentStack.size() - index - 1]);

    return c->isValidComponent() ? c : 0;
}

void Component::bringModalComponentToFront()
{
    ComponentPeer* lastOne = 0;

    for (int i = 0; i < getNumCurrentlyModalComponents(); ++i)
    {
        Component* const c = getCurrentlyModalComponent (i);

        if (c == 0)
            break;

        ComponentPeer* peer = c->getPeer();

        if (peer != 0 && peer != lastOne)
        {
            if (lastOne == 0)
            {
                peer->toFront (true);
                peer->grabFocus();
            }
            else
                peer->toBehind (lastOne);

            lastOne = peer;
        }
    }
}

//==============================================================================
void Component::setBroughtToFrontOnMouseClick (const bool shouldBeBroughtToFront) throw()
{
    flags.bringToFrontOnClickFlag = shouldBeBroughtToFront;
}

bool Component::isBroughtToFrontOnMouseClick() const throw()
{
    return flags.bringToFrontOnClickFlag;
}

//==============================================================================
void Component::setMouseCursor (const MouseCursor& cursor) throw()
{
    cursor_ = cursor;

    if (flags.visibleFlag)
    {
        const Point<int> mousePos (getMouseXYRelative());

        if (flags.draggingFlag || reallyContains (mousePos.getX(), mousePos.getY(), false))
        {
            internalUpdateMouseCursor (false);
        }
    }
}

const MouseCursor Component::getMouseCursor()
{
    return cursor_;
}

void Component::updateMouseCursor() const throw()
{
    sendFakeMouseMove();
}

void Component::internalUpdateMouseCursor (bool forcedUpdate) throw()
{
    ComponentPeer* const peer = getPeer();

    if (peer != 0)
    {
        MouseCursor mc (getLookAndFeel().getMouseCursorFor (*this));

        if (isUnboundedMouseModeOn
             && ((! unboundedMouseOffset.isOrigin()) || ! isCursorVisibleUntilOffscreen))
        {
            mc = MouseCursor::NoCursor;
            forcedUpdate = true;
        }

        static void* currentCursorHandle = 0;

        if (forcedUpdate || mc.getHandle() != currentCursorHandle)
        {
            currentCursorHandle = mc.getHandle();
            mc.showInWindow (peer);
        }
    }
}

//==============================================================================
void Component::setRepaintsOnMouseActivity (const bool shouldRepaint) throw()
{
    flags.repaintOnMouseActivityFlag = shouldRepaint;
}

//==============================================================================
void Component::repaintParent() throw()
{
    if (flags.visibleFlag)
        internalRepaint (0, 0, getWidth(), getHeight());
}

void Component::repaint() throw()
{
    repaint (0, 0, getWidth(), getHeight());
}

void Component::repaint (const int x, const int y,
                         const int w, const int h) throw()
{
    deleteAndZero (bufferedImage_);

    if (flags.visibleFlag)
        internalRepaint (x, y, w, h);
}

void Component::internalRepaint (int x, int y, int w, int h)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (x < 0)
    {
        w += x;
        x = 0;
    }

    if (x + w > getWidth())
        w = getWidth() - x;

    if (w > 0)
    {
        if (y < 0)
        {
            h += y;
            y = 0;
        }

        if (y + h > getHeight())
            h = getHeight() - y;

        if (h > 0)
        {
            if (parentComponent_ != 0)
            {
                x += getX();
                y += getY();

                if (parentComponent_->flags.visibleFlag)
                    parentComponent_->internalRepaint (x, y, w, h);
            }
            else if (flags.hasHeavyweightPeerFlag)
            {
                ComponentPeer* const peer = getPeer();

                if (peer != 0)
                    peer->repaint (x, y, w, h);
            }
        }
    }
}

//==============================================================================
void Component::paintEntireComponent (Graphics& originalContext)
{
    jassert (! originalContext.isClipEmpty());

#ifdef JUCE_DEBUG
    flags.isInsidePaintCall = true;
#endif

    Graphics* g = &originalContext;
    Image* effectImage = 0;

    if (effect_ != 0)
    {
        effectImage = Image::createNativeImage (flags.opaqueFlag ? Image::RGB : Image::ARGB,
                                                getWidth(), getHeight(),
                                                ! flags.opaqueFlag);

        g = new Graphics (*effectImage);
    }

    g->saveState();
    clipObscuredRegions (*g, g->getClipBounds(), 0, 0);

    if (! g->isClipEmpty())
    {
        if (bufferedImage_ != 0)
        {
            g->setColour (Colours::black);
            g->drawImageAt (bufferedImage_, 0, 0);
        }
        else
        {
            if (flags.bufferToImageFlag)
            {
                if (bufferedImage_ == 0)
                {
                    bufferedImage_ = Image::createNativeImage (flags.opaqueFlag ? Image::RGB : Image::ARGB,
                                                               getWidth(), getHeight(), ! flags.opaqueFlag);

                    Graphics imG (*bufferedImage_);
                    paint (imG);
                }

                g->setColour (Colours::black);
                g->drawImageAt (bufferedImage_, 0, 0);
            }
            else
            {
                paint (*g);
                g->resetToDefaultState();
            }
        }
    }

    g->restoreState();

    for (int i = 0; i < childComponentList_.size(); ++i)
    {
        Component* const child = childComponentList_.getUnchecked (i);

        if (child->isVisible())
        {
            g->saveState();

            if (g->reduceClipRegion (child->getX(), child->getY(),
                                     child->getWidth(), child->getHeight()))
            {
                for (int j = i + 1; j < childComponentList_.size(); ++j)
                {
                    const Component* const sibling = childComponentList_.getUnchecked (j);

                    if (sibling->flags.opaqueFlag && sibling->isVisible())
                        g->excludeClipRegion (sibling->getX(), sibling->getY(),
                                              sibling->getWidth(), sibling->getHeight());
                }

                if (! g->isClipEmpty())
                {
                    g->setOrigin (child->getX(), child->getY());

                    child->paintEntireComponent (*g);
                }
            }

            g->restoreState();
        }
    }

    JUCE_TRY
    {
        g->saveState();
        paintOverChildren (*g);
        g->restoreState();
    }
    JUCE_CATCH_EXCEPTION

    if (effect_ != 0)
    {
        delete g;

        effect_->applyEffect (*effectImage, originalContext);
        delete effectImage;
    }

#ifdef JUCE_DEBUG
    flags.isInsidePaintCall = false;
#endif
}

//==============================================================================
Image* Component::createComponentSnapshot (const Rectangle<int>& areaToGrab,
                                           const bool clipImageToComponentBounds)
{
    Rectangle<int> r (areaToGrab);

    if (clipImageToComponentBounds)
        r = r.getIntersection (Rectangle<int> (0, 0, getWidth(), getHeight()));

    Image* const componentImage = Image::createNativeImage (flags.opaqueFlag ? Image::RGB : Image::ARGB,
                                                            jmax (1, r.getWidth()),
                                                            jmax (1, r.getHeight()),
                                                            true);

    Graphics imageContext (*componentImage);
    imageContext.setOrigin (-r.getX(),
                            -r.getY());

    paintEntireComponent (imageContext);

    return componentImage;
}

void Component::setComponentEffect (ImageEffectFilter* const effect)
{
    if (effect_ != effect)
    {
        effect_ = effect;
        repaint();
    }
}

//==============================================================================
LookAndFeel& Component::getLookAndFeel() const throw()
{
    const Component* c = this;

    do
    {
        if (c->lookAndFeel_ != 0)
            return *(c->lookAndFeel_);

        c = c->parentComponent_;
    }
    while (c != 0);

    return LookAndFeel::getDefaultLookAndFeel();
}

void Component::setLookAndFeel (LookAndFeel* const newLookAndFeel)
{
    if (lookAndFeel_ != newLookAndFeel)
    {
        lookAndFeel_ = newLookAndFeel;

        sendLookAndFeelChange();
    }
}

void Component::lookAndFeelChanged()
{
}

void Component::sendLookAndFeelChange()
{
    repaint();

    lookAndFeelChanged();

    // (it's not a great idea to do anything that would delete this component
    //  during the lookAndFeelChanged() callback)
    jassert (isValidComponent());

    const ComponentDeletionWatcher deletionChecker (this);

    for (int i = childComponentList_.size(); --i >= 0;)
    {
        childComponentList_.getUnchecked (i)->sendLookAndFeelChange();

        if (deletionChecker.hasBeenDeleted())
            return;

        i = jmin (i, childComponentList_.size());
    }
}

static const var::identifier getColourPropertyId (const int colourId)
{
    String s;
    s.preallocateStorage (18);
    s << T("jcclr_") << colourId;
    return s;
}

const Colour Component::findColour (const int colourId, const bool inheritFromParent) const throw()
{
    var* v = properties.getItem (getColourPropertyId (colourId));

    if (v != 0)
        return Colour ((int) *v);

    if (inheritFromParent && parentComponent_ != 0)
        return parentComponent_->findColour (colourId, true);

    return getLookAndFeel().findColour (colourId);
}

bool Component::isColourSpecified (const int colourId) const throw()
{
    return properties.contains (getColourPropertyId (colourId));
}

void Component::removeColour (const int colourId)
{
    if (properties.remove (getColourPropertyId (colourId)))
        colourChanged();
}

void Component::setColour (const int colourId, const Colour& colour)
{
    if (properties.set (getColourPropertyId (colourId), (int) colour.getARGB()))
        colourChanged();
}

void Component::copyAllExplicitColoursTo (Component& target) const throw()
{
    bool changed = false;

    for (int i = properties.size(); --i >= 0;)
    {
        const var::identifier name (properties.getName(i));

        if (name.name.startsWith (T("jcclr_")))
            if (target.properties.set (name, properties [name]))
                changed = true;
    }

    if (changed)
        target.colourChanged();
}

void Component::colourChanged()
{
}

//==============================================================================
const Rectangle<int> Component::getUnclippedArea() const
{
    int x = 0, y = 0, w = getWidth(), h = getHeight();

    Component* p = parentComponent_;
    int px = getX();
    int py = getY();

    while (p != 0)
    {
        if (! Rectangle<int>::intersectRectangles (x, y, w, h, -px, -py, p->getWidth(), p->getHeight()))
            return Rectangle<int>();

        px += p->getX();
        py += p->getY();
        p = p->parentComponent_;
    }

    return Rectangle<int> (x, y, w, h);
}

void Component::clipObscuredRegions (Graphics& g, const Rectangle<int>& clipRect,
                                     const int deltaX, const int deltaY) const throw()
{
    for (int i = childComponentList_.size(); --i >= 0;)
    {
        const Component* const c = childComponentList_.getUnchecked(i);

        if (c->isVisible())
        {
            Rectangle<int> newClip (clipRect.getIntersection (c->bounds_));

            if (! newClip.isEmpty())
            {
                if (c->isOpaque())
                {
                    g.excludeClipRegion (deltaX + newClip.getX(),
                                         deltaY + newClip.getY(),
                                         newClip.getWidth(),
                                         newClip.getHeight());
                }
                else
                {
                    newClip.translate (-c->getX(), -c->getY());
                    c->clipObscuredRegions (g, newClip,
                                            c->getX() + deltaX,
                                            c->getY() + deltaY);
                }
            }
        }
    }
}

void Component::getVisibleArea (RectangleList& result,
                                const bool includeSiblings) const
{
    result.clear();
    const Rectangle<int> unclipped (getUnclippedArea());

    if (! unclipped.isEmpty())
    {
        result.add (unclipped);

        if (includeSiblings)
        {
            const Component* const c = getTopLevelComponent();

            c->subtractObscuredRegions (result, c->relativePositionToOtherComponent (this, Point<int>()),
                                        Rectangle<int> (0, 0, c->getWidth(), c->getHeight()),
                                        this);
        }

        subtractObscuredRegions (result, Point<int>(), unclipped, 0);
        result.consolidate();
    }
}

void Component::subtractObscuredRegions (RectangleList& result,
                                         const Point<int>& delta,
                                         const Rectangle<int>& clipRect,
                                         const Component* const compToAvoid) const throw()
{
    for (int i = childComponentList_.size(); --i >= 0;)
    {
        const Component* const c = childComponentList_.getUnchecked(i);

        if (c != compToAvoid && c->isVisible())
        {
            if (c->isOpaque())
            {
                Rectangle<int> childBounds (c->bounds_.getIntersection (clipRect));
                childBounds.translate (delta.getX(), delta.getY());

                result.subtract (childBounds);
            }
            else
            {
                Rectangle<int> newClip (clipRect.getIntersection (c->bounds_));
                newClip.translate (-c->getX(), -c->getY());

                c->subtractObscuredRegions (result, c->getPosition() + delta,
                                            newClip, compToAvoid);
            }
        }
    }
}

//==============================================================================
void Component::mouseEnter (const MouseEvent&)
{
    // base class does nothing
}

void Component::mouseExit (const MouseEvent&)
{
    // base class does nothing
}

void Component::mouseDown (const MouseEvent&)
{
    // base class does nothing
}

void Component::mouseUp (const MouseEvent&)
{
    // base class does nothing
}

void Component::mouseDrag (const MouseEvent&)
{
    // base class does nothing
}

void Component::mouseMove (const MouseEvent&)
{
    // base class does nothing
}

void Component::mouseDoubleClick (const MouseEvent&)
{
    // base class does nothing
}

void Component::mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
{
    // the base class just passes this event up to its parent..

    if (parentComponent_ != 0)
        parentComponent_->mouseWheelMove (e.getEventRelativeTo (parentComponent_),
                                          wheelIncrementX, wheelIncrementY);
}


//==============================================================================
void Component::resized()
{
    // base class does nothing
}

void Component::moved()
{
    // base class does nothing
}

void Component::childBoundsChanged (Component*)
{
    // base class does nothing
}

void Component::parentSizeChanged()
{
    // base class does nothing
}

void Component::addComponentListener (ComponentListener* const newListener) throw()
{
    if (componentListeners_ == 0)
        componentListeners_ = new VoidArray();

    componentListeners_->addIfNotAlreadyThere (newListener);
}

void Component::removeComponentListener (ComponentListener* const listenerToRemove) throw()
{
    jassert (isValidComponent());

    if (componentListeners_ != 0)
        componentListeners_->removeValue (listenerToRemove);
}

//==============================================================================
void Component::inputAttemptWhenModal()
{
    bringModalComponentToFront();
    getLookAndFeel().playAlertSound();
}

bool Component::canModalEventBeSentToComponent (const Component*)
{
    return false;
}

void Component::internalModalInputAttempt()
{
    Component* const current = getCurrentlyModalComponent();

    if (current != 0)
        current->inputAttemptWhenModal();
}


//==============================================================================
void Component::paint (Graphics&)
{
    // all painting is done in the subclasses

    jassert (! isOpaque()); // if your component's opaque, you've gotta paint it!
}

void Component::paintOverChildren (Graphics&)
{
    // all painting is done in the subclasses
}

//==============================================================================
void Component::handleMessage (const Message& message)
{
    if (message.intParameter1 == exitModalStateMessage)
    {
        exitModalState (message.intParameter2);
    }
    else if (message.intParameter1 == customCommandMessage)
    {
        handleCommandMessage (message.intParameter2);
    }
}

//==============================================================================
void Component::postCommandMessage (const int commandId) throw()
{
    postMessage (new Message (customCommandMessage, commandId, 0, 0));
}

void Component::handleCommandMessage (int)
{
    // used by subclasses
}

//==============================================================================
void Component::addMouseListener (MouseListener* const newListener,
                                  const bool wantsEventsForAllNestedChildComponents) throw()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (mouseListeners_ == 0)
        mouseListeners_ = new VoidArray();

    if (! mouseListeners_->contains (newListener))
    {
        if (wantsEventsForAllNestedChildComponents)
        {
            mouseListeners_->insert (0, newListener);
            ++numDeepMouseListeners;
        }
        else
        {
            mouseListeners_->add (newListener);
        }
    }
}

void Component::removeMouseListener (MouseListener* const listenerToRemove) throw()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (mouseListeners_ != 0)
    {
        const int index = mouseListeners_->indexOf (listenerToRemove);

        if (index >= 0)
        {
            if (index < numDeepMouseListeners)
                --numDeepMouseListeners;

            mouseListeners_->remove (index);
        }
    }
}

//==============================================================================
void Component::internalMouseEnter (MouseInputSource& source, const Point<int>& relativePos, const Time& time)
{
    if (isCurrentlyBlockedByAnotherModalComponent())
    {
        // if something else is modal, always just show a normal mouse cursor
        if (componentUnderMouse == this)
        {
            ComponentPeer* const peer = getPeer();

            if (peer != 0)
            {
                MouseCursor mc (MouseCursor::NormalCursor);
                mc.showInWindow (peer);
            }
        }

        return;
    }

    if (isValidComponent() && ! flags.mouseInsideFlag)
    {
        flags.mouseInsideFlag = true;
        flags.mouseOverFlag = true;
        flags.draggingFlag = false;

        const ComponentDeletionWatcher deletionChecker (this);

        if (flags.repaintOnMouseActivityFlag)
            repaint();

        const MouseEvent me (source, relativePos, source.getCurrentModifiers(),
                             this, time, relativePos,
                             time, 0, false);

        mouseEnter (me);

        if (deletionChecker.hasBeenDeleted())
            return;

        Desktop::getInstance().resetTimer();

        for (int i = Desktop::getInstance().mouseListeners.size(); --i >= 0;)
        {
            ((MouseListener*) Desktop::getInstance().mouseListeners[i])->mouseEnter (me);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, Desktop::getInstance().mouseListeners.size());
        }

        if (mouseListeners_ != 0)
        {
            for (int i = mouseListeners_->size(); --i >= 0;)
            {
                ((MouseListener*) mouseListeners_->getUnchecked(i))->mouseEnter (me);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, mouseListeners_->size());
            }
        }

        const Component* p = parentComponent_;

        while (p != 0)
        {
            const ComponentDeletionWatcher parentDeletionChecker (p);

            for (int i = p->numDeepMouseListeners; --i >= 0;)
            {
                ((MouseListener*) (p->mouseListeners_->getUnchecked(i)))->mouseEnter (me);

                if (deletionChecker.hasBeenDeleted() || parentDeletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, p->numDeepMouseListeners);
            }

            p = p->parentComponent_;
        }
    }

    if (componentUnderMouse == this)
        internalUpdateMouseCursor (true);
}

void Component::internalMouseExit (MouseInputSource& source, const Point<int>& relativePos, const Time& time)
{
    const ComponentDeletionWatcher deletionChecker (this);

    if (flags.draggingFlag)
    {
        internalMouseUp (source, relativePos, time, source.getCurrentModifiers().getRawFlags());

        if (deletionChecker.hasBeenDeleted())
            return;
    }

    enableUnboundedMouseMovement (false);

    if (flags.mouseInsideFlag || flags.mouseOverFlag)
    {
        flags.mouseInsideFlag = false;
        flags.mouseOverFlag = false;
        flags.draggingFlag = false;

        if (flags.repaintOnMouseActivityFlag)
            repaint();

        const MouseEvent me (source, relativePos, source.getCurrentModifiers(),
                             this, time, relativePos,
                             time, 0, false);
        mouseExit (me);

        if (deletionChecker.hasBeenDeleted())
            return;

        Desktop::getInstance().resetTimer();

        for (int i = Desktop::getInstance().mouseListeners.size(); --i >= 0;)
        {
            ((MouseListener*) Desktop::getInstance().mouseListeners[i])->mouseExit (me);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, Desktop::getInstance().mouseListeners.size());
        }

        if (mouseListeners_ != 0)
        {
            for (int i = mouseListeners_->size(); --i >= 0;)
            {
                ((MouseListener*) mouseListeners_->getUnchecked (i))->mouseExit (me);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, mouseListeners_->size());
            }
        }

        const Component* p = parentComponent_;

        while (p != 0)
        {
            const ComponentDeletionWatcher parentDeletionChecker (p);

            for (int i = p->numDeepMouseListeners; --i >= 0;)
            {
                ((MouseListener*) (p->mouseListeners_->getUnchecked (i)))->mouseExit (me);

                if (deletionChecker.hasBeenDeleted() || parentDeletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, p->numDeepMouseListeners);
            }

            p = p->parentComponent_;
        }
    }
}

//==============================================================================
class InternalDragRepeater  : public Timer
{
public:
    InternalDragRepeater()
    {}

    ~InternalDragRepeater()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (InternalDragRepeater)

    void timerCallback()
    {
        Desktop& desktop = Desktop::getInstance();
        int numMiceDown = 0;

        for (int i = desktop.getNumMouseInputSources(); --i >= 0;)
        {
            MouseInputSource* const source = desktop.getMouseSource(i);
            if (source->isDragging())
            {
                source->triggerFakeMove();
                ++numMiceDown;
            }
        }

        if (numMiceDown == 0)
            deleteInstance();
    }

    juce_UseDebuggingNewOperator

private:
    InternalDragRepeater (const InternalDragRepeater&);
    InternalDragRepeater& operator= (const InternalDragRepeater&);
};

juce_ImplementSingleton_SingleThreaded (InternalDragRepeater)


void Component::beginDragAutoRepeat (const int interval)
{
    if (interval > 0)
    {
        if (InternalDragRepeater::getInstance()->getTimerInterval() != interval)
            InternalDragRepeater::getInstance()->startTimer (interval);
    }
    else
    {
        InternalDragRepeater::deleteInstance();
    }
}

//==============================================================================
void Component::internalMouseDown (MouseInputSource& source, const Point<int>& relativePos, const Time& time)
{
    Desktop& desktop = Desktop::getInstance();

    const ComponentDeletionWatcher deletionChecker (this);

    if (isCurrentlyBlockedByAnotherModalComponent())
    {
        internalModalInputAttempt();

        if (deletionChecker.hasBeenDeleted())
            return;

        // If processing the input attempt has exited the modal loop, we'll allow the event
        // to be delivered..
        if (isCurrentlyBlockedByAnotherModalComponent())
        {
            // allow blocked mouse-events to go to global listeners..
            const MouseEvent me (source, relativePos, source.getCurrentModifiers(),
                                 this, time, relativePos, time,
                                 source.getNumberOfMultipleClicks(), false);

            desktop.resetTimer();

            for (int i = desktop.mouseListeners.size(); --i >= 0;)
            {
                ((MouseListener*) desktop.mouseListeners[i])->mouseDown (me);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, desktop.mouseListeners.size());
            }

            return;
        }
    }

    {
        Component* c = this;

        while (c != 0)
        {
            if (c->isBroughtToFrontOnMouseClick())
            {
                c->toFront (true);

                if (deletionChecker.hasBeenDeleted())
                    return;
            }

            c = c->parentComponent_;
        }
    }

    if (! flags.dontFocusOnMouseClickFlag)
        grabFocusInternal (focusChangedByMouseClick);

    if (! deletionChecker.hasBeenDeleted())
    {
        flags.draggingFlag = true;
        flags.mouseOverFlag = true;

        if (flags.repaintOnMouseActivityFlag)
            repaint();

        const MouseEvent me (source, relativePos, source.getCurrentModifiers(),
                             this, time, relativePos, time,
                             source.getNumberOfMultipleClicks(), false);
        mouseDown (me);

        if (deletionChecker.hasBeenDeleted())
            return;

        desktop.resetTimer();

        for (int i = desktop.mouseListeners.size(); --i >= 0;)
        {
            ((MouseListener*) desktop.mouseListeners[i])->mouseDown (me);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, desktop.mouseListeners.size());
        }

        if (mouseListeners_ != 0)
        {
            for (int i = mouseListeners_->size(); --i >= 0;)
            {
                ((MouseListener*) mouseListeners_->getUnchecked (i))->mouseDown (me);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, mouseListeners_->size());
            }
        }

        const Component* p = parentComponent_;

        while (p != 0)
        {
            const ComponentDeletionWatcher parentDeletionChecker (p);

            for (int i = p->numDeepMouseListeners; --i >= 0;)
            {
                ((MouseListener*) (p->mouseListeners_->getUnchecked (i)))->mouseDown (me);

                if (deletionChecker.hasBeenDeleted() || parentDeletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, p->numDeepMouseListeners);
            }

            p = p->parentComponent_;
        }
    }
}

//==============================================================================
void Component::internalMouseUp (MouseInputSource& source, const Point<int>& relativePos, const Time& time, const ModifierKeys& oldModifiers)
{
    if (isValidComponent() && flags.draggingFlag)
    {
        Desktop& desktop = Desktop::getInstance();

        flags.draggingFlag = false;

        const ComponentDeletionWatcher deletionChecker (this);

        if (flags.repaintOnMouseActivityFlag)
            repaint();

        const MouseEvent me (source, relativePos + unboundedMouseOffset,
                             oldModifiers, this, time,
                             globalPositionToRelative (source.getLastMouseDownPosition()),
                             source.getLastMouseDownTime(),
                             source.getNumberOfMultipleClicks(),
                             source.hasMouseMovedSignificantlySincePressed());

        mouseUp (me);

        if (deletionChecker.hasBeenDeleted())
            return;

        desktop.resetTimer();

        for (int i = desktop.mouseListeners.size(); --i >= 0;)
        {
            ((MouseListener*) desktop.mouseListeners[i])->mouseUp (me);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, desktop.mouseListeners.size());
        }

        if (mouseListeners_ != 0)
        {
            for (int i = mouseListeners_->size(); --i >= 0;)
            {
                ((MouseListener*) mouseListeners_->getUnchecked (i))->mouseUp (me);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, mouseListeners_->size());
            }
        }

        {
            const Component* p = parentComponent_;

            while (p != 0)
            {
                const ComponentDeletionWatcher parentDeletionChecker (p);

                for (int i = p->numDeepMouseListeners; --i >= 0;)
                {
                    ((MouseListener*) (p->mouseListeners_->getUnchecked (i)))->mouseUp (me);

                    if (deletionChecker.hasBeenDeleted() || parentDeletionChecker.hasBeenDeleted())
                        return;

                    i = jmin (i, p->numDeepMouseListeners);
                }

                p = p->parentComponent_;
            }
        }

        // check for double-click
        if (me.getNumberOfClicks() >= 2)
        {
            const int numListeners = (mouseListeners_ != 0) ? mouseListeners_->size() : 0;

            mouseDoubleClick (me);

            int i;
            for (i = desktop.mouseListeners.size(); --i >= 0;)
            {
                ((MouseListener*) desktop.mouseListeners[i])->mouseDoubleClick (me);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, desktop.mouseListeners.size());
            }

            for (i = numListeners; --i >= 0;)
            {
                if (deletionChecker.hasBeenDeleted() || mouseListeners_ == 0)
                    return;

                MouseListener* const ml = (MouseListener*)((*mouseListeners_)[i]);
                if (ml != 0)
                    ml->mouseDoubleClick (me);
            }

            if (deletionChecker.hasBeenDeleted())
                return;

            const Component* p = parentComponent_;

            while (p != 0)
            {
                const ComponentDeletionWatcher parentDeletionChecker (p);

                for (i = p->numDeepMouseListeners; --i >= 0;)
                {
                    ((MouseListener*) (p->mouseListeners_->getUnchecked (i)))->mouseDoubleClick (me);

                    if (deletionChecker.hasBeenDeleted() || parentDeletionChecker.hasBeenDeleted())
                        return;

                    i = jmin (i, p->numDeepMouseListeners);
                }

                p = p->parentComponent_;
            }
        }
    }

    enableUnboundedMouseMovement (false);
}

void Component::internalMouseDrag (MouseInputSource& source, const Point<int>& relativePos, const Time& time)
{
    if (isValidComponent() && flags.draggingFlag)
    {
        Desktop& desktop = Desktop::getInstance();

        flags.mouseOverFlag = reallyContains (relativePos.getX(), relativePos.getY(), false);

        const ComponentDeletionWatcher deletionChecker (this);

        const MouseEvent me (source, relativePos + unboundedMouseOffset,
                             source.getCurrentModifiers(), this, time,
                             globalPositionToRelative (source.getLastMouseDownPosition()),
                             source.getLastMouseDownTime(),
                             source.getNumberOfMultipleClicks(),
                             source.hasMouseMovedSignificantlySincePressed());

        mouseDrag (me);

        if (deletionChecker.hasBeenDeleted())
            return;

        desktop.resetTimer();

        for (int i = desktop.mouseListeners.size(); --i >= 0;)
        {
            ((MouseListener*) desktop.mouseListeners[i])->mouseDrag (me);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, desktop.mouseListeners.size());
        }

        if (mouseListeners_ != 0)
        {
            for (int i = mouseListeners_->size(); --i >= 0;)
            {
                ((MouseListener*) mouseListeners_->getUnchecked (i))->mouseDrag (me);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, mouseListeners_->size());
            }
        }

        const Component* p = parentComponent_;

        while (p != 0)
        {
            const ComponentDeletionWatcher parentDeletionChecker (p);

            for (int i = p->numDeepMouseListeners; --i >= 0;)
            {
                ((MouseListener*) (p->mouseListeners_->getUnchecked (i)))->mouseDrag (me);

                if (deletionChecker.hasBeenDeleted() || parentDeletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, p->numDeepMouseListeners);
            }

            p = p->parentComponent_;
        }

        if (this == componentUnderMouse)
        {
            if (isUnboundedMouseModeOn)
            {
                const Rectangle<int> screenArea (getParentMonitorArea().expanded (-2, -2));
                Point<int> mousePos (source.getScreenPosition());

                if (! screenArea.contains (mousePos))
                {
                    int deltaX = 0, deltaY = 0;

                    if (mousePos.getX() <= screenArea.getX() || mousePos.getX() >= screenArea.getRight())
                        deltaX = getScreenX() + getWidth() / 2 - mousePos.getX();

                    if (mousePos.getY() <= screenArea.getY() || mousePos.getY() >= screenArea.getBottom())
                        deltaY = getScreenY() + getHeight() / 2 - mousePos.getY();

                    unboundedMouseOffset -= Point<int> (deltaX, deltaY);

                    Desktop::setMousePosition (mousePos + Point<int> (deltaX, deltaY));
                }
                else if (isCursorVisibleUntilOffscreen
                          && (! unboundedMouseOffset.isOrigin())
                          && screenArea.contains (mousePos + unboundedMouseOffset))
                {
                    mousePos += unboundedMouseOffset;
                    unboundedMouseOffset = Point<int>();

                    Desktop::setMousePosition (mousePos);
                }
            }

            internalUpdateMouseCursor (false);
        }
    }
}

void Component::internalMouseMove (MouseInputSource& source, const Point<int>& relativePos, const Time& time)
{
    const ComponentDeletionWatcher deletionChecker (this);

    if (isValidComponent())
    {
        Desktop& desktop = Desktop::getInstance();

        const MouseEvent me (source, relativePos, source.getCurrentModifiers(),
                             this, time, relativePos,
                             time, 0, false);

        if (isCurrentlyBlockedByAnotherModalComponent())
        {
            // allow blocked mouse-events to go to global listeners..
            desktop.sendMouseMove();
        }
        else
        {
            if (this == componentUnderMouse)
                internalUpdateMouseCursor (false);

            flags.mouseOverFlag = true;

            mouseMove (me);

            if (deletionChecker.hasBeenDeleted())
                return;

            desktop.resetTimer();

            for (int i = desktop.mouseListeners.size(); --i >= 0;)
            {
                ((MouseListener*) desktop.mouseListeners[i])->mouseMove (me);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, desktop.mouseListeners.size());
            }

            if (mouseListeners_ != 0)
            {
                for (int i = mouseListeners_->size(); --i >= 0;)
                {
                    ((MouseListener*) mouseListeners_->getUnchecked (i))->mouseMove (me);

                    if (deletionChecker.hasBeenDeleted())
                        return;

                    i = jmin (i, mouseListeners_->size());
                }
            }

            const Component* p = parentComponent_;

            while (p != 0)
            {
                const ComponentDeletionWatcher parentDeletionChecker (p);

                for (int i = p->numDeepMouseListeners; --i >= 0;)
                {
                    ((MouseListener*) (p->mouseListeners_->getUnchecked (i)))->mouseMove (me);

                    if (deletionChecker.hasBeenDeleted() || parentDeletionChecker.hasBeenDeleted())
                        return;

                    i = jmin (i, p->numDeepMouseListeners);
                }

                p = p->parentComponent_;
            }
        }
    }
}

void Component::internalMouseWheel (MouseInputSource& source, const Point<int>& relativePos,
                                    const Time& time, const float amountX, const float amountY)
{
    Desktop& desktop = Desktop::getInstance();
    const ComponentDeletionWatcher deletionChecker (this);

    const float wheelIncrementX = amountX * (1.0f / 256.0f);
    const float wheelIncrementY = amountY * (1.0f / 256.0f);

    const MouseEvent me (source, relativePos, source.getCurrentModifiers(),
                         this, time, relativePos, time, 0, false);

    if (isCurrentlyBlockedByAnotherModalComponent())
    {
        // allow blocked mouse-events to go to global listeners..
        for (int i = desktop.mouseListeners.size(); --i >= 0;)
        {
            ((MouseListener*) desktop.mouseListeners[i])->mouseWheelMove (me, wheelIncrementX, wheelIncrementY);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, desktop.mouseListeners.size());
        }
    }
    else
    {
        mouseWheelMove (me, wheelIncrementX, wheelIncrementY);

        if (deletionChecker.hasBeenDeleted())
            return;

        for (int i = desktop.mouseListeners.size(); --i >= 0;)
        {
            ((MouseListener*) desktop.mouseListeners[i])->mouseWheelMove (me, wheelIncrementX, wheelIncrementY);

            if (deletionChecker.hasBeenDeleted())
                return;

            i = jmin (i, desktop.mouseListeners.size());
        }

        if (mouseListeners_ != 0)
        {
            for (int i = mouseListeners_->size(); --i >= 0;)
            {
                ((MouseListener*) mouseListeners_->getUnchecked (i))->mouseWheelMove (me, wheelIncrementX, wheelIncrementY);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, mouseListeners_->size());
            }
        }

        const Component* p = parentComponent_;

        while (p != 0)
        {
            const ComponentDeletionWatcher parentDeletionChecker (p);

            for (int i = p->numDeepMouseListeners; --i >= 0;)
            {
                ((MouseListener*) (p->mouseListeners_->getUnchecked (i)))->mouseWheelMove (me, wheelIncrementX, wheelIncrementY);

                if (deletionChecker.hasBeenDeleted() || parentDeletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, p->numDeepMouseListeners);
            }

            p = p->parentComponent_;
        }
    }
}

void Component::sendFakeMouseMove() const
{
    Desktop::getInstance().getMainMouseSource().triggerFakeMove();
}

void Component::broughtToFront()
{
}

void Component::internalBroughtToFront()
{
    if (isValidComponent())
    {
        if (flags.hasHeavyweightPeerFlag)
            Desktop::getInstance().componentBroughtToFront (this);

        const ComponentDeletionWatcher deletionChecker (this);
        broughtToFront();

        if (deletionChecker.hasBeenDeleted())
            return;

        if (componentListeners_ != 0)
        {
            for (int i = componentListeners_->size(); --i >= 0;)
            {
                ((ComponentListener*) componentListeners_->getUnchecked (i))
                    ->componentBroughtToFront (*this);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, componentListeners_->size());
            }
        }

        // when brought to the front and there's a modal component blocking this one,
        // we need to bring the modal one to the front instead..

        Component* const cm = getCurrentlyModalComponent();

        if (cm != 0 && cm->getTopLevelComponent() != getTopLevelComponent())
            bringModalComponentToFront();
    }
}

void Component::focusGained (FocusChangeType)
{
    // base class does nothing
}

void Component::internalFocusGain (const FocusChangeType cause)
{
    const ComponentDeletionWatcher deletionChecker (this);

    focusGained (cause);

    if (! deletionChecker.hasBeenDeleted())
        internalChildFocusChange (cause);
}

void Component::focusLost (FocusChangeType)
{
    // base class does nothing
}

void Component::internalFocusLoss (const FocusChangeType cause)
{
    const ComponentDeletionWatcher deletionChecker (this);

    focusLost (focusChangedDirectly);

    if (! deletionChecker.hasBeenDeleted())
        internalChildFocusChange (cause);
}

void Component::focusOfChildComponentChanged (FocusChangeType /*cause*/)
{
    // base class does nothing
}

void Component::internalChildFocusChange (FocusChangeType cause)
{
    const bool childIsNowFocused = hasKeyboardFocus (true);

    if (flags.childCompFocusedFlag != childIsNowFocused)
    {
        flags.childCompFocusedFlag = childIsNowFocused;

        const ComponentDeletionWatcher deletionChecker (this);
        focusOfChildComponentChanged (cause);

        if (deletionChecker.hasBeenDeleted())
            return;
    }

    if (parentComponent_ != 0)
        parentComponent_->internalChildFocusChange (cause);
}

//==============================================================================
bool Component::isEnabled() const throw()
{
    return (! flags.isDisabledFlag)
            && (parentComponent_ == 0 || parentComponent_->isEnabled());
}

void Component::setEnabled (const bool shouldBeEnabled)
{
    if (flags.isDisabledFlag == shouldBeEnabled)
    {
        flags.isDisabledFlag = ! shouldBeEnabled;

        // if any parent components are disabled, setting our flag won't make a difference,
        // so no need to send a change message
        if (parentComponent_ == 0 || parentComponent_->isEnabled())
            sendEnablementChangeMessage();
    }
}

void Component::sendEnablementChangeMessage()
{
    const ComponentDeletionWatcher deletionChecker (this);

    enablementChanged();

    if (deletionChecker.hasBeenDeleted())
        return;

    for (int i = getNumChildComponents(); --i >= 0;)
    {
        Component* const c = getChildComponent (i);

        if (c != 0)
        {
            c->sendEnablementChangeMessage();

            if (deletionChecker.hasBeenDeleted())
                return;
        }
    }
}

void Component::enablementChanged()
{
}

//==============================================================================
void Component::setWantsKeyboardFocus (const bool wantsFocus) throw()
{
    flags.wantsFocusFlag = wantsFocus;
}

void Component::setMouseClickGrabsKeyboardFocus (const bool shouldGrabFocus)
{
    flags.dontFocusOnMouseClickFlag = ! shouldGrabFocus;
}

bool Component::getMouseClickGrabsKeyboardFocus() const throw()
{
    return ! flags.dontFocusOnMouseClickFlag;
}

bool Component::getWantsKeyboardFocus() const throw()
{
    return flags.wantsFocusFlag && ! flags.isDisabledFlag;
}

void Component::setFocusContainer (const bool shouldBeFocusContainer) throw()
{
    flags.isFocusContainerFlag = shouldBeFocusContainer;
}

bool Component::isFocusContainer() const throw()
{
    return flags.isFocusContainerFlag;
}

int Component::getExplicitFocusOrder() const throw()
{
    return properties ["_jexfo"];
}

void Component::setExplicitFocusOrder (const int newFocusOrderIndex) throw()
{
    properties.set ("_jexfo", newFocusOrderIndex);
}

KeyboardFocusTraverser* Component::createFocusTraverser()
{
    if (flags.isFocusContainerFlag || parentComponent_ == 0)
        return new KeyboardFocusTraverser();

    return parentComponent_->createFocusTraverser();
}

void Component::takeKeyboardFocus (const FocusChangeType cause)
{
    // give the focus to this component
    if (currentlyFocusedComponent != this)
    {
        JUCE_TRY
        {
            // get the focus onto our desktop window
            ComponentPeer* const peer = getPeer();

            if (peer != 0)
            {
                const ComponentDeletionWatcher deletionChecker (this);

                peer->grabFocus();

                if (peer->isFocused() && currentlyFocusedComponent != this)
                {
                    Component* const componentLosingFocus = currentlyFocusedComponent;

                    currentlyFocusedComponent = this;

                    Desktop::getInstance().triggerFocusCallback();

                    // call this after setting currentlyFocusedComponent so that the one that's
                    // losing it has a chance to see where focus is going
                    if (componentLosingFocus->isValidComponent())
                        componentLosingFocus->internalFocusLoss (cause);

                    if (currentlyFocusedComponent == this)
                    {
                        focusGained (cause);

                        if (! deletionChecker.hasBeenDeleted())
                            internalChildFocusChange (cause);
                    }
                }
            }
        }
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS
        catch (const std::exception& e)
        {
            currentlyFocusedComponent = 0;
            Desktop::getInstance().triggerFocusCallback();
            JUCEApplication::sendUnhandledException (&e, __FILE__, __LINE__);
        }
        catch (...)
        {
            currentlyFocusedComponent = 0;
            Desktop::getInstance().triggerFocusCallback();
            JUCEApplication::sendUnhandledException (0, __FILE__, __LINE__);
        }
#endif
    }
}

void Component::grabFocusInternal (const FocusChangeType cause, const bool canTryParent)
{
    if (isShowing())
    {
        if (flags.wantsFocusFlag && (isEnabled() || parentComponent_ == 0))
        {
            takeKeyboardFocus (cause);
        }
        else
        {
            if (isParentOf (currentlyFocusedComponent)
                 && currentlyFocusedComponent->isShowing())
            {
                // do nothing if the focused component is actually a child of ours..
            }
            else
            {
                // find the default child component..
                ScopedPointer <KeyboardFocusTraverser> traverser (createFocusTraverser());

                if (traverser != 0)
                {
                    Component* const defaultComp = traverser->getDefaultComponent (this);
                    traverser = 0;

                    if (defaultComp != 0)
                    {
                        defaultComp->grabFocusInternal (cause, false);
                        return;
                    }
                }

                if (canTryParent && parentComponent_ != 0)
                {
                    // if no children want it and we're allowed to try our parent comp,
                    // then pass up to parent, which will try our siblings.
                    parentComponent_->grabFocusInternal (cause, true);
                }
            }
        }
    }
}

void Component::grabKeyboardFocus()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    grabFocusInternal (focusChangedDirectly);
}

void Component::moveKeyboardFocusToSibling (const bool moveToNext)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    checkMessageManagerIsLocked

    if (parentComponent_ != 0)
    {
        ScopedPointer <KeyboardFocusTraverser> traverser (createFocusTraverser());

        if (traverser != 0)
        {
            Component* const nextComp = moveToNext ? traverser->getNextComponent (this)
                                                   : traverser->getPreviousComponent (this);
            traverser = 0;

            if (nextComp != 0)
            {
                if (nextComp->isCurrentlyBlockedByAnotherModalComponent())
                {
                    const ComponentDeletionWatcher deletionChecker (nextComp);
                    internalModalInputAttempt();

                    if (deletionChecker.hasBeenDeleted()
                         || nextComp->isCurrentlyBlockedByAnotherModalComponent())
                        return;
                }

                nextComp->grabFocusInternal (focusChangedByTabKey);
                return;
            }
        }

        parentComponent_->moveKeyboardFocusToSibling (moveToNext);
    }
}

bool Component::hasKeyboardFocus (const bool trueIfChildIsFocused) const throw()
{
    return (currentlyFocusedComponent == this)
            || (trueIfChildIsFocused && isParentOf (currentlyFocusedComponent));
}

Component* JUCE_CALLTYPE Component::getCurrentlyFocusedComponent() throw()
{
    return currentlyFocusedComponent;
}

void Component::giveAwayFocus()
{
    // use a copy so we can clear the value before the call
    Component* const componentLosingFocus = currentlyFocusedComponent;
    currentlyFocusedComponent = 0;
    Desktop::getInstance().triggerFocusCallback();

    if (componentLosingFocus->isValidComponent())
        componentLosingFocus->internalFocusLoss (focusChangedDirectly);
}

//==============================================================================
bool Component::isMouseOver() const throw()
{
    return flags.mouseOverFlag;
}

bool Component::isMouseButtonDown() const throw()
{
    return flags.draggingFlag;
}

bool Component::isMouseOverOrDragging() const throw()
{
    return flags.mouseOverFlag || flags.draggingFlag;
}

bool JUCE_CALLTYPE Component::isMouseButtonDownAnywhere() throw()
{
    return ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown();
}

const Point<int> Component::getMouseXYRelative() const
{
    return globalPositionToRelative (Desktop::getMousePosition()) + unboundedMouseOffset;
}

void Component::enableUnboundedMouseMovement (bool enable,
                                              bool keepCursorVisibleUntilOffscreen) throw()
{
    enable = enable && isMouseButtonDown();
    isCursorVisibleUntilOffscreen = keepCursorVisibleUntilOffscreen;

    if (enable != isUnboundedMouseModeOn)
    {
        if ((! enable) && ((! isCursorVisibleUntilOffscreen)
                            || ! unboundedMouseOffset.isOrigin()))
        {
            // when released, return the mouse to within the component's bounds
            Desktop::setMousePosition (relativePositionToGlobal (Rectangle<int> (0, 0, getWidth(), getHeight())
                                                                    .getConstrainedPoint (getMouseXYRelative())));
        }

        isUnboundedMouseModeOn = enable;
        unboundedMouseOffset = Point<int>();

        internalUpdateMouseCursor (true);
    }
}

Component* JUCE_CALLTYPE Component::getComponentUnderMouse() throw()
{
    return componentUnderMouse;
}

//==============================================================================
const Rectangle<int> Component::getParentMonitorArea() const throw()
{
    return Desktop::getInstance()
            .getMonitorAreaContaining (relativePositionToGlobal (Point<int> (getWidth() / 2,
                                                                             getHeight() / 2)));
}

//==============================================================================
void Component::addKeyListener (KeyListener* const newListener) throw()
{
    if (keyListeners_ == 0)
        keyListeners_ = new VoidArray();

    keyListeners_->addIfNotAlreadyThere (newListener);
}

void Component::removeKeyListener (KeyListener* const listenerToRemove) throw()
{
    if (keyListeners_ != 0)
        keyListeners_->removeValue (listenerToRemove);
}

bool Component::keyPressed (const KeyPress&)
{
    return false;
}

bool Component::keyStateChanged (const bool /*isKeyDown*/)
{
    return false;
}

void Component::modifierKeysChanged (const ModifierKeys& modifiers)
{
    if (parentComponent_ != 0)
        parentComponent_->modifierKeysChanged (modifiers);
}

void Component::internalModifierKeysChanged()
{
    sendFakeMouseMove();

    modifierKeysChanged (ModifierKeys::getCurrentModifiers());
}

//==============================================================================
ComponentPeer* Component::getPeer() const throw()
{
    if (flags.hasHeavyweightPeerFlag)
        return ComponentPeer::getPeerFor (this);
    else if (parentComponent_ != 0)
        return parentComponent_->getPeer();
    else
        return 0;
}

//==============================================================================
ComponentDeletionWatcher::ComponentDeletionWatcher (const Component* const componentToWatch_) throw()
    : componentToWatch (componentToWatch_),
      componentUID (componentToWatch_->getComponentUID())
{
    // not possible to check on an already-deleted object..
    jassert (componentToWatch_->isValidComponent());
}

ComponentDeletionWatcher::~ComponentDeletionWatcher() throw() {}

bool ComponentDeletionWatcher::hasBeenDeleted() const throw()
{
    return ! (componentToWatch->isValidComponent()
                && componentToWatch->getComponentUID() == componentUID);
}

const Component* ComponentDeletionWatcher::getComponent() const throw()
{
    return hasBeenDeleted() ? 0 : componentToWatch;
}


END_JUCE_NAMESPACE
