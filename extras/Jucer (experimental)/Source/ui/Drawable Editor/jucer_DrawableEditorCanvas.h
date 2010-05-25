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

#ifndef __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__
#define __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__

#include "jucer_DrawableEditor.h"
#include "../../model/Drawable/jucer_DrawableTypeHandler.h"


//==============================================================================
class DrawableEditorCanvas  : public EditorCanvasBase,
                              public Timer
{
public:
    DrawableEditorCanvas (DrawableEditor& editor_)
        : editor (editor_)
    {
        initialise();
        getDocument().getRoot().addListener (this);
    }

    ~DrawableEditorCanvas()
    {
        getDocument().getRoot().removeListener (this);
        shutdown();
    }

    Component* createComponentHolder()
    {
        return new DrawableComponent (this);
    }

    void documentChanged()
    {
        DrawableDocument& doc = getDocument();

        if (drawable == 0)
        {
            Drawable* newDrawable = Drawable::createFromValueTree (doc.getRootDrawableNode().getState(), &doc);
            drawable = dynamic_cast <DrawableComposite*> (newDrawable);
            jassert (drawable != 0);
            getComponentHolder()->repaint();
        }
        else
        {
            const Rectangle<float> damage (drawable->refreshFromValueTree (doc.getRootDrawableNode().getState(), &doc));
            getComponentHolder()->repaint (damage.getSmallestIntegerContainer());
        }

        startTimer (500);
    }

    const Rectangle<int> getCanvasBounds()
    {
        return drawable->getBounds().getSmallestIntegerContainer();
    }

    void setCanvasBounds (const Rectangle<int>& newBounds)  {}
    bool canResizeCanvas() const                            { return false; }

    MarkerListBase& getMarkerList (bool isX)
    {
        return getDocument().getMarkerList (isX);
    }

    const SelectedItems::ItemType findObjectIdAt (const Point<int>& position)
    {
        if (drawable != 0)
        {
            for (int i = drawable->getNumDrawables(); --i >= 0;)
            {
                Drawable* d = drawable->getDrawable (i);

                if (d->hitTest ((float) position.getX(), (float) position.getY()))
                    return d->getName();
            }
        }

        return String::empty;
    }

    void showPopupMenu (bool isClickOnSelectedObject)
    {
        PopupMenu m;

        if (isClickOnSelectedObject)
        {
            m.addCommandItem (commandManager, CommandIDs::toFront);
            m.addCommandItem (commandManager, CommandIDs::toBack);
            m.addSeparator();
            m.addCommandItem (commandManager, StandardApplicationCommandIDs::del);
            const int r = m.show();
            (void) r;
        }
        else
        {
            getDocument().addNewItemMenuItems (m);
            const int r = m.show();
            getDocument().performNewItemMenuItem (r);
        }
    }

    void objectDoubleClicked (const MouseEvent& e, const ValueTree& state)
    {
    }

    bool hasSizeGuides() const  { return false; }

    const ValueTree getObjectState (const String& objectId)
    {
        return getDocument().findDrawableState (objectId, false);
    }

    const Rectangle<float> getObjectPositionFloat (const ValueTree& state)
    {
        if (drawable != 0)
        {
            Drawable* d = drawable->getDrawableWithName (Drawable::ValueTreeWrapperBase (state).getID());

            if (d != 0)
                return d->getBounds();
        }

        return Rectangle<float>();
    }

    bool setObjectPositionFloat (const ValueTree& state, const Rectangle<float>& newPos)
    {
        if (drawable != 0)
        {
            Drawable* d = drawable->getDrawableWithName (Drawable::ValueTreeWrapperBase (state).getID());

            if (d != 0)
            {
                d->refreshFromValueTree (state, &getDocument());
                DrawableTypeInstance di (getDocument(), state);
                return di.setBounds (d, newPos);
            }
        }

        return false;
    }

    const Rectangle<int> getObjectPosition (const ValueTree& state)
    {
        return getObjectPositionFloat (state).getSmallestIntegerContainer();
    }

    RelativeRectangle getObjectCoords (const ValueTree& state)
    {
        return RelativeRectangle();
    }

    SelectedItems& getSelection()
    {
        return editor.getSelection();
    }

    void deselectNonDraggableObjects()
    {
    }

    void findLassoItemsInArea (Array <SelectedItems::ItemType>& itemsFound, const Rectangle<int>& area)
    {
        const Rectangle<float> floatArea (area.toFloat());

        if (drawable != 0)
        {
            for (int i = drawable->getNumDrawables(); --i >= 0;)
            {
                Drawable* d = drawable->getDrawable (i);

                if (d->getBounds().intersects (floatArea))
                    itemsFound.add (d->getName());
            }
        }
    }

    //==============================================================================
    class DragOperation  : public EditorDragOperation
    {
    public:
        DragOperation (DrawableEditorCanvas* canvas_,
                       const MouseEvent& e,
                       Component* snapGuideParentComp_,
                       const ResizableBorderComponent::Zone& zone_)
            : EditorDragOperation (canvas_, e, snapGuideParentComp_, zone_)
        {
        }

        ~DragOperation()
        {
            getUndoManager().beginNewTransaction();
        }

    protected:
        DrawableDocument& getDocument() throw()                 { return static_cast <DrawableEditorCanvas*> (canvas)->getDocument(); }

        void getSnapPointsX (Array<float>& points, bool /*includeCentre*/)  { points.add (0.0f); }
        void getSnapPointsY (Array<float>& points, bool /*includeCentre*/)  { points.add (0.0f); }

        UndoManager& getUndoManager()                           { return *getDocument().getUndoManager(); }

        const Rectangle<float> getObjectPosition (const ValueTree& state)
        {
            return static_cast <DrawableEditorCanvas*> (canvas)->getObjectPositionFloat (state);
        }

        bool setObjectPosition (ValueTree& state, const Rectangle<float>& newBounds)
        {
            return static_cast <DrawableEditorCanvas*> (canvas)->setObjectPositionFloat (state, newBounds);
        }

        float getMarkerPosition (const ValueTree& marker, bool isX)
        {
            return 0;
        }
    };

    DragOperation* createDragOperation (const MouseEvent& e, Component* snapGuideParentComponent,
                                        const ResizableBorderComponent::Zone& zone)
    {
        DragOperation* d = new DragOperation (this, e, snapGuideParentComponent, zone);

        Array<ValueTree> selected, unselected;

        DrawableComposite::ValueTreeWrapper mainGroup (getDocument().getRootDrawableNode());

        for (int i = mainGroup.getNumDrawables(); --i >= 0;)
        {
            const ValueTree v (mainGroup.getDrawableState (i));

            if (editor.getSelection().isSelected (v[Drawable::ValueTreeWrapperBase::idProperty]))
                selected.add (v);
            else
                unselected.add (v);
        }

        d->initialise (selected, unselected);
        return d;
    }

    UndoManager& getUndoManager()
    {
        return *getDocument().getUndoManager();
    }

    DrawableEditor& getEditor() throw()                         { return editor; }
    DrawableDocument& getDocument() throw()                     { return editor.getDocument(); }

    void timerCallback()
    {
        stopTimer();

        if (! Component::isMouseButtonDownAnywhere())
            getUndoManager().beginNewTransaction();
    }

    //==============================================================================
    class DrawableComponent   : public Component
    {
    public:
        DrawableComponent (DrawableEditorCanvas* canvas_)
            : canvas (canvas_)
        {
            setOpaque (true);
        }

        ~DrawableComponent()
        {
        }

        void updateDrawable()
        {
            repaint();
        }

        void paint (Graphics& g)
        {
            g.fillAll (Colours::white);

            const Point<int> origin (canvas->getOrigin());
            g.setOrigin (origin.getX(), origin.getY());

            if (origin.getX() > 0)
            {
                g.setColour (Colour::greyLevel (0.8f));
                g.drawVerticalLine (0, 0, 10000.0f);
            }

            if (origin.getY() > 0)
            {
                g.setColour (Colour::greyLevel (0.8f));
                g.drawHorizontalLine (0, 0, 10000.0f);
            }

            canvas->drawable->draw (g, 1.0f);
        }

    private:
        DrawableEditorCanvas* canvas;
        DrawableEditor& getEditor() const   { return canvas->getEditor(); }
    };

    ScopedPointer<DrawableComposite> drawable;

private:
    //==============================================================================
    DrawableEditor& editor;
};


#endif   // __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__
