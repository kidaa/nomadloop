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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_LocalisedStrings.h"


//==============================================================================
LocalisedStrings::LocalisedStrings (const String& fileContents)
{
    loadFromText (fileContents);
}

LocalisedStrings::LocalisedStrings (const File& fileToLoad)
{
    loadFromText (fileToLoad.loadFileAsString());
}

LocalisedStrings::~LocalisedStrings()
{
}

//==============================================================================
const String LocalisedStrings::translate (const String& text) const
{
    return translations.getValue (text, text);
}

namespace
{
    CriticalSection currentMappingsLock;
    LocalisedStrings* currentMappings = 0;

    int findCloseQuote (const String& text, int startPos)
    {
        juce_wchar lastChar = 0;

        for (;;)
        {
            const juce_wchar c = text [startPos];

            if (c == 0 || (c == '"' && lastChar != '\\'))
                break;

            lastChar = c;
            ++startPos;
        }

        return startPos;
    }

    const String unescapeString (const String& s)
    {
        return s.replace ("\\\"", "\"")
                .replace ("\\\'", "\'")
                .replace ("\\t", "\t")
                .replace ("\\r", "\r")
                .replace ("\\n", "\n");
    }
}

void LocalisedStrings::loadFromText (const String& fileContents)
{
    StringArray lines;
    lines.addLines (fileContents);

    for (int i = 0; i < lines.size(); ++i)
    {
        String line (lines[i].trim());

        if (line.startsWithChar ('"'))
        {
            int closeQuote = findCloseQuote (line, 1);

            const String originalText (unescapeString (line.substring (1, closeQuote)));

            if (originalText.isNotEmpty())
            {
                const int openingQuote = findCloseQuote (line, closeQuote + 1);
                closeQuote = findCloseQuote (line, openingQuote + 1);

                const String newText (unescapeString (line.substring (openingQuote + 1, closeQuote)));

                if (newText.isNotEmpty())
                    translations.set (originalText, newText);
            }
        }
        else if (line.startsWithIgnoreCase ("language:"))
        {
            languageName = line.substring (9).trim();
        }
        else if (line.startsWithIgnoreCase ("countries:"))
        {
            countryCodes.addTokens (line.substring (10).trim(), true);
            countryCodes.trim();
            countryCodes.removeEmptyStrings();
        }
    }
}

void LocalisedStrings::setIgnoresCase (const bool shouldIgnoreCase)
{
    translations.setIgnoresCase (shouldIgnoreCase);
}

//==============================================================================
void LocalisedStrings::setCurrentMappings (LocalisedStrings* newTranslations)
{
    const ScopedLock sl (currentMappingsLock);

    delete currentMappings;
    currentMappings = newTranslations;
}

LocalisedStrings* LocalisedStrings::getCurrentMappings()
{
    return currentMappings;
}

const String LocalisedStrings::translateWithCurrentMappings (const String& text)
{
    const ScopedLock sl (currentMappingsLock);

    if (currentMappings != 0)
        return currentMappings->translate (text);

    return text;
}

const String LocalisedStrings::translateWithCurrentMappings (const char* text)
{
    return translateWithCurrentMappings (String (text));
}


END_JUCE_NAMESPACE
