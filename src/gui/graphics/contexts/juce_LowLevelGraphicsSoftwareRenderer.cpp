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

#include "juce_LowLevelGraphicsSoftwareRenderer.h"
#include "juce_EdgeTable.h"
#include "../imaging/juce_Image.h"
#include "../colour/juce_PixelFormats.h"
#include "../geometry/juce_PathStrokeType.h"
#include "../geometry/juce_Rectangle.h"
#include "../../../core/juce_SystemStats.h"
#include "../../../core/juce_Singleton.h"
#include "../../../utilities/juce_DeletedAtShutdown.h"

#if (JUCE_WINDOWS || JUCE_LINUX) && ! JUCE_64BIT
 #define JUCE_USE_SSE_INSTRUCTIONS 1
#endif

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4127) // "expression is constant" warning

 #if JUCE_DEBUG
  #pragma optimize ("t", on)  // optimise just this file, to avoid sluggish graphics when debugging
  #pragma warning (disable: 4714) // warning about forcedinline methods not being inlined
 #endif
#endif

namespace SoftwareRendererClasses
{

//==============================================================================
template <class PixelType, bool replaceExisting = false>
class SolidColourEdgeTableRenderer
{
public:
    SolidColourEdgeTableRenderer (const Image::BitmapData& data_, const PixelARGB& colour)
        : data (data_),
          sourceColour (colour)
    {
        if (sizeof (PixelType) == 3)
        {
            areRGBComponentsEqual = sourceColour.getRed() == sourceColour.getGreen()
                                        && sourceColour.getGreen() == sourceColour.getBlue();
            filler[0].set (sourceColour);
            filler[1].set (sourceColour);
            filler[2].set (sourceColour);
            filler[3].set (sourceColour);
        }
    }

    forcedinline void setEdgeTableYPos (const int y) throw()
    {
        linePixels = (PixelType*) data.getLinePointer (y);
    }

    forcedinline void handleEdgeTablePixel (const int x, const int alphaLevel) const throw()
    {
        if (replaceExisting)
            linePixels[x].set (sourceColour);
        else
            linePixels[x].blend (sourceColour, alphaLevel);
    }

    forcedinline void handleEdgeTablePixelFull (const int x) const throw()
    {
        if (replaceExisting)
            linePixels[x].set (sourceColour);
        else
            linePixels[x].blend (sourceColour);
    }

    forcedinline void handleEdgeTableLine (const int x, const int width, const int alphaLevel) const throw()
    {
        PixelARGB p (sourceColour);
        p.multiplyAlpha (alphaLevel);

        PixelType* dest = linePixels + x;

        if (replaceExisting || p.getAlpha() >= 0xff)
            replaceLine (dest, p, width);
        else
            blendLine (dest, p, width);
    }

    forcedinline void handleEdgeTableLineFull (const int x, const int width) const throw()
    {
        PixelType* dest = linePixels + x;

        if (replaceExisting || sourceColour.getAlpha() >= 0xff)
            replaceLine (dest, sourceColour, width);
        else
            blendLine (dest, sourceColour, width);
    }

private:
    const Image::BitmapData& data;
    PixelType* linePixels;
    PixelARGB sourceColour;
    PixelRGB filler [4];
    bool areRGBComponentsEqual;

    inline void blendLine (PixelType* dest, const PixelARGB& colour, int width) const throw()
    {
        do
        {
            dest->blend (colour);
            ++dest;
        } while (--width > 0);
    }

    forcedinline void replaceLine (PixelRGB* dest, const PixelARGB& colour, int width) const throw()
    {
        if (areRGBComponentsEqual)  // if all the component values are the same, we can cheat..
        {
            memset (dest, colour.getRed(), width * 3);
        }
        else
        {
            if (width >> 5)
            {
                const int* const intFiller = (const int*) filler;

                while (width > 8 && (((pointer_sized_int) dest) & 7) != 0)
                {
                    dest->set (colour);
                    ++dest;
                    --width;
                }

                while (width > 4)
                {
                    ((int*) dest) [0] = intFiller[0];
                    ((int*) dest) [1] = intFiller[1];
                    ((int*) dest) [2] = intFiller[2];
                    dest = (PixelRGB*) (((uint8*) dest) + 12);
                    width -= 4;
                }
            }

            while (--width >= 0)
            {
                dest->set (colour);
                ++dest;
            }
        }
    }

    forcedinline void replaceLine (PixelAlpha* const dest, const PixelARGB& colour, int const width) const throw()
    {
        memset (dest, colour.getAlpha(), width);
    }

    forcedinline void replaceLine (PixelARGB* dest, const PixelARGB& colour, int width) const throw()
    {
        do
        {
            dest->set (colour);
            ++dest;

        } while (--width > 0);
    }

    SolidColourEdgeTableRenderer (const SolidColourEdgeTableRenderer&);
    SolidColourEdgeTableRenderer& operator= (const SolidColourEdgeTableRenderer&);
};

//==============================================================================
class LinearGradientPixelGenerator
{
public:
    LinearGradientPixelGenerator (const ColourGradient& gradient, const AffineTransform& transform, const PixelARGB* const lookupTable_, const int numEntries_)
        : lookupTable (lookupTable_), numEntries (numEntries_)
    {
        jassert (numEntries_ >= 0);
        Point<float> p1 (gradient.point1);
        Point<float> p2 (gradient.point2);

        if (! transform.isIdentity())
        {
            const Line<float> l (p2, p1);
            Point<float> p3 = l.getPointAlongLine (0.0f, 100.0f);

            p1.applyTransform (transform);
            p2.applyTransform (transform);
            p3.applyTransform (transform);

            const Line<float> l2 (p2, p3);
            p2 = l2.getPointAlongLineProportionally (l2.findNearestPointTo (p1));
        }

        vertical = std::abs (p1.getX() - p2.getX()) < 0.001f;
        horizontal = std::abs (p1.getY() - p2.getY()) < 0.001f;

        if (vertical)
        {
            scale = roundToInt ((numEntries << (int) numScaleBits) / (double) (p2.getY() - p1.getY()));
            start = roundToInt (p1.getY() * scale);
        }
        else if (horizontal)
        {
            scale = roundToInt ((numEntries << (int) numScaleBits) / (double) (p2.getX() - p1.getX()));
            start = roundToInt (p1.getX() * scale);
        }
        else
        {
            grad = (p2.getY() - p1.getY()) / (double) (p1.getX() - p2.getX());
            yTerm = p1.getY() - p1.getX() / grad;
            scale = roundToInt ((numEntries << (int) numScaleBits) / (yTerm * grad - (p2.getY() * grad - p2.getX())));
            grad *= scale;
        }
    }

    forcedinline void setY (const int y) throw()
    {
        if (vertical)
            linePix = lookupTable [jlimit (0, numEntries, (y * scale - start) >> (int) numScaleBits)];
        else if (! horizontal)
            start = roundToInt ((y - yTerm) * grad);
    }

    inline const PixelARGB getPixel (const int x) const throw()
    {
        return vertical ? linePix
                        : lookupTable [jlimit (0, numEntries, (x * scale - start) >> (int) numScaleBits)];
    }

private:
    const PixelARGB* const lookupTable;
    const int numEntries;
    PixelARGB linePix;
    int start, scale;
    double grad, yTerm;
    bool vertical, horizontal;
    enum { numScaleBits = 12 };

    LinearGradientPixelGenerator (const LinearGradientPixelGenerator&);
    LinearGradientPixelGenerator& operator= (const LinearGradientPixelGenerator&);
};

//==============================================================================
class RadialGradientPixelGenerator
{
public:
    RadialGradientPixelGenerator (const ColourGradient& gradient, const AffineTransform&,
                                  const PixelARGB* const lookupTable_, const int numEntries_)
        : lookupTable (lookupTable_),
          numEntries (numEntries_),
          gx1 (gradient.point1.getX()),
          gy1 (gradient.point1.getY())
    {
        jassert (numEntries_ >= 0);
        const Point<float> diff (gradient.point1 - gradient.point2);
        maxDist = diff.getX() * diff.getX() + diff.getY() * diff.getY();
        invScale = numEntries / std::sqrt (maxDist);
        jassert (roundToInt (std::sqrt (maxDist) * invScale) <= numEntries);
    }

    forcedinline void setY (const int y) throw()
    {
        dy = y - gy1;
        dy *= dy;
    }

    inline const PixelARGB getPixel (const int px) const throw()
    {
        double x = px - gx1;
        x *= x;
        x += dy;

        return lookupTable [x >= maxDist ? numEntries : roundToInt (std::sqrt (x) * invScale)];
    }

protected:
    const PixelARGB* const lookupTable;
    const int numEntries;
    const double gx1, gy1;
    double maxDist, invScale, dy;

    RadialGradientPixelGenerator (const RadialGradientPixelGenerator&);
    RadialGradientPixelGenerator& operator= (const RadialGradientPixelGenerator&);
};

//==============================================================================
class TransformedRadialGradientPixelGenerator   : public RadialGradientPixelGenerator
{
public:
    TransformedRadialGradientPixelGenerator (const ColourGradient& gradient, const AffineTransform& transform,
                                             const PixelARGB* const lookupTable_, const int numEntries_)
        : RadialGradientPixelGenerator (gradient, transform, lookupTable_, numEntries_),
          inverseTransform (transform.inverted())
    {
        tM10 = inverseTransform.mat10;
        tM00 = inverseTransform.mat00;
    }

