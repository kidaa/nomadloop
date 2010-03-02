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

#ifndef __JUCE_RECTANGLE_JUCEHEADER__
#define __JUCE_RECTANGLE_JUCEHEADER__

#include "../../../text/juce_StringArray.h"
#include "juce_Point.h"
class RectangleList;


//==============================================================================
/**
    Manages a rectangle and allows geometric operations to be performed on it.

    @see RectangleList, Path, Line, Point
*/
template <typename ValueType>
class Rectangle
{
public:
    //==============================================================================
    /** Creates a rectangle of zero size.

        The default co-ordinates will be (0, 0, 0, 0).
    */
    Rectangle() throw()
      : x (0), y (0), w (0), h (0)
    {
    }

    /** Creates a copy of another rectangle. */
    Rectangle (const Rectangle& other) throw()
      : x (other.x), y (other.y),
        w (other.w), h (other.h)
    {
    }

    /** Creates a rectangle with a given position and size. */
    Rectangle (const ValueType initialX, const ValueType initialY,
               const ValueType width, const ValueType height) throw()
      : x (initialX), y (initialY),
        w (width), h (height)
    {
    }

    /** Creates a rectangle with a given size, and a position of (0, 0). */
    Rectangle (const ValueType width, const ValueType height) throw()
      : x (0), y (0), w (width), h (height)
    {
    }

    Rectangle& operator= (const Rectangle& other) throw()
    {
        x = other.x; y = other.y;
        w = other.w; h = other.h;
        return *this;
    }

    /** Destructor. */
    ~Rectangle() throw() {}

    //==============================================================================
    /** Returns true if the rectangle's width and height are both zero or less */
    bool isEmpty() const throw()                                    { return w <= 0 || h <= 0; }

    /** Returns the x co-ordinate of the rectangle's left-hand-side. */
    inline ValueType getX() const throw()                           { return x; }

    /** Returns the y co-ordinate of the rectangle's top edge. */
    inline ValueType getY() const throw()                           { return y; }

    /** Returns the width of the rectangle. */
    inline ValueType getWidth() const throw()                       { return w; }

    /** Returns the height of the rectangle. */
    inline ValueType getHeight() const throw()                      { return h; }

    /** Returns the x co-ordinate of the rectangle's right-hand-side. */
    inline ValueType getRight() const throw()                       { return x + w; }

    /** Returns the y co-ordinate of the rectangle's bottom edge. */
    inline ValueType getBottom() const throw()                      { return y + h; }

    /** Returns the x co-ordinate of the rectangle's centre. */
    ValueType getCentreX() const throw()                            { return x + w / (ValueType) 2; }

    /** Returns the y co-ordinate of the rectangle's centre. */
    ValueType getCentreY() const throw()                            { return y + h / (ValueType) 2; }

    /** Returns the centre point of the rectangle. */
    const Point<ValueType> getCentre() const throw()                { return Point<ValueType> (x + w / (ValueType) 2, y + h / (ValueType) 2); }

    /** Returns the aspect ratio of the rectangle's width / height.
        If widthOverHeight is true, it returns width / height; if widthOverHeight is false,
        it returns height / width. */
    ValueType getAspectRatio (const bool widthOverHeight = true) const throw()                      { return widthOverHeight ? w / h : h / w; }

    //==============================================================================
    /** Returns the rectangle's top-left position as a Point. */
    const Point<ValueType> getPosition() const throw()                                              { return Point<ValueType> (x, y); }

    /** Changes the position of the rectangle's top-left corner (leaving its size unchanged). */
    void setPosition (const Point<ValueType>& newPos) throw()                                       { x = newPos.getX(); y = newPos.getY(); }

    /** Changes the position of the rectangle's top-left corner (leaving its size unchanged). */
    void setPosition (const ValueType newX, const ValueType newY) throw()                           { x = newX; y = newY; }

    /** Returns a rectangle with the same size as this one, but a new position. */
    const Rectangle withPosition (const Point<ValueType>& newPos) const throw()                     { return Rectangle (newPos.getX(), newPos.getY(), w, h); }

    /** Changes the rectangle's size, leaving the position of its top-left corner unchanged. */
    void setSize (const ValueType newWidth, const ValueType newHeight) throw()                      { w = newWidth; h = newHeight; }

