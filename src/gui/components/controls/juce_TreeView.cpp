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

#include "juce_TreeView.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../../maths/juce_BigInteger.h"
#include "../mouse/juce_DragAndDropContainer.h"
#include "../mouse/juce_MouseInputSource.h"
#include "../../graphics/imaging/juce_Image.h"


//==============================================================================
class TreeViewContentComponent  : public Component,
                                  public TooltipClient
{
public:
    TreeViewContentComponent (TreeView& owner_)
        : owner (owner_),
          buttonUnderMouse (0),
          isDragging (false)
    {
    }

    ~TreeViewContentComponent()
    {
    }

    void mouseDown (const MouseEvent& e)
    {
        updateButtonUnderMouse (e);

        isDragging = false;
        needSelectionOnMouseUp = false;

        Rectangle<int> pos;
        TreeViewItem* const item = findItemAt (e.y, pos);

        if (item == 0)
            return;

        // (if the open/close buttons are hidden, we'll treat clicks to the left of the item
        // as selection clicks)
        if (e.x < pos.getX() && owner.openCloseButtonsVisible)
        {
            if (e.x >= pos.getX() - owner.getIndentSize())
                item->setOpen (! item->isOpen());

            // (clicks to the left of an open/close button are ignored)
        }
        else
        {
            // mouse-down inside the body of the item..
            if (! owner.isMultiSelectEnabled())
                item->setSelected (true, true);
            else if (item->isSelected())
                needSelectionOnMouseUp = ! e.mods.isPopupMenu();
            else
                selectBasedOnModifiers (item, e.mods);

            if (e.x >= pos.getX())
                item->itemClicked (e.withNewPosition (e.getPosition() - pos.getPosition()));
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        updateButtonUnderMouse (e);

        if (needSelectionOnMouseUp && e.mouseWasClicked())
        {
            Rectangle<int> pos;
            TreeViewItem* const item = findItemAt (e.y, pos);

            if (item != 0)
                selectBasedOnModifiers (item, e.mods);
        }
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        if (e.getNumberOfClicks() != 3)  // ignore triple clicks
        {
            Rectangle<int> pos;
            TreeViewItem* const item = findItemAt (e.y, pos);

            if (item != 0 && (e.x >= pos.getX() || ! owner.openCloseButtonsVisible))
                item->itemDoubleClicked (e.withNewPosition (e.getPosition() - pos.getPosition()));
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (isEnabled()
             && ! (isDragging || e.mouseWasClicked()
                    || e.getDistanceFromDragStart() < 5
                    || e.mods.isPopupMenu()))
        {
            isDragging = true;

            Rectangle<int> pos;
            TreeViewItem* const item = findItemAt (e.getMouseDownY(), pos);

            if (item != 0 && e.getMouseDownX() >= pos.getX())
            {
                const String dragDescription (item->getDragSourceDescription());

                if (dragDescription.isNotEmpty())
                {
                    DragAndDropContainer* const dragContainer
                        = DragAndDropContainer::findParentDragContainerFor (this);

                    if (dragContainer != 0)
                    {
                        pos.setSize (pos.getWidth(), item->itemHeight);
                        Image dragImage (Component::createComponentSnapshot (pos, true));
                        dragImage.multiplyAllAlphas (0.6f);

                        Point<int> imageOffset (pos.getPosition() - e.getPosition());
                        dragContainer->startDragging (dragDescription, &owner, dragImage, true, &imageOffset);
                    }
                    else
                    {
                        // to be able to do a drag-and-drop operation, the treeview needs to
                        // be inside a component which is also a DragAndDropContainer.
                        jassertfalse;
                    }
                }
            }
        }
    }

    void mouseMove (const MouseEvent& e)
    {
        updateButtonUnderMouse (e);
    }

    void mouseExit (const MouseEvent& e)
    {
        updateButtonUnderMouse (e);
    }

    void paint (Graphics& g)
    {
        if (owner.rootItem != 0)
        {
            owner.handleAsyncUpdate();

            if (! owner.rootItemVisible)
                g.setOrigin (0, -owner.rootItem->itemHeight);

            owner.rootItem->paintRecursively (g, getWidth());
        }
    }

    TreeViewItem* findItemAt (int y, Rectangle<int>& itemPosition) const
    {
        if (owner.rootItem != 0)
        {
            owner.handleAsyncUpdate();

            if (! owner.rootItemVisible)
                y += owner.rootItem->itemHeight;

            TreeViewItem* const ti = owner.rootItem->findItemRecursively (y);

            if (ti != 0)
                itemPosition = ti->getItemPosition (false);

            return ti;
        }

        return 0;
    }

    void updateComponents()
    {
        const int visibleTop = -getY();
        const int visibleBottom = visibleTop + getParentHeight();

        {
            for (int i = items.size(); --i >= 0;)
                items.getUnchecked(i)->shouldKeep = false;
        }

        {
            TreeViewItem* item = owner.rootItem;
            int y = (item != 0 && ! owner.rootItemVisible) ? -item->itemHeight : 0;

            while (item != 0 && y < visibleBottom)
            {
                y += item->itemHeight;

                if (y >= visibleTop)
                {
                    RowItem* const ri = findItem (item->uid);

                    if (ri != 0)
                    {
                        ri->shouldKeep = true;
                    }
                    else
                    {
                        Component* const comp = item->createItemComponent();

                        if (comp != 0)
                        {
                            items.add (new RowItem (item, comp, item->uid));
                            addAndMakeVisible (comp);
                        }
                    }
                }

                item = item->getNextVisibleItem (true);
            }
        }

        for (int i = items.size(); --i >= 0;)
        {
            RowItem* const ri = items.getUnchecked(i);
            bool keep = false;

            if (isParentOf (ri->component))
            {
                if (ri->shouldKeep)
                {
                    Rectangle<int> pos (ri->item->getItemPosition (false));
                    pos.setSize (pos.getWidth(), ri->item->itemHeight);

                    if (pos.getBottom() >= visibleTop && pos.getY() < visibleBottom)
                    {
                        keep = true;
                        ri->component->setBounds (pos);
                    }
                }

                if ((! keep) && isMouseDraggingInChildCompOf (ri->component))
                {
                    keep = true;
                    ri->component->setSize (0, 0);
                }
            }

            if (! keep)
                items.remove (i);
        }
    }

    void updateButtonUnderMouse (const MouseEvent& e)
    {
        TreeViewItem* newItem = 0;

        if (owner.openCloseButtonsVisible)
        {
            Rectangle<int> pos;
            TreeViewItem* item = findItemAt (e.y, pos);

            if (item != 0 && e.x < pos.getX() && e.x >= pos.getX() - owner.getIndentSize())
            {
                newItem = item;

                if (! newItem->mightContainSubItems())
                    newItem = 0;
            }
        }

        if (buttonUnderMouse != newItem)
        {
            if (buttonUnderMouse != 0 && containsItem (buttonUnderMouse))
            {
                const Rectangle<int> r (buttonUnderMouse->getItemPosition (false));
                repaint (0, r.getY(), r.getX(), buttonUnderMouse->getItemHeight());
            }

            buttonUnderMouse = newItem;

            if (buttonUnderMouse != 0)
            {
                const Rectangle<int> r (buttonUnderMouse->getItemPosition (false));
                repaint (0, r.getY(), r.getX(), buttonUnderMouse->getItemHeight());
            }
        }
    }

    bool isMouseOverButton (TreeViewItem* const item) const throw()
    {
        return item == buttonUnderMouse;
    }

    void resized()
    {
        owner.itemsChanged();
    }

    const String getTooltip()
    {
        Rectangle<int> pos;
        TreeViewItem* const item = findItemAt (getMouseXYRelative().getY(), pos);

        if (item != 0)
            return item->getTooltip();

        return owner.getTooltip();
    }

private:
    //==============================================================================
    TreeView& owner;

    struct RowItem
    {
        RowItem (TreeViewItem* const item_, Component* const component_, const int itemUID)
            : component (component_), item (item_), uid (itemUID), shouldKeep (true)
        {
        }

        ~RowItem()
        {
            delete component.get();
        }

        WeakReference<Component> component;
        TreeViewItem* item;
        int uid;
        bool shouldKeep;
    };

    OwnedArray <RowItem> items;

    TreeViewItem* buttonUnderMouse;
    bool isDragging, needSelectionOnMouseUp;

    void selectBasedOnModifiers (TreeViewItem* const item, const ModifierKeys& modifiers)
    {
        TreeViewItem* firstSelected = 0;

        if (modifiers.isShiftDown() && ((firstSelected = owner.getSelectedItem (0)) != 0))
        {
            TreeViewItem* const lastSelected = owner.getSelectedItem (owner.getNumSelectedItems() - 1);
            jassert (lastSelected != 0);

            int rowStart = firstSelected->getRowNumberInTree();
            int rowEnd = lastSelected->getRowNumberInTree();
            if (rowStart > rowEnd)
                swapVariables (rowStart, rowEnd);

            int ourRow = item->getRowNumberInTree();
            int otherEnd = ourRow < rowEnd ? rowStart : rowEnd;

            if (ourRow > otherEnd)
                swapVariables (ourRow, otherEnd);

            for (int i = ourRow; i <= otherEnd; ++i)
                owner.getItemOnRow (i)->setSelected (true, false);
        }
        else
        {
            const bool cmd = modifiers.isCommandDown();
            item->setSelected ((! cmd) || ! item->isSelected(), ! cmd);
        }
    }

    bool containsItem (TreeViewItem* const item) const throw()
    {
        for (int i = items.size(); --i >= 0;)
            if (items.getUnchecked(i)->item == item)
                return true;

        return false;
    }

    RowItem* findItem (const int uid) const throw()
    {
        for (int i = items.size(); --i >= 0;)
        {
            RowItem* const ri = items.getUnchecked(i);
            if (ri->uid == uid)
                return ri;
        }

        return 0;
    }

    static bool isMouseDraggingInChildCompOf (Component* const comp)
    {
        for (int i = Desktop::getInstance().getNumMouseSources(); --i >= 0;)
        {
            MouseInputSource* const source = Desktop::getInstance().getMouseSource(i);

            if (source->isDragging())
            {
                Component* const underMouse = source->getComponentUnderMouse();

                if (underMouse != 0 && (comp == underMouse || comp->isParentOf (underMouse)))
                    return true;
            }
        }

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeViewContentComponent);
};

//==============================================================================
class TreeView::TreeViewport  : public Viewport
{
public:
    TreeViewport() throw()  : lastX (-1)    {}
    ~TreeViewport() throw()                 {}

    void updateComponents (const bool triggerResize = false)
    {
        TreeViewContentComponent* const tvc = static_cast <TreeViewContentComponent*> (getViewedComponent());
        if (tvc != 0)
        {
            if (triggerResize)
                tvc->resized();
            else
                tvc->updateComponents();
        }

        repaint();
    }

    void visibleAreaChanged (int x, int, int, int)
    {
        const bool hasScrolledSideways = (x != lastX);
        lastX = x;
        updateComponents (hasScrolledSideways);
    }

private:
    //==============================================================================
    int lastX;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeViewport);
};


//==============================================================================
TreeView::TreeView (const String& componentName)
    : Component (componentName),
      rootItem (0),
      indentSize (24),
      defaultOpenness (false),
      needsRecalculating (true),
      rootItemVisible (true),
      multiSelectEnabled (false),
      openCloseButtonsVisible (true)
{
    addAndMakeVisible (viewport = new TreeViewport());
    viewport->setViewedComponent (new TreeViewContentComponent (*this));
    viewport->setWantsKeyboardFocus (false);
    setWantsKeyboardFocus (true);
}

TreeView::~TreeView()
{
    if (rootItem != 0)
        rootItem->setOwnerView (0);
}

void TreeView::setRootItem (TreeViewItem* const newRootItem)
{
    if (rootItem != newRootItem)
    {
        if (newRootItem != 0)
        {
            jassert (newRootItem->ownerView == 0); // can't use a tree item in more than one tree at once..

            if (newRootItem->ownerView != 0)
                newRootItem->ownerView->setRootItem (0);
        }

        if (rootItem != 0)
            rootItem->setOwnerView (0);

        rootItem = newRootItem;

        if (newRootItem != 0)
            newRootItem->setOwnerView (this);

        needsRecalculating = true;
        handleAsyncUpdate();

        if (rootItem != 0 && (defaultOpenness || ! rootItemVisible))
        {
            rootItem->setOpen (false); // force a re-open
            rootItem->setOpen (true);
        }
    }
}

void TreeView::deleteRootItem()
{
    const ScopedPointer <TreeViewItem> deleter (rootItem);
    setRootItem (0);
}

void TreeView::setRootItemVisible (const bool shouldBeVisible)
{
    rootItemVisible = shouldBeVisible;

    if (rootItem != 0 && (defaultOpenness || ! rootItemVisible))
    {
        rootItem->setOpen (false); // force a re-open
        rootItem->setOpen (true);
    }

    itemsChanged();
}

void TreeView::colourChanged()
{
    setOpaque (findColour (backgroundColourId).isOpaque());
    repaint();
}

void TreeView::setIndentSize (const int newIndentSize)
{
    if (indentSize != newIndentSize)
    {
        indentSize = newIndentSize;
        resized();
    }
}

void TreeView::setDefaultOpenness (const bool isOpenByDefault)
{
    if (defaultOpenness != isOpenByDefault)
    {
        defaultOpenness = isOpenByDefault;
        itemsChanged();
    }
}

void TreeView::setMultiSelectEnabled (const bool canMultiSelect)
{
    multiSelectEnabled = canMultiSelect;
}

void TreeView::setOpenCloseButtonsVisible (const bool shouldBeVisible)
{
    if (openCloseButtonsVisible != shouldBeVisible)
    {
        openCloseButtonsVisible = shouldBeVisible;
        itemsChanged();
    }
}

Viewport* TreeView::getViewport() const throw()
{
    return viewport;
}

//==============================================================================
void TreeView::clearSelectedItems()
{
    if (rootItem != 0)
        rootItem->deselectAllRecursively();
}

int TreeView::getNumSelectedItems (int maximumDepthToSearchTo) const throw()
{
    return (rootItem != 0) ? rootItem->countSelectedItemsRecursively (maximumDepthToSearchTo) : 0;
}

TreeViewItem* TreeView::getSelectedItem (const int index) const throw()
{
    return (rootItem != 0) ? rootItem->getSelectedItemWithIndex (index) : 0;
}

int TreeView::getNumRowsInTree() const
{
    if (rootItem != 0)
        return rootItem->getNumRows() - (rootItemVisible ? 0 : 1);

    return 0;
}

TreeViewItem* TreeView::getItemOnRow (int index) const
{
    if (! rootItemVisible)
        ++index;

    if (rootItem != 0 && index >= 0)
        return rootItem->getItemOnRow (index);

    return 0;
}

TreeViewItem* TreeView::getItemAt (int y) const throw()
{
    TreeViewContentComponent* const tc = static_cast <TreeViewContentComponent*> (viewport->getViewedComponent());
    Rectangle<int> pos;
    return tc->findItemAt (tc->getLocalPoint (this, Point<int> (0, y)).getY(), pos);
}

TreeViewItem* TreeView::findItemFromIdentifierString (const String& identifierString) const
{
    if (rootItem == 0)
        return 0;

    return rootItem->findItemFromIdentifierString (identifierString);
}

//==============================================================================
XmlElement* TreeView::getOpennessState (const bool alsoIncludeScrollPosition) const
{
    XmlElement* e = 0;

    if (rootItem != 0)
    {
        e = rootItem->getOpennessState();

        if (e != 0 && alsoIncludeScrollPosition)
            e->setAttribute ("scrollPos", viewport->getViewPositionY());
    }

    return e;
}

void TreeView::restoreOpennessState (const XmlElement& newState)
{
    if (rootItem != 0)
    {
        rootItem->restoreOpennessState (newState);

        if (newState.hasAttribute ("scrollPos"))
            viewport->setViewPosition (viewport->getViewPositionX(),
                                       newState.getIntAttribute ("scrollPos"));
    }
}

//==============================================================================
void TreeView::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));
}