    forcedinline void setY (const int y) throw()
    {
        lineYM01 = inverseTransform.mat01 * y + inverseTransform.mat02 - gx1;
        lineYM11 = inverseTransform.mat11 * y + inverseTransform.mat12 - gy1;
    }

    inline const PixelARGB getPixel (const int px) const throw()
    {
        double x = px;
        const double y = tM10 * x + lineYM11;
        x = tM00 * x + lineYM01;
        x *= x;
        x += y * y;

        if (x >= maxDist)
            return lookupTable [numEntries];
        else
            return lookupTable [jmin (numEntries, roundToInt (std::sqrt (x) * invScale))];
    }

private:
    double tM10, tM00, lineYM01, lineYM11;
    const AffineTransform inverseTransform;

    TransformedRadialGradientPixelGenerator (const TransformedRadialGradientPixelGenerator&);
    TransformedRadialGradientPixelGenerator& operator= (const TransformedRadialGradientPixelGenerator&);
};

//==============================================================================
template <class PixelType, class GradientType>
class GradientEdgeTableRenderer  : public GradientType
{
public:
    GradientEdgeTableRenderer (const Image::BitmapData& destData_, const ColourGradient& gradient, const AffineTransform& transform,
                               const PixelARGB* const lookupTable_, const int numEntries_)
        : GradientType (gradient, transform, lookupTable_, numEntries_ - 1),
          destData (destData_)
    {
    }

    forcedinline void setEdgeTableYPos (const int y) throw()
    {
        linePixels = (PixelType*) destData.getLinePointer (y);
        GradientType::setY (y);
    }

    forcedinline void handleEdgeTablePixel (const int x, const int alphaLevel) const throw()
    {
        linePixels[x].blend (GradientType::getPixel (x), alphaLevel);
    }

    forcedinline void handleEdgeTablePixelFull (const int x) const throw()
    {
        linePixels[x].blend (GradientType::getPixel (x));
    }

    void handleEdgeTableLine (int x, int width, const int alphaLevel) const throw()
    {
        PixelType* dest = linePixels + x;

        if (alphaLevel < 0xff)
        {
            do
            {
                (dest++)->blend (GradientType::getPixel (x++), alphaLevel);
            } while (--width > 0);
        }
        else
        {
            do
            {
                (dest++)->blend (GradientType::getPixel (x++));
            } while (--width > 0);
        }
    }

    void handleEdgeTableLineFull (int x, int width) const throw()
    {
        PixelType* dest = linePixels + x;

        do
        {
            (dest++)->blend (GradientType::getPixel (x++));
        } while (--width > 0);
    }

private:
    const Image::BitmapData& destData;
    PixelType* linePixels;

    GradientEdgeTableRenderer (const GradientEdgeTableRenderer&);
    GradientEdgeTableRenderer& operator= (const GradientEdgeTableRenderer&);
};

//==============================================================================
static forcedinline int safeModulo (int n, const int divisor) throw()
{
    jassert (divisor > 0);
    n %= divisor;
    return (n < 0) ? (n + divisor) : n;
}

//==============================================================================
template <class DestPixelType, class SrcPixelType, bool repeatPattern>
class ImageFillEdgeTableRenderer
{
public:
    ImageFillEdgeTableRenderer (const Image::BitmapData& destData_,
                                const Image::BitmapData& srcData_,
                                const int extraAlpha_,
                                const int x, const int y)
        : destData (destData_),
          srcData (srcData_),
          extraAlpha (extraAlpha_ + 1),
          xOffset (repeatPattern ? safeModulo (x, srcData_.width) - srcData_.width : x),
          yOffset (repeatPattern ? safeModulo (y, srcData_.height) - srcData_.height : y)
    {
    }

    forcedinline void setEdgeTableYPos (int y) throw()
    {
        linePixels = (DestPixelType*) destData.getLinePointer (y);

        y -= yOffset;
        if (repeatPattern)
        {
            jassert (y >= 0);
            y %= srcData.height;
        }

        sourceLineStart = (SrcPixelType*) srcData.getLinePointer (y);
    }

    forcedinline void handleEdgeTablePixel (const int x, int alphaLevel) const throw()
    {
        alphaLevel = (alphaLevel * extraAlpha) >> 8;

        linePixels[x].blend (sourceLineStart [repeatPattern ? ((x - xOffset) % srcData.width) : (x - xOffset)], alphaLevel);
    }

    forcedinline void handleEdgeTablePixelFull (const int x) const throw()
    {
        linePixels[x].blend (sourceLineStart [repeatPattern ? ((x - xOffset) % srcData.width) : (x - xOffset)], extraAlpha);
    }

    void handleEdgeTableLine (int x, int width, int alphaLevel) const throw()
    {
        DestPixelType* dest = linePixels + x;
        alphaLevel = (alphaLevel * extraAlpha) >> 8;
        x -= xOffset;

        jassert (repeatPattern || (x >= 0 && x + width <= srcData.width));

        if (alphaLevel < 0xfe)
        {
            do
            {
                dest++ ->blend (sourceLineStart [repeatPattern ? (x++ % srcData.width) : x++], alphaLevel);
            } while (--width > 0);
        }
        else
        {
            if (repeatPattern)
            {
                do
                {
                    dest++ ->blend (sourceLineStart [x++ % srcData.width]);
                } while (--width > 0);
            }
            else
            {
                copyRow (dest, sourceLineStart + x, width);
            }
        }
    }

    void handleEdgeTableLineFull (int x, int width) const throw()
    {
        DestPixelType* dest = linePixels + x;
        x -= xOffset;

        jassert (repeatPattern || (x >= 0 && x + width <= srcData.width));

        if (extraAlpha < 0xfe)
        {
            do
            {
                dest++ ->blend (sourceLineStart [repeatPattern ? (x++ % srcData.width) : x++], extraAlpha);
            } while (--width > 0);
        }
        else
        {
            if (repeatPattern)
            {
                do
                {
                    dest++ ->blend (sourceLineStart [x++ % srcData.width]);
                } while (--width > 0);
            }
            else
            {
                copyRow (dest, sourceLineStart + x, width);
            }
        }
    }

    void clipEdgeTableLine (EdgeTable& et, int x, int y, int width)
    {
        jassert (x - xOffset >= 0 && x + width - xOffset <= srcData.width);
        SrcPixelType* s = (SrcPixelType*) srcData.getLinePointer (y - yOffset);
        uint8* mask = (uint8*) (s + x - xOffset);

        if (sizeof (SrcPixelType) == sizeof (PixelARGB))
            mask += PixelARGB::indexA;

        et.clipLineToMask (x, y, mask, sizeof (SrcPixelType), width);
    }

private:
    const Image::BitmapData& destData;
    const Image::BitmapData& srcData;
    const int extraAlpha, xOffset, yOffset;
    DestPixelType* linePixels;
    SrcPixelType* sourceLineStart;

    template <class PixelType1, class PixelType2>
    forcedinline static void copyRow (PixelType1* dest, PixelType2* src, int width) throw()
    {
        do
        {
            dest++ ->blend (*src++);
        } while (--width > 0);
    }

    forcedinline static void copyRow (PixelRGB* dest, PixelRGB* src, int width) throw()
    {
        memcpy (dest, src, width * sizeof (PixelRGB));
    }

    ImageFillEdgeTableRenderer (const ImageFillEdgeTableRenderer&);
    ImageFillEdgeTableRenderer& operator= (const ImageFillEdgeTableRenderer&);
};

//==============================================================================
template <class DestPixelType, class SrcPixelType, bool repeatPattern>
class TransformedImageFillEdgeTableRenderer
{
public:
    TransformedImageFillEdgeTableRenderer (const Image::BitmapData& destData_,
                                           const Image::BitmapData& srcData_,
                                           const AffineTransform& transform,
                                           const int extraAlpha_,
                                           const bool betterQuality_)
        : interpolator (transform),
          destData (destData_),
          srcData (srcData_),
          extraAlpha (extraAlpha_ + 1),
          betterQuality (betterQuality_),
          pixelOffset (betterQuality_ ? 0.5f : 0.0f),
          pixelOffsetInt (betterQuality_ ? -128 : 0),
          maxX (srcData_.width - 1),
          maxY (srcData_.height - 1),
          scratchSize (2048)
    {
        scratchBuffer.malloc (scratchSize);
    }

    ~TransformedImageFillEdgeTableRenderer()
    {
    }

    forcedinline void setEdgeTableYPos (const int newY) throw()
    {
        y = newY;
        linePixels = (DestPixelType*) destData.getLinePointer (newY);
    }

    forcedinline void handleEdgeTablePixel (const int x, int alphaLevel) throw()
    {
        alphaLevel *= extraAlpha;
        alphaLevel >>= 8;

        SrcPixelType p;
        generate (&p, x, 1);

        linePixels[x].blend (p, alphaLevel);
    }

    forcedinline void handleEdgeTablePixelFull (const int x) throw()
    {
        SrcPixelType p;
        generate (&p, x, 1);

        linePixels[x].blend (p, extraAlpha);
    }

