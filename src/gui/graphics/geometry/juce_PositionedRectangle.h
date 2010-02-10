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

#ifndef __JUCE_POSITIONEDRECTANGLE_JUCEHEADER__
#define __JUCE_POSITIONEDRECTANGLE_JUCEHEADER__

#include "../../components/juce_Component.h"


//==============================================================================
/**
    A rectangle whose co-ordinates can be defined in terms of absolute or
    proportional distances.

    Designed mainly for storing component positions, this gives you a lot of
    control over how each co-ordinate is stored, either as an absolute position,
    or as a proportion of the size of a parent rectangle.

    It also allows you to define the anchor points by which the rectangle is
    positioned, so for example you could specify that the top right of the
    rectangle should be an absolute distance from its parent's bottom-right corner.

    This object can be stored as a string, which takes the form "x y w h", including
    symbols like '%' and letters to indicate the anchor point. See its toString()
    method for more info.

    Example usage:
    @code
    class MyComponent
    {
        void resized()
        {
            // this will set the child component's x to be 20% of our width, its y
            // to be 30, its width to be 150, and its height to be 50% of our
            // height..
            const PositionedRectangle pos1 ("20% 30 150 50%");
            pos1.applyToComponent (*myChildComponent1);

            // this will inset the child component with a gap of 10 pixels
            // around each of its edges..
            const PositionedRectangle pos2 ("10 10 20M 20M");
            pos2.applyToComponent (*myChildComponent2);
        }
    };
    @endcode
*/
class JUCE_API  PositionedRectangle
{
public:
    //==============================================================================
    /** Creates an empty rectangle with all co-ordinates set to zero.

        The default anchor point is top-left; the default
    */
    PositionedRectangle() throw();

    /** Initialises a PositionedRectangle from a saved string version.

        The string must be in the format generated by toString().
    */
    PositionedRectangle (const String& stringVersion) throw();

    /** Creates a copy of another PositionedRectangle. */
    PositionedRectangle (const PositionedRectangle& other) throw();

    /** Copies another PositionedRectangle. */
    const PositionedRectangle& operator= (const PositionedRectangle& other) throw();

    /** Destructor. */
    ~PositionedRectangle() throw();

    //==============================================================================
    /** Returns a string version of this position, from which it can later be
        re-generated.

        The format is four co-ordinates, "x y w h".

        - If a co-ordinate is absolute, it is stored as an integer, e.g. "100".
        - If a co-ordinate is proportional to its parent's width or height, it is stored
          as a percentage, e.g. "80%".
        - If the X or Y co-ordinate is relative to the parent's right or bottom edge, the
          number has "R" appended to it, e.g. "100R" means a distance of 100 pixels from
          the parent's right-hand edge.
        - If the X or Y co-ordinate is relative to the parent's centre, the number has "C"
          appended to it, e.g. "-50C" would be 50 pixels left of the parent's centre.
        - If the X or Y co-ordinate should be anchored at the component's right or bottom
          edge, then it has "r" appended to it. So "-50Rr" would mean that this component's
          right-hand edge should be 50 pixels left of the parent's right-hand edge.
        - If the X or Y co-ordinate should be anchored at the component's centre, then it
          has "c" appended to it. So "-50Rc" would mean that this component's
          centre should be 50 pixels left of the parent's right-hand edge. "40%c" means that
          this component's centre should be placed 40% across the parent's width.
        - If it's a width or height that should use the parentSizeMinusAbsolute mode, then
          the number has "M" appended to it.

        To reload a stored string, use the constructor that takes a string parameter.
    */
    const String toString() const throw();

    //==============================================================================
    /** Calculates the absolute position, given the size of the space that
        it should go in.

        This will work out any proportional distances and sizes relative to the
        target rectangle, and will return the absolute position.

        @see applyToComponent
    */
    const Rectangle<int> getRectangle (const Rectangle<int>& targetSpaceToBeRelativeTo) const throw();