void TreeView::resized()
{
    viewport->setBounds (getLocalBounds());

    itemsChanged();
    handleAsyncUpdate();
}

void TreeView::enablementChanged()
{
    repaint();
}

void TreeView::moveSelectedRow (int delta)
{
    if (delta == 0)
        return;

    int rowSelected = 0;

    TreeViewItem* const firstSelected = getSelectedItem (0);
    if (firstSelected != 0)
        rowSelected = firstSelected->getRowNumberInTree();

    rowSelected = jlimit (0, getNumRowsInTree() - 1, rowSelected + delta);

    for (;;)
    {
        TreeViewItem* item = getItemOnRow (rowSelected);

        if (item != 0)
        {
            if (! item->canBeSelected())
            {
                // if the row we want to highlight doesn't allow it, try skipping
                // to the next item..
                const int nextRowToTry = jlimit (0, getNumRowsInTree() - 1,
                                                 rowSelected + (delta < 0 ? -1 : 1));

                if (rowSelected != nextRowToTry)
                {
                    rowSelected = nextRowToTry;
                    continue;
                }
                else
                {
                    break;
                }
            }

            item->setSelected (true, true);

            scrollToKeepItemVisible (item);
        }

        break;
    }
}

void TreeView::scrollToKeepItemVisible (TreeViewItem* item)
{
    if (item != 0 && item->ownerView == this)
    {
        handleAsyncUpdate();

        item = item->getDeepestOpenParentItem();

        int y = item->y;
        int viewTop = viewport->getViewPositionY();

        if (y < viewTop)
        {
            viewport->setViewPosition (viewport->getViewPositionX(), y);
        }
        else if (y + item->itemHeight > viewTop + viewport->getViewHeight())
        {
            viewport->setViewPosition (viewport->getViewPositionX(),
                                       (y + item->itemHeight) - viewport->getViewHeight());
        }
    }
}