    void handleEdgeTableLine (const int x, int width, int alphaLevel) throw()
    {
        if (width > scratchSize)
        {
            scratchSize = width;
            scratchBuffer.malloc (scratchSize);
        }

        SrcPixelType* span = scratchBuffer;
        generate (span, x, width);

        DestPixelType* dest = linePixels + x;
        alphaLevel *= extraAlpha;
        alphaLevel >>= 8;

        if (alphaLevel < 0xfe)
        {
            do
            {
                dest++ ->blend (*span++, alphaLevel);
            } while (--width > 0);
        }
        else
        {
            do
            {
                dest++ ->blend (*span++);
            } while (--width > 0);
        }
    }

    forcedinline void handleEdgeTableLineFull (const int x, int width) throw()
    {
        handleEdgeTableLine (x, width, 255);
    }

    void clipEdgeTableLine (EdgeTable& et, int x, int y_, int width)
    {
        if (width > scratchSize)
        {
            scratchSize = width;
            scratchBuffer.malloc (scratchSize);
        }

        y = y_;
        generate (scratchBuffer, x, width);

        et.clipLineToMask (x, y_,
                           reinterpret_cast<uint8*> (scratchBuffer.getData()) + SrcPixelType::indexA,
                           sizeof (SrcPixelType), width);
    }

private:
    //==============================================================================
    void generate (PixelARGB* dest, const int x, int numPixels) throw()
    {
        this->interpolator.setStartOfLine (x + pixelOffset, y + pixelOffset, numPixels);

        do
        {
            int hiResX, hiResY;
            this->interpolator.next (hiResX, hiResY);
            hiResX += pixelOffsetInt;
            hiResY += pixelOffsetInt;

            int loResX = hiResX >> 8;
            int loResY = hiResY >> 8;

            if (repeatPattern)
            {
                loResX = safeModulo (loResX, srcData.width);
                loResY = safeModulo (loResY, srcData.height);
            }

            if (betterQuality
                 && ((unsigned int) loResX) < (unsigned int) maxX
                 && ((unsigned int) loResY) < (unsigned int) maxY)
            {
                uint32 c[4] = { 256 * 128, 256 * 128, 256 * 128, 256 * 128 };
                hiResX &= 255;
                hiResY &= 255;

                const uint8* src = this->srcData.getPixelPointer (loResX, loResY);

                uint32 weight = (256 - hiResX) * (256 - hiResY);
                c[0] += weight * src[0];
                c[1] += weight * src[1];
                c[2] += weight * src[2];
                c[3] += weight * src[3];

                weight = hiResX * (256 - hiResY);
                c[0] += weight * src[4];
                c[1] += weight * src[5];
                c[2] += weight * src[6];
                c[3] += weight * src[7];

                src += this->srcData.lineStride;

                weight = (256 - hiResX) * hiResY;
                c[0] += weight * src[0];
                c[1] += weight * src[1];
                c[2] += weight * src[2];
                c[3] += weight * src[3];

                weight = hiResX * hiResY;
                c[0] += weight * src[4];
                c[1] += weight * src[5];
                c[2] += weight * src[6];
                c[3] += weight * src[7];

                dest->setARGB ((uint8) (c[PixelARGB::indexA] >> 16),
                               (uint8) (c[PixelARGB::indexR] >> 16),
                               (uint8) (c[PixelARGB::indexG] >> 16),
                               (uint8) (c[PixelARGB::indexB] >> 16));
            }
            else
            {
                if (! repeatPattern)
                {
                    // Beyond the edges, just repeat the edge pixels and leave the anti-aliasing to be handled by the edgetable
                    if (loResX < 0)     loResX = 0;
                    if (loResY < 0)     loResY = 0;
                    if (loResX > maxX)  loResX = maxX;
                    if (loResY > maxY)  loResY = maxY;
                }

                dest->set (*(const PixelARGB*) this->srcData.getPixelPointer (loResX, loResY));
            }

            ++dest;

        } while (--numPixels > 0);
    }

    void generate (PixelRGB* dest, const int x, int numPixels) throw()
    {
        this->interpolator.setStartOfLine (x + pixelOffset, y + pixelOffset, numPixels);

        do
        {
            int hiResX, hiResY;
            this->interpolator.next (hiResX, hiResY);
            hiResX += pixelOffsetInt;
            hiResY += pixelOffsetInt;
            int loResX = hiResX >> 8;
            int loResY = hiResY >> 8;

            if (repeatPattern)
            {
                loResX = safeModulo (loResX, srcData.width);
                loResY = safeModulo (loResY, srcData.height);
            }

            if (betterQuality
                 && ((unsigned int) loResX) < (unsigned int) maxX
                 && ((unsigned int) loResY) < (unsigned int) maxY)
            {
                uint32 c[3] = { 256 * 128, 256 * 128, 256 * 128 };
                hiResX &= 255;
                hiResY &= 255;

                const uint8* src = this->srcData.getPixelPointer (loResX, loResY);

                unsigned int weight = (256 - hiResX) * (256 - hiResY);
                c[0] += weight * src[0];
                c[1] += weight * src[1];
                c[2] += weight * src[2];

                weight = hiResX * (256 - hiResY);
                c[0] += weight * src[3];
                c[1] += weight * src[4];
                c[2] += weight * src[5];

                src += this->srcData.lineStride;

                weight = (256 - hiResX) * hiResY;
                c[0] += weight * src[0];
                c[1] += weight * src[1];
                c[2] += weight * src[2];

                weight = hiResX * hiResY;
                c[0] += weight * src[3];
                c[1] += weight * src[4];
                c[2] += weight * src[5];

                dest->setARGB ((uint8) 255,
                               (uint8) (c[PixelRGB::indexR] >> 16),
                               (uint8) (c[PixelRGB::indexG] >> 16),
                               (uint8) (c[PixelRGB::indexB] >> 16));
            }
            else
            {
                if (! repeatPattern)
                {
                    // Beyond the edges, just repeat the edge pixels and leave the anti-aliasing to be handled by the edgetable
                    if (loResX < 0)     loResX = 0;
                    if (loResY < 0)     loResY = 0;
                    if (loResX > maxX)  loResX = maxX;
                    if (loResY > maxY)  loResY = maxY;
                }

                dest->set (*(const PixelRGB*) this->srcData.getPixelPointer (loResX, loResY));
            }

            ++dest;

        } while (--numPixels > 0);
    }

    void generate (PixelAlpha* dest, const int x, int numPixels) throw()
    {
        this->interpolator.setStartOfLine (x + pixelOffset, y + pixelOffset, numPixels);

        do
        {
            int hiResX, hiResY;
            this->interpolator.next (hiResX, hiResY);
            hiResX += pixelOffsetInt;
            hiResY += pixelOffsetInt;
            int loResX = hiResX >> 8;
            int loResY = hiResY >> 8;

            if (repeatPattern)
            {
                loResX = safeModulo (loResX, srcData.width);
                loResY = safeModulo (loResY, srcData.height);
            }

            if (betterQuality
                 && ((unsigned int) loResX) < (unsigned int) maxX
                 && ((unsigned int) loResY) < (unsigned int) maxY)
            {
                hiResX &= 255;
                hiResY &= 255;

                const uint8* src = this->srcData.getPixelPointer (loResX, loResY);
                uint32 c = 256 * 128;
                c += src[0] * ((256 - hiResX) * (256 - hiResY));
                c += src[1] * (hiResX * (256 - hiResY));
                src += this->srcData.lineStride;
                c += src[0] * ((256 - hiResX) * hiResY);
                c += src[1] * (hiResX * hiResY);

                *((uint8*) dest) = (uint8) c;
            }
            else
            {
                if (! repeatPattern)
                {
                    // Beyond the edges, just repeat the edge pixels and leave the anti-aliasing to be handled by the edgetable
                    if (loResX < 0)     loResX = 0;
                    if (loResY < 0)     loResY = 0;
                    if (loResX > maxX)  loResX = maxX;
                    if (loResY > maxY)  loResY = maxY;
                }

                *((uint8*) dest) = *(this->srcData.getPixelPointer (loResX, loResY));
            }

            ++dest;

        } while (--numPixels > 0);
    }

    //==============================================================================
    class TransformedImageSpanInterpolator
    {
    public:
        TransformedImageSpanInterpolator (const AffineTransform& transform) throw()
            : inverseTransform (transform.inverted())
        {}

        void setStartOfLine (float x, float y, const int numPixels) throw()
        {
            float x1 = x, y1 = y;
            inverseTransform.transformPoint (x1, y1);
            x += numPixels;
            inverseTransform.transformPoint (x, y);

            xBresenham.set ((int) (x1 * 256.0f), (int) (x * 256.0f), numPixels);
            yBresenham.set ((int) (y1 * 256.0f), (int) (y * 256.0f), numPixels);
        }

        void next (int& x, int& y) throw()
        {
            x = xBresenham.n;
            xBresenham.stepToNext();
            y = yBresenham.n;
            yBresenham.stepToNext();
        }

    private:
        class BresenhamInterpolator
        {
        public:
            BresenhamInterpolator() throw() {}

            void set (const int n1, const int n2, const int numSteps_) throw()
            {
                numSteps = jmax (1, numSteps_);
                step = (n2 - n1) / numSteps;
                remainder = modulo = (n2 - n1) % numSteps;
                n = n1;

                if (modulo <= 0)
                {
                    modulo += numSteps;
                    remainder += numSteps;
                    --step;
                }

                modulo -= numSteps;
            }

            forcedinline void stepToNext() throw()
            {
                modulo += remainder;
                n += step;

                if (modulo > 0)
                {
                    modulo -= numSteps;
                    ++n;
                }
            }

