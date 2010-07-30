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

#include "juce_RelativeCoordinate.h"
#include "../drawables/juce_DrawablePath.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
namespace RelativeCoordinateHelpers
{
    static bool isOrigin (const String& name)
    {
        return name.isEmpty()
                || name == RelativeCoordinate::Strings::parentLeft
                || name == RelativeCoordinate::Strings::parentTop;
    }

    static const String getExtentAnchorName (const bool isHorizontal) throw()
    {
        return isHorizontal ? RelativeCoordinate::Strings::parentRight
                            : RelativeCoordinate::Strings::parentBottom;
    }

    static const String getObjectName (const String& fullName)
    {
        return fullName.upToFirstOccurrenceOf (".", false, false);
    }

    static const String getEdgeName (const String& fullName)
    {
        return fullName.fromFirstOccurrenceOf (".", false, false);
    }

    static const RelativeCoordinate findCoordinate (const String& name, const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
    {
        return nameFinder != 0 ? nameFinder->findNamedCoordinate (getObjectName (name), getEdgeName (name))
                               : RelativeCoordinate();
    }

    //==============================================================================
    struct RecursionException  : public std::runtime_error
    {
        RecursionException()   : std::runtime_error ("Recursive RelativeCoordinate expression")
        {
        }
    };

    //==============================================================================
    static void skipWhitespace (const juce_wchar* const s, int& i)
    {
        while (CharacterFunctions::isWhitespace (s[i]))
            ++i;
    }

    static void skipComma (const juce_wchar* const s, int& i)
    {
        skipWhitespace (s, i);
        if (s[i] == ',')
            ++i;
    }

    static const String readAnchorName (const juce_wchar* const s, int& i)
    {
        skipWhitespace (s, i);

        if (CharacterFunctions::isLetter (s[i]) || s[i] == '_')
        {
            int start = i;
            while (CharacterFunctions::isLetterOrDigit (s[i]) || s[i] == '_' || s[i] == '.')
                ++i;

            return String (s + start, i - start);
        }

        return String::empty;
    }

    static double readNumber (const juce_wchar* const s, int& i)
    {
        skipWhitespace (s, i);

        int start = i;
        if (CharacterFunctions::isDigit (s[i]) || s[i] == '.' || s[i] == '-')
            ++i;

        while (CharacterFunctions::isDigit (s[i]) || s[i] == '.')
            ++i;

        if ((s[i] == 'e' || s[i] == 'E')
             && (CharacterFunctions::isDigit (s[i + 1])
                  || s[i + 1] == '-'
                  || s[i + 1] == '+'))
        {
            i += 2;

            while (CharacterFunctions::isDigit (s[i]))
                ++i;
        }

        const double value = String (s + start, i - start).getDoubleValue();
        while (CharacterFunctions::isWhitespace (s[i]) || s[i] == ',')
            ++i;

        return value;
    }

    static const RelativeCoordinate readNextCoordinate (const juce_wchar* const s, int& i, const bool isHorizontal)
    {
        String anchor1 (readAnchorName (s, i));
        double value = 0;

        if (anchor1.isNotEmpty())
        {
            skipWhitespace (s, i);

            if (s[i] == '+')
                value = readNumber (s, ++i);
            else if (s[i] == '-')
                value = -readNumber (s, ++i);

            return RelativeCoordinate (value, anchor1);
        }
        else
        {
            value = readNumber (s, i);
            skipWhitespace (s, i);

            if (s[i] == '%')
            {
                value /= 100.0;
                skipWhitespace (s, ++i);
                String anchor2;

                if (s[i] == '*')
                {
                    anchor1 = readAnchorName (s, ++i);

                    skipWhitespace (s, i);

                    if (s[i] == '-' && s[i + 1] == '>')
                    {
                        i += 2;
                        anchor2 = readAnchorName (s, i);
                    }
                    else
                    {
                        anchor2 = anchor1;
                        anchor1 = String::empty;
                    }
                }
                else
                {
                    anchor1 = String::empty;
                    anchor2 = getExtentAnchorName (isHorizontal);
                }

                return RelativeCoordinate (value, anchor1, anchor2);
            }

            return RelativeCoordinate (value);
        }
    }

    static const String limitedAccuracyString (const double n)
    {
        if (! (n < -0.001 || n > 0.001)) // to detect NaN and inf as well as for rounding
            return "0";

        return String (n, 3).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
    }
}

//==============================================================================
const String RelativeCoordinate::Strings::parent ("parent");
const String RelativeCoordinate::Strings::left ("left");
const String RelativeCoordinate::Strings::right ("right");
const String RelativeCoordinate::Strings::top ("top");
const String RelativeCoordinate::Strings::bottom ("bottom");
const String RelativeCoordinate::Strings::parentLeft ("parent.left");
const String RelativeCoordinate::Strings::parentTop ("parent.top");
const String RelativeCoordinate::Strings::parentRight ("parent.right");
const String RelativeCoordinate::Strings::parentBottom ("parent.bottom");

//==============================================================================
RelativeCoordinate::RelativeCoordinate()
    : value (0)
{
}

RelativeCoordinate::RelativeCoordinate (const double absoluteDistanceFromOrigin)
    : value (absoluteDistanceFromOrigin)
{
}

RelativeCoordinate::RelativeCoordinate (const double absoluteDistance, const String& source)
    : anchor1 (source.trim()),
      value (absoluteDistance)
{
}

RelativeCoordinate::RelativeCoordinate (const double relativeProportion, const String& pos1, const String& pos2)
    : anchor1 (pos1.trim()),
      anchor2 (pos2.trim()),
      value (relativeProportion)
{
}

RelativeCoordinate::RelativeCoordinate (const String& s, const bool isHorizontal)
    : value (0)
{
    int i = 0;
    *this = RelativeCoordinateHelpers::readNextCoordinate (s, i, isHorizontal);
}

RelativeCoordinate::~RelativeCoordinate()
{
}

bool RelativeCoordinate::operator== (const RelativeCoordinate& other) const throw()
{
    return value == other.value && anchor1 == other.anchor1 && anchor2 == other.anchor2;
}

bool RelativeCoordinate::operator!= (const RelativeCoordinate& other) const throw()
{
    return ! operator== (other);
}

//==============================================================================
const RelativeCoordinate RelativeCoordinate::getAnchorCoordinate1() const
{
    return RelativeCoordinate (0.0, anchor1);
}

const RelativeCoordinate RelativeCoordinate::getAnchorCoordinate2() const
{
    return RelativeCoordinate (0.0, anchor2);
}

double RelativeCoordinate::resolveAnchor (const String& anchorName, const NamedCoordinateFinder* nameFinder, int recursionCounter)
{
    if (RelativeCoordinateHelpers::isOrigin (anchorName))
        return 0.0;

    return RelativeCoordinateHelpers::findCoordinate (anchorName, nameFinder).resolve (nameFinder, recursionCounter + 1);
}

double RelativeCoordinate::resolve (const NamedCoordinateFinder* nameFinder, int recursionCounter) const
{
    if (recursionCounter > 150)
    {
        jassertfalse
        throw RelativeCoordinateHelpers::RecursionException();
    }

    const double pos1 = resolveAnchor (anchor1, nameFinder, recursionCounter);

    return isProportional() ? pos1 + (resolveAnchor (anchor2, nameFinder, recursionCounter) - pos1) * value
                            : pos1 + value;
}

double RelativeCoordinate::resolve (const NamedCoordinateFinder* nameFinder) const
{
    try
    {
        return resolve (nameFinder, 0);
    }
    catch (RelativeCoordinateHelpers::RecursionException&)
    {}

    return 0.0;
}

bool RelativeCoordinate::isRecursive (const NamedCoordinateFinder* nameFinder) const
{
    try
    {
        (void) resolve (nameFinder, 0);
    }
    catch (RelativeCoordinateHelpers::RecursionException&)
    {
        return true;
    }

    return false;
}

void RelativeCoordinate::moveToAbsolute (double newPos, const NamedCoordinateFinder* nameFinder)
{
    try
    {
        const double pos1 = resolveAnchor (anchor1, nameFinder, 0);

        if (isProportional())
        {
            const double size = resolveAnchor (anchor2, nameFinder, 0) - pos1;

            if (size != 0)
                value = (newPos - pos1) / size;
        }
        else
        {
            value = newPos - pos1;
        }
    }
    catch (RelativeCoordinateHelpers::RecursionException&)
    {}
}

void RelativeCoordinate::toggleProportionality (const NamedCoordinateFinder* nameFinder,
                                                const String& proportionalAnchor1, const String& proportionalAnchor2)
{
    const double oldValue = resolve (nameFinder);

    anchor1 = proportionalAnchor1;
    anchor2 = isProportional() ? String::empty : proportionalAnchor2;

    moveToAbsolute (oldValue, nameFinder);
}

bool RelativeCoordinate::references (const String& coordName, const NamedCoordinateFinder* nameFinder) const
{
    using namespace RelativeCoordinateHelpers;

    if (isOrigin (anchor1) && ! isProportional())
        return isOrigin (coordName);

    return anchor1 == coordName
            || anchor2 == coordName
            || findCoordinate (anchor1, nameFinder).references (coordName, nameFinder)
            || (isProportional() && findCoordinate (anchor2, nameFinder).references (coordName, nameFinder));
}

bool RelativeCoordinate::isDynamic() const
{
    return anchor2.isNotEmpty() || ! RelativeCoordinateHelpers::isOrigin (anchor1);
}

//==============================================================================
const String RelativeCoordinate::toString() const
{
    using namespace RelativeCoordinateHelpers;

    if (isProportional())
    {
        const String percent (limitedAccuracyString (value * 100.0));

        if (isOrigin (anchor1))
        {
            if (anchor2 == Strings::parentRight || anchor2 == Strings::parentBottom)
                return percent + "%";
            else
                return percent + "% * " + anchor2;
        }
        else
            return percent + "% * " + anchor1 + " -> " + anchor2;
    }
    else
    {
        if (isOrigin (anchor1))
            return limitedAccuracyString (value);
        else if (value > 0)
            return anchor1 + " + " + limitedAccuracyString (value);
        else if (value < 0)
            return anchor1 + " - " + limitedAccuracyString (-value);
        else
            return anchor1;
    }
}

//==============================================================================
const double RelativeCoordinate::getEditableNumber() const
{
    return isProportional() ? value * 100.0 : value;
}

void RelativeCoordinate::setEditableNumber (const double newValue)
{
    value = isProportional() ? newValue / 100.0 : newValue;
}

//==============================================================================
const String RelativeCoordinate::getAnchorName1 (const String& returnValueIfOrigin) const
{
    return RelativeCoordinateHelpers::isOrigin (anchor1) ? returnValueIfOrigin : anchor1;
}

const String RelativeCoordinate::getAnchorName2 (const String& returnValueIfOrigin) const
{
    return RelativeCoordinateHelpers::isOrigin (anchor2) ? returnValueIfOrigin : anchor2;
}

void RelativeCoordinate::changeAnchor1 (const String& newAnchorName, const NamedCoordinateFinder* nameFinder)
{
    jassert (newAnchorName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_."));

    const double oldValue = resolve (nameFinder);
    anchor1 = RelativeCoordinateHelpers::isOrigin (newAnchorName) ? String::empty : newAnchorName;
    moveToAbsolute (oldValue, nameFinder);
}

void RelativeCoordinate::changeAnchor2 (const String& newAnchorName, const NamedCoordinateFinder* nameFinder)
{
    jassert (isProportional());
    jassert (newAnchorName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_."));

    const double oldValue = resolve (nameFinder);
    anchor2 = RelativeCoordinateHelpers::isOrigin (newAnchorName) ? String::empty : newAnchorName;
    moveToAbsolute (oldValue, nameFinder);
}

void RelativeCoordinate::renameAnchorIfUsed (const String& oldName, const String& newName, const NamedCoordinateFinder* nameFinder)
{
    using namespace RelativeCoordinateHelpers;
    jassert (oldName.isNotEmpty());
    jassert (newName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_"));

    if (newName.isEmpty())
    {
        if (getObjectName (anchor1) == oldName
             || getObjectName (anchor2) == oldName)
        {
            value = resolve (nameFinder);
            anchor1 = String::empty;
            anchor2 = String::empty;
        }
    }
    else
    {
        if (getObjectName (anchor1) == oldName)
            anchor1 = newName + "." + getEdgeName (anchor1);

        if (getObjectName (anchor2) == oldName)
            anchor2 = newName + "." + getEdgeName (anchor2);
    }
}

//==============================================================================
RelativePoint::RelativePoint()
{
}

RelativePoint::RelativePoint (const Point<float>& absolutePoint)
    : x (absolutePoint.getX()), y (absolutePoint.getY())
{
}

RelativePoint::RelativePoint (const float x_, const float y_)
    : x (x_), y (y_)
{
}

RelativePoint::RelativePoint (const RelativeCoordinate& x_, const RelativeCoordinate& y_)
    : x (x_), y (y_)
{
}

RelativePoint::RelativePoint (const String& s)
{
    int i = 0;
    x = RelativeCoordinateHelpers::readNextCoordinate (s, i, true);
    RelativeCoordinateHelpers::skipComma (s, i);
    y = RelativeCoordinateHelpers::readNextCoordinate (s, i, false);
}

bool RelativePoint::operator== (const RelativePoint& other) const throw()
{
    return x == other.x && y == other.y;
}

bool RelativePoint::operator!= (const RelativePoint& other) const throw()
{
    return ! operator== (other);
}

const Point<float> RelativePoint::resolve (const RelativeCoordinate::NamedCoordinateFinder* nameFinder) const
{
    return Point<float> ((float) x.resolve (nameFinder),
                         (float) y.resolve (nameFinder));
}

void RelativePoint::moveToAbsolute (const Point<float>& newPos, const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
{
    x.moveToAbsolute (newPos.getX(), nameFinder);
    y.moveToAbsolute (newPos.getY(), nameFinder);
}

const String RelativePoint::toString() const
{
    return x.toString() + ", " + y.toString();
}

void RelativePoint::renameAnchorIfUsed (const String& oldName, const String& newName, const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
{
    x.renameAnchorIfUsed (oldName, newName, nameFinder);
    y.renameAnchorIfUsed (oldName, newName, nameFinder);
}

bool RelativePoint::isDynamic() const
{
    return x.isDynamic() || y.isDynamic();
}


//==============================================================================
RelativeRectangle::RelativeRectangle()
{
}

RelativeRectangle::RelativeRectangle (const RelativeCoordinate& left_, const RelativeCoordinate& right_,
                                      const RelativeCoordinate& top_, const RelativeCoordinate& bottom_)
    : left (left_), right (right_), top (top_), bottom (bottom_)
{
}

RelativeRectangle::RelativeRectangle (const Rectangle<float>& rect, const String& componentName)
    : left (rect.getX()),
      right (rect.getWidth(), componentName + "." + RelativeCoordinate::Strings::left),
      top (rect.getY()),
      bottom (rect.getHeight(), componentName + "." + RelativeCoordinate::Strings::top)
{
}

RelativeRectangle::RelativeRectangle (const String& s)
{
    int i = 0;
    left = RelativeCoordinateHelpers::readNextCoordinate (s, i, true);
    RelativeCoordinateHelpers::skipComma (s, i);
    top = RelativeCoordinateHelpers::readNextCoordinate (s, i, false);
    RelativeCoordinateHelpers::skipComma (s, i);
    right = RelativeCoordinateHelpers::readNextCoordinate (s, i, true);
    RelativeCoordinateHelpers::skipComma (s, i);
    bottom = RelativeCoordinateHelpers::readNextCoordinate (s, i, false);
}

bool RelativeRectangle::operator== (const RelativeRectangle& other) const throw()
{
    return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
}

bool RelativeRectangle::operator!= (const RelativeRectangle& other) const throw()
{
    return ! operator== (other);
}

const Rectangle<float> RelativeRectangle::resolve (const RelativeCoordinate::NamedCoordinateFinder* nameFinder) const
{
    const double l = left.resolve (nameFinder);
    const double r = right.resolve (nameFinder);
    const double t = top.resolve (nameFinder);
    const double b = bottom.resolve (nameFinder);

    return Rectangle<float> ((float) l, (float) t, (float) (r - l), (float) (b - t));
}

void RelativeRectangle::moveToAbsolute (const Rectangle<float>& newPos, const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
{
    left.moveToAbsolute (newPos.getX(), nameFinder);
    right.moveToAbsolute (newPos.getRight(), nameFinder);
    top.moveToAbsolute (newPos.getY(), nameFinder);
    bottom.moveToAbsolute (newPos.getBottom(), nameFinder);
}

const String RelativeRectangle::toString() const
{
    return left.toString() + ", " + top.toString() + ", " + right.toString() + ", " + bottom.toString();
}

void RelativeRectangle::renameAnchorIfUsed (const String& oldName, const String& newName,
                                               const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
{
    left.renameAnchorIfUsed (oldName, newName, nameFinder);
    right.renameAnchorIfUsed (oldName, newName, nameFinder);
    top.renameAnchorIfUsed (oldName, newName, nameFinder);
    bottom.renameAnchorIfUsed (oldName, newName, nameFinder);
}


//==============================================================================
RelativePointPath::RelativePointPath()
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
}

RelativePointPath::RelativePointPath (const RelativePointPath& other)
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
    ValueTree state (DrawablePath::valueTreeType);
    other.writeTo (state, 0);
    parse (state);
}

RelativePointPath::RelativePointPath (const ValueTree& drawable)
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
    parse (drawable);
}

RelativePointPath::RelativePointPath (const Path& path)
{
    usesNonZeroWinding = path.isUsingNonZeroWinding();

    Path::Iterator i (path);

    while (i.next())
    {
        switch (i.elementType)
        {
            case Path::Iterator::startNewSubPath:   elements.add (new StartSubPath (RelativePoint (i.x1, i.y1))); break;
            case Path::Iterator::lineTo:            elements.add (new LineTo (RelativePoint (i.x1, i.y1))); break;
            case Path::Iterator::quadraticTo:       elements.add (new QuadraticTo (RelativePoint (i.x1, i.y1), RelativePoint (i.x2, i.y2))); break;
            case Path::Iterator::cubicTo:           elements.add (new CubicTo (RelativePoint (i.x1, i.y1), RelativePoint (i.x2, i.y2), RelativePoint (i.x3, i.y3))); break;
            case Path::Iterator::closePath:         elements.add (new CloseSubPath()); break;
            default:                                jassertfalse; break;
        }
    }
}

void RelativePointPath::writeTo (ValueTree state, UndoManager* undoManager) const
{
    DrawablePath::ValueTreeWrapper wrapper (state);
    wrapper.setUsesNonZeroWinding (usesNonZeroWinding, undoManager);

    ValueTree pathTree (wrapper.getPathState());
    pathTree.removeAllChildren (undoManager);

    for (int i = 0; i < elements.size(); ++i)
        pathTree.addChild (elements.getUnchecked(i)->createTree(), -1, undoManager);
}

void RelativePointPath::parse (const ValueTree& state)
{
    DrawablePath::ValueTreeWrapper wrapper (state);
    usesNonZeroWinding = wrapper.usesNonZeroWinding();
    RelativePoint points[3];

    const ValueTree pathTree (wrapper.getPathState());
    const int num = pathTree.getNumChildren();
    for (int i = 0; i < num; ++i)
    {
        const DrawablePath::ValueTreeWrapper::Element e (pathTree.getChild(i));

        const int numCps = e.getNumControlPoints();
        for (int j = 0; j < numCps; ++j)
        {
            points[j] = e.getControlPoint (j);
            containsDynamicPoints = containsDynamicPoints || points[j].isDynamic();
        }

        const Identifier type (e.getType());

        if (type == DrawablePath::ValueTreeWrapper::Element::startSubPathElement)
            elements.add (new StartSubPath (points[0]));
        else if (type == DrawablePath::ValueTreeWrapper::Element::closeSubPathElement)
            elements.add (new CloseSubPath());
        else if (type == DrawablePath::ValueTreeWrapper::Element::lineToElement)
            elements.add (new LineTo (points[0]));
        else if (type == DrawablePath::ValueTreeWrapper::Element::quadraticToElement)
            elements.add (new QuadraticTo (points[0], points[1]));
        else if (type == DrawablePath::ValueTreeWrapper::Element::cubicToElement)
            elements.add (new CubicTo (points[0], points[1], points[2]));
        else
            jassertfalse;
    }
}

RelativePointPath::~RelativePointPath()
{
}

void RelativePointPath::swapWith (RelativePointPath& other) throw()
{
    elements.swapWithArray (other.elements);
    swapVariables (usesNonZeroWinding, other.usesNonZeroWinding);
}

void RelativePointPath::createPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder)
{
    for (int i = 0; i < elements.size(); ++i)
        elements.getUnchecked(i)->addToPath (path, coordFinder);
}

bool RelativePointPath::containsAnyDynamicPoints() const
{
    return containsDynamicPoints;
}

//==============================================================================
RelativePointPath::ElementBase::ElementBase (const ElementType type_) : type (type_)
{
}

//==============================================================================
RelativePointPath::StartSubPath::StartSubPath (const RelativePoint& pos)
    : ElementBase (startSubPathElement), startPos (pos)
{
}

const ValueTree RelativePointPath::StartSubPath::createTree() const
{
    ValueTree v (DrawablePath::ValueTreeWrapper::Element::startSubPathElement);
    v.setProperty (DrawablePath::ValueTreeWrapper::point1, startPos.toString(), 0);
    return v;
}

void RelativePointPath::StartSubPath::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder) const
{
    path.startNewSubPath (startPos.resolve (coordFinder));
}

RelativePoint* RelativePointPath::StartSubPath::getControlPoints (int& numPoints)
{
    numPoints = 1;
    return &startPos;
}

//==============================================================================
RelativePointPath::CloseSubPath::CloseSubPath()
    : ElementBase (closeSubPathElement)
{
}

const ValueTree RelativePointPath::CloseSubPath::createTree() const
{
    return ValueTree (DrawablePath::ValueTreeWrapper::Element::closeSubPathElement);
}

void RelativePointPath::CloseSubPath::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder*) const
{
    path.closeSubPath();
}

RelativePoint* RelativePointPath::CloseSubPath::getControlPoints (int& numPoints)
{
    numPoints = 0;
    return 0;
}

//==============================================================================
RelativePointPath::LineTo::LineTo (const RelativePoint& endPoint_)
    : ElementBase (lineToElement), endPoint (endPoint_)
{
}

const ValueTree RelativePointPath::LineTo::createTree() const
{
    ValueTree v (DrawablePath::ValueTreeWrapper::Element::lineToElement);
    v.setProperty (DrawablePath::ValueTreeWrapper::point1, endPoint.toString(), 0);
    return v;
}

void RelativePointPath::LineTo::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder) const
{
    path.lineTo (endPoint.resolve (coordFinder));
}

RelativePoint* RelativePointPath::LineTo::getControlPoints (int& numPoints)
{
    numPoints = 1;
    return &endPoint;
}

//==============================================================================
RelativePointPath::QuadraticTo::QuadraticTo (const RelativePoint& controlPoint, const RelativePoint& endPoint)
    : ElementBase (quadraticToElement)
{
    controlPoints[0] = controlPoint;
    controlPoints[1] = endPoint;
}

const ValueTree RelativePointPath::QuadraticTo::createTree() const
{
    ValueTree v (DrawablePath::ValueTreeWrapper::Element::quadraticToElement);
    v.setProperty (DrawablePath::ValueTreeWrapper::point1, controlPoints[0].toString(), 0);
    v.setProperty (DrawablePath::ValueTreeWrapper::point2, controlPoints[1].toString(), 0);
    return v;
}

void RelativePointPath::QuadraticTo::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder) const
{
    path.quadraticTo (controlPoints[0].resolve (coordFinder),
                      controlPoints[1].resolve (coordFinder));
}

