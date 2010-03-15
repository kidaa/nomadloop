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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_StringArray.h"


//==============================================================================
StringArray::StringArray() throw()
{
}

StringArray::StringArray (const StringArray& other)
    : strings (other.strings)
{
}

StringArray::StringArray (const juce_wchar** const initialStrings,
                          const int numberOfStrings)
{
    for (int i = 0; i < numberOfStrings; ++i)
        add (initialStrings [i]);
}

StringArray::StringArray (const char** const initialStrings,
                          const int numberOfStrings)
{
    for (int i = 0; i < numberOfStrings; ++i)
        add (initialStrings [i]);
}

StringArray::StringArray (const juce_wchar** const initialStrings)
{
    int i = 0;

    while (initialStrings[i] != 0)
        add (initialStrings [i++]);
}

StringArray::StringArray (const char** const initialStrings)
{
    int i = 0;

    while (initialStrings[i] != 0)
        add (initialStrings [i++]);
}

StringArray& StringArray::operator= (const StringArray& other)
{
    strings = other.strings;
    return *this;
}

StringArray::~StringArray()
{
}

bool StringArray::operator== (const StringArray& other) const
{
    if (other.size() != size())
        return false;

    for (int i = size(); --i >= 0;)
        if (other.strings.getReference(i) != strings.getReference(i))
            return false;

    return true;
}

bool StringArray::operator!= (const StringArray& other) const
{
    return ! operator== (other);
}

void StringArray::clear()
{
    strings.clear();
}

const String& StringArray::operator[] (const int index) const throw()
{
    if (((unsigned int) index) < (unsigned int) strings.size())
        return strings.getReference (index);

    return String::empty;
}

void StringArray::add (const String& newString)
{
    strings.add (newString);
}

void StringArray::insert (const int index, const String& newString)
{
    strings.insert (index, newString);
}

void StringArray::addIfNotAlreadyThere (const String& newString, const bool ignoreCase)
{
    if (! contains (newString, ignoreCase))
        add (newString);
}

void StringArray::addArray (const StringArray& otherArray, int startIndex, int numElementsToAdd)
{
    if (startIndex < 0)
    {
        jassertfalse
        startIndex = 0;
    }

    if (numElementsToAdd < 0 || startIndex + numElementsToAdd > otherArray.size())
        numElementsToAdd = otherArray.size() - startIndex;

    while (--numElementsToAdd >= 0)
        strings.add (otherArray.strings.getReference (startIndex++));
}

void StringArray::set (const int index, const String& newString)
{
    strings.set (index, newString);
}

bool StringArray::contains (const String& stringToLookFor, const bool ignoreCase) const
{
    if (ignoreCase)
    {
        for (int i = size(); --i >= 0;)
            if (strings.getReference(i).equalsIgnoreCase (stringToLookFor))
                return true;
    }
    else
    {
        for (int i = size(); --i >= 0;)
            if (stringToLookFor == strings.getReference(i))
                return true;
    }

    return false;
}

int StringArray::indexOf (const String& stringToLookFor, const bool ignoreCase, int i) const
{
    if (i < 0)
        i = 0;

    const int numElements = size();

    if (ignoreCase)
    {
        while (i < numElements)
        {
            if (strings.getReference(i).equalsIgnoreCase (stringToLookFor))
                return i;

            ++i;
        }
    }
    else
    {
        while (i < numElements)
        {
            if (stringToLookFor == strings.getReference (i))
                return i;

            ++i;
        }
    }

    return -1;
}

//==============================================================================
void StringArray::remove (const int index)
{
    strings.remove (index);
}

void StringArray::removeString (const String& stringToRemove,
                                const bool ignoreCase)
{
    if (ignoreCase)
    {
        for (int i = size(); --i >= 0;)
            if (strings.getReference(i).equalsIgnoreCase (stringToRemove))
                strings.remove (i);
    }
    else
    {
        for (int i = size(); --i >= 0;)
            if (stringToRemove == strings.getReference (i))
                strings.remove (i);
    }
}

//==============================================================================
void StringArray::removeEmptyStrings (const bool removeWhitespaceStrings)
{
    if (removeWhitespaceStrings)
    {
        for (int i = size(); --i >= 0;)
            if (! strings.getReference(i).containsNonWhitespaceChars())
                strings.remove (i);
    }
    else
    {
        for (int i = size(); --i >= 0;)
            if (strings.getReference(i).isEmpty())
                strings.remove (i);
    }
}

void StringArray::trim()
{
    for (int i = size(); --i >= 0;)
    {
        String& s = strings.getReference(i);
        s = s.trim();
    }
}

//==============================================================================
class InternalStringArrayComparator_CaseSensitive
{
public:
    static int compareElements (String& first, String& second)      { return first.compare (second); }
};

class InternalStringArrayComparator_CaseInsensitive
{
public:
    static int compareElements (String& first, String& second)      { return first.compareIgnoreCase (second); }
};

void StringArray::sort (const bool ignoreCase)
{
    if (ignoreCase)
    {
        InternalStringArrayComparator_CaseInsensitive comp;
        strings.sort (comp);
    }
    else
    {
        InternalStringArrayComparator_CaseSensitive comp;
        strings.sort (comp);
    }
}