    /** Returns a rectangle with the same position as this one, but a new size. */
    const Rectangle withSize (const ValueType newWidth, const ValueType newHeight) const throw()    { return Rectangle (x, y, newWidth, newHeight); }

    /** Changes all the rectangle's co-ordinates. */
    void setBounds (const ValueType newX, const ValueType newY,
                    const ValueType newWidth, const ValueType newHeight) throw()
    {
        x = newX; y = newY; w = newWidth; h = newHeight;
    }

    /** Changes the rectangle's width */
    void setWidth (const ValueType newWidth) throw()                { w = newWidth; }

    /** Changes the rectangle's height */
    void setHeight (const ValueType newHeight) throw()              { h = newHeight; }

    /** Moves the x position, adjusting the width so that the right-hand edge remains in the same place.
        If the x is moved to be on the right of the current right-hand edge, the width will be set to zero.
    */
    void setLeft (const ValueType newLeft) throw()
    {
        w = jmax (ValueType(), x + w - newLeft);
        x = newLeft;
    }

    /** Moves the y position, adjusting the height so that the bottom edge remains in the same place.
        If the y is moved to be below the current bottom edge, the height will be set to zero.
    */
    void setTop (const ValueType newTop) throw()
    {
        h = jmax (ValueType(), y + h - newTop);
        y = newTop;
    }

    /** Adjusts the width so that the right-hand edge of the rectangle has this new value.
        If the new right is below the current X value, the X will be pushed down to match it.
        @see getRight
    */
    void setRight (const ValueType newRight) throw()
    {
        x = jmin (x, newRight);
        w = newRight - x;
    }

    /** Adjusts the height so that the bottom edge of the rectangle has this new value.
        If the new bottom is lower than the current Y value, the Y will be pushed down to match it.
        @see getBottom
    */
    void setBottom (const ValueType newBottom) throw()
    {
        y = jmin (y, newBottom);
        h = newBottom - y;
    }

    //==============================================================================
    /** Moves the rectangle's position by adding amount to its x and y co-ordinates. */
    void translate (const ValueType deltaX,
                    const ValueType deltaY) throw()
    {
        x += deltaX;
        y += deltaY;
    }

    /** Returns a rectangle which is the same as this one moved by a given amount. */
    const Rectangle translated (const ValueType deltaX,
                                const ValueType deltaY) const throw()
    {
        return Rectangle (x + deltaX, y + deltaY, w, h);
    }

    /** Expands the rectangle by a given amount.

        Effectively, its new size is (x - deltaX, y - deltaY, w + deltaX * 2, h + deltaY * 2).
        @see expanded, reduce, reduced
    */
    void expand (const ValueType deltaX,
                 const ValueType deltaY) throw()
    {
        const ValueType nw = jmax (ValueType(), w + deltaX * 2);
        const ValueType nh = jmax (ValueType(), h + deltaY * 2);
        setBounds (x - deltaX, y - deltaY, nw, nh);
    }

    /** Returns a rectangle that is larger than this one by a given amount.

        Effectively, the rectangle returned is (x - deltaX, y - deltaY, w + deltaX * 2, h + deltaY * 2).
        @see expand, reduce, reduced
    */
    const Rectangle expanded (const ValueType deltaX,
                              const ValueType deltaY) const throw()
    {
        const ValueType nw = jmax (ValueType(), w + deltaX * 2);
        const ValueType nh = jmax (ValueType(), h + deltaY * 2);
        return Rectangle (x - deltaX, y - deltaY, nw, nh);
    }

    /** Shrinks the rectangle by a given amount.

        Effectively, its new size is (x + deltaX, y + deltaY, w - deltaX * 2, h - deltaY * 2).
        @see reduced, expand, expanded
    */
    void reduce (const ValueType deltaX,
                 const ValueType deltaY) throw()
    {
        expand (-deltaX, -deltaY);
    }

    /** Returns a rectangle that is smaller than this one by a given amount.

        Effectively, the rectangle returned is (x + deltaX, y + deltaY, w - deltaX * 2, h - deltaY * 2).
        @see reduce, expand, expanded
    */
    const Rectangle reduced (const ValueType deltaX,
                             const ValueType deltaY) const throw()
    {
        return expanded (-deltaX, -deltaY);
    }

    //==============================================================================
    /** Returns true if the two rectangles are identical. */
    bool operator== (const Rectangle& other) const throw()
    {
        return x == other.x && y == other.y
            && w == other.w && h == other.h;
    }