            int n;

        private:
            int numSteps, step, modulo, remainder;
        };

        const AffineTransform inverseTransform;
        BresenhamInterpolator xBresenham, yBresenham;

        TransformedImageSpanInterpolator (const TransformedImageSpanInterpolator&);
        TransformedImageSpanInterpolator& operator= (const TransformedImageSpanInterpolator&);
    };

    //==============================================================================
    TransformedImageSpanInterpolator interpolator;
    const Image::BitmapData& destData;
    const Image::BitmapData& srcData;
    const int extraAlpha;
    const bool betterQuality;
    const float pixelOffset;
    const int pixelOffsetInt, maxX, maxY;
    int y;
    DestPixelType* linePixels;
    HeapBlock <SrcPixelType> scratchBuffer;
    int scratchSize;

    TransformedImageFillEdgeTableRenderer (const TransformedImageFillEdgeTableRenderer&);
    TransformedImageFillEdgeTableRenderer& operator= (const TransformedImageFillEdgeTableRenderer&);
};

//==============================================================================
class ClipRegionBase  : public ReferenceCountedObject
{
public:
    ClipRegionBase() {}
    virtual ~ClipRegionBase() {}

    typedef ReferenceCountedObjectPtr<ClipRegionBase> Ptr;

    virtual const Ptr clone() const = 0;
    virtual const Ptr applyClipTo (const Ptr& target) const = 0;

    virtual const Ptr clipToRectangle (const Rectangle<int>& r) = 0;
    virtual const Ptr clipToRectangleList (const RectangleList& r) = 0;
    virtual const Ptr excludeClipRectangle (const Rectangle<int>& r) = 0;
    virtual const Ptr clipToPath (const Path& p, const AffineTransform& transform) = 0;
    virtual const Ptr clipToEdgeTable (const EdgeTable& et) = 0;
    virtual const Ptr clipToImageAlpha (const Image& image, const Rectangle<int>& srcClip, const AffineTransform& t, const bool betterQuality) = 0;

    virtual bool clipRegionIntersects (const Rectangle<int>& r) const = 0;
    virtual const Rectangle<int> getClipBounds() const = 0;

