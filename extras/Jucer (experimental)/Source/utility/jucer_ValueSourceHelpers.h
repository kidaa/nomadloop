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

#ifndef __JUCER_VALUEREMAPPERSOURCE_JUCEHEADER__
#define __JUCER_VALUEREMAPPERSOURCE_JUCEHEADER__


//==============================================================================
/**
*/
template <typename Type>
class NumericValueSource   : public Value::ValueSource,
                             public Value::Listener
{
public:
    NumericValueSource (const Value& sourceValue_)
       : sourceValue (sourceValue_)
    {
        sourceValue.addListener (this);
    }

    ~NumericValueSource() {}

    void valueChanged (Value&)   { sendChangeMessage (true); }
    const var getValue() const   { return (Type) sourceValue.getValue(); }

    void setValue (const var& newValue)
    {
        const Type newVal = (Type) newValue;

        if (newVal != (Type) getValue())  // this test is important, because if a property is missing, it won't
            sourceValue = newVal;        // create it (causing an unwanted undo action) when a control sets it to 0
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    Value sourceValue;

    NumericValueSource (const NumericValueSource&);
    const NumericValueSource& operator= (const NumericValueSource&);
};


#endif   // __JUCER_VALUEREMAPPERSOURCE_JUCEHEADER__