    /** Same as getRectangle(), but returning the values as doubles rather than ints.
    */
    void getRectangleDouble (const Rectangle<int>& targetSpaceToBeRelativeTo,
                             double& x,
                             double& y,
                             double& width,
                             double& height) const throw();

    /** This sets the bounds of the given component to this position.

        This is equivalent to writing:
        @code
        comp.setBounds (getRectangle (Rectangle<int> (0, 0, comp.getParentWidth(), comp.getParentHeight())));
        @endcode

        @see getRectangle, updateFromComponent
    */
    void applyToComponent (Component& comp) const throw();

    //==============================================================================
    /** Updates this object's co-ordinates to match the given rectangle.

        This will set all co-ordinates based on the given rectangle, re-calculating
        any proportional distances, and using the current anchor points.

        So for example if the x co-ordinate mode is currently proportional, this will
        re-calculate x based on the rectangle's relative position within the target
        rectangle's width.

        If the target rectangle's width or height are zero then it may not be possible
        to re-calculate some proportional co-ordinates. In this case, those co-ordinates
        will not be changed.
    */
    void updateFrom (const Rectangle<int>& newPosition,
                     const Rectangle<int>& targetSpaceToBeRelativeTo) throw();

    /** Same functionality as updateFrom(), but taking doubles instead of ints.
    */
    void updateFromDouble (const double x, const double y,
                           const double width, const double height,
                           const Rectangle<int>& targetSpaceToBeRelativeTo) throw();

    /** Updates this object's co-ordinates to match the bounds of this component.

        This is equivalent to calling updateFrom() with the component's bounds and
        it parent size.

        If the component doesn't currently have a parent, then proportional co-ordinates
        might not be updated because it would need to know the parent's size to do the
        maths for this.
    */
    void updateFromComponent (const Component& comp) throw();

    //==============================================================================
    /** Specifies the point within the rectangle, relative to which it should be positioned. */
    enum AnchorPoint
    {
        anchorAtLeftOrTop              = 1 << 0,    /**< The x or y co-ordinate specifies where the left or top edge of the rectangle should be. */
        anchorAtRightOrBottom          = 1 << 1,    /**< The x or y co-ordinate specifies where the right or bottom edge of the rectangle should be. */
        anchorAtCentre                 = 1 << 2     /**< The x or y co-ordinate specifies where the centre of the rectangle should be. */
    };

    /** Specifies how an x or y co-ordinate should be interpreted. */
    enum PositionMode
    {
        absoluteFromParentTopLeft       = 1 << 3,   /**< The x or y co-ordinate specifies an absolute distance from the parent's top or left edge. */
        absoluteFromParentBottomRight   = 1 << 4,   /**< The x or y co-ordinate specifies an absolute distance from the parent's bottom or right edge. */
        absoluteFromParentCentre        = 1 << 5,   /**< The x or y co-ordinate specifies an absolute distance from the parent's centre. */
        proportionOfParentSize          = 1 << 6    /**< The x or y co-ordinate specifies a proportion of the parent's width or height, measured from the parent's top or left. */
    };

    /** Specifies how the width or height should be interpreted. */
    enum SizeMode
    {
        absoluteSize                    = 1 << 0,   /**< The width or height specifies an absolute size. */
        parentSizeMinusAbsolute         = 1 << 1,   /**< The width or height is an amount that should be subtracted from the parent's width or height. */
        proportionalSize                = 1 << 2,   /**< The width or height specifies a proportion of the parent's width or height. */
    };

    //==============================================================================
    /** Sets all options for all co-ordinates.

        This requires a reference rectangle to be specified, because if you're changing any
        of the modes from proportional to absolute or vice-versa, then it'll need to convert
        the co-ordinates, and will need to know the parent size so it can calculate this.
    */
    void setModes (const AnchorPoint xAnchorMode,
                   const PositionMode xPositionMode,
                   const AnchorPoint yAnchorMode,
                   const PositionMode yPositionMode,
                   const SizeMode widthMode,
                   const SizeMode heightMode,
                   const Rectangle<int>& targetSpaceToBeRelativeTo) throw();

