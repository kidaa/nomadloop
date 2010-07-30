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

#include "juce_DrawableComposite.h"
#include "juce_DrawablePath.h"
#include "juce_DrawableImage.h"
#include "juce_DrawableText.h"
#include "../imaging/juce_Image.h"


//==============================================================================
DrawableComposite::DrawableComposite()
    : bounds (Point<float>(), Point<float> (100.0f, 0.0f), Point<float> (0.0f, 100.0f))
{
    setContentArea (RelativeRectangle (RelativeCoordinate (0.0),
                                       RelativeCoordinate (100.0),
                                       RelativeCoordinate (0.0),
                                       RelativeCoordinate (100.0)));
}

DrawableComposite::DrawableComposite (const DrawableComposite& other)
{
    bounds = other.bounds;

    for (int i = 0; i < other.drawables.size(); ++i)
        drawables.add (other.drawables.getUnchecked(i)->createCopy());

    markersX.addCopiesOf (other.markersX);
    markersY.addCopiesOf (other.markersY);
}

DrawableComposite::~DrawableComposite()
{
}

//==============================================================================
void DrawableComposite::insertDrawable (Drawable* drawable, const int index)
{
    if (drawable != 0)
    {
        jassert (! drawables.contains (drawable)); // trying to add a drawable that's already in here!
        jassert (drawable->parent == 0); // A drawable can only live inside one parent at a time!
        drawables.insert (index, drawable);
        drawable->parent = this;
    }
}

void DrawableComposite::insertDrawable (const Drawable& drawable, const int index)
{
    insertDrawable (drawable.createCopy(), index);
}

void DrawableComposite::removeDrawable (const int index, const bool deleteDrawable)
{
    drawables.remove (index, deleteDrawable);
}

Drawable* DrawableComposite::getDrawableWithName (const String& name) const throw()
{
    for (int i = drawables.size(); --i >= 0;)
        if (drawables.getUnchecked(i)->getName() == name)
            return drawables.getUnchecked(i);

    return 0;
}

void DrawableComposite::bringToFront (const int index)
{
    if (index >= 0 && index < drawables.size() - 1)
        drawables.move (index, -1);
}

void DrawableComposite::setBoundingBox (const RelativeParallelogram& newBoundingBox)
{
    bounds = newBoundingBox;
}

//==============================================================================
DrawableComposite::Marker::Marker (const DrawableComposite::Marker& other)
    : name (other.name), position (other.position)
{
}

DrawableComposite::Marker::Marker (const String& name_, const RelativeCoordinate& position_)
    : name (name_), position (position_)
{
}

bool DrawableComposite::Marker::operator!= (const DrawableComposite::Marker& other) const throw()
{
    return name != other.name || position != other.position;
}

//==============================================================================
const char* const DrawableComposite::contentLeftMarkerName ("left");
const char* const DrawableComposite::contentRightMarkerName ("right");
const char* const DrawableComposite::contentTopMarkerName ("top");
const char* const DrawableComposite::contentBottomMarkerName ("bottom");

const RelativeRectangle DrawableComposite::getContentArea() const
{
    jassert (markersX.size() >= 2 && getMarker (true, 0)->name == contentLeftMarkerName && getMarker (true, 1)->name == contentRightMarkerName);
    jassert (markersY.size() >= 2 && getMarker (false, 0)->name == contentTopMarkerName && getMarker (false, 1)->name == contentBottomMarkerName);

    return RelativeRectangle (markersX.getUnchecked(0)->position, markersX.getUnchecked(1)->position,
                              markersY.getUnchecked(0)->position, markersY.getUnchecked(1)->position);
}

void DrawableComposite::setContentArea (const RelativeRectangle& newArea)
{
    setMarker (contentLeftMarkerName, true, newArea.left);
    setMarker (contentRightMarkerName, true, newArea.right);
    setMarker (contentTopMarkerName, false, newArea.top);
    setMarker (contentBottomMarkerName, false, newArea.bottom);
}

void DrawableComposite::resetBoundingBoxToContentArea()
{
    const RelativeRectangle content (getContentArea());

    setBoundingBox (RelativeParallelogram (RelativePoint (content.left, content.top),
                                           RelativePoint (content.right, content.top),
                                           RelativePoint (content.left, content.bottom)));
}