    virtual void fillRectWithColour (Image::BitmapData& destData, const Rectangle<int>& area, const PixelARGB& colour, bool replaceContents) const = 0;
    virtual void fillRectWithColour (Image::BitmapData& destData, const Rectangle<float>& area, const PixelARGB& colour) const = 0;
    virtual void fillAllWithColour (Image::BitmapData& destData, const PixelARGB& colour, bool replaceContents) const = 0;
    virtual void fillAllWithGradient (Image::BitmapData& destData, ColourGradient& gradient, const AffineTransform& transform, bool isIdentity) const = 0;
    virtual void renderImageTransformed (const Image::BitmapData& destData, const Image::BitmapData& srcData, const int alpha, const AffineTransform& t, bool betterQuality, bool tiledFill) const = 0;
    virtual void renderImageUntransformed (const Image::BitmapData& destData, const Image::BitmapData& srcData, const int alpha, int x, int y, bool tiledFill) const = 0;

protected:
    //==============================================================================
    template <class Iterator>
    static void renderImageTransformedInternal (Iterator& iter, const Image::BitmapData& destData, const Image::BitmapData& srcData,
                                                const int alpha, const AffineTransform& transform, bool betterQuality, bool tiledFill)
    {
        switch (destData.pixelFormat)
        {
        case Image::ARGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelARGB, true> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                else            { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelARGB, false> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelRGB, true> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                else            { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelRGB, false> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                break;
            default:
                if (tiledFill)  { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelAlpha, true> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                else            { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelAlpha, false> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                break;
            }
            break;

        case Image::RGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelARGB, true> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                else            { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelARGB, false> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelRGB, true> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                else            { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelRGB, false> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                break;
            default:
                if (tiledFill)  { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelAlpha, true> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                else            { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelAlpha, false> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                break;
            }
            break;

        default:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelARGB, true> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                else            { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelARGB, false> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelRGB, true> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                else            { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelRGB, false> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                break;
            default:
                if (tiledFill)  { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelAlpha, true> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                else            { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelAlpha, false> r (destData, srcData, transform, alpha, betterQuality); iter.iterate (r); }
                break;
            }
            break;
        }
    }

    template <class Iterator>
    static void renderImageUntransformedInternal (Iterator& iter, const Image::BitmapData& destData, const Image::BitmapData& srcData, const int alpha, int x, int y, bool tiledFill)
    {
        switch (destData.pixelFormat)
        {
        case Image::ARGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { ImageFillEdgeTableRenderer <PixelARGB, PixelARGB, true> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFillEdgeTableRenderer <PixelARGB, PixelARGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { ImageFillEdgeTableRenderer <PixelARGB, PixelRGB, true> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFillEdgeTableRenderer <PixelARGB, PixelRGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            default:
                if (tiledFill)  { ImageFillEdgeTableRenderer <PixelARGB, PixelAlpha, true> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFillEdgeTableRenderer <PixelARGB, PixelAlpha, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            }
            break;

        case Image::RGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { ImageFillEdgeTableRenderer <PixelRGB, PixelARGB, true> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFillEdgeTableRenderer <PixelRGB, PixelARGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { ImageFillEdgeTableRenderer <PixelRGB, PixelRGB, true> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFillEdgeTableRenderer <PixelRGB, PixelRGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            default:
                if (tiledFill)  { ImageFillEdgeTableRenderer <PixelRGB, PixelAlpha, true> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFillEdgeTableRenderer <PixelRGB, PixelAlpha, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            }
            break;

        default:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { ImageFillEdgeTableRenderer <PixelAlpha, PixelARGB, true> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFillEdgeTableRenderer <PixelAlpha, PixelARGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { ImageFillEdgeTableRenderer <PixelAlpha, PixelRGB, true> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFillEdgeTableRenderer <PixelAlpha, PixelRGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            default:
                if (tiledFill)  { ImageFillEdgeTableRenderer <PixelAlpha, PixelAlpha, true> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFillEdgeTableRenderer <PixelAlpha, PixelAlpha, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            }
            break;
        }
    }

    template <class Iterator, class DestPixelType>
    static void renderSolidFill (Iterator& iter, const Image::BitmapData& destData, const PixelARGB& fillColour, const bool replaceContents, DestPixelType*)
    {
        jassert (destData.pixelStride == sizeof (DestPixelType));
        if (replaceContents)
        {
            SolidColourEdgeTableRenderer <DestPixelType, true> r (destData, fillColour);
            iter.iterate (r);
        }
        else
        {
            SolidColourEdgeTableRenderer <DestPixelType, false> r (destData, fillColour);
            iter.iterate (r);
        }
    }

    template <class Iterator, class DestPixelType>
    static void renderGradient (Iterator& iter, const Image::BitmapData& destData, const ColourGradient& g, const AffineTransform& transform,
                                const PixelARGB* const lookupTable, const int numLookupEntries, const bool isIdentity, DestPixelType*)
    {
        jassert (destData.pixelStride == sizeof (DestPixelType));

        if (g.isRadial)
        {
            if (isIdentity)
            {
                GradientEdgeTableRenderer <DestPixelType, RadialGradientPixelGenerator> renderer (destData, g, transform, lookupTable, numLookupEntries);
                iter.iterate (renderer);
            }
            else
            {
                GradientEdgeTableRenderer <DestPixelType, TransformedRadialGradientPixelGenerator> renderer (destData, g, transform, lookupTable, numLookupEntries);
                iter.iterate (renderer);
            }
        }
        else
        {
            GradientEdgeTableRenderer <DestPixelType, LinearGradientPixelGenerator> renderer (destData, g, transform, lookupTable, numLookupEntries);
            iter.iterate (renderer);
        }
    }
};

//==============================================================================
class ClipRegion_EdgeTable  : public ClipRegionBase
{
public:
    ClipRegion_EdgeTable (const EdgeTable& e) : edgeTable (e) {}
    ClipRegion_EdgeTable (const Rectangle<int>& r) : edgeTable (r) {}
    ClipRegion_EdgeTable (const Rectangle<float>& r) : edgeTable (r) {}
    ClipRegion_EdgeTable (const RectangleList& r) : edgeTable (r) {}
    ClipRegion_EdgeTable (const Rectangle<int>& bounds, const Path& p, const AffineTransform& t) : edgeTable (bounds, p, t) {}
    ClipRegion_EdgeTable (const ClipRegion_EdgeTable& other) : edgeTable (other.edgeTable) {}
    ~ClipRegion_EdgeTable() {}

    const Ptr clone() const
    {
        return new ClipRegion_EdgeTable (*this);
    }

    const Ptr applyClipTo (const Ptr& target) const
    {
        return target->clipToEdgeTable (edgeTable);
    }

    const Ptr clipToRectangle (const Rectangle<int>& r)
    {
        edgeTable.clipToRectangle (r);
        return edgeTable.isEmpty() ? 0 : this;
    }

    const Ptr clipToRectangleList (const RectangleList& r)
    {
        RectangleList inverse (edgeTable.getMaximumBounds());

        if (inverse.subtract (r))
            for (RectangleList::Iterator iter (inverse); iter.next();)
                edgeTable.excludeRectangle (*iter.getRectangle());

        return edgeTable.isEmpty() ? 0 : this;
    }

    const Ptr excludeClipRectangle (const Rectangle<int>& r)
    {
        edgeTable.excludeRectangle (r);
        return edgeTable.isEmpty() ? 0 : this;
    }

    const Ptr clipToPath (const Path& p, const AffineTransform& transform)
    {
        EdgeTable et (edgeTable.getMaximumBounds(), p, transform);
        edgeTable.clipToEdgeTable (et);
        return edgeTable.isEmpty() ? 0 : this;
    }

    const Ptr clipToEdgeTable (const EdgeTable& et)
    {
        edgeTable.clipToEdgeTable (et);
        return edgeTable.isEmpty() ? 0 : this;
    }

    const Ptr clipToImageAlpha (const Image& image, const Rectangle<int>& srcClip, const AffineTransform& transform, const bool betterQuality)
    {
        const Image::BitmapData srcData (image, srcClip.getX(), srcClip.getY(), srcClip.getWidth(), srcClip.getHeight());

        if (transform.isOnlyTranslation())
        {
            // If our translation doesn't involve any distortion, just use a simple blit..
            const int tx = (int) (transform.getTranslationX() * 256.0f);
            const int ty = (int) (transform.getTranslationY() * 256.0f);

            if ((! betterQuality) || ((tx | ty) & 224) == 0)
            {
                const int imageX = ((tx + 128) >> 8);
                const int imageY = ((ty + 128) >> 8);

                if (image.getFormat() == Image::ARGB)
                    straightClipImage (srcData, imageX, imageY, (PixelARGB*) 0);
                else
                    straightClipImage (srcData, imageX, imageY, (PixelAlpha*) 0);

                return edgeTable.isEmpty() ? 0 : this;
            }
        }

        if (transform.isSingularity())
            return 0;

        {
            Path p;
            p.addRectangle (0, 0, (float) srcData.width, (float) srcData.height);
            EdgeTable et2 (edgeTable.getMaximumBounds(), p, transform);
            edgeTable.clipToEdgeTable (et2);
        }

        if (! edgeTable.isEmpty())
        {
            if (image.getFormat() == Image::ARGB)
                transformedClipImage (srcData, transform, betterQuality, (PixelARGB*) 0);
            else
                transformedClipImage (srcData, transform, betterQuality, (PixelAlpha*) 0);
        }

        return edgeTable.isEmpty() ? 0 : this;
    }

    bool clipRegionIntersects (const Rectangle<int>& r) const
    {
        return edgeTable.getMaximumBounds().intersects (r);
    }

    const Rectangle<int> getClipBounds() const
    {
        return edgeTable.getMaximumBounds();
    }

    void fillRectWithColour (Image::BitmapData& destData, const Rectangle<int>& area, const PixelARGB& colour, bool replaceContents) const
    {
        const Rectangle<int> totalClip (edgeTable.getMaximumBounds());
        const Rectangle<int> clipped (totalClip.getIntersection (area));

        if (! clipped.isEmpty())
        {
            ClipRegion_EdgeTable et (clipped);
            et.edgeTable.clipToEdgeTable (edgeTable);
            et.fillAllWithColour (destData, colour, replaceContents);
        }
    }

    void fillRectWithColour (Image::BitmapData& destData, const Rectangle<float>& area, const PixelARGB& colour) const
    {
        const Rectangle<float> totalClip (edgeTable.getMaximumBounds().toFloat());
        const Rectangle<float> clipped (totalClip.getIntersection (area));

        if (! clipped.isEmpty())
        {
            ClipRegion_EdgeTable et (clipped);
            et.edgeTable.clipToEdgeTable (edgeTable);
            et.fillAllWithColour (destData, colour, false);
        }
    }

    void fillAllWithColour (Image::BitmapData& destData, const PixelARGB& colour, bool replaceContents) const
    {
        switch (destData.pixelFormat)
        {
            case Image::ARGB:   renderSolidFill (edgeTable, destData, colour, replaceContents, (PixelARGB*) 0); break;
            case Image::RGB:    renderSolidFill (edgeTable, destData, colour, replaceContents, (PixelRGB*) 0); break;
            default:            renderSolidFill (edgeTable, destData, colour, replaceContents, (PixelAlpha*) 0); break;
        }
    }

    void fillAllWithGradient (Image::BitmapData& destData, ColourGradient& gradient, const AffineTransform& transform, bool isIdentity) const
    {
        HeapBlock <PixelARGB> lookupTable;
        const int numLookupEntries = gradient.createLookupTable (transform, lookupTable);
        jassert (numLookupEntries > 0);

        switch (destData.pixelFormat)
        {
            case Image::ARGB:   renderGradient (edgeTable, destData, gradient, transform, lookupTable, numLookupEntries, isIdentity, (PixelARGB*) 0); break;
            case Image::RGB:    renderGradient (edgeTable, destData, gradient, transform, lookupTable, numLookupEntries, isIdentity, (PixelRGB*) 0); break;
            default:            renderGradient (edgeTable, destData, gradient, transform, lookupTable, numLookupEntries, isIdentity, (PixelAlpha*) 0); break;
        }
    }

    void renderImageTransformed (const Image::BitmapData& destData, const Image::BitmapData& srcData, const int alpha, const AffineTransform& transform, bool betterQuality, bool tiledFill) const
    {
        renderImageTransformedInternal (edgeTable, destData, srcData, alpha, transform, betterQuality, tiledFill);
    }

    void renderImageUntransformed (const Image::BitmapData& destData, const Image::BitmapData& srcData, const int alpha, int x, int y, bool tiledFill) const
    {
        renderImageUntransformedInternal (edgeTable, destData, srcData, alpha, x, y, tiledFill);
    }

    EdgeTable edgeTable;

private:
    //==============================================================================
    template <class SrcPixelType>
    void transformedClipImage (const Image::BitmapData& srcData, const AffineTransform& transform, const bool betterQuality, const SrcPixelType*)
    {
        TransformedImageFillEdgeTableRenderer <SrcPixelType, SrcPixelType, false> renderer (srcData, srcData, transform, 255, betterQuality);

        for (int y = 0; y < edgeTable.getMaximumBounds().getHeight(); ++y)
            renderer.clipEdgeTableLine (edgeTable, edgeTable.getMaximumBounds().getX(), y + edgeTable.getMaximumBounds().getY(),
                                        edgeTable.getMaximumBounds().getWidth());
    }

    template <class SrcPixelType>
    void straightClipImage (const Image::BitmapData& srcData, int imageX, int imageY, const SrcPixelType*)
    {
        Rectangle<int> r (imageX, imageY, srcData.width, srcData.height);
        edgeTable.clipToRectangle (r);

        ImageFillEdgeTableRenderer <SrcPixelType, SrcPixelType, false> renderer (srcData, srcData, 255, imageX, imageY);

        for (int y = 0; y < r.getHeight(); ++y)
            renderer.clipEdgeTableLine (edgeTable, r.getX(), y + r.getY(), r.getWidth());
    }

    ClipRegion_EdgeTable& operator= (const ClipRegion_EdgeTable&);
};


//==============================================================================
class ClipRegion_RectangleList  : public ClipRegionBase
{
public:
    ClipRegion_RectangleList (const Rectangle<int>& r) : clip (r) {}
    ClipRegion_RectangleList (const RectangleList& r) : clip (r) {}
    ClipRegion_RectangleList (const ClipRegion_RectangleList& other) : clip (other.clip) {}
    ~ClipRegion_RectangleList() {}

    const Ptr clone() const
    {
        return new ClipRegion_RectangleList (*this);
    }

    const Ptr applyClipTo (const Ptr& target) const
    {
        return target->clipToRectangleList (clip);
    }

    const Ptr clipToRectangle (const Rectangle<int>& r)
    {
        clip.clipTo (r);
        return clip.isEmpty() ? 0 : this;
    }

    const Ptr clipToRectangleList (const RectangleList& r)
    {
        clip.clipTo (r);
        return clip.isEmpty() ? 0 : this;
    }

    const Ptr excludeClipRectangle (const Rectangle<int>& r)
    {
        clip.subtract (r);
        return clip.isEmpty() ? 0 : this;
    }

    const Ptr clipToPath (const Path& p, const AffineTransform& transform)
    {
        return Ptr (new ClipRegion_EdgeTable (clip))->clipToPath (p, transform);
    }

    const Ptr clipToEdgeTable (const EdgeTable& et)
    {
        return Ptr (new ClipRegion_EdgeTable (clip))->clipToEdgeTable (et);
    }

    const Ptr clipToImageAlpha (const Image& image, const Rectangle<int>& srcClip, const AffineTransform& transform, const bool betterQuality)
    {
        return Ptr (new ClipRegion_EdgeTable (clip))->clipToImageAlpha (image, srcClip, transform, betterQuality);
    }

    bool clipRegionIntersects (const Rectangle<int>& r) const
    {
        return clip.intersects (r);
    }

    const Rectangle<int> getClipBounds() const
    {
        return clip.getBounds();
    }

    void fillRectWithColour (Image::BitmapData& destData, const Rectangle<int>& area, const PixelARGB& colour, bool replaceContents) const
    {
        SubRectangleIterator iter (clip, area);

        switch (destData.pixelFormat)
        {
            case Image::ARGB:   renderSolidFill (iter, destData, colour, replaceContents, (PixelARGB*) 0); break;
            case Image::RGB:    renderSolidFill (iter, destData, colour, replaceContents, (PixelRGB*) 0); break;
            default:            renderSolidFill (iter, destData, colour, replaceContents, (PixelAlpha*) 0); break;
        }
    }

    void fillRectWithColour (Image::BitmapData& destData, const Rectangle<float>& area, const PixelARGB& colour) const
    {
        SubRectangleIteratorFloat iter (clip, area);

        switch (destData.pixelFormat)
        {
            case Image::ARGB:   renderSolidFill (iter, destData, colour, false, (PixelARGB*) 0); break;
            case Image::RGB:    renderSolidFill (iter, destData, colour, false, (PixelRGB*) 0); break;
            default:            renderSolidFill (iter, destData, colour, false, (PixelAlpha*) 0); break;
        }
    }

    void fillAllWithColour (Image::BitmapData& destData, const PixelARGB& colour, bool replaceContents) const
    {
        switch (destData.pixelFormat)
        {
            case Image::ARGB:   renderSolidFill (*this, destData, colour, replaceContents, (PixelARGB*) 0); break;
            case Image::RGB:    renderSolidFill (*this, destData, colour, replaceContents, (PixelRGB*) 0); break;
            default:            renderSolidFill (*this, destData, colour, replaceContents, (PixelAlpha*) 0); break;
        }
    }

    void fillAllWithGradient (Image::BitmapData& destData, ColourGradient& gradient, const AffineTransform& transform, bool isIdentity) const
    {
        HeapBlock <PixelARGB> lookupTable;
        const int numLookupEntries = gradient.createLookupTable (transform, lookupTable);
        jassert (numLookupEntries > 0);

        switch (destData.pixelFormat)
        {
            case Image::ARGB:   renderGradient (*this, destData, gradient, transform, lookupTable, numLookupEntries, isIdentity, (PixelARGB*) 0); break;
            case Image::RGB:    renderGradient (*this, destData, gradient, transform, lookupTable, numLookupEntries, isIdentity, (PixelRGB*) 0); break;
            default:            renderGradient (*this, destData, gradient, transform, lookupTable, numLookupEntries, isIdentity, (PixelAlpha*) 0); break;
        }
    }

    void renderImageTransformed (const Image::BitmapData& destData, const Image::BitmapData& srcData, const int alpha, const AffineTransform& transform, bool betterQuality, bool tiledFill) const
    {
        renderImageTransformedInternal (*this, destData, srcData, alpha, transform, betterQuality, tiledFill);
    }

    void renderImageUntransformed (const Image::BitmapData& destData, const Image::BitmapData& srcData, const int alpha, int x, int y, bool tiledFill) const
    {
        renderImageUntransformedInternal (*this, destData, srcData, alpha, x, y, tiledFill);
    }

    RectangleList clip;

    //==============================================================================
    template <class Renderer>
    void iterate (Renderer& r) const throw()
    {
        RectangleList::Iterator iter (clip);

        while (iter.next())
        {
            const Rectangle<int> rect (*iter.getRectangle());
            const int x = rect.getX();
            const int w = rect.getWidth();
            jassert (w > 0);
            const int bottom = rect.getBottom();

            for (int y = rect.getY(); y < bottom; ++y)
            {
                r.setEdgeTableYPos (y);
                r.handleEdgeTableLineFull (x, w);
            }
        }
    }

private:
    //==============================================================================
    class SubRectangleIterator
    {
    public:
        SubRectangleIterator (const RectangleList& clip_, const Rectangle<int>& area_)
            : clip (clip_), area (area_)
        {
        }

        template <class Renderer>
        void iterate (Renderer& r) const throw()
        {
            RectangleList::Iterator iter (clip);

            while (iter.next())
            {
                const Rectangle<int> rect (iter.getRectangle()->getIntersection (area));

                if (! rect.isEmpty())
                {
                    const int x = rect.getX();
                    const int w = rect.getWidth();
                    const int bottom = rect.getBottom();

                    for (int y = rect.getY(); y < bottom; ++y)
                    {
                        r.setEdgeTableYPos (y);
                        r.handleEdgeTableLineFull (x, w);
                    }
                }
            }
        }

    private:
        const RectangleList& clip;
        const Rectangle<int> area;

        SubRectangleIterator (const SubRectangleIterator&);
        SubRectangleIterator& operator= (const SubRectangleIterator&);
    };

    //==============================================================================
    class SubRectangleIteratorFloat
    {
    public:
        SubRectangleIteratorFloat (const RectangleList& clip_, const Rectangle<float>& area_)
            : clip (clip_), area (area_)
        {
        }

        template <class Renderer>
        void iterate (Renderer& r) const throw()
        {
            int left    = roundToInt (area.getX() * 256.0f);
            int top     = roundToInt (area.getY() * 256.0f);
            int right   = roundToInt (area.getRight() * 256.0f);
            int bottom  = roundToInt (area.getBottom() * 256.0f);

            int totalTop, totalLeft, totalBottom, totalRight;
            int topAlpha, leftAlpha, bottomAlpha, rightAlpha;

            if ((top >> 8) == (bottom >> 8))
            {
                topAlpha = bottom - top;
                bottomAlpha = 0;
                totalTop = top >> 8;
                totalBottom = bottom = top = totalTop + 1;
            }
            else
            {
                if ((top & 255) == 0)
                {
                    topAlpha = 0;
                    top = totalTop = (top >> 8);
                }
                else
                {
                    topAlpha = 255 - (top & 255);
                    totalTop = (top >> 8);
                    top = totalTop + 1;
                }

                bottomAlpha = bottom & 255;
                bottom >>= 8;
                totalBottom = bottom + (bottomAlpha != 0 ? 1 : 0);
            }

            if ((left >> 8) == (right >> 8))
            {
                leftAlpha = right - left;
                rightAlpha = 0;
                totalLeft = (left >> 8);
                totalRight = right = left = totalLeft + 1;
            }
            else
            {
                if ((left & 255) == 0)
                {
                    leftAlpha = 0;
                    left = totalLeft = (left >> 8);
                }
                else
                {
                    leftAlpha = 255 - (left & 255);
                    totalLeft = (left >> 8);
                    left = totalLeft + 1;
                }

                rightAlpha = right & 255;
                right >>= 8;
                totalRight = right + (rightAlpha != 0 ? 1 : 0);
            }

            RectangleList::Iterator iter (clip);

            while (iter.next())
            {
                const int clipLeft   = iter.getRectangle()->getX();
                const int clipRight  = iter.getRectangle()->getRight();
                const int clipTop    = iter.getRectangle()->getY();
                const int clipBottom = iter.getRectangle()->getBottom();

                if (totalBottom > clipTop && totalTop < clipBottom && totalRight > clipLeft && totalLeft < clipRight)
                {
                    if (right - left == 1 && leftAlpha + rightAlpha == 0) // special case for 1-pix vertical lines
                    {
                        if (topAlpha != 0 && totalTop >= clipTop)
                        {
                            r.setEdgeTableYPos (totalTop);
                            r.handleEdgeTablePixel (left, topAlpha);
                        }

                        const int endY = jmin (bottom, clipBottom);
                        for (int y = jmax (clipTop, top); y < endY; ++y)
                        {
                            r.setEdgeTableYPos (y);
                            r.handleEdgeTablePixelFull (left);
                        }

                        if (bottomAlpha != 0 && bottom < clipBottom)
                        {
                            r.setEdgeTableYPos (bottom);
                            r.handleEdgeTablePixel (left, bottomAlpha);
                        }
                    }
                    else
                    {
                        const int clippedLeft = jmax (left, clipLeft);
                        const int clippedWidth = jmin (right, clipRight) - clippedLeft;
                        const bool doLeftAlpha = leftAlpha != 0 && totalLeft >= clipLeft;
                        const bool doRightAlpha = rightAlpha != 0 && right < clipRight;

                        if (topAlpha != 0 && totalTop >= clipTop)
                        {
                            r.setEdgeTableYPos (totalTop);

                            if (doLeftAlpha)
                                r.handleEdgeTablePixel (totalLeft, (leftAlpha * topAlpha) >> 8);

                            if (clippedWidth > 0)
                                r.handleEdgeTableLine (clippedLeft, clippedWidth, topAlpha);

                            if (doRightAlpha)
                                r.handleEdgeTablePixel (right, (rightAlpha * topAlpha) >> 8);
                        }

                        const int endY = jmin (bottom, clipBottom);
                        for (int y = jmax (clipTop, top); y < endY; ++y)
                        {
                            r.setEdgeTableYPos (y);

                            if (doLeftAlpha)
                                r.handleEdgeTablePixel (totalLeft, leftAlpha);

                            if (clippedWidth > 0)
                                r.handleEdgeTableLineFull (clippedLeft, clippedWidth);

                            if (doRightAlpha)
                                r.handleEdgeTablePixel (right, rightAlpha);
                        }

                        if (bottomAlpha != 0 && bottom < clipBottom)
                        {
                            r.setEdgeTableYPos (bottom);

                            if (doLeftAlpha)
                                r.handleEdgeTablePixel (totalLeft, (leftAlpha * bottomAlpha) >> 8);

                            if (clippedWidth > 0)
                                r.handleEdgeTableLine (clippedLeft, clippedWidth, bottomAlpha);

                            if (doRightAlpha)
                                r.handleEdgeTablePixel (right, (rightAlpha * bottomAlpha) >> 8);
                        }
                    }
                }
            }
        }

    private:
        const RectangleList& clip;
        const Rectangle<float>& area;

        SubRectangleIteratorFloat (const SubRectangleIteratorFloat&);
        SubRectangleIteratorFloat& operator= (const SubRectangleIteratorFloat&);
    };

    ClipRegion_RectangleList& operator= (const ClipRegion_RectangleList&);
};

}

//==============================================================================
class LowLevelGraphicsSoftwareRenderer::SavedState
{
public:
    SavedState (const Rectangle<int>& clip_, const int xOffset_, const int yOffset_)
        : clip (new SoftwareRendererClasses::ClipRegion_RectangleList (clip_)),
          xOffset (xOffset_), yOffset (yOffset_), interpolationQuality (Graphics::mediumResamplingQuality)
    {
    }

    SavedState (const RectangleList& clip_, const int xOffset_, const int yOffset_)
        : clip (new SoftwareRendererClasses::ClipRegion_RectangleList (clip_)),
          xOffset (xOffset_), yOffset (yOffset_), interpolationQuality (Graphics::mediumResamplingQuality)
    {
    }

    SavedState (const SavedState& other)
        : clip (other.clip), xOffset (other.xOffset), yOffset (other.yOffset), font (other.font),
          fillType (other.fillType), interpolationQuality (other.interpolationQuality)
    {
    }

    ~SavedState()
    {
    }

    void setOrigin (const int x, const int y) throw()
    {
        xOffset += x;
        yOffset += y;
    }

    bool clipToRectangle (const Rectangle<int>& r)
    {
        if (clip != 0)
        {
            cloneClipIfMultiplyReferenced();
            clip = clip->clipToRectangle (r.translated (xOffset, yOffset));
        }

        return clip != 0;
    }

    bool clipToRectangleList (const RectangleList& r)
    {
        if (clip != 0)
        {
            cloneClipIfMultiplyReferenced();

            RectangleList offsetList (r);
            offsetList.offsetAll (xOffset, yOffset);
            clip = clip->clipToRectangleList (offsetList);
        }

        return clip != 0;
    }

    bool excludeClipRectangle (const Rectangle<int>& r)
    {
        if (clip != 0)
        {
            cloneClipIfMultiplyReferenced();
            clip = clip->excludeClipRectangle (r.translated (xOffset, yOffset));
        }

        return clip != 0;
    }

    void clipToPath (const Path& p, const AffineTransform& transform)
    {
        if (clip != 0)
        {
            cloneClipIfMultiplyReferenced();
            clip = clip->clipToPath (p, transform.translated ((float) xOffset, (float) yOffset));
        }
    }

    void clipToImageAlpha (const Image& image, const Rectangle<int>& srcClip, const AffineTransform& t)
    {
        if (clip != 0)
        {
            if (image.hasAlphaChannel())
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToImageAlpha (image, srcClip, t.translated ((float) xOffset, (float) yOffset),
                                               interpolationQuality != Graphics::lowResamplingQuality);
            }
            else
            {
                Path p;
                p.addRectangle (srcClip);
                clipToPath (p, t);
            }
        }
    }

    bool clipRegionIntersects (const Rectangle<int>& r) const
    {
        return clip != 0 && clip->clipRegionIntersects (r.translated (xOffset, yOffset));
    }

    const Rectangle<int> getClipBounds() const
    {
        return clip == 0 ? Rectangle<int>() : clip->getClipBounds().translated (-xOffset, -yOffset);
    }

    //==============================================================================
    void fillRect (Image& image, const Rectangle<int>& r, const bool replaceContents)
    {
        if (clip != 0)
        {
            if (fillType.isColour())
            {
                Image::BitmapData destData (image, 0, 0, image.getWidth(), image.getHeight(), true);
                clip->fillRectWithColour (destData, r.translated (xOffset, yOffset), fillType.colour.getPixelARGB(), replaceContents);
            }
            else
            {
                const Rectangle<int> totalClip (clip->getClipBounds());
                const Rectangle<int> clipped (totalClip.getIntersection (r.translated (xOffset, yOffset)));

                if (! clipped.isEmpty())
                    fillShape (image, new SoftwareRendererClasses::ClipRegion_RectangleList (clipped), false);
            }
        }
    }

    void fillRect (Image& image, const Rectangle<float>& r)
    {
        if (clip != 0)
        {
            if (fillType.isColour())
            {
                Image::BitmapData destData (image, 0, 0, image.getWidth(), image.getHeight(), true);
                clip->fillRectWithColour (destData, r.translated ((float) xOffset, (float) yOffset), fillType.colour.getPixelARGB());
            }
            else
            {
                const Rectangle<float> totalClip (clip->getClipBounds().toFloat());
                const Rectangle<float> clipped (totalClip.getIntersection (r.translated ((float) xOffset, (float) yOffset)));

                if (! clipped.isEmpty())
                    fillShape (image, new SoftwareRendererClasses::ClipRegion_EdgeTable (clipped), false);
            }
        }
    }

    void fillPath (Image& image, const Path& path, const AffineTransform& transform)
    {
        if (clip != 0)
            fillShape (image, new SoftwareRendererClasses::ClipRegion_EdgeTable (clip->getClipBounds(), path, transform.translated ((float) xOffset, (float) yOffset)), false);
    }

    void fillEdgeTable (Image& image, const EdgeTable& edgeTable, const float x, const int y)
    {
        if (clip != 0)
        {
            SoftwareRendererClasses::ClipRegion_EdgeTable* edgeTableClip = new SoftwareRendererClasses::ClipRegion_EdgeTable (edgeTable);
            SoftwareRendererClasses::ClipRegionBase::Ptr shapeToFill (edgeTableClip);
            edgeTableClip->edgeTable.translate (x + xOffset, y + yOffset);
            fillShape (image, shapeToFill, false);
        }
    }

    void fillShape (Image& image, SoftwareRendererClasses::ClipRegionBase::Ptr shapeToFill, const bool replaceContents)
    {
        jassert (clip != 0);

        shapeToFill = clip->applyClipTo (shapeToFill);

        if (shapeToFill != 0)
        {
            Image::BitmapData destData (image, 0, 0, image.getWidth(), image.getHeight(), true);

            if (fillType.isGradient())
            {
                jassert (! replaceContents); // that option is just for solid colours

                ColourGradient g2 (*(fillType.gradient));
                g2.multiplyOpacity (fillType.getOpacity());
                g2.point1.addXY (-0.5f, -0.5f);
                g2.point2.addXY (-0.5f, -0.5f);
                AffineTransform transform (fillType.transform.translated ((float) xOffset, (float) yOffset));
                const bool isIdentity = transform.isOnlyTranslation();

                if (isIdentity)
                {
                    // If our translation doesn't involve any distortion, we can speed it up..
                    g2.point1.applyTransform (transform);
                    g2.point2.applyTransform (transform);
                    transform = AffineTransform::identity;
                }

                shapeToFill->fillAllWithGradient (destData, g2, transform, isIdentity);
            }
            else if (fillType.isTiledImage())
            {
                renderImage (image, *(fillType.image), fillType.image->getBounds(), fillType.transform, shapeToFill);
            }
            else
            {
                shapeToFill->fillAllWithColour (destData, fillType.colour.getPixelARGB(), replaceContents);
            }
        }
    }

    //==============================================================================
    void renderImage (Image& destImage, const Image& sourceImage, const Rectangle<int>& srcClip,
                      const AffineTransform& t, const SoftwareRendererClasses::ClipRegionBase* const tiledFillClipRegion)
    {
        const AffineTransform transform (t.translated ((float) xOffset, (float) yOffset));

        const Image::BitmapData destData (destImage, 0, 0, destImage.getWidth(), destImage.getHeight(), true);
        const Image::BitmapData srcData (sourceImage, srcClip.getX(), srcClip.getY(), srcClip.getWidth(), srcClip.getHeight());
        const int alpha = fillType.colour.getAlpha();
        const bool betterQuality = (interpolationQuality != Graphics::lowResamplingQuality);

        if (transform.isOnlyTranslation())
        {
            // If our translation doesn't involve any distortion, just use a simple blit..
            int tx = (int) (transform.getTranslationX() * 256.0f);
            int ty = (int) (transform.getTranslationY() * 256.0f);

            if ((! betterQuality) || ((tx | ty) & 224) == 0)
            {
                tx = ((tx + 128) >> 8);
                ty = ((ty + 128) >> 8);

                if (tiledFillClipRegion != 0)
                {
                    tiledFillClipRegion->renderImageUntransformed (destData, srcData, alpha, tx, ty, true);
                }
                else
                {
                    SoftwareRendererClasses::ClipRegionBase::Ptr c (new SoftwareRendererClasses::ClipRegion_EdgeTable (Rectangle<int> (tx, ty, srcClip.getWidth(), srcClip.getHeight()).getIntersection (destImage.getBounds())));
                    c = clip->applyClipTo (c);

                    if (c != 0)
                        c->renderImageUntransformed (destData, srcData, alpha, tx, ty, false);
                }

                return;
            }
        }

        if (transform.isSingularity())
            return;

        if (tiledFillClipRegion != 0)
        {
            tiledFillClipRegion->renderImageTransformed (destData, srcData, alpha, transform, betterQuality, true);
        }
        else
        {
            Path p;
            p.addRectangle (0.0f, 0.0f, (float) srcClip.getWidth(), (float) srcClip.getHeight());

            SoftwareRendererClasses::ClipRegionBase::Ptr c (clip->clone());
            c = c->clipToPath (p, transform);

            if (c != 0)
                c->renderImageTransformed (destData, srcData, alpha, transform, betterQuality, true);
        }
    }

    //==============================================================================
    SoftwareRendererClasses::ClipRegionBase::Ptr clip;
    int xOffset, yOffset;
    Font font;
    FillType fillType;
    Graphics::ResamplingQuality interpolationQuality;

private:
    void cloneClipIfMultiplyReferenced()
    {
        if (clip->getReferenceCount() > 1)
            clip = clip->clone();
    }

    SavedState& operator= (const SavedState&);
};


//==============================================================================
LowLevelGraphicsSoftwareRenderer::LowLevelGraphicsSoftwareRenderer (Image& image_)
    : image (image_)
{
    currentState = new SavedState (image_.getBounds(), 0, 0);
}

LowLevelGraphicsSoftwareRenderer::LowLevelGraphicsSoftwareRenderer (Image& image_, const int xOffset, const int yOffset,
                                                                    const RectangleList& initialClip)
    : image (image_)
{
    currentState = new SavedState (initialClip, xOffset, yOffset);
}

LowLevelGraphicsSoftwareRenderer::~LowLevelGraphicsSoftwareRenderer()
{
}

bool LowLevelGraphicsSoftwareRenderer::isVectorDevice() const
{
    return false;
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::setOrigin (int x, int y)
{
    currentState->setOrigin (x, y);
}

bool LowLevelGraphicsSoftwareRenderer::clipToRectangle (const Rectangle<int>& r)
{
    return currentState->clipToRectangle (r);
}

bool LowLevelGraphicsSoftwareRenderer::clipToRectangleList (const RectangleList& clipRegion)
{
    return currentState->clipToRectangleList (clipRegion);
}

void LowLevelGraphicsSoftwareRenderer::excludeClipRectangle (const Rectangle<int>& r)
{
    currentState->excludeClipRectangle (r);
}

void LowLevelGraphicsSoftwareRenderer::clipToPath (const Path& path, const AffineTransform& transform)
{
    currentState->clipToPath (path, transform);
}

void LowLevelGraphicsSoftwareRenderer::clipToImageAlpha (const Image& sourceImage, const Rectangle<int>& srcClip, const AffineTransform& transform)
{
    currentState->clipToImageAlpha (sourceImage, srcClip, transform);
}

bool LowLevelGraphicsSoftwareRenderer::clipRegionIntersects (const Rectangle<int>& r)
{
    return currentState->clipRegionIntersects (r);
}

const Rectangle<int> LowLevelGraphicsSoftwareRenderer::getClipBounds() const
{
    return currentState->getClipBounds();
}

bool LowLevelGraphicsSoftwareRenderer::isClipEmpty() const
{
    return currentState->clip == 0;
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::saveState()
{
    stateStack.add (new SavedState (*currentState));
}

void LowLevelGraphicsSoftwareRenderer::restoreState()
{
    SavedState* const top = stateStack.getLast();

    if (top != 0)
    {
        currentState = top;
        stateStack.removeLast (1, false);
    }
    else
    {
        jassertfalse; // trying to pop with an empty stack!
    }
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::setFill (const FillType& fillType)
{
    currentState->fillType = fillType;
}

void LowLevelGraphicsSoftwareRenderer::setOpacity (float newOpacity)
{
    currentState->fillType.setOpacity (newOpacity);
}

void LowLevelGraphicsSoftwareRenderer::setInterpolationQuality (Graphics::ResamplingQuality quality)
{
    currentState->interpolationQuality = quality;
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::fillRect (const Rectangle<int>& r, const bool replaceExistingContents)
{
    currentState->fillRect (image, r, replaceExistingContents);
}

void LowLevelGraphicsSoftwareRenderer::fillPath (const Path& path, const AffineTransform& transform)
{
    currentState->fillPath (image, path, transform);
}

void LowLevelGraphicsSoftwareRenderer::drawImage (const Image& sourceImage, const Rectangle<int>& srcClip,
                                                  const AffineTransform& transform, const bool fillEntireClipAsTiles)
{
    jassert (sourceImage.getBounds().contains (srcClip));

    currentState->renderImage (image, sourceImage, srcClip, transform,
                               fillEntireClipAsTiles ? currentState->clip : 0);
}

void LowLevelGraphicsSoftwareRenderer::drawLine (const Line <float>& line)
{
    Path p;
    p.addLineSegment (line.getStartX(), line.getStartY(), line.getEndX(), line.getEndY(), 1.0f);
    fillPath (p, AffineTransform::identity);
}

void LowLevelGraphicsSoftwareRenderer::drawVerticalLine (const int x, float top, float bottom)
{
    if (bottom > top)
        currentState->fillRect (image, Rectangle<float> ((float) x, top, 1.0f, bottom - top));
}

void LowLevelGraphicsSoftwareRenderer::drawHorizontalLine (const int y, float left, float right)
{
    if (right > left)
        currentState->fillRect (image, Rectangle<float> (left, (float) y, right - left, 1.0f));
}

//==============================================================================
class LowLevelGraphicsSoftwareRenderer::CachedGlyph
{
public:
    CachedGlyph() : glyph (0), lastAccessCount (0) {}
    ~CachedGlyph()  {}

    void draw (SavedState& state, Image& image, const float x, const float y) const
    {
        if (edgeTable != 0)
            state.fillEdgeTable (image, *edgeTable, x, roundToInt (y));
    }

    void generate (const Font& newFont, const int glyphNumber)
    {
        font = newFont;
        glyph = glyphNumber;
        edgeTable = 0;

        Path glyphPath;
        font.getTypeface()->getOutlineForGlyph (glyphNumber, glyphPath);

        if (! glyphPath.isEmpty())
        {
            const float fontHeight = font.getHeight();
            const AffineTransform transform (AffineTransform::scale (fontHeight * font.getHorizontalScale(), fontHeight)
                                                             .translated (0.0f, -0.5f));

            edgeTable = new EdgeTable (glyphPath.getBoundsTransformed (transform).getSmallestIntegerContainer().expanded (1, 0),
                                       glyphPath, transform);
        }
    }

    int glyph, lastAccessCount;
    Font font;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ScopedPointer <EdgeTable> edgeTable;

    CachedGlyph (const CachedGlyph&);
    CachedGlyph& operator= (const CachedGlyph&);
};

//==============================================================================
class LowLevelGraphicsSoftwareRenderer::GlyphCache  : private DeletedAtShutdown
{
public:
    GlyphCache()
        : accessCounter (0), hits (0), misses (0)
    {
        for (int i = 120; --i >= 0;)
            glyphs.add (new CachedGlyph());
    }

    ~GlyphCache()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (GlyphCache);

    //==============================================================================
    void drawGlyph (SavedState& state, Image& image, const Font& font, const int glyphNumber, float x, float y)
    {
        ++accessCounter;
        int oldestCounter = std::numeric_limits<int>::max();
        CachedGlyph* oldest = 0;

        for (int i = glyphs.size(); --i >= 0;)
        {
            CachedGlyph* const glyph = glyphs.getUnchecked (i);

            if (glyph->glyph == glyphNumber && glyph->font == font)
            {
                ++hits;
                glyph->lastAccessCount = accessCounter;
                glyph->draw (state, image, x, y);
                return;
            }

            if (glyph->lastAccessCount <= oldestCounter)
            {
                oldestCounter = glyph->lastAccessCount;
                oldest = glyph;
            }
        }

        if (hits + ++misses > (glyphs.size() << 4))
        {
            if (misses * 2 > hits)
            {
                for (int i = 32; --i >= 0;)
                    glyphs.add (new CachedGlyph());
            }

            hits = misses = 0;
            oldest = glyphs.getLast();
        }

        jassert (oldest != 0);
        oldest->lastAccessCount = accessCounter;
        oldest->generate (font, glyphNumber);
        oldest->draw (state, image, x, y);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class OwnedArray <CachedGlyph>;
    OwnedArray <CachedGlyph> glyphs;
    int accessCounter, hits, misses;

    GlyphCache (const GlyphCache&);
    GlyphCache& operator= (const GlyphCache&);
};

juce_ImplementSingleton_SingleThreaded (LowLevelGraphicsSoftwareRenderer::GlyphCache);


void LowLevelGraphicsSoftwareRenderer::setFont (const Font& newFont)
{
    currentState->font = newFont;
}

const Font LowLevelGraphicsSoftwareRenderer::getFont()
{
    return currentState->font;
}

void LowLevelGraphicsSoftwareRenderer::drawGlyph (int glyphNumber, const AffineTransform& transform)
{
    Font& f = currentState->font;

    if (transform.isOnlyTranslation())
    {
        GlyphCache::getInstance()->drawGlyph (*currentState, image, f, glyphNumber,
                                              transform.getTranslationX(),
                                              transform.getTranslationY());
    }
    else
    {
        Path p;
        f.getTypeface()->getOutlineForGlyph (glyphNumber, p);
        fillPath (p, AffineTransform::scale (f.getHeight() * f.getHorizontalScale(), f.getHeight()).followedBy (transform));
    }
}

#if JUCE_MSVC
 #pragma warning (pop)

 #if JUCE_DEBUG
  #pragma optimize ("", on)  // resets optimisations to the project defaults
 #endif
#endif

END_JUCE_NAMESPACE