bool TreeView::keyPressed (const KeyPress& key)
{
    if (key.isKeyCode (KeyPress::upKey))
    {
        moveSelectedRow (-1);
    }
    else if (key.isKeyCode (KeyPress::downKey))
    {
        moveSelectedRow (1);
    }
    else if (key.isKeyCode (KeyPress::pageDownKey) || key.isKeyCode (KeyPress::pageUpKey))
    {
        if (rootItem != 0)
        {
            int rowsOnScreen = getHeight() / jmax (1, rootItem->itemHeight);

            if (key.isKeyCode (KeyPress::pageUpKey))
                rowsOnScreen = -rowsOnScreen;

            moveSelectedRow (rowsOnScreen);
        }
    }
    else if (key.isKeyCode (KeyPress::homeKey))
    {
        moveSelectedRow (-0x3fffffff);
    }
    else if (key.isKeyCode (KeyPress::endKey))
    {
        moveSelectedRow (0x3fffffff);
    }
    else if (key.isKeyCode (KeyPress::returnKey))
    {
        TreeViewItem* const firstSelected = getSelectedItem (0);
        if (firstSelected != 0)
            firstSelected->setOpen (! firstSelected->isOpen());
    }
    else if (key.isKeyCode (KeyPress::leftKey))
    {
        TreeViewItem* const firstSelected = getSelectedItem (0);

        if (firstSelected != 0)
        {
            if (firstSelected->isOpen())
            {
                firstSelected->setOpen (false);
            }
            else
            {
                TreeViewItem* parent = firstSelected->parentItem;

                if ((! rootItemVisible) && parent == rootItem)
                    parent = 0;

                if (parent != 0)
                {
                    parent->setSelected (true, true);
                    scrollToKeepItemVisible (parent);
                }
            }
        }
    }
    else if (key.isKeyCode (KeyPress::rightKey))
    {
        TreeViewItem* const firstSelected = getSelectedItem (0);

        if (firstSelected != 0)
        {
            if (firstSelected->isOpen() || ! firstSelected->mightContainSubItems())
                moveSelectedRow (1);
            else
                firstSelected->setOpen (true);
        }
    }
    else
    {
        return false;
    }

    return true;
}