void DrawableComposite::resetContentAreaAndBoundingBoxToFitChildren()
{
    const Rectangle<float> bounds (getUntransformedBounds (false));

    setContentArea (RelativeRectangle (RelativeCoordinate (bounds.getX()),
                                       RelativeCoordinate (bounds.getRight()),
                                       RelativeCoordinate (bounds.getY()),
                                       RelativeCoordinate (bounds.getBottom())));
    resetBoundingBoxToContentArea();
}

int DrawableComposite::getNumMarkers (const bool xAxis) const throw()
{
    return (xAxis ? markersX : markersY).size();
}

const DrawableComposite::Marker* DrawableComposite::getMarker (const bool xAxis, const int index) const throw()
{
    return (xAxis ? markersX : markersY) [index];
}

void DrawableComposite::setMarker (const String& name, const bool xAxis, const RelativeCoordinate& position)
{
    OwnedArray <Marker>& markers = (xAxis ? markersX : markersY);

    for (int i = 0; i < markers.size(); ++i)
    {
        Marker* const m = markers.getUnchecked(i);
        if (m->name == name)
        {
            if (m->position != position)
            {
                m->position = position;
                invalidatePoints();
            }

            return;
        }
    }

    (xAxis ? markersX : markersY).add (new Marker (name, position));
    invalidatePoints();
}

void DrawableComposite::removeMarker (const bool xAxis, const int index)
{
    jassert (index >= 2);

    if (index >= 2)
        (xAxis ? markersX : markersY).remove (index);
}

//==============================================================================
const AffineTransform DrawableComposite::calculateTransform() const
{
    Point<float> resolved[3];
    bounds.resolveThreePoints (resolved, parent);

    const Rectangle<float> content (getContentArea().resolve (parent));

    return AffineTransform::fromTargetPoints (content.getX(), content.getY(), resolved[0].getX(), resolved[0].getY(),
                                              content.getRight(), content.getY(), resolved[1].getX(), resolved[1].getY(),
                                              content.getX(), content.getBottom(), resolved[2].getX(), resolved[2].getY());
}

void DrawableComposite::render (const Drawable::RenderingContext& context) const
{
    if (drawables.size() > 0 && context.opacity > 0)
    {
        if (context.opacity >= 1.0f || drawables.size() == 1)
        {
            Drawable::RenderingContext contextCopy (context);
            contextCopy.transform = calculateTransform().followedBy (context.transform);

            for (int i = 0; i < drawables.size(); ++i)
                drawables.getUnchecked(i)->render (contextCopy);
        }
        else
        {
            // To correctly render a whole composite layer with an overall transparency,
            // we need to render everything opaquely into a temp buffer, then blend that
            // with the target opacity...
            const Rectangle<int> clipBounds (context.g.getClipBounds());
            Image tempImage (Image::ARGB, clipBounds.getWidth(), clipBounds.getHeight(), true);

            {
                Graphics tempG (tempImage);
                tempG.setOrigin (-clipBounds.getX(), -clipBounds.getY());
                Drawable::RenderingContext tempContext (tempG, context.transform, 1.0f);
                render (tempContext);
            }

            context.g.setOpacity (context.opacity);
            context.g.drawImageAt (tempImage, clipBounds.getX(), clipBounds.getY());
        }
    }
}

const RelativeCoordinate DrawableComposite::findNamedCoordinate (const String& objectName, const String& edge) const
{
    if (objectName == RelativeCoordinate::Strings::parent)
    {
        if (edge == RelativeCoordinate::Strings::right || edge == RelativeCoordinate::Strings::bottom)
        {
            jassertfalse; // a Drawable doesn't have a fixed right-hand or bottom edge - use a marker instead if you need a point of reference.
            return RelativeCoordinate (100.0);
        }
    }

    int i;
    for (i = 0; i < markersX.size(); ++i)
    {
        Marker* const m = markersX.getUnchecked(i);
        if (m->name == objectName)
            return m->position;
    }

    for (i = 0; i < markersY.size(); ++i)
    {
        Marker* const m = markersY.getUnchecked(i);
        if (m->name == objectName)
            return m->position;
    }

    return RelativeCoordinate();
}

