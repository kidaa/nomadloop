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

#include "../../jucer_Headers.h"
#include "../../utility/jucer_TickIterator.h"
#include "jucer_EditorCanvas.h"
#include "jucer_EditorPanel.h"


//==============================================================================
class EditorCanvasBase::ResizeFrame    : public EditorCanvasBase::OverlayItemComponent
{
public:
    ResizeFrame (EditorCanvasBase* canvas_, const String& objectId_, const ValueTree& objectState_)
        : OverlayItemComponent (canvas_),
          objectState (objectState_),
          objectId (objectId_),
          borderThickness (4),
          isDragging (false)
    {
        jassert (objectState.isValid());
    }

    ~ResizeFrame()
    {
    }

    void paint (Graphics& g)
    {
        g.setColour (resizableBorderColour);
        g.drawRect (0, 0, getWidth(), getHeight(), borderThickness);
    }

    void mouseEnter (const MouseEvent& e)                       { updateDragZone (e.getPosition()); }
    void mouseExit (const MouseEvent& e)                        { updateDragZone (e.getPosition()); }
    void mouseMove (const MouseEvent& e)                        { updateDragZone (e.getPosition()); }

    void mouseDown (const MouseEvent& e)
    {
        updateDragZone (e.getPosition());

        if (e.mods.isPopupMenu())
        {
            isDragging = false;
            canvas->showPopupMenu (true);
        }
        else
        {
            isDragging = true;
            canvas->beginDrag (e.getEventRelativeTo (getParentComponent()), dragZone);
            canvas->showSizeGuides();
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (isDragging)
        {
            canvas->continueDrag (e.getEventRelativeTo (getParentComponent()));
            autoScrollForMouseEvent (e);
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        if (isDragging)
        {
            canvas->hideSizeGuides();
            canvas->endDrag (e.getEventRelativeTo (getParentComponent()));
            updateDragZone (e.getPosition());
        }
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        canvas->objectDoubleClicked (e, objectState);
    }

    bool hitTest (int x, int y)
    {
        if (ModifierKeys::getCurrentModifiers().isAnyModifierKeyDown())
            return ! getCentreArea().contains (x, y);

        return true;
    }

    bool updatePosition()
    {
        if (! objectState.getParent().isValid())
            return false;

        const Rectangle<int> bounds (canvas->getObjectPosition (objectState));
        setBoundsInTargetSpace (bounds.expanded (borderThickness, borderThickness));

        for (int i = sizeGuides.size(); --i >= 0;)
        {
            sizeGuides.getUnchecked(i)->setVisible (isVisible());
            sizeGuides.getUnchecked(i)->updatePosition (bounds);
        }

        return true;
    }

    const String& getTargetObjectID() const     { return objectId; }


    //==============================================================================
    class SizeGuideComponent   : public OverlayItemComponent,
                                 public ComponentListener
    {
    public:
        enum Type    { left, right, top, bottom };

        //==============================================================================
        SizeGuideComponent (EditorCanvasBase* canvas_, const ValueTree& state_, Type type_)
            : OverlayItemComponent (canvas_), state (state_), type (type_)
        {
            setAlwaysOnTop (true);
            canvas->addAndMakeVisible (this);
            setInterceptsMouseClicks (false, false);
        }

        //==============================================================================
        void paint (Graphics& g)
        {
            const float dashes[] = { 4.0f, 3.0f };

            g.setColour (resizableBorderColour);
            g.drawDashedLine (0.5f, 0.5f, getWidth() - 0.5f, getHeight() - 0.5f, dashes, 2, 1.0f);
        }

        //==============================================================================
        void updatePosition (const Rectangle<int>& bounds)
        {
            RelativeRectangle coords (canvas->getObjectCoords (state));
            RelativeCoordinate coord;
            Rectangle<int> r;

            switch (type)
            {
                case left:    coord = coords.left;   r.setBounds (bounds.getX(), 0, 1, bounds.getY()); break;
                case right:   coord = coords.right;  r.setBounds (bounds.getRight(), 0, 1, bounds.getY()); break;
                case top:     coord = coords.top;    r.setBounds (0, bounds.getY(), bounds.getX(), 1); break;
                case bottom:  coord = coords.bottom; r.setBounds (0, bounds.getBottom(), bounds.getX(), 1); break;
                default:      jassertfalse; break;
            }

            setBoundsInTargetSpace (r);
            label.update (getParentComponent(), coord.toString(), resizableBorderColour.withAlpha (0.9f), getX(), getY(), type != left, type != top);
        }

    private:
        ValueTree state;
        Type type;
        FloatingLabelComponent label;
    };

    void showSizeGuides()
    {
        if (sizeGuides.size() == 0 && canvas->hasSizeGuides())
        {
            sizeGuides.add (new SizeGuideComponent (canvas, objectState, SizeGuideComponent::left));
            sizeGuides.add (new SizeGuideComponent (canvas, objectState, SizeGuideComponent::right));
            sizeGuides.add (new SizeGuideComponent (canvas, objectState, SizeGuideComponent::top));
            sizeGuides.add (new SizeGuideComponent (canvas, objectState, SizeGuideComponent::bottom));
        }
    }

    void hideSizeGuides()
    {
        sizeGuides.clear();
    }

private:
    ValueTree objectState;
    String objectId;
    ResizableBorderComponent::Zone dragZone;
    const int borderThickness;
    OwnedArray <SizeGuideComponent> sizeGuides;
    bool isDragging;

    const Rectangle<int> getCentreArea() const
    {
        return getLocalBounds().reduced (borderThickness, borderThickness);
    }

    void updateDragZone (const Point<int>& p)
    {
        ResizableBorderComponent::Zone newZone
            = ResizableBorderComponent::Zone::fromPositionOnBorder (getLocalBounds(),
                                                                    BorderSize (borderThickness), p);

        if (dragZone != newZone)
        {
            dragZone = newZone;
            setMouseCursor (newZone.getMouseCursor());
        }
    }
};

//==============================================================================
class EditorCanvasBase::MarkerComponent   : public EditorCanvasBase::OverlayItemComponent
{
public:
    MarkerComponent (EditorCanvasBase* canvas_, const ValueTree& marker_, bool isX_, int headSize_)
        : OverlayItemComponent (canvas_), marker (marker_), isX (isX_), headSize (headSize_ - 2),
          dragStartPos (0), isDragging (false)
    {
    }

    ~MarkerComponent()
    {
    }

    void paint (Graphics& g)
    {
        g.setColour (Colours::lightgreen.withAlpha (isMouseOverOrDragging() ? 0.8f : 0.4f));
        g.fillPath (path);
    }

    void updatePosition()
    {
        RelativeCoordinate coord (getMarkerList().getCoordinate (marker));
        const int pos = roundToInt (coord.resolve (&getMarkerList()));
        const int width = 8;

        if (isX)
            setBoundsInTargetSpace (Rectangle<int> (pos - width, -headSize, width * 2, getParentHeight()));
        else
            setBoundsInTargetSpace (Rectangle<int> (-headSize, pos - width, getParentWidth(), width * 2));

        labelText = "name: " + getMarkerList().getName (marker) + "\nposition: " + coord.toString();
        updateLabel();
    }

    void updateLabel()
    {
        if (isMouseOverOrDragging() && (getWidth() > 1 || getHeight() > 1))
            label.update (getParentComponent(), labelText, Colours::darkgreen,
                          isX ? getBounds().getCentreX() : getX() + headSize,
                          isX ? getY() + headSize : getBounds().getCentreY(), true, true);
        else
            label.remove();
    }

    bool hitTest (int x, int y)
    {
        return (isX ? y : x) < headSize;
    }

    void resized()
    {
        const float lineThickness = 1.0f;
        path.clear();

        if (isX)
        {
            const float centre = getWidth() / 2 + 0.5f;
            path.addLineSegment (centre, 2.0f, centre, getHeight() + 1.0f, lineThickness);
            path.addTriangle (1.0f, 0.0f, centre * 2.0f - 1.0f, 0.0f, centre, headSize + 1.0f);
        }
        else
        {
            const float centre = getHeight() / 2 + 0.5f;
            path.addLineSegment (2.0f, centre, getWidth() + 1.0f, centre, lineThickness);
            path.addTriangle (0.0f, centre * 2.0f - 1.0f, 0.0f, 1.0f, headSize + 1.0f, centre);
        }

        updateLabel();
    }

    void mouseDown (const MouseEvent& e)
    {
        mouseDownPos = e.getEventRelativeTo (getParentComponent()).getMouseDownPosition();
        toFront (false);
        updateLabel();

        canvas->getSelection().selectOnly (MarkerListBase::getId (marker));

        if (e.mods.isPopupMenu())
        {
            isDragging = false;
        }
        else
        {
            isDragging = true;
            canvas->getUndoManager().beginNewTransaction();

            RelativeCoordinate coord (getMarkerList().getCoordinate (marker));
            dragStartPos = coord.resolve (&getMarkerList());
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (isDragging)
        {
            autoScrollForMouseEvent (e);
            const MouseEvent e2 (e.getEventRelativeTo (getParentComponent()));

            canvas->getUndoManager().undoCurrentTransactionOnly();

            Rectangle<int> axis;
            if (isX)
                axis.setBounds (0, 0, getParentWidth(), headSize);
            else
                axis.setBounds (0, 0, headSize, getParentHeight());

            if (axis.expanded (30, 30).contains (e.x, e.y))
            {
                RelativeCoordinate coord (getMarkerList().getCoordinate (marker));

                // (can't use getDistanceFromDragStart() because it doesn't take into account auto-scrolling)
                coord.moveToAbsolute (jmax (0, roundToInt (dragStartPos + (isX ? e2.x - mouseDownPos.getX()
                                                                               : e2.y - mouseDownPos.getY()))),
                                      &getMarkerList());
                getMarkerList().setCoordinate (marker, coord);
            }
            else
            {
                getMarkerList().deleteMarker (marker);
            }
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        canvas->getUndoManager().beginNewTransaction();
        updateLabel();
    }

    void mouseEnter (const MouseEvent& e)
    {
        updateLabel();
        repaint();
    }

    void mouseExit (const MouseEvent& e)
    {
        updateLabel();
        repaint();
    }

    MarkerListBase& getMarkerList()      { return canvas->getMarkerList (isX); }

    ValueTree marker;
    const bool isX;

private:
    const int headSize;
    Path path;
    double dragStartPos;
    bool isDragging;
    FloatingLabelComponent label;
    String labelText;
    Point<int> mouseDownPos;
};


//==============================================================================
class EditorCanvasBase::OverlayComponent  : public Component,
                                            public LassoSource <SelectedItems::ItemType>,
                                            public ChangeListener
{
public:
    OverlayComponent (EditorCanvasBase* canvas_)
        : canvas (canvas_)
    {
        setWantsKeyboardFocus (true);
        getSelection().addChangeListener (this);
    }

    ~OverlayComponent()
    {
        getSelection().removeChangeListener (this);
        lasso = 0;
        resizers.clear();
        deleteAllChildren();
    }

    //==============================================================================
    void mouseDown (const MouseEvent& e)
    {
        lasso = 0;
        mouseDownCompUID = SelectedItems::ItemType();
        isDraggingClickedComp = false;

        const MouseEvent e2 (e.getEventRelativeTo (canvas->getComponentHolder()));
        const SelectedItems::ItemType underMouse (canvas->findObjectIdAt (canvas->screenSpaceToObjectSpace (e2.getPosition())));

        if (e.mods.isPopupMenu())
        {
            if (underMouse.isNotEmpty() && ! getSelection().isSelected (underMouse))
                getSelection().selectOnly (underMouse);

            canvas->showPopupMenu (underMouse.isNotEmpty());
        }
        else
        {
            if (underMouse.isEmpty() || e.mods.isAltDown())
            {
                canvas->deselectNonDraggableObjects();
                addAndMakeVisible (lasso = new LassoComponent <SelectedItems::ItemType>());
                lasso->beginLasso (e, this);
            }
            else
            {
                mouseDownCompUID = underMouse;
                canvas->deselectNonDraggableObjects();
                mouseDownResult = getSelection().addToSelectionOnMouseDown (mouseDownCompUID, e.mods);

                updateResizeFrames();
                hideSizeGuides();
                showSizeGuides();
            }
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (lasso != 0)
        {
            lasso->dragLasso (e);
        }
        else if (mouseDownCompUID.isNotEmpty() && (! e.mouseWasClicked()) && (! e.mods.isPopupMenu()))
        {
            if (! isDraggingClickedComp)
            {
                isDraggingClickedComp = true;
                getSelection().addToSelectionOnMouseUp (mouseDownCompUID, e.mods, true, mouseDownResult);
                canvas->beginDrag (e, ResizableBorderComponent::Zone (ResizableBorderComponent::Zone::centre));
            }

            canvas->continueDrag (e);
            showSizeGuides();
        }

        autoScrollForMouseEvent (e);
    }

    void mouseUp (const MouseEvent& e)
    {
        hideSizeGuides();

        if (lasso != 0)
        {
            lasso->endLasso();
            lasso = 0;

            if (e.mouseWasClicked())
                getSelection().deselectAll();
        }
        else if (! e.mods.isPopupMenu())
        {
            if (! isDraggingClickedComp)
                getSelection().addToSelectionOnMouseUp (mouseDownCompUID, e.mods, ! e.mouseWasClicked(), mouseDownResult);
        }

        canvas->endDrag (e);
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        const BorderSize& border = canvas->border;
        const Rectangle<int> xAxis (border.getLeft(), 0, getWidth() - border.getLeftAndRight(), border.getTop());
        const Rectangle<int> yAxis (0, border.getTop(), border.getLeft(), getHeight() - border.getTopAndBottom());

        if (xAxis.contains (e.x, e.y))
        {
            canvas->getMarkerList (true).createMarker ("Marker", e.x - xAxis.getX());
        }
        else if (yAxis.contains (e.x, e.y))
        {
            canvas->getMarkerList (false).createMarker ("Marker", e.y - yAxis.getY());
        }
        else
        {
            const MouseEvent e2 (e.getEventRelativeTo (canvas->getComponentHolder()));
            const SelectedItems::ItemType underMouse (canvas->findObjectIdAt (canvas->screenSpaceToObjectSpace (e2.getPosition())));

            if (underMouse.isNotEmpty())
            {
                const ValueTree state (canvas->getObjectState (underMouse));
                canvas->objectDoubleClicked (e2, state);
            }
        }
    }

    void findLassoItemsInArea (Array <SelectedItems::ItemType>& itemsFound, const Rectangle<int>& area)
    {
        const Rectangle<int> sourceArea (area + relativePositionToOtherComponent (canvas->getComponentHolder(), Point<int>()));
        canvas->findLassoItemsInArea (itemsFound, canvas->screenSpaceToObjectSpace (sourceArea));
    }

    SelectedItems& getSelection()               { return canvas->getSelection(); }
    SelectedItems& getLassoSelection()          { return getSelection(); }

    void resized()
    {
        updateMarkers();
        updateResizeFrames();
    }

    void changeListenerCallback (void*)
    {
        updateResizeFrames();
    }

    void modifierKeysChanged (const ModifierKeys&)
    {
        Desktop::getInstance().getMainMouseSource().triggerFakeMove();
    }

    void showSizeGuides()
    {
        if (canvas->hasSizeGuides())
        {
            for (int i = getNumChildComponents(); --i >= 0;)
            {
                ResizeFrame* resizer = dynamic_cast <ResizeFrame*> (getChildComponent(i));
                if (resizer != 0)
                    resizer->showSizeGuides();
            }
        }
    }

    void hideSizeGuides()
    {
        if (canvas->hasSizeGuides())
        {
            for (int i = getNumChildComponents(); --i >= 0;)
            {
                ResizeFrame* resizer = dynamic_cast <ResizeFrame*> (getChildComponent(i));
                if (resizer != 0)
                    resizer->hideSizeGuides();
            }
        }
    }

    void updateResizeFrames()
    {
        SelectedItems& selection = getSelection();
        StringArray requiredIds;
        const int num = selection.getNumSelected();

        int i;
        for (i = 0; i < num; ++i)
            requiredIds.add (selection.getSelectedItem(i));

        for (i = resizers.size(); --i >= 0;)
        {
            ResizeFrame* resizer = resizers.getUnchecked(i);
            const int index = requiredIds.indexOf (resizer->getTargetObjectID());

            if (index >= 0)
            {
                if (resizer->updatePosition())
                {
                    requiredIds.remove (index);
                }
                else
                {
                    resizers.remove (i);
                    canvas->getSelection().deselect (requiredIds[i]);
                }
            }
            else
            {
                resizers.remove (i);
            }
        }

        for (i = requiredIds.size(); --i >= 0;)
        {
            const ValueTree state (canvas->getObjectState (requiredIds[i]));

            if (state.isValid()) // (the id may be a marker)
            {
                ResizeFrame* frame = new ResizeFrame (canvas, requiredIds[i], state);
                resizers.add (frame);
                addAndMakeVisible (frame);
                frame->updatePosition();
            }
        }
    }

    void update()
    {
        updateResizeFrames();
        updateMarkers();
    }

private:
    //==============================================================================
    EditorCanvasBase* canvas;
    ScopedPointer <LassoComponent <SelectedItems::ItemType> > lasso;
    bool mouseDownResult, isDraggingClickedComp;
    SelectedItems::ItemType mouseDownCompUID;
    OwnedArray <ResizeFrame> resizers;

    void updateMarkers (bool isX)
    {
        Array<ValueTree> requiredMarkers;

        MarkerListBase& markerList = canvas->getMarkerList (isX);
        const int num = markerList.size();
        int i;
        for (i = 0; i < num; ++i)
            requiredMarkers.add (markerList.getMarker (i));

        for (i = getNumChildComponents(); --i >= 0;)
        {
            MarkerComponent* marker = dynamic_cast <MarkerComponent*> (getChildComponent(i));

            if (marker != 0 && marker->isX == isX)
            {
                if (requiredMarkers.contains (marker->marker))
                {
                    marker->setVisible (true);
                    marker->updatePosition();
                    requiredMarkers.removeValue (marker->marker);
                }
                else
                {
                    if (marker->isMouseButtonDown())
                        marker->setBounds (-1, -1, 1, 1);
                    else
                        delete marker;
                }
            }
        }

        for (i = requiredMarkers.size(); --i >= 0;)
        {
            MarkerComponent* marker = new MarkerComponent (canvas, requiredMarkers.getReference(i),
                                                           isX, isX ? canvas->border.getTop()
                                                                    : canvas->border.getLeft());
            addAndMakeVisible (marker);
            marker->updatePosition();
        }
    }

    void updateMarkers()
    {
        updateMarkers (true);
        updateMarkers (false);
    }
};

//==============================================================================
class EditorCanvasBase::DocumentResizeFrame    : public Component
{
public:
    DocumentResizeFrame (EditorCanvasBase* canvas_)
        : canvas (canvas_), resizerThickness (4)
    {
    }

    ~DocumentResizeFrame()
    {
    }

    void paint (Graphics& g)
    {
        const Rectangle<int> content (getContentArea());

        g.setColour (Colour::greyLevel (0.1f).withAlpha (0.3f));
        g.drawRect (content.expanded (1, 1), 1);

        const int bottomGap = getHeight() - content.getBottom();
        g.setFont (bottomGap - 5.0f);

        g.setColour (Colour::greyLevel (0.9f));
        g.drawText (String (content.getWidth()) + " x " + String (content.getHeight()),
                    0, 0, jmax (content.getRight(), jmin (60, getWidth())), getHeight(), Justification::bottomRight, false);
    }

    void mouseMove (const MouseEvent& e)
    {
        updateDragZone (e.getPosition());
    }

    void mouseDown (const MouseEvent& e)
    {
        updateDragZone (e.getPosition());
        dragStartBounds = canvas->getCanvasBounds();
        canvas->showSizeGuides();
    }

    void mouseDrag (const MouseEvent& e)
    {
        Rectangle<int> newBounds (dragStartBounds);

        if (dragZone.isDraggingRightEdge())
            newBounds.setWidth (jmax (1, newBounds.getWidth() + e.getDistanceFromDragStartX()));

        if (dragZone.isDraggingBottomEdge())
            newBounds.setHeight (jmax (1, newBounds.getHeight() + e.getDistanceFromDragStartY()));

        canvas->setCanvasBounds (newBounds);
    }

    void mouseUp (const MouseEvent& e)
    {
        canvas->hideSizeGuides();
        updateDragZone (e.getPosition());
    }

    void updateDragZone (const Point<int>& p)
    {
        ResizableBorderComponent::Zone newZone
            = ResizableBorderComponent::Zone::fromPositionOnBorder (getContentArea().expanded (resizerThickness, resizerThickness),
                                                                    BorderSize (0, 0, resizerThickness, resizerThickness), p);

        if (dragZone != newZone)
        {
            dragZone = newZone;
            setMouseCursor (newZone.getMouseCursor());
        }
    }

    bool hitTest (int x, int y)
    {
        if (! canvas->canResizeCanvas())
            return false;

        const Rectangle<int> content (getContentArea());

        return (x >= content.getRight() || y >= content.getBottom())
                 && (! content.contains (x, y))
                 && content.expanded (resizerThickness, resizerThickness).contains (x, y);
    }

private:
    EditorCanvasBase* canvas;
    ResizableBorderComponent::Zone dragZone;
    Rectangle<int> dragStartBounds;
    const int resizerThickness;

    const Rectangle<int> getContentArea() const     { return canvas->getContentArea(); }
};


//==============================================================================
EditorCanvasBase::EditorCanvasBase()
    : border (14),
      scaleFactor (1.0)
{
    //setOpaque (true);
}

EditorCanvasBase::~EditorCanvasBase()
{
    jassert (overlay == 0);
}

void EditorCanvasBase::initialise()
{
    addAndMakeVisible (componentHolder = createComponentHolder());
    addAndMakeVisible (overlay = new OverlayComponent (this));
    overlay->addAndMakeVisible (resizeFrame = new DocumentResizeFrame (this));

    handleAsyncUpdate();
}

void EditorCanvasBase::shutdown()
{
    dragger = 0;
    deleteAndZero (overlay);
    deleteAllChildren();
}

EditorPanelBase* EditorCanvasBase::getPanel() const
{
    return findParentComponentOfClass ((EditorPanelBase*) 0);
}

const Point<int> EditorCanvasBase::screenSpaceToObjectSpace (const Point<int>& p) const
{
    return p - origin;
}

const Point<int> EditorCanvasBase::objectSpaceToScreenSpace (const Point<int>& p) const
{
    return p + origin;
}

const Rectangle<int> EditorCanvasBase::screenSpaceToObjectSpace (const Rectangle<int>& r) const
{
    return r - origin;
}

const Rectangle<int> EditorCanvasBase::objectSpaceToScreenSpace (const Rectangle<int>& r) const
{
    return r + origin;
}

//==============================================================================
void EditorCanvasBase::paint (Graphics& g)
{
    g.setFont (border.getTop() - 5.0f);
    g.setColour (Colour::greyLevel (0.9f));

    //g.drawHorizontalLine (border.getTop() - 1, 2.0f, (float) getWidth() - border.getRight());
    //g.drawVerticalLine (border.getLeft() - 1, 2.0f, (float) getHeight() - border.getBottom());

    drawXAxis (g, Rectangle<int> (border.getLeft(), 0, componentHolder->getWidth(), border.getTop()));
    drawYAxis (g, Rectangle<int> (0, border.getTop(), border.getLeft(), componentHolder->getHeight()));
}

void EditorCanvasBase::drawXAxis (Graphics& g, const Rectangle<int>& r)
{
    TickIterator ticks (-origin.getX(), r.getWidth(), 1.0, 10, 50);
    float pos, tickLength;
    String label;

    while (ticks.getNextTick (pos, tickLength, label))
    {
        if (pos > 0)
        {
            g.drawVerticalLine (r.getX() + (int) pos, r.getBottom() - tickLength * r.getHeight(), (float) r.getBottom());
            g.drawSingleLineText (label, r.getX() + (int) pos + 2, (int) r.getBottom() - 6);
        }
    }
}

void EditorCanvasBase::drawYAxis (Graphics& g, const Rectangle<int>& r)
{
    TickIterator ticks (-origin.getY(), r.getHeight(), 1.0, 10, 80);
    float pos, tickLength;
    String label;

    while (ticks.getNextTick (pos, tickLength, label))
    {
        if (pos > 0)
        {
            g.drawHorizontalLine (r.getY() + (int) pos, r.getRight() - tickLength * r.getWidth(), (float) r.getRight());
            g.drawTextAsPath (label, AffineTransform::rotation (float_Pi / -2.0f)
                                                     .translated (r.getRight() - 6.0f, r.getY() + pos - 2.0f));
        }
    }
}

const Rectangle<int> EditorCanvasBase::getContentArea() const
{
    return border.subtractedFrom (getLocalBounds());
}

//==============================================================================
void EditorCanvasBase::handleAsyncUpdate()
{
    documentChanged();

    const Rectangle<int> canvasBounds (getCanvasBounds());

    const Point<int> newOrigin (jmax (0, -canvasBounds.getX()), jmax (0, -canvasBounds.getY()));
    if (origin != newOrigin)
    {
        repaint();
        origin = newOrigin;
    }

    setSize (jmax (canvasBounds.getWidth(), canvasBounds.getRight()) + border.getLeftAndRight(),
             jmax (canvasBounds.getHeight(), canvasBounds.getBottom()) + border.getTopAndBottom());

    overlay->update();
}

void EditorCanvasBase::resized()
{
    componentHolder->setBounds (getContentArea());
    overlay->setBounds (getLocalBounds());
    resizeFrame->setBounds (getLocalBounds());
    overlay->update();
}

//==============================================================================
void EditorCanvasBase::showSizeGuides()   { overlay->showSizeGuides(); }
void EditorCanvasBase::hideSizeGuides()   { overlay->hideSizeGuides(); }


//==============================================================================
void EditorCanvasBase::beginDrag (const MouseEvent& e, const ResizableBorderComponent::Zone& zone)
{
    dragger = createDragOperation (e, overlay, zone);
}

void EditorCanvasBase::continueDrag (const MouseEvent& e)
{
    if (dragger != 0)
        dragger->drag (e);
}

void EditorCanvasBase::endDrag (const MouseEvent& e)
{
    if (dragger != 0)
    {
        dragger->drag (e);
        dragger = 0;
    }
}

//==============================================================================
EditorCanvasBase::OverlayItemComponent::OverlayItemComponent (EditorCanvasBase* canvas_)
    : canvas (canvas_)
{
}

EditorCanvasBase::OverlayItemComponent::~OverlayItemComponent()
{
}

void EditorCanvasBase::OverlayItemComponent::setBoundsInTargetSpace (const Rectangle<int>& r)
{
    setBounds (canvas->objectSpaceToScreenSpace (r)
                + canvas->getComponentHolder()->relativePositionToOtherComponent (getParentComponent(), Point<int>()));
}
