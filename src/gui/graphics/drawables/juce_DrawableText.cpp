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


#include "juce_DrawableText.h"


//==============================================================================
DrawableText::DrawableText()
    : colour (Colours::white)
{
}

DrawableText::~DrawableText()
{
}

//==============================================================================
void DrawableText::setText (const GlyphArrangement& newText)
{
    text = newText;
}

void DrawableText::setText (const String& newText, const Font& fontToUse)
{
    text.clear();
    text.addLineOfText (fontToUse, newText, 0.0f, 0.0f);
}

void DrawableText::setColour (const Colour& newColour)
{
    colour = newColour;
}

//==============================================================================
void DrawableText::render (const Drawable::RenderingContext& context) const
{
    context.g.setColour (colour.withMultipliedAlpha (context.opacity));
    text.draw (context.g, context.transform);
}

const Rectangle<float> DrawableText::getBounds() const
{
    float x, y, w, h;
    text.getBoundingBox (0, -1, x, y, w, h, false); // (really returns top, left, bottom, right)
    w -= x;
    h -= y;
    return Rectangle<float> (x, y, w, h);
}

bool DrawableText::hitTest (float x, float y) const
{
    return text.findGlyphIndexAt (x, y) >= 0;
}

Drawable* DrawableText::createCopy() const
{
    DrawableText* const dt = new DrawableText();

    dt->text = text;
    dt->colour = colour;

    return dt;
}

//==============================================================================
ValueTree DrawableText::createValueTree() const throw()
{
    ValueTree v (T("Text"));

    if (getName().isNotEmpty())
        v.setProperty ("id", getName(), 0);

    jassertfalse // xxx not finished!
    return v;
}

DrawableText* DrawableText::createFromValueTree (const ValueTree& tree) throw()
{
    if (! tree.hasType ("Text"))
        return 0;

    DrawableText* dt = new DrawableText();

    dt->setName (tree ["id"]);

    jassertfalse // xxx not finished!

    return dt;
}


END_JUCE_NAMESPACE