const Rectangle<float> DrawableComposite::getUntransformedBounds (const bool includeMarkers) const
{
    Rectangle<float> bounds;

    int i;
    for (i = 0; i < drawables.size(); ++i)
        bounds = bounds.getUnion (drawables.getUnchecked(i)->getBounds());

    if (includeMarkers)
    {
        if (markersX.size() > 0)
        {
            float minX = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::min();

            for (i = markersX.size(); --i >= 0;)
            {
                const Marker* m = markersX.getUnchecked(i);
                const float pos = (float) m->position.resolve (this);
                minX = jmin (minX, pos);
                maxX = jmax (maxX, pos);
            }

            if (minX <= maxX)
            {
                if (bounds.getHeight() > 0)
                {
                    minX = jmin (minX, bounds.getX());
                    maxX = jmax (maxX, bounds.getRight());
                }

                bounds.setLeft (minX);
                bounds.setWidth (maxX - minX);
            }
        }

        if (markersY.size() > 0)
        {
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::min();

            for (i = markersY.size(); --i >= 0;)
            {
                const Marker* m = markersY.getUnchecked(i);
                const float pos = (float) m->position.resolve (this);
                minY = jmin (minY, pos);
                maxY = jmax (maxY, pos);
            }

            if (minY <= maxY)
            {
                if (bounds.getHeight() > 0)
                {
                    minY = jmin (minY, bounds.getY());
                    maxY = jmax (maxY, bounds.getBottom());
                }

                bounds.setTop (minY);
                bounds.setHeight (maxY - minY);
            }
        }
    }

    return bounds;
}

const Rectangle<float> DrawableComposite::getBounds() const
{
    return getUntransformedBounds (true).transformed (calculateTransform());
}

bool DrawableComposite::hitTest (float x, float y) const
{
    calculateTransform().inverted().transformPoint (x, y);

    for (int i = 0; i < drawables.size(); ++i)
        if (drawables.getUnchecked(i)->hitTest (x, y))
            return true;

    return false;
}

Drawable* DrawableComposite::createCopy() const
{
    return new DrawableComposite (*this);
}

void DrawableComposite::invalidatePoints()
{
    for (int i = 0; i < drawables.size(); ++i)
        drawables.getUnchecked(i)->invalidatePoints();
}

//==============================================================================
const Identifier DrawableComposite::valueTreeType ("Group");

const Identifier DrawableComposite::ValueTreeWrapper::topLeft ("topLeft");
const Identifier DrawableComposite::ValueTreeWrapper::topRight ("topRight");
const Identifier DrawableComposite::ValueTreeWrapper::bottomLeft ("bottomLeft");
const Identifier DrawableComposite::ValueTreeWrapper::childGroupTag ("Drawables");
const Identifier DrawableComposite::ValueTreeWrapper::markerGroupTagX ("MarkersX");
const Identifier DrawableComposite::ValueTreeWrapper::markerGroupTagY ("MarkersY");
const Identifier DrawableComposite::ValueTreeWrapper::markerTag ("Marker");
const Identifier DrawableComposite::ValueTreeWrapper::nameProperty ("name");
const Identifier DrawableComposite::ValueTreeWrapper::posProperty ("position");

//==============================================================================
DrawableComposite::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : ValueTreeWrapperBase (state_)
{
    jassert (state.hasType (valueTreeType));
}

ValueTree DrawableComposite::ValueTreeWrapper::getChildList() const
{
    return state.getChildWithName (childGroupTag);
}

ValueTree DrawableComposite::ValueTreeWrapper::getChildListCreating (UndoManager* undoManager)
{
    return state.getOrCreateChildWithName (childGroupTag, undoManager);
}

int DrawableComposite::ValueTreeWrapper::getNumDrawables() const
{
    return getChildList().getNumChildren();
}

ValueTree DrawableComposite::ValueTreeWrapper::getDrawableState (int index) const
{
    return getChildList().getChild (index);
}

ValueTree DrawableComposite::ValueTreeWrapper::getDrawableWithId (const String& objectId, bool recursive) const
{
    if (getID() == objectId)
        return state;

    if (! recursive)
    {
        return getChildList().getChildWithProperty (idProperty, objectId);
    }
    else
    {
        const ValueTree childList (getChildList());

        for (int i = getNumDrawables(); --i >= 0;)
        {
            const ValueTree& child = childList.getChild (i);

            if (child [Drawable::ValueTreeWrapperBase::idProperty] == objectId)
                return child;

            if (child.hasType (DrawableComposite::valueTreeType))
            {
                ValueTree v (DrawableComposite::ValueTreeWrapper (child).getDrawableWithId (objectId, true));

                if (v.isValid())
                    return v;
            }
        }

        return ValueTree::invalid;
    }
}

int DrawableComposite::ValueTreeWrapper::indexOfDrawable (const ValueTree& item) const
{
    return getChildList().indexOf (item);
}

void DrawableComposite::ValueTreeWrapper::addDrawable (const ValueTree& newDrawableState, int index, UndoManager* undoManager)
{
    getChildListCreating (undoManager).addChild (newDrawableState, index, undoManager);
}