    /** Returns true if the two rectangles are not identical. */
    bool operator!= (const Rectangle& other) const throw()
    {
        return x != other.x || y != other.y
            || w != other.w || h != other.h;
    }

    /** Returns true if this co-ordinate is inside the rectangle. */
    bool contains (const ValueType xCoord, const ValueType yCoord) const throw()
    {
        return xCoord >= x && yCoord >= y && xCoord < x + w && yCoord < y + h;
    }

    /** Returns true if this co-ordinate is inside the rectangle. */
    bool contains (const Point<ValueType> point) const throw()
    {
        return point.getX() >= x && point.getY() >= y && point.getX() < x + w && point.getY() < y + h;
    }

    /** Returns true if this other rectangle is completely inside this one. */
    bool contains (const Rectangle& other) const throw()
    {
        return x <= other.x && y <= other.y
            && x + w >= other.x + other.w && y + h >= other.y + other.h;
    }

    /** Returns the nearest point to the specified point that lies within this rectangle. */
    const Point<ValueType> getConstrainedPoint (const Point<ValueType>& point) const throw()
    {
        return Point<ValueType> (jlimit (x, x + w, point.getX()),
                                 jlimit (y, y + h, point.getY()));
    }

    /** Returns true if any part of another rectangle overlaps this one. */
    bool intersects (const Rectangle& other) const throw()
    {
        return x + w > other.x
            && y + h > other.y
            && x < other.x + other.w
            && y < other.y + other.h
            && w > ValueType() && h > ValueType();
    }

    /** Returns the region that is the overlap between this and another rectangle.

        If the two rectangles don't overlap, the rectangle returned will be empty.
    */
    const Rectangle getIntersection (const Rectangle& other) const throw()
    {
        const ValueType nx = jmax (x, other.x);
        const ValueType ny = jmax (y, other.y);
        const ValueType nw = jmin (x + w, other.x + other.w) - nx;
        const ValueType nh = jmin (y + h, other.y + other.h) - ny;

        if (nw >= ValueType() && nh >= ValueType())
            return Rectangle (nx, ny, nw, nh);

        return Rectangle();
    }

    /** Clips a rectangle so that it lies only within this one.

        This is a non-static version of intersectRectangles().

        Returns false if the two regions didn't overlap.
    */
    bool intersectRectangle (ValueType& otherX, ValueType& otherY, ValueType& otherW, ValueType& otherH) const throw()
    {
        const int maxX = jmax (otherX, x);
        otherW = jmin (otherX + otherW, x + w) - maxX;

        if (otherW > 0)
        {
            const int maxY = jmax (otherY, y);
            otherH = jmin (otherY + otherH, y + h) - maxY;

            if (otherH > 0)
            {
                otherX = maxX; otherY = maxY;
                return true;
            }
        }

        return false;
    }

    /** Returns the smallest rectangle that contains both this one and the one
        passed-in.
    */
    const Rectangle getUnion (const Rectangle& other) const throw()
    {
        const ValueType newX = jmin (x, other.x);
        const ValueType newY = jmin (y, other.y);

        return Rectangle (newX, newY,
                          jmax (x + w, other.x + other.w) - newX,
                          jmax (y + h, other.y + other.h) - newY);
    }

    /** If this rectangle merged with another one results in a simple rectangle, this
        will set this rectangle to the result, and return true.

        Returns false and does nothing to this rectangle if the two rectangles don't overlap,
        or if they form a complex region.
    */
    bool enlargeIfAdjacent (const Rectangle& other) throw()
    {
        if (x == other.x && getRight() == other.getRight()
             && (other.getBottom() >= y && other.y <= getBottom()))
        {
            const ValueType newY = jmin (y, other.y);
            h = jmax (getBottom(), other.getBottom()) - newY;
            y = newY;
            return true;
        }
        else if (y == other.y && getBottom() == other.getBottom()
                  && (other.getRight() >= x && other.x <= getRight()))
        {
            const ValueType newX = jmin (x, other.x);
            w = jmax (getRight(), other.getRight()) - newX;
            x = newX;
            return true;
        }

        return false;
    }