void TreeView::itemsChanged() throw()
{
    needsRecalculating = true;
    repaint();
    triggerAsyncUpdate();
}

void TreeView::handleAsyncUpdate()
{
    if (needsRecalculating)
    {
        needsRecalculating = false;

        const ScopedLock sl (nodeAlterationLock);

        if (rootItem != 0)
            rootItem->updatePositions (rootItemVisible ? 0 : -rootItem->itemHeight);

        viewport->updateComponents();

        if (rootItem != 0)
        {
            viewport->getViewedComponent()
                ->setSize (jmax (viewport->getMaximumVisibleWidth(), rootItem->totalWidth),
                           rootItem->totalHeight - (rootItemVisible ? 0 : rootItem->itemHeight));
        }
        else
        {
            viewport->getViewedComponent()->setSize (0, 0);
        }
    }
}

//==============================================================================
class TreeView::InsertPointHighlight   : public Component
{
public:
    InsertPointHighlight()
        : lastItem (0)
    {
        setSize (100, 12);
        setAlwaysOnTop (true);
        setInterceptsMouseClicks (false, false);
    }

    void setTargetPosition (TreeViewItem* const item, int insertIndex, const int x, const int y, const int width) throw()
    {
        lastItem = item;
        lastIndex = insertIndex;
        const int offset = getHeight() / 2;
        setBounds (x - offset, y - offset, width - (x - offset), getHeight());
    }

    void paint (Graphics& g)
    {
        Path p;
        const float h = (float) getHeight();
        p.addEllipse (2.0f, 2.0f, h - 4.0f, h - 4.0f);
        p.startNewSubPath (h - 2.0f, h / 2.0f);
        p.lineTo ((float) getWidth(), h / 2.0f);

        g.setColour (findColour (TreeView::dragAndDropIndicatorColourId, true));
        g.strokePath (p, PathStrokeType (2.0f));
    }

    TreeViewItem* lastItem;
    int lastIndex;

private:
    JUCE_DECLARE_NON_COPYABLE (InsertPointHighlight);
};