void DrawableComposite::ValueTreeWrapper::moveDrawableOrder (int currentIndex, int newIndex, UndoManager* undoManager)
{
    getChildListCreating (undoManager).moveChild (currentIndex, newIndex, undoManager);
}

void DrawableComposite::ValueTreeWrapper::removeDrawable (const ValueTree& child, UndoManager* undoManager)
{
    getChildList().removeChild (child, undoManager);
}

const RelativeParallelogram DrawableComposite::ValueTreeWrapper::getBoundingBox() const
{
    return RelativeParallelogram (state.getProperty (topLeft, "0, 0"),
                                  state.getProperty (topRight, "100, 0"),
                                  state.getProperty (bottomLeft, "0, 100"));
}

void DrawableComposite::ValueTreeWrapper::setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager)
{
    state.setProperty (topLeft, newBounds.topLeft.toString(), undoManager);
    state.setProperty (topRight, newBounds.topRight.toString(), undoManager);
    state.setProperty (bottomLeft, newBounds.bottomLeft.toString(), undoManager);
}

void DrawableComposite::ValueTreeWrapper::resetBoundingBoxToContentArea (UndoManager* undoManager)
{
    const RelativeRectangle content (getContentArea());

    setBoundingBox (RelativeParallelogram (RelativePoint (content.left, content.top),
                                           RelativePoint (content.right, content.top),
                                           RelativePoint (content.left, content.bottom)), undoManager);
}

const RelativeRectangle DrawableComposite::ValueTreeWrapper::getContentArea() const
{
    return RelativeRectangle (getMarker (true, getMarkerState (true, 0)).position,
                              getMarker (true, getMarkerState (true, 1)).position,
                              getMarker (false, getMarkerState (false, 0)).position,
                              getMarker (false, getMarkerState (false, 1)).position);
}

void DrawableComposite::ValueTreeWrapper::setContentArea (const RelativeRectangle& newArea, UndoManager* undoManager)
{
    setMarker (true, Marker (contentLeftMarkerName, newArea.left), undoManager);
    setMarker (true, Marker (contentRightMarkerName, newArea.right), undoManager);
    setMarker (false, Marker (contentTopMarkerName, newArea.top), undoManager);
    setMarker (false, Marker (contentBottomMarkerName, newArea.bottom), undoManager);
}

ValueTree DrawableComposite::ValueTreeWrapper::getMarkerList (bool xAxis) const
{
    return state.getChildWithName (xAxis ? markerGroupTagX : markerGroupTagY);
}

ValueTree DrawableComposite::ValueTreeWrapper::getMarkerListCreating (bool xAxis, UndoManager* undoManager)
{
    return state.getOrCreateChildWithName (xAxis ? markerGroupTagX : markerGroupTagY, undoManager);
}

int DrawableComposite::ValueTreeWrapper::getNumMarkers (bool xAxis) const
{
    return getMarkerList (xAxis).getNumChildren();
}

const ValueTree DrawableComposite::ValueTreeWrapper::getMarkerState (bool xAxis, int index) const
{
    return getMarkerList (xAxis).getChild (index);
}

const ValueTree DrawableComposite::ValueTreeWrapper::getMarkerState (bool xAxis, const String& name) const
{
    return getMarkerList (xAxis).getChildWithProperty (nameProperty, name);
}

bool DrawableComposite::ValueTreeWrapper::containsMarker (bool xAxis, const ValueTree& state) const
{
    return state.isAChildOf (getMarkerList (xAxis));
}

const DrawableComposite::Marker DrawableComposite::ValueTreeWrapper::getMarker (bool xAxis, const ValueTree& state) const
{
    jassert (containsMarker (xAxis, state));

    return Marker (state [nameProperty], RelativeCoordinate (state [posProperty].toString(), xAxis));
}

void DrawableComposite::ValueTreeWrapper::setMarker (bool xAxis, const Marker& m, UndoManager* undoManager)
{
    ValueTree markerList (getMarkerListCreating (xAxis, undoManager));
    ValueTree marker (markerList.getChildWithProperty (nameProperty, m.name));

    if (marker.isValid())
    {
        marker.setProperty (posProperty, m.position.toString(), undoManager);
    }
    else
    {
        marker = ValueTree (markerTag);
        marker.setProperty (nameProperty, m.name, 0);
        marker.setProperty (posProperty, m.position.toString(), 0);
        markerList.addChild (marker, -1, undoManager);
    }
}