    /** If after removing another rectangle from this one the result is a simple rectangle,
        this will set this object's bounds to be the result, and return true.

        Returns false and does nothing to this rectangle if the two rectangles don't overlap,
        or if removing the other one would form a complex region.
    */
    bool reduceIfPartlyContainedIn (const Rectangle& other) throw()
    {
        int inside = 0;
        const int otherR = other.getRight();
        if (x >= other.x && x < otherR) inside = 1;
        const int otherB = other.getBottom();
        if (y >= other.y && y < otherB) inside |= 2;
        const int r = x + w;
        if (r >= other.x && r < otherR) inside |= 4;
        const int b = y + h;
        if (b >= other.y && b < otherB) inside |= 8;

        switch (inside)
        {
            case 1 + 2 + 8:     w = r - otherR; x = otherR; return true;
            case 1 + 2 + 4:     h = b - otherB; y = otherB; return true;
            case 2 + 4 + 8:     w = other.x - x; return true;
            case 1 + 4 + 8:     h = other.y - y; return true;
        }

        return false;
    }

    /** Returns the smallest rectangle that can contain the shape created by applying
        a transform to this rectangle.

        This should only be used on floating point rectangles.
    */
    const Rectangle<ValueType> transformed (const AffineTransform& transform) const throw()
    {
        float x1 = x,     y1 = y;
        float x2 = x + w, y2 = y;
        float x3 = x,     y3 = y + h;
        float x4 = x2,    y4 = y3;

        transform.transformPoint (x1, y1);
        transform.transformPoint (x2, y2);
        transform.transformPoint (x3, y3);
        transform.transformPoint (x4, y4);

        const float x = jmin (x1, x2, x3, x4);
        const float y = jmin (y1, y2, y3, y4);

        return Rectangle (x, y,
                          jmax (x1, x2, x3, x4) - x,
                          jmax (y1, y2, y3, y4) - y);
    }

    /** Returns the smallest integer-aligned rectangle that completely contains this one.
        This is only relevent for floating-point rectangles, of course.
    */
    const Rectangle<int> getSmallestIntegerContainer() const throw()
    {
        const int x1 = (int) floorf ((float) x);
        const int y1 = (int) floorf ((float) y);
        const int x2 = (int) floorf ((float) (x + w + 0.9999f));
        const int y2 = (int) floorf ((float) (y + h + 0.9999f));

        return Rectangle<int> (x1, y1, x2 - x1, y2 - y1);
    }

    //==============================================================================
    /** Static utility to intersect two sets of rectangular co-ordinates.

        Returns false if the two regions didn't overlap.

        @see intersectRectangle
    */
    static bool intersectRectangles (ValueType& x1, ValueType& y1, ValueType& w1, ValueType& h1,
                                     const ValueType x2, const ValueType y2, const ValueType w2, const ValueType h2) throw()
    {
        const ValueType x = jmax (x1, x2);
        w1 = jmin (x1 + w1, x2 + w2) - x;

        if (w1 > 0)
        {
            const ValueType y = jmax (y1, y2);
            h1 = jmin (y1 + h1, y2 + h2) - y;

            if (h1 > 0)
            {
                x1 = x; y1 = y;
                return true;
            }
        }

        return false;
    }

    //==============================================================================
    /** Creates a string describing this rectangle.

        The string will be of the form "x y width height", e.g. "100 100 400 200".

        Coupled with the fromString() method, this is very handy for things like
        storing rectangles (particularly component positions) in XML attributes.

        @see fromString
    */
    const String toString() const
    {
        String s;
        s.preallocateStorage (16);
        s << x << T(' ') << y << T(' ') << w << T(' ') << h;
        return s;
    }

    /** Parses a string containing a rectangle's details.

        The string should contain 4 integer tokens, in the form "x y width height". They
        can be comma or whitespace separated.

        This method is intended to go with the toString() method, to form an easy way
        of saving/loading rectangles as strings.

        @see toString
    */
    static const Rectangle fromString (const String& stringVersion)
    {
        StringArray toks;
        toks.addTokens (stringVersion.trim(), T(",; \t\r\n"), 0);

        return Rectangle (toks[0].trim().getIntValue(),
                          toks[1].trim().getIntValue(),
                          toks[2].trim().getIntValue(),
                          toks[3].trim().getIntValue());
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class RectangleList;
    ValueType x, y, w, h;
};


#endif   // __JUCE_RECTANGLE_JUCEHEADER__