void StringArray::move (const int currentIndex, int newIndex) throw()
{
    strings.move (currentIndex, newIndex);
}


//==============================================================================
const String StringArray::joinIntoString (const String& separator, int start, int numberToJoin) const
{
    const int last = (numberToJoin < 0) ? size()
                                        : jmin (size(), start + numberToJoin);

    if (start < 0)
        start = 0;

    if (start >= last)
        return String::empty;

    if (start == last - 1)
        return strings.getReference (start);

    const int separatorLen = separator.length();
    int charsNeeded = separatorLen * (last - start - 1);

    for (int i = start; i < last; ++i)
        charsNeeded += strings.getReference(i).length();

    String result;
    result.preallocateStorage (charsNeeded);

    juce_wchar* dest = (juce_wchar*) result;

    while (start < last)
    {
        const String& s = strings.getReference (start);
        const int len = s.length();

        if (len > 0)
        {
            s.copyToUnicode (dest, len);
            dest += len;
        }

        if (++start < last && separatorLen > 0)
        {
            separator.copyToUnicode (dest, separatorLen);
            dest += separatorLen;
        }
    }

    *dest = 0;

    return result;
}

int StringArray::addTokens (const String& text, const bool preserveQuotedStrings)
{
    return addTokens (text, T(" \n\r\t"), preserveQuotedStrings ? T("\"") : 0);
}

int StringArray::addTokens (const String& text, const String& breakCharacters, const String& quoteCharacters)
{
    int num = 0;

    if (text.isNotEmpty())
    {
        bool insideQuotes = false;
        juce_wchar currentQuoteChar = 0;

        int i = 0;
        int tokenStart = 0;

        for (;;)
        {
            const juce_wchar c = text[i];

            bool isBreak = (c == 0);

            if (! (insideQuotes || isBreak))
            {
                const juce_wchar* b = breakCharacters;
                while (*b != 0)
                {
                    if (*b++ == c)
                    {
                        isBreak = true;
                        break;
                    }
                }
            }

            if (! isBreak)
            {
                bool isQuote = false;
                const juce_wchar* q = quoteCharacters;
                while (*q != 0)
                {
                    if (*q++ == c)
                    {
                        isQuote = true;
                        break;
                    }
                }

                if (isQuote)
                {
                    if (insideQuotes)
                    {
                        // only break out of quotes-mode if we find a matching quote to the
                        // one that we opened with..
                        if (currentQuoteChar == c)
                            insideQuotes = false;
                    }
                    else
                    {
                        insideQuotes = true;
                        currentQuoteChar = c;
                    }
                }
            }
            else
            {
                add (String (static_cast <const juce_wchar*> (text) + tokenStart, i - tokenStart));

                ++num;
                tokenStart = i + 1;
            }

            if (c == 0)
                break;

            ++i;
        }
    }

    return num;
}

int StringArray::addLines (const String& sourceText)
{
    int numLines = 0;
    const juce_wchar* text = sourceText;

    while (*text != 0)
    {
        const juce_wchar* const startOfLine = text;

        while (*text != 0)
        {
            if (*text == T('\r'))
            {
                ++text;
                if (*text == T('\n'))
                    ++text;

                break;
            }

            if (*text == T('\n'))
            {
                ++text;
                break;
            }

            ++text;
        }

        const juce_wchar* endOfLine = text;
        if (endOfLine > startOfLine && (*(endOfLine - 1) == T('\r') || *(endOfLine - 1) == T('\n')))
            --endOfLine;

        if (endOfLine > startOfLine && (*(endOfLine - 1) == T('\r') || *(endOfLine - 1) == T('\n')))
            --endOfLine;

        add (String (startOfLine, jmax (0, (int) (endOfLine - startOfLine))));

        ++numLines;
    }

    return numLines;
}

//==============================================================================
void StringArray::removeDuplicates (const bool ignoreCase)
{
    for (int i = 0; i < size() - 1; ++i)
    {
        const String s (strings.getReference(i));

        int nextIndex = i + 1;

        for (;;)
        {
            nextIndex = indexOf (s, ignoreCase, nextIndex);

            if (nextIndex < 0)
                break;

            strings.remove (nextIndex);
        }
    }
}

void StringArray::appendNumbersToDuplicates (const bool ignoreCase,
                                             const bool appendNumberToFirstInstance,
                                             const tchar* const preNumberString,
                                             const tchar* const postNumberString)
{
    for (int i = 0; i < size() - 1; ++i)
    {
        String& s = strings.getReference(i);

        int nextIndex = indexOf (s, ignoreCase, i + 1);

        if (nextIndex >= 0)
        {
            const String original (s);

            int number = 0;

            if (appendNumberToFirstInstance)
                s = original + preNumberString + String (++number) + postNumberString;
            else
                ++number;

            while (nextIndex >= 0)
            {
                set (nextIndex, (*this)[nextIndex] + preNumberString + String (++number) + postNumberString);
                nextIndex = indexOf (original, ignoreCase, nextIndex + 1);
            }
        }
    }
}

void StringArray::minimiseStorageOverheads()
{
    strings.minimiseStorageOverheads();
}

END_JUCE_NAMESPACE