void DrawableComposite::ValueTreeWrapper::removeMarker (bool xAxis, const ValueTree& state, UndoManager* undoManager)
{
    if (state [nameProperty].toString() != contentLeftMarkerName
         && state [nameProperty].toString() != contentRightMarkerName
         && state [nameProperty].toString() != contentTopMarkerName
         && state [nameProperty].toString() != contentBottomMarkerName)
        return getMarkerList (xAxis).removeChild (state, undoManager);
}

//==============================================================================
const Rectangle<float> DrawableComposite::refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    const ValueTreeWrapper wrapper (tree);
    setName (wrapper.getID());

    Rectangle<float> damage;
    bool redrawAll = false;

    const RelativeParallelogram newBounds (wrapper.getBoundingBox());
    if (bounds != newBounds)
    {
        redrawAll = true;
        damage = getBounds();
        bounds = newBounds;
    }

    const int numMarkersX = wrapper.getNumMarkers (true);
    const int numMarkersY = wrapper.getNumMarkers (false);

    // Remove deleted markers...
    if (markersX.size() > numMarkersX || markersY.size() > numMarkersY)
    {
        if (! redrawAll)
        {
            redrawAll = true;
            damage = getBounds();
        }

        markersX.removeRange (jmax (2, numMarkersX), markersX.size());
        markersY.removeRange (jmax (2, numMarkersY), markersY.size());
    }

    // Update markers and add new ones..
    int i;
    for (i = 0; i < numMarkersX; ++i)
    {
        const Marker newMarker (wrapper.getMarker (true, wrapper.getMarkerState (true, i)));
        Marker* m = markersX[i];

        if (m == 0 || newMarker != *m)
        {
            if (! redrawAll)
            {
                redrawAll = true;
                damage = getBounds();
            }

            if (m == 0)
                markersX.add (new Marker (newMarker));
            else
                *m = newMarker;
        }
    }

    for (i = 0; i < numMarkersY; ++i)
    {
        const Marker newMarker (wrapper.getMarker (false, wrapper.getMarkerState (false, i)));
        Marker* m = markersY[i];

        if (m == 0 || newMarker != *m)
        {
            if (! redrawAll)
            {
                redrawAll = true;
                damage = getBounds();
            }

            if (m == 0)
                markersY.add (new Marker (newMarker));
            else
                *m = newMarker;
        }
    }

    // Remove deleted drawables..
    for (i = drawables.size(); --i >= wrapper.getNumDrawables();)
    {
        Drawable* const d = drawables.getUnchecked(i);

        if (! redrawAll)
            damage = damage.getUnion (d->getBounds());

        d->parent = 0;
        drawables.remove (i);
    }

    // Update drawables and add new ones..
    for (i = 0; i < wrapper.getNumDrawables(); ++i)
    {
        const ValueTree newDrawable (wrapper.getDrawableState (i));
        Drawable* d = drawables[i];

        if (d != 0)
        {
            if (newDrawable.hasType (d->getValueTreeType()))
            {
                const Rectangle<float> area (d->refreshFromValueTree (newDrawable, imageProvider));

                if (! redrawAll)
                    damage = damage.getUnion (area);
            }
            else
            {
                if (! redrawAll)
                    damage = damage.getUnion (d->getBounds());

                d = createChildFromValueTree (this, newDrawable, imageProvider);
                drawables.set (i, d);

                if (! redrawAll)
                    damage = damage.getUnion (d->getBounds());
            }
        }
        else
        {
            d = createChildFromValueTree (this, newDrawable, imageProvider);
            drawables.set (i, d);

            if (! redrawAll)
                damage = damage.getUnion (d->getBounds());
        }
    }

    if (redrawAll)
        damage = damage.getUnion (getBounds());
    else if (! damage.isEmpty())
        damage = damage.transformed (calculateTransform());

    return damage;
}

const ValueTree DrawableComposite::createValueTree (ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getName(), 0);
    v.setBoundingBox (bounds, 0);

    int i;
    for (i = 0; i < drawables.size(); ++i)
        v.addDrawable (drawables.getUnchecked(i)->createValueTree (imageProvider), -1, 0);

    for (i = 0; i < markersX.size(); ++i)
        v.setMarker (true, *markersX.getUnchecked(i), 0);

    for (i = 0; i < markersY.size(); ++i)
        v.setMarker (false, *markersY.getUnchecked(i), 0);

    return tree;
}


END_JUCE_NAMESPACE