//==============================================================================
class TreeView::TargetGroupHighlight   : public Component
{
public:
    TargetGroupHighlight()
    {
        setAlwaysOnTop (true);
        setInterceptsMouseClicks (false, false);
    }

    void setTargetPosition (TreeViewItem* const item) throw()
    {
        Rectangle<int> r (item->getItemPosition (true));
        r.setHeight (item->getItemHeight());
        setBounds (r);
    }

    void paint (Graphics& g)
    {
        g.setColour (findColour (TreeView::dragAndDropIndicatorColourId, true));
        g.drawRoundedRectangle (1.0f, 1.0f, getWidth() - 2.0f, getHeight() - 2.0f, 3.0f, 2.0f);
    }

private:
    JUCE_DECLARE_NON_COPYABLE (TargetGroupHighlight);
};

//==============================================================================
void TreeView::showDragHighlight (TreeViewItem* item, int insertIndex, int x, int y) throw()
{
    beginDragAutoRepeat (100);

    if (dragInsertPointHighlight == 0)
    {
        addAndMakeVisible (dragInsertPointHighlight = new InsertPointHighlight());
        addAndMakeVisible (dragTargetGroupHighlight = new TargetGroupHighlight());
    }

    dragInsertPointHighlight->setTargetPosition (item, insertIndex, x, y, viewport->getViewWidth());
    dragTargetGroupHighlight->setTargetPosition (item);
}

void TreeView::hideDragHighlight() throw()
{
    dragInsertPointHighlight = 0;
    dragTargetGroupHighlight = 0;
}

TreeViewItem* TreeView::getInsertPosition (int& x, int& y, int& insertIndex,
                                           const StringArray& files, const String& sourceDescription,
                                           Component* sourceComponent) const throw()
{
    insertIndex = 0;
    TreeViewItem* item = getItemAt (y);

    if (item == 0)
        return 0;

    Rectangle<int> itemPos (item->getItemPosition (true));
    insertIndex = item->getIndexInParent();
    const int oldY = y;
    y = itemPos.getY();

    if (item->getNumSubItems() == 0 || ! item->isOpen())
    {
        if (files.size() > 0 ? item->isInterestedInFileDrag (files)
                             : item->isInterestedInDragSource (sourceDescription, sourceComponent))
        {
            // Check if we're trying to drag into an empty group item..
            if (oldY > itemPos.getY() + itemPos.getHeight() / 4
                 && oldY < itemPos.getBottom() - itemPos.getHeight() / 4)
            {
                insertIndex = 0;
                x = itemPos.getX() + getIndentSize();
                y = itemPos.getBottom();
                return item;
            }
        }
    }

    if (oldY > itemPos.getCentreY())
    {
        y += item->getItemHeight();

        while (item->isLastOfSiblings() && item->parentItem != 0
                && item->parentItem->parentItem != 0)
        {
            if (x > itemPos.getX())
                break;

            item = item->parentItem;
            itemPos = item->getItemPosition (true);
            insertIndex = item->getIndexInParent();
        }

        ++insertIndex;
    }

    x = itemPos.getX();
    return item->parentItem;
}

void TreeView::handleDrag (const StringArray& files, const String& sourceDescription, Component* sourceComponent, int x, int y)
{
    const bool scrolled = viewport->autoScroll (x, y, 20, 10);

    int insertIndex;
    TreeViewItem* const item = getInsertPosition (x, y, insertIndex, files, sourceDescription, sourceComponent);

    if (item != 0)
    {
        if (scrolled || dragInsertPointHighlight == 0
             || dragInsertPointHighlight->lastItem != item
             || dragInsertPointHighlight->lastIndex != insertIndex)
        {
            if (files.size() > 0 ? item->isInterestedInFileDrag (files)
                                 : item->isInterestedInDragSource (sourceDescription, sourceComponent))
                showDragHighlight (item, insertIndex, x, y);
            else
                hideDragHighlight();
        }
    }
    else
    {
        hideDragHighlight();
    }
}

void TreeView::handleDrop (const StringArray& files, const String& sourceDescription, Component* sourceComponent, int x, int y)
{
    hideDragHighlight();

    int insertIndex;
    TreeViewItem* const item = getInsertPosition (x, y, insertIndex, files, sourceDescription, sourceComponent);

    if (item != 0)
    {
        if (files.size() > 0)
        {
            if (item->isInterestedInFileDrag (files))
                item->filesDropped (files, insertIndex);
        }
        else
        {
            if (item->isInterestedInDragSource (sourceDescription, sourceComponent))
                item->itemDropped (sourceDescription, sourceComponent, insertIndex);
        }
    }
}

//==============================================================================
bool TreeView::isInterestedInFileDrag (const StringArray&)
{
    return true;
}

void TreeView::fileDragEnter (const StringArray& files, int x, int y)
{
    fileDragMove (files, x, y);
}

void TreeView::fileDragMove (const StringArray& files, int x, int y)
{
    handleDrag (files, String::empty, 0, x, y);
}

void TreeView::fileDragExit (const StringArray&)
{
    hideDragHighlight();
}

void TreeView::filesDropped (const StringArray& files, int x, int y)
{
    handleDrop (files, String::empty, 0, x, y);
}

bool TreeView::isInterestedInDragSource (const String& /*sourceDescription*/, Component* /*sourceComponent*/)
{
    return true;
}

void TreeView::itemDragEnter (const String& sourceDescription, Component* sourceComponent, int x, int y)
{
    itemDragMove (sourceDescription, sourceComponent, x, y);
}

