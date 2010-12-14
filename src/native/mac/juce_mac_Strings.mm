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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
namespace
{
    const String nsStringToJuce (NSString* s)
    {
        return String::fromUTF8 ([s UTF8String]);
    }

    NSString* juceStringToNS (const String& s)
    {
        return [NSString stringWithUTF8String: s.toUTF8()];
    }

    //==============================================================================
    const String convertUTF16ToString (const UniChar* utf16)
    {
        String s;

        while (*utf16 != 0)
            s += (juce_wchar) *utf16++;

        return s;
    }
}

const String PlatformUtilities::cfStringToJuceString (CFStringRef cfString)
{
    String result;

    if (cfString != 0)
    {
        CFRange range = { 0, CFStringGetLength (cfString) };
        HeapBlock <UniChar> u (range.length + 1);
        CFStringGetCharacters (cfString, range, u);
        u[range.length] = 0;
        result = convertUTF16ToString (u);
    }

    return result;
}

CFStringRef PlatformUtilities::juceStringToCFString (const String& s)
{
    const int len = s.length();
    HeapBlock <UniChar> temp (len + 2);

    for (int i = 0; i <= len; ++i)
        temp[i] = s[i];

    return CFStringCreateWithCharacters (kCFAllocatorDefault, temp, len);
}

const String PlatformUtilities::convertToPrecomposedUnicode (const String& s)
{
#if JUCE_IOS
    const ScopedAutoReleasePool pool;
    return nsStringToJuce ([juceStringToNS (s) precomposedStringWithCanonicalMapping]);
#else
    UnicodeMapping map;

    map.unicodeEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                              kUnicodeNoSubset,
                                              kTextEncodingDefaultFormat);

    map.otherEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                            kUnicodeCanonicalCompVariant,
                                            kTextEncodingDefaultFormat);

    map.mappingVersion = kUnicodeUseLatestMapping;

    UnicodeToTextInfo conversionInfo = 0;
    String result;

    if (CreateUnicodeToTextInfo (&map, &conversionInfo) == noErr)
    {
        const int len = s.length();

        HeapBlock <UniChar> tempIn, tempOut;
        tempIn.calloc (len + 2);
        tempOut.calloc (len + 2);

        for (int i = 0; i <= len; ++i)
            tempIn[i] = s[i];

        ByteCount bytesRead = 0;
        ByteCount outputBufferSize = 0;

        if (ConvertFromUnicodeToText (conversionInfo,
                                      len * sizeof (UniChar), tempIn,
                                      kUnicodeDefaultDirectionMask,
                                      0, 0, 0, 0,
                                      len * sizeof (UniChar), &bytesRead,
                                      &outputBufferSize, tempOut) == noErr)
        {
            result.preallocateStorage (bytesRead / sizeof (UniChar) + 2);

            juce_wchar* t = result;

            unsigned int i;
            for (i = 0; i < bytesRead / sizeof (UniChar); ++i)
                t[i] = (juce_wchar) tempOut[i];

            t[i] = 0;
        }

        DisposeUnicodeToTextInfo (&conversionInfo);
    }

    return result;
#endif
}

//==============================================================================
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

void SystemClipboard::copyTextToClipboard (const String& text)
{
#if JUCE_IOS
    [[UIPasteboard generalPasteboard] setValue: juceStringToNS (text)
                             forPasteboardType: @"public.text"];
#else
    [[NSPasteboard generalPasteboard] declareTypes: [NSArray arrayWithObject: NSStringPboardType]
                                             owner: nil];

    [[NSPasteboard generalPasteboard] setString: juceStringToNS (text)
                                        forType: NSStringPboardType];
#endif
}

const String SystemClipboard::getTextFromClipboard()
{
#if JUCE_IOS
    NSString* text = [[UIPasteboard generalPasteboard] valueForPasteboardType: @"public.text"];
#else
    NSString* text = [[NSPasteboard generalPasteboard] stringForType: NSStringPboardType];
#endif

    return text == 0 ? String::empty
                     : nsStringToJuce (text);
}

#endif

#endif