RelativePoint* RelativePointPath::QuadraticTo::getControlPoints (int& numPoints)
{
    numPoints = 2;
    return controlPoints;
}

//==============================================================================
RelativePointPath::CubicTo::CubicTo (const RelativePoint& controlPoint1, const RelativePoint& controlPoint2, const RelativePoint& endPoint)
    : ElementBase (cubicToElement)
{
    controlPoints[0] = controlPoint1;
    controlPoints[1] = controlPoint2;
    controlPoints[2] = endPoint;
}

const ValueTree RelativePointPath::CubicTo::createTree() const
{
    ValueTree v (DrawablePath::ValueTreeWrapper::Element::cubicToElement);
    v.setProperty (DrawablePath::ValueTreeWrapper::point1, controlPoints[0].toString(), 0);
    v.setProperty (DrawablePath::ValueTreeWrapper::point2, controlPoints[1].toString(), 0);
    v.setProperty (DrawablePath::ValueTreeWrapper::point3, controlPoints[2].toString(), 0);
    return v;
}

void RelativePointPath::CubicTo::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder) const
{
    path.cubicTo (controlPoints[0].resolve (coordFinder),
                  controlPoints[1].resolve (coordFinder),
                  controlPoints[2].resolve (coordFinder));
}

RelativePoint* RelativePointPath::CubicTo::getControlPoints (int& numPoints)
{
    numPoints = 3;
    return controlPoints;
}