void TreeView::itemDragMove (const String& sourceDescription, Component* sourceComponent, int x, int y)
{
    handleDrag (StringArray(), sourceDescription, sourceComponent, x, y);
}

void TreeView::itemDragExit (const String& /*sourceDescription*/, Component* /*sourceComponent*/)
{
    hideDragHighlight();
}

void TreeView::itemDropped (const String& sourceDescription, Component* sourceComponent, int x, int y)
{
    handleDrop (StringArray(), sourceDescription, sourceComponent, x, y);
}

//==============================================================================
enum TreeViewOpenness
{
    opennessDefault = 0,
    opennessClosed = 1,
    opennessOpen = 2
};

TreeViewItem::TreeViewItem()
    : ownerView (0),
      parentItem (0),
      y (0),
      itemHeight (0),
      totalHeight (0),
      selected (false),
      redrawNeeded (true),
      drawLinesInside (true),
      drawsInLeftMargin (false),
      openness (opennessDefault)
{
    static int nextUID = 0;
    uid = nextUID++;
}

TreeViewItem::~TreeViewItem()
{
}

const String TreeViewItem::getUniqueName() const
{
    return String::empty;
}

void TreeViewItem::itemOpennessChanged (bool)
{
}

int TreeViewItem::getNumSubItems() const throw()
{
    return subItems.size();
}

TreeViewItem* TreeViewItem::getSubItem (const int index) const throw()
{
    return subItems [index];
}

void TreeViewItem::clearSubItems()
{
    if (subItems.size() > 0)
    {
        if (ownerView != 0)
        {
            const ScopedLock sl (ownerView->nodeAlterationLock);
            subItems.clear();
            treeHasChanged();
        }
        else
        {
            subItems.clear();
        }
    }
}

void TreeViewItem::addSubItem (TreeViewItem* const newItem, const int insertPosition)
{
    if (newItem != 0)
    {
        newItem->parentItem = this;
        newItem->setOwnerView (ownerView);
        newItem->y = 0;
        newItem->itemHeight = newItem->getItemHeight();
        newItem->totalHeight = 0;
        newItem->itemWidth = newItem->getItemWidth();
        newItem->totalWidth = 0;

        if (ownerView != 0)
        {
            const ScopedLock sl (ownerView->nodeAlterationLock);
            subItems.insert (insertPosition, newItem);
            treeHasChanged();

            if (newItem->isOpen())
                newItem->itemOpennessChanged (true);
        }
        else
        {
            subItems.insert (insertPosition, newItem);

            if (newItem->isOpen())
                newItem->itemOpennessChanged (true);
        }
    }
}

void TreeViewItem::removeSubItem (const int index, const bool deleteItem)
{
    if (ownerView != 0)
    {
        const ScopedLock sl (ownerView->nodeAlterationLock);

        if (isPositiveAndBelow (index, subItems.size()))
        {
            subItems.remove (index, deleteItem);
            treeHasChanged();
        }
    }
    else
    {
        subItems.remove (index, deleteItem);
    }
}

bool TreeViewItem::isOpen() const throw()
{
    if (openness == opennessDefault)
        return ownerView != 0 && ownerView->defaultOpenness;
    else
        return openness == opennessOpen;
}

void TreeViewItem::setOpen (const bool shouldBeOpen)
{
    if (isOpen() != shouldBeOpen)
    {
        openness = shouldBeOpen ? opennessOpen
                                : opennessClosed;
        treeHasChanged();

        itemOpennessChanged (isOpen());
    }
}

bool TreeViewItem::isSelected() const throw()
{
    return selected;
}

void TreeViewItem::deselectAllRecursively()
{
    setSelected (false, false);

    for (int i = 0; i < subItems.size(); ++i)
        subItems.getUnchecked(i)->deselectAllRecursively();
}

void TreeViewItem::setSelected (const bool shouldBeSelected,
                                const bool deselectOtherItemsFirst)
{
    if (shouldBeSelected && ! canBeSelected())
        return;

    if (deselectOtherItemsFirst)
        getTopLevelItem()->deselectAllRecursively();

    if (shouldBeSelected != selected)
    {
        selected = shouldBeSelected;
        if (ownerView != 0)
            ownerView->repaint();

        itemSelectionChanged (shouldBeSelected);
    }
}

void TreeViewItem::paintItem (Graphics&, int, int)
{
}

void TreeViewItem::paintOpenCloseButton (Graphics& g, int width, int height, bool isMouseOver)
{
    ownerView->getLookAndFeel()
       .drawTreeviewPlusMinusBox (g, 0, 0, width, height, ! isOpen(), isMouseOver);
}

void TreeViewItem::itemClicked (const MouseEvent&)
{
}

void TreeViewItem::itemDoubleClicked (const MouseEvent&)
{
    if (mightContainSubItems())
        setOpen (! isOpen());
}

void TreeViewItem::itemSelectionChanged (bool)
{
}

const String TreeViewItem::getTooltip()
{
    return String::empty;
}

const String TreeViewItem::getDragSourceDescription()
{
    return String::empty;
}

bool TreeViewItem::isInterestedInFileDrag (const StringArray&)
{
    return false;
}

void TreeViewItem::filesDropped (const StringArray& /*files*/, int /*insertIndex*/)
{
}

bool TreeViewItem::isInterestedInDragSource (const String& /*sourceDescription*/, Component* /*sourceComponent*/)
{
    return false;
}

void TreeViewItem::itemDropped (const String& /*sourceDescription*/, Component* /*sourceComponent*/, int /*insertIndex*/)
{
}

