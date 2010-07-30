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

#include "juce_FillType.h"
#include "../colour/juce_Colours.h"


//==============================================================================
FillType::FillType() throw()
    : colour (0xff000000), image (0)
{
}

FillType::FillType (const Colour& colour_) throw()
    : colour (colour_), image (0)
{
}

FillType::FillType (const ColourGradient& gradient_)
    : colour (0xff000000), gradient (new ColourGradient (gradient_)), image (0)
{
}

FillType::FillType (const Image& image_, const AffineTransform& transform_) throw()
    : colour (0xff000000), image (image_), transform (transform_)
{
}

FillType::FillType (const FillType& other)
    : colour (other.colour),
      gradient (other.gradient != 0 ? new ColourGradient (*other.gradient) : 0),
      image (other.image), transform (other.transform)
{
}

FillType& FillType::operator= (const FillType& other)
{
    if (this != &other)
    {
        colour = other.colour;
        gradient = (other.gradient != 0 ? new ColourGradient (*other.gradient) : 0);
        image = other.image;
        transform = other.transform;
    }

    return *this;
}

FillType::~FillType() throw()
{
}

bool FillType::operator== (const FillType& other) const
{
    return colour == other.colour && image == other.image
            && transform == other.transform
            && (gradient == other.gradient
                 || (gradient != 0 && other.gradient != 0 && *gradient == *other.gradient));
}

bool FillType::operator!= (const FillType& other) const
{
    return ! operator== (other);
}

void FillType::setColour (const Colour& newColour) throw()
{
    gradient = 0;
    image = Image::null;
    colour = newColour;
}

void FillType::setGradient (const ColourGradient& newGradient)
{
    if (gradient != 0)
    {
        *gradient = newGradient;
    }
    else
    {
        image = Image::null;
        gradient = new ColourGradient (newGradient);
        colour = Colours::black;
    }
}

void FillType::setTiledImage (const Image& image_, const AffineTransform& transform_) throw()
{
    gradient = 0;
    image = image_;
    transform = transform_;
    colour = Colours::black;
}

void FillType::setOpacity (const float newOpacity) throw()
{
    colour = colour.withAlpha (newOpacity);
}

bool FillType::isInvisible() const throw()
{
    return colour.isTransparent() || (gradient != 0 && gradient->isInvisible());
}


END_JUCE_NAMESPACE