    /** Returns the anchoring mode for the x co-ordinate.
        To change any of the modes, use setModes().
    */
    AnchorPoint getAnchorPointX() const throw();

    /** Returns the positioning mode for the x co-ordinate.
        To change any of the modes, use setModes().
    */
    PositionMode getPositionModeX() const throw();

    /** Returns the raw x co-ordinate.

        If the x position mode is absolute, then this will be the absolute value. If it's
        proportional, then this will be a fractional proportion, where 1.0 means the full
        width of the parent space.
    */
    double getX() const throw()                             { return x; }

    /** Sets the raw value of the x co-ordinate.

        See getX() for the meaning of this value.
    */
    void setX (const double newX) throw()                   { x = newX; }

    /** Returns the anchoring mode for the y co-ordinate.
        To change any of the modes, use setModes().
    */
    AnchorPoint getAnchorPointY() const throw();

    /** Returns the positioning mode for the y co-ordinate.
        To change any of the modes, use setModes().
    */
    PositionMode getPositionModeY() const throw();

    /** Returns the raw y co-ordinate.

        If the y position mode is absolute, then this will be the absolute value. If it's
        proportional, then this will be a fractional proportion, where 1.0 means the full
        height of the parent space.
    */
    double getY() const throw()                             { return y; }

    /** Sets the raw value of the y co-ordinate.

        See getY() for the meaning of this value.
    */
    void setY (const double newY) throw()                   { y = newY; }

    /** Returns the mode used to calculate the width.
        To change any of the modes, use setModes().
    */
    SizeMode getWidthMode() const throw();

    /** Returns the raw width value.

        If the width mode is absolute, then this will be the absolute value. If the mode is
        proportional, then this will be a fractional proportion, where 1.0 means the full
        width of the parent space.
    */
    double getWidth() const throw()                         { return w; }

    /** Sets the raw width value.

        See getWidth() for the details about what this value means.
    */
    void setWidth (const double newWidth) throw()           { w = newWidth; }

    /** Returns the mode used to calculate the height.
        To change any of the modes, use setModes().
    */
    SizeMode getHeightMode() const throw();

    /** Returns the raw height value.

        If the height mode is absolute, then this will be the absolute value. If the mode is
        proportional, then this will be a fractional proportion, where 1.0 means the full
        height of the parent space.
    */
    double getHeight() const throw()                        { return h; }

    /** Sets the raw height value.

        See getHeight() for the details about what this value means.
    */
    void setHeight (const double newHeight) throw()         { h = newHeight; }

    //==============================================================================
    /** If the size and position are constance, and wouldn't be affected by changes
        in the parent's size, then this will return true.
    */
    bool isPositionAbsolute() const throw();

    //==============================================================================
    /** Compares two objects. */
    const bool operator== (const PositionedRectangle& other) const throw();

    /** Compares two objects. */
    const bool operator!= (const PositionedRectangle& other) const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    double x, y, w, h;
    uint8 xMode, yMode, wMode, hMode;

    void addPosDescription (String& result, const uint8 mode, const double value) const throw();
    void addSizeDescription (String& result, const uint8 mode, const double value) const throw();
    void decodePosString (const String& s, uint8& mode, double& value) throw();
    void decodeSizeString (const String& s, uint8& mode, double& value) throw();
    void applyPosAndSize (double& xOut, double& wOut, const double x, const double w,
                          const uint8 xMode, const uint8 wMode,
                          const int parentPos, const int parentSize) const throw();
    void updatePosAndSize (double& xOut, double& wOut, double x, const double w,
                           const uint8 xMode, const uint8 wMode,
                           const int parentPos, const int parentSize) const throw();
};


#endif   // __JUCE_POSITIONEDRECTANGLE_JUCEHEADER__