const Rectangle<int> TreeViewItem::getItemPosition (const bool relativeToTreeViewTopLeft) const throw()
{
    const int indentX = getIndentX();
    int width = itemWidth;

    if (ownerView != 0 && width < 0)
        width = ownerView->viewport->getViewWidth() - indentX;

    Rectangle<int> r (indentX, y, jmax (0, width), totalHeight);

    if (relativeToTreeViewTopLeft)
        r -= ownerView->viewport->getViewPosition();

    return r;
}

void TreeViewItem::treeHasChanged() const throw()
{
    if (ownerView != 0)
        ownerView->itemsChanged();
}

void TreeViewItem::repaintItem() const
{
    if (ownerView != 0 && areAllParentsOpen())
    {
        Rectangle<int> r (getItemPosition (true));
        r.setLeft (0);
        ownerView->viewport->repaint (r);
    }
}

bool TreeViewItem::areAllParentsOpen() const throw()
{
    return parentItem == 0
            || (parentItem->isOpen() && parentItem->areAllParentsOpen());
}

void TreeViewItem::updatePositions (int newY)
{
    y = newY;
    itemHeight = getItemHeight();
    totalHeight = itemHeight;
    itemWidth = getItemWidth();
    totalWidth = jmax (itemWidth, 0) + getIndentX();

    if (isOpen())
    {
        newY += totalHeight;

        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const ti = subItems.getUnchecked(i);

            ti->updatePositions (newY);
            newY += ti->totalHeight;
            totalHeight += ti->totalHeight;
            totalWidth = jmax (totalWidth, ti->totalWidth);
        }
    }
}

TreeViewItem* TreeViewItem::getDeepestOpenParentItem() throw()
{
    TreeViewItem* result = this;
    TreeViewItem* item = this;

    while (item->parentItem != 0)
    {
        item = item->parentItem;

        if (! item->isOpen())
            result = item;
    }

    return result;
}

void TreeViewItem::setOwnerView (TreeView* const newOwner) throw()
{
    ownerView = newOwner;

    for (int i = subItems.size(); --i >= 0;)
        subItems.getUnchecked(i)->setOwnerView (newOwner);
}

int TreeViewItem::getIndentX() const throw()
{
    const int indentWidth = ownerView->getIndentSize();
    int x = ownerView->rootItemVisible ? indentWidth : 0;

    if (! ownerView->openCloseButtonsVisible)
        x -= indentWidth;

    TreeViewItem* p = parentItem;

    while (p != 0)
    {
        x += indentWidth;
        p = p->parentItem;
    }

    return x;
}

void TreeViewItem::setDrawsInLeftMargin (bool canDrawInLeftMargin) throw()
{
    drawsInLeftMargin = canDrawInLeftMargin;
}

void TreeViewItem::paintRecursively (Graphics& g, int width)
{
    jassert (ownerView != 0);
    if (ownerView == 0)
        return;

    const int indent = getIndentX();
    const int itemW = itemWidth < 0 ? width - indent : itemWidth;

    {
        Graphics::ScopedSaveState ss (g);
        g.setOrigin (indent, 0);

        if (g.reduceClipRegion (drawsInLeftMargin ? -indent : 0, 0,
                                drawsInLeftMargin ? itemW + indent : itemW, itemHeight))
            paintItem (g, itemW, itemHeight);
    }

    g.setColour (ownerView->findColour (TreeView::linesColourId));

    const float halfH = itemHeight * 0.5f;
    int depth = 0;
    TreeViewItem* p = parentItem;

    while (p != 0)
    {
        ++depth;
        p = p->parentItem;
    }

    if (! ownerView->rootItemVisible)
        --depth;

    const int indentWidth = ownerView->getIndentSize();

    if (depth >= 0 && ownerView->openCloseButtonsVisible)
    {
        float x = (depth + 0.5f) * indentWidth;

        if (depth >= 0)
        {
            if (parentItem != 0 && parentItem->drawLinesInside)
                g.drawLine (x, 0, x, isLastOfSiblings() ? halfH : (float) itemHeight);


            if ((parentItem != 0 && parentItem->drawLinesInside)
                 || (parentItem == 0 && drawLinesInside))
                g.drawLine (x, halfH, x + indentWidth / 2, halfH);
        }

        p = parentItem;
        int d = depth;

        while (p != 0 && --d >= 0)
        {
            x -= (float) indentWidth;

            if ((p->parentItem == 0 || p->parentItem->drawLinesInside)
                 && ! p->isLastOfSiblings())
            {
                g.drawLine (x, 0, x, (float) itemHeight);
            }

            p = p->parentItem;
        }

        if (mightContainSubItems())
        {
            Graphics::ScopedSaveState ss (g);

            g.setOrigin (depth * indentWidth, 0);
            g.reduceClipRegion (0, 0, indentWidth, itemHeight);

            paintOpenCloseButton (g, indentWidth, itemHeight,
                static_cast <TreeViewContentComponent*> (ownerView->viewport->getViewedComponent())
                    ->isMouseOverButton (this));
        }
    }

    if (isOpen())
    {
        const Rectangle<int> clip (g.getClipBounds());

        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const ti = subItems.getUnchecked(i);

            const int relY = ti->y - y;

            if (relY >= clip.getBottom())
                break;

            if (relY + ti->totalHeight >= clip.getY())
            {
                Graphics::ScopedSaveState ss (g);

                g.setOrigin (0, relY);

                if (g.reduceClipRegion (0, 0, width, ti->totalHeight))
                    ti->paintRecursively (g, width);
            }
        }
    }
}

bool TreeViewItem::isLastOfSiblings() const throw()
{
    return parentItem == 0
            || parentItem->subItems.getLast() == this;
}

int TreeViewItem::getIndexInParent() const throw()
{
    return parentItem == 0 ? 0
                           : parentItem->subItems.indexOf (this);
}