//==============================================================================
RelativeParallelogram::RelativeParallelogram()
{
}

RelativeParallelogram::RelativeParallelogram (const RelativePoint& topLeft_, const RelativePoint& topRight_, const RelativePoint& bottomLeft_)
    : topLeft (topLeft_), topRight (topRight_), bottomLeft (bottomLeft_)
{
}

RelativeParallelogram::RelativeParallelogram (const String& topLeft_, const String& topRight_, const String& bottomLeft_)
    : topLeft (topLeft_), topRight (topRight_), bottomLeft (bottomLeft_)
{
}

RelativeParallelogram::~RelativeParallelogram()
{
}

void RelativeParallelogram::resolveThreePoints (Point<float>* points, RelativeCoordinate::NamedCoordinateFinder* const coordFinder) const
{
    points[0] = topLeft.resolve (coordFinder);
    points[1] = topRight.resolve (coordFinder);
    points[2] = bottomLeft.resolve (coordFinder);
}

void RelativeParallelogram::resolveFourCorners (Point<float>* points, RelativeCoordinate::NamedCoordinateFinder* const coordFinder) const
{
    resolveThreePoints (points, coordFinder);
    points[3] = points[1] + (points[2] - points[0]);
}

const Rectangle<float> RelativeParallelogram::getBounds (RelativeCoordinate::NamedCoordinateFinder* const coordFinder) const
{
    Point<float> points[4];
    resolveFourCorners (points, coordFinder);
    return Rectangle<float>::findAreaContainingPoints (points, 4);
}

