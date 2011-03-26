/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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


//==============================================================================
const String RelativeCoordinate::Strings::parent ("parent");
const String RelativeCoordinate::Strings::left ("left");
const String RelativeCoordinate::Strings::right ("right");
const String RelativeCoordinate::Strings::top ("top");
const String RelativeCoordinate::Strings::bottom ("bottom");
const String RelativeCoordinate::Strings::x ("x");
const String RelativeCoordinate::Strings::y ("y");
const String RelativeCoordinate::Strings::width ("width");
const String RelativeCoordinate::Strings::height ("height");

RelativeCoordinate::StandardStrings::Type RelativeCoordinate::StandardStrings::getTypeOf (const String& s) throw()
{
    if (s == Strings::left)    return left;
    if (s == Strings::right)   return right;
    if (s == Strings::top)     return top;
    if (s == Strings::bottom)  return bottom;
    if (s == Strings::x)       return x;
    if (s == Strings::y)       return y;
    if (s == Strings::width)   return width;
    if (s == Strings::height)  return height;
    if (s == Strings::parent)  return parent;
    return unknown;
}

//==============================================================================
RelativeCoordinate::RelativeCoordinate()
{
}

RelativeCoordinate::RelativeCoordinate (const Expression& term_)
    : term (term_)
{
}

RelativeCoordinate::RelativeCoordinate (const RelativeCoordinate& other)
    : term (other.term)
{
}

RelativeCoordinate& RelativeCoordinate::operator= (const RelativeCoordinate& other)
{
    term = other.term;
    return *this;
}

RelativeCoordinate::RelativeCoordinate (const double absoluteDistanceFromOrigin)
    : term (absoluteDistanceFromOrigin)
{
}

RelativeCoordinate::RelativeCoordinate (const String& s)
{
    try
    {
        term = Expression (s);
    }
    catch (...)
    {}
}

RelativeCoordinate::~RelativeCoordinate()
{
}

bool RelativeCoordinate::operator== (const RelativeCoordinate& other) const throw()
{
    return term.toString() == other.term.toString();
}

bool RelativeCoordinate::operator!= (const RelativeCoordinate& other) const throw()
{
    return ! operator== (other);
}

double RelativeCoordinate::resolve (const Expression::Scope* scope) const
{
    try
    {
        if (scope != 0)
            return term.evaluate (*scope);
        else
            return term.evaluate();
    }
    catch (...)
    {}

    return 0.0;
}

bool RelativeCoordinate::isRecursive (const Expression::Scope* scope) const
{
    try
    {
        if (scope != 0)
            term.evaluate (*scope);
        else
            term.evaluate();
    }
    catch (...)
    {
        return true;
    }

    return false;
}

void RelativeCoordinate::moveToAbsolute (double newPos, const Expression::Scope* scope)
{
    try
    {
        if (scope != 0)
        {
            term = term.adjustedToGiveNewResult (newPos, *scope);
        }
        else
        {
            Expression::Scope defaultScope;
            term = term.adjustedToGiveNewResult (newPos, defaultScope);
        }
    }
    catch (...)
    {}
}

bool RelativeCoordinate::isDynamic() const
{
    return term.usesAnySymbols();
}

const String RelativeCoordinate::toString() const
{
    return term.toString();
}



END_JUCE_NAMESPACE