TreeViewItem* TreeViewItem::getTopLevelItem() throw()
{
    return parentItem == 0 ? this
                           : parentItem->getTopLevelItem();
}

int TreeViewItem::getNumRows() const throw()
{
    int num = 1;

    if (isOpen())
    {
        for (int i = subItems.size(); --i >= 0;)
            num += subItems.getUnchecked(i)->getNumRows();
    }

    return num;
}

TreeViewItem* TreeViewItem::getItemOnRow (int index) throw()
{
    if (index == 0)
        return this;

    if (index > 0 && isOpen())
    {
        --index;

        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const item = subItems.getUnchecked(i);

            if (index == 0)
                return item;

            const int numRows = item->getNumRows();

            if (numRows > index)
                return item->getItemOnRow (index);

            index -= numRows;
        }
    }

    return 0;
}

TreeViewItem* TreeViewItem::findItemRecursively (int targetY) throw()
{
    if (isPositiveAndBelow (targetY, totalHeight))
    {
        const int h = itemHeight;

        if (targetY < h)
            return this;

        if (isOpen())
        {
            targetY -= h;

            for (int i = 0; i < subItems.size(); ++i)
            {
                TreeViewItem* const ti = subItems.getUnchecked(i);

                if (targetY < ti->totalHeight)
                    return ti->findItemRecursively (targetY);

                targetY -= ti->totalHeight;
            }
        }
    }

    return 0;
}

int TreeViewItem::countSelectedItemsRecursively (int depth) const throw()
{
    int total = isSelected() ? 1 : 0;

    if (depth != 0)
        for (int i = subItems.size(); --i >= 0;)
            total += subItems.getUnchecked(i)->countSelectedItemsRecursively (depth - 1);

    return total;
}

TreeViewItem* TreeViewItem::getSelectedItemWithIndex (int index) throw()
{
    if (isSelected())
    {
        if (index == 0)
            return this;

        --index;
    }

    if (index >= 0)
    {
        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const item = subItems.getUnchecked(i);

            TreeViewItem* const found = item->getSelectedItemWithIndex (index);

            if (found != 0)
                return found;

            index -= item->countSelectedItemsRecursively (-1);
        }
    }

    return 0;
}

int TreeViewItem::getRowNumberInTree() const throw()
{
    if (parentItem != 0 && ownerView != 0)
    {
        int n = 1 + parentItem->getRowNumberInTree();

        int ourIndex = parentItem->subItems.indexOf (this);
        jassert (ourIndex >= 0);

        while (--ourIndex >= 0)
            n += parentItem->subItems [ourIndex]->getNumRows();

        if (parentItem->parentItem == 0
             && ! ownerView->rootItemVisible)
            --n;

        return n;
    }
    else
    {
        return 0;
    }
}

void TreeViewItem::setLinesDrawnForSubItems (const bool drawLines) throw()
{
    drawLinesInside = drawLines;
}

TreeViewItem* TreeViewItem::getNextVisibleItem (const bool recurse) const throw()
{
    if (recurse && isOpen() && subItems.size() > 0)
        return subItems [0];

    if (parentItem != 0)
    {
        const int nextIndex = parentItem->subItems.indexOf (this) + 1;

        if (nextIndex >= parentItem->subItems.size())
            return parentItem->getNextVisibleItem (false);

        return parentItem->subItems [nextIndex];
    }

    return 0;
}

const String TreeViewItem::getItemIdentifierString() const
{
    String s;

    if (parentItem != 0)
        s = parentItem->getItemIdentifierString();

    return s + "/" + getUniqueName().replaceCharacter ('/', '\\');
}

TreeViewItem* TreeViewItem::findItemFromIdentifierString (const String& identifierString)
{
    const String thisId (getUniqueName());

    if (thisId == identifierString)
        return this;

    if (identifierString.startsWith (thisId + "/"))
    {
        const String remainingPath (identifierString.substring (thisId.length() + 1));

        bool wasOpen = isOpen();
        setOpen (true);

        for (int i = subItems.size(); --i >= 0;)
        {
            TreeViewItem* item = subItems.getUnchecked(i)->findItemFromIdentifierString (remainingPath);

            if (item != 0)
                return item;
        }

        setOpen (wasOpen);
    }

    return 0;
}

void TreeViewItem::restoreOpennessState (const XmlElement& e) throw()
{
    if (e.hasTagName ("CLOSED"))
    {
        setOpen (false);
    }
    else if (e.hasTagName ("OPEN"))
    {
        setOpen (true);

        forEachXmlChildElement (e, n)
        {
            const String id (n->getStringAttribute ("id"));

            for (int i = 0; i < subItems.size(); ++i)
            {
                TreeViewItem* const ti = subItems.getUnchecked(i);

                if (ti->getUniqueName() == id)
                {
                    ti->restoreOpennessState (*n);
                    break;
                }
            }
        }
    }
}

XmlElement* TreeViewItem::getOpennessState() const throw()
{
    const String name (getUniqueName());

    if (name.isNotEmpty())
    {
        XmlElement* e;

        if (isOpen())
        {
            e = new XmlElement ("OPEN");

            for (int i = 0; i < subItems.size(); ++i)
                e->addChildElement (subItems.getUnchecked(i)->getOpennessState());
        }
        else
        {
            e = new XmlElement ("CLOSED");
        }

        e->setAttribute ("id", name);

        return e;
    }
    else
    {
        // trying to save the openness for an element that has no name - this won't
        // work because it needs the names to identify what to open.
        jassertfalse;
    }

    return 0;
}

END_JUCE_NAMESPACE