void RelativeParallelogram::getPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* const coordFinder) const
{
    Point<float> points[4];
    resolveFourCorners (points, coordFinder);

    path.startNewSubPath (points[0]);
    path.lineTo (points[1]);
    path.lineTo (points[3]);
    path.lineTo (points[2]);
    path.closeSubPath();
}

const AffineTransform RelativeParallelogram::resetToPerpendicular (RelativeCoordinate::NamedCoordinateFinder* const coordFinder)
{
    Point<float> corners[3];
    resolveThreePoints (corners, coordFinder);

    const Line<float> top (corners[0], corners[1]);
    const Line<float> left (corners[0], corners[2]);
    const Point<float> newTopRight (corners[0] + Point<float> (top.getLength(), 0.0f));
    const Point<float> newBottomLeft (corners[0] + Point<float> (0.0f, left.getLength()));

    topRight.moveToAbsolute (newTopRight, coordFinder);
    bottomLeft.moveToAbsolute (newBottomLeft, coordFinder);

    return AffineTransform::fromTargetPoints (corners[0].getX(), corners[0].getY(), corners[0].getX(), corners[0].getY(),
                                              corners[1].getX(), corners[1].getY(), newTopRight.getX(), newTopRight.getY(),
                                              corners[2].getX(), corners[2].getY(), newBottomLeft.getX(), newBottomLeft.getY());
}

bool RelativeParallelogram::operator== (const RelativeParallelogram& other) const throw()
{
    return topLeft == other.topLeft && topRight == other.topRight && bottomLeft == other.bottomLeft;
}

bool RelativeParallelogram::operator!= (const RelativeParallelogram& other) const throw()
{
    return ! operator== (other);
}

const Point<float> RelativeParallelogram::getInternalCoordForPoint (const Point<float>* const corners, Point<float> target) throw()
{
    const Point<float> tr (corners[1] - corners[0]);
    const Point<float> bl (corners[2] - corners[0]);
    target -= corners[0];

    return Point<float> (Line<float> (Point<float>(), tr).getIntersection (Line<float> (target, target - bl)).getDistanceFromOrigin(),
                         Line<float> (Point<float>(), bl).getIntersection (Line<float> (target, target - tr)).getDistanceFromOrigin());
}

const Point<float> RelativeParallelogram::getPointForInternalCoord (const Point<float>* const corners, const Point<float>& point) throw()
{
    return corners[0]
            + Line<float> (Point<float>(), corners[1] - corners[0]).getPointAlongLine (point.getX())
            + Line<float> (Point<float>(), corners[2] - corners[0]).getPointAlongLine (point.getY());
}


END_JUCE_NAMESPACE
