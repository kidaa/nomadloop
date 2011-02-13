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

#include "juce_Drawable.h"
#include "juce_DrawableComposite.h"
#include "juce_DrawablePath.h"
#include "juce_DrawableRectangle.h"
#include "juce_DrawableImage.h"
#include "juce_DrawableText.h"
#include "../imaging/juce_ImageFileFormat.h"
#include "../../../text/juce_XmlDocument.h"
#include "../../../io/files/juce_FileInputStream.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
Drawable::Drawable()
{
    setInterceptsMouseClicks (false, false);
    setPaintingIsUnclipped (true);
}

Drawable::~Drawable()
{
}

//==============================================================================
void Drawable::draw (Graphics& g, float opacity, const AffineTransform& transform) const
{
    const_cast <Drawable*> (this)->nonConstDraw (g, opacity, transform);
}

void Drawable::nonConstDraw (Graphics& g, float opacity, const AffineTransform& transform)
{
    Graphics::ScopedSaveState ss (g);

    const float oldOpacity = getAlpha();
    setAlpha (opacity);
    g.addTransform (AffineTransform::translation ((float) -originRelativeToComponent.getX(),
                                                  (float) -originRelativeToComponent.getY())
                        .followedBy (getTransform())
                        .followedBy (transform));

    if (! g.isClipEmpty())
        paintEntireComponent (g, false);

    setAlpha (oldOpacity);
}

void Drawable::drawAt (Graphics& g, float x, float y, float opacity) const
{
    draw (g, opacity, AffineTransform::translation (x, y));
}

void Drawable::drawWithin (Graphics& g, const Rectangle<float>& destArea, const RectanglePlacement& placement, float opacity) const
{
    draw (g, opacity, placement.getTransformToFit (getDrawableBounds(), destArea));
}

//==============================================================================
DrawableComposite* Drawable::getParent() const
{
    return dynamic_cast <DrawableComposite*> (getParentComponent());
}

void Drawable::transformContextToCorrectOrigin (Graphics& g)
{
    g.setOrigin (originRelativeToComponent.getX(),
                 originRelativeToComponent.getY());
}

void Drawable::parentHierarchyChanged()
{
    setBoundsToEnclose (getDrawableBounds());
}

void Drawable::setBoundsToEnclose (const Rectangle<float>& area)
{
    Drawable* const parent = getParent();
    Point<int> parentOrigin;
    if (parent != 0)
        parentOrigin = parent->originRelativeToComponent;

    const Rectangle<int> newBounds (area.getSmallestIntegerContainer() + parentOrigin);
    originRelativeToComponent = parentOrigin - newBounds.getPosition();
    setBounds (newBounds);
}

//==============================================================================
void Drawable::setOriginWithOriginalSize (const Point<float>& originWithinParent)
{
    setTransform (AffineTransform::translation (originWithinParent.getX(), originWithinParent.getY()));
}

void Drawable::setTransformToFit (const Rectangle<float>& area, const RectanglePlacement& placement)
{
    if (! area.isEmpty())
        setTransform (placement.getTransformToFit (getDrawableBounds(), area));
}

//==============================================================================
Drawable* Drawable::createFromImageData (const void* data, const size_t numBytes)
{
    Drawable* result = 0;

    Image image (ImageFileFormat::loadFrom (data, (int) numBytes));

    if (image.isValid())
    {
        DrawableImage* const di = new DrawableImage();
        di->setImage (image);
        result = di;
    }
    else
    {
        const String asString (String::createStringFromData (data, (int) numBytes));

        XmlDocument doc (asString);
        ScopedPointer <XmlElement> outer (doc.getDocumentElement (true));

        if (outer != 0 && outer->hasTagName ("svg"))
        {
            ScopedPointer <XmlElement> svg (doc.getDocumentElement());

            if (svg != 0)
                result = Drawable::createFromSVG (*svg);
        }
    }

    return result;
}

Drawable* Drawable::createFromImageDataStream (InputStream& dataSource)
{
    MemoryOutputStream mo;
    mo.writeFromInputStream (dataSource, -1);

    return createFromImageData (mo.getData(), mo.getDataSize());
}

Drawable* Drawable::createFromImageFile (const File& file)
{
    const ScopedPointer <FileInputStream> fin (file.createInputStream());

    return fin != 0 ? createFromImageDataStream (*fin) : 0;
}

//==============================================================================
template <class DrawableClass>
class DrawableTypeHandler  : public ComponentBuilder::TypeHandler
{
public:
    DrawableTypeHandler()
        : ComponentBuilder::TypeHandler (DrawableClass::valueTreeType)
    {
    }

    Component* addNewComponentFromState (const ValueTree& state, Component* parent)
    {
        DrawableClass* const d = new DrawableClass();

        if (parent != 0)
            parent->addAndMakeVisible (d);

        updateComponentFromState (d, state);
        return d;
    }

    void updateComponentFromState (Component* component, const ValueTree& state)
    {
        DrawableClass* const d = dynamic_cast <DrawableClass*> (component);
        jassert (d != 0);
        d->refreshFromValueTree (state, *this->getBuilder());
    }
};

void Drawable::registerDrawableTypeHandlers (ComponentBuilder& builder)
{
    builder.registerTypeHandler (new DrawableTypeHandler <DrawablePath>());
    builder.registerTypeHandler (new DrawableTypeHandler <DrawableComposite>());
    builder.registerTypeHandler (new DrawableTypeHandler <DrawableRectangle>());
    builder.registerTypeHandler (new DrawableTypeHandler <DrawableImage>());
    builder.registerTypeHandler (new DrawableTypeHandler <DrawableText>());
}

Drawable* Drawable::createFromValueTree (const ValueTree& tree, ComponentBuilder::ImageProvider* imageProvider)
{
    ComponentBuilder builder (tree);
    builder.setImageProvider (imageProvider);
    registerDrawableTypeHandlers (builder);

    ScopedPointer<Component> comp (builder.createComponent());
    Drawable* const d = dynamic_cast<Drawable*> (static_cast <Component*> (comp));

    if (d != 0)
        comp.release();

    return d;
}

//==============================================================================
Drawable::ValueTreeWrapperBase::ValueTreeWrapperBase (const ValueTree& state_)
    : state (state_)
{
}

const String Drawable::ValueTreeWrapperBase::getID() const
{
    return state [ComponentBuilder::idProperty];
}

void Drawable::ValueTreeWrapperBase::setID (const String& newID)
{
    if (newID.isEmpty())
        state.removeProperty (ComponentBuilder::idProperty, 0);
    else
        state.setProperty (ComponentBuilder::idProperty, newID, 0);
}


END_JUCE_NAMESPACE
