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

#include "juce_KeyPress.h"
#include "../juce_Component.h"


//==============================================================================
KeyPress::KeyPress() throw()
    : keyCode (0),
      mods (0),
      textCharacter (0)
{
}

KeyPress::KeyPress (const int keyCode_,
                    const ModifierKeys& mods_,
                    const juce_wchar textCharacter_) throw()
    : keyCode (keyCode_),
      mods (mods_),
      textCharacter (textCharacter_)
{
}

KeyPress::KeyPress (const int keyCode_) throw()
    : keyCode (keyCode_),
      textCharacter (0)
{
}

KeyPress::KeyPress (const KeyPress& other) throw()
    : keyCode (other.keyCode),
      mods (other.mods),
      textCharacter (other.textCharacter)
{
}

KeyPress& KeyPress::operator= (const KeyPress& other) throw()
{
    keyCode = other.keyCode;
    mods = other.mods;
    textCharacter = other.textCharacter;

    return *this;
}

bool KeyPress::operator== (const KeyPress& other) const throw()
{
    return mods.getRawFlags() == other.mods.getRawFlags()
            && (textCharacter == other.textCharacter
                 || textCharacter == 0
                 || other.textCharacter == 0)
            && (keyCode == other.keyCode
                 || (keyCode < 256
                      && other.keyCode < 256
                      && CharacterFunctions::toLowerCase ((juce_wchar) keyCode)
                           == CharacterFunctions::toLowerCase ((juce_wchar) other.keyCode)));
}

bool KeyPress::operator!= (const KeyPress& other) const throw()
{
    return ! operator== (other);
}

bool KeyPress::isCurrentlyDown() const
{
    return isKeyCurrentlyDown (keyCode)
            && (ModifierKeys::getCurrentModifiers().getRawFlags() & ModifierKeys::allKeyboardModifiers)
                  == (mods.getRawFlags() & ModifierKeys::allKeyboardModifiers);
}

//==============================================================================
namespace KeyPressHelpers
{
    struct KeyNameAndCode
    {
        const char* name;
        int code;
    };

    const KeyNameAndCode translations[] =
    {
        { "spacebar",       KeyPress::spaceKey },
        { "return",         KeyPress::returnKey },
        { "escape",         KeyPress::escapeKey },
        { "backspace",      KeyPress::backspaceKey },
        { "cursor left",    KeyPress::leftKey },
        { "cursor right",   KeyPress::rightKey },
        { "cursor up",      KeyPress::upKey },
        { "cursor down",    KeyPress::downKey },
        { "page up",        KeyPress::pageUpKey },
        { "page down",      KeyPress::pageDownKey },
        { "home",           KeyPress::homeKey },
        { "end",            KeyPress::endKey },
        { "delete",         KeyPress::deleteKey },
        { "insert",         KeyPress::insertKey },
        { "tab",            KeyPress::tabKey },
        { "play",           KeyPress::playKey },
        { "stop",           KeyPress::stopKey },
        { "fast forward",   KeyPress::fastForwardKey },
        { "rewind",         KeyPress::rewindKey }
    };

    const String numberPadPrefix()      { return "numpad "; }
}

//==============================================================================
const KeyPress KeyPress::createFromDescription (const String& desc)
{
    int modifiers = 0;

    if (desc.containsWholeWordIgnoreCase ("ctrl")
         || desc.containsWholeWordIgnoreCase ("control")
         || desc.containsWholeWordIgnoreCase ("ctl"))
        modifiers |= ModifierKeys::ctrlModifier;

    if (desc.containsWholeWordIgnoreCase ("shift")
         || desc.containsWholeWordIgnoreCase ("shft"))
        modifiers |= ModifierKeys::shiftModifier;

    if (desc.containsWholeWordIgnoreCase ("alt")
         || desc.containsWholeWordIgnoreCase ("option"))
        modifiers |= ModifierKeys::altModifier;

    if (desc.containsWholeWordIgnoreCase ("command")
         || desc.containsWholeWordIgnoreCase ("cmd"))
        modifiers |= ModifierKeys::commandModifier;

    int key = 0;

    for (int i = 0; i < numElementsInArray (KeyPressHelpers::translations); ++i)
    {
        if (desc.containsWholeWordIgnoreCase (String (KeyPressHelpers::translations[i].name)))
        {
            key = KeyPressHelpers::translations[i].code;
            break;
        }
    }

    if (key == 0)
    {
        // see if it's a numpad key..
        if (desc.containsIgnoreCase (KeyPressHelpers::numberPadPrefix()))
        {
            const juce_wchar lastChar = desc.trimEnd().getLastCharacter();

            if (lastChar >= '0' && lastChar <= '9')
                key = numberPad0 + lastChar - '0';
            else if (lastChar == '+')
                key = numberPadAdd;
            else if (lastChar == '-')
                key = numberPadSubtract;
            else if (lastChar == '*')
                key = numberPadMultiply;
            else if (lastChar == '/')
                key = numberPadDivide;
            else if (lastChar == '.')
                key = numberPadDecimalPoint;
            else if (lastChar == '=')
                key = numberPadEquals;
            else if (desc.endsWith ("separator"))
                key = numberPadSeparator;
            else if (desc.endsWith ("delete"))
                key = numberPadDelete;
        }

        if (key == 0)
        {
            // see if it's a function key..
            if (! desc.containsChar ('#')) // avoid mistaking hex-codes like "#f1"
                for (int i = 1; i <= 12; ++i)
                    if (desc.containsWholeWordIgnoreCase ("f" + String (i)))
                        key = F1Key + i - 1;

            if (key == 0)
            {
                // give up and use the hex code..
                const int hexCode = desc.fromFirstOccurrenceOf ("#", false, false)
                                        .toLowerCase()
                                        .retainCharacters ("0123456789abcdef")
                                        .getHexValue32();

                if (hexCode > 0)
                    key = hexCode;
                else
                    key = CharacterFunctions::toUpperCase (desc.getLastCharacter());
            }
        }
    }

    return KeyPress (key, ModifierKeys (modifiers), 0);
}

const String KeyPress::getTextDescription() const
{
    String desc;

    if (keyCode > 0)
    {
        // some keyboard layouts use a shift-key to get the slash, but in those cases, we
        // want to store it as being a slash, not shift+whatever.
        if (textCharacter == '/')
            return "/";

        if (mods.isCtrlDown())
            desc << "ctrl + ";

        if (mods.isShiftDown())
            desc << "shift + ";

#if JUCE_MAC
          // only do this on the mac, because on Windows ctrl and command are the same,
          // and this would get confusing
          if (mods.isCommandDown())
              desc << "command + ";

        if (mods.isAltDown())
            desc << "option + ";
#else
        if (mods.isAltDown())
            desc << "alt + ";
#endif

        for (int i = 0; i < numElementsInArray (KeyPressHelpers::translations); ++i)
            if (keyCode == KeyPressHelpers::translations[i].code)
                return desc + KeyPressHelpers::translations[i].name;

        if (keyCode >= F1Key && keyCode <= F16Key)
            desc << 'F' << (1 + keyCode - F1Key);
        else if (keyCode >= numberPad0 && keyCode <= numberPad9)
            desc << KeyPressHelpers::numberPadPrefix() << (keyCode - numberPad0);
        else if (keyCode >= 33 && keyCode < 176)
            desc += CharacterFunctions::toUpperCase ((juce_wchar) keyCode);
        else if (keyCode == numberPadAdd)
            desc << KeyPressHelpers::numberPadPrefix() << '+';
        else if (keyCode == numberPadSubtract)
            desc << KeyPressHelpers::numberPadPrefix() << '-';
        else if (keyCode == numberPadMultiply)
            desc << KeyPressHelpers::numberPadPrefix() << '*';
        else if (keyCode == numberPadDivide)
            desc << KeyPressHelpers::numberPadPrefix() << '/';
        else if (keyCode == numberPadSeparator)
            desc << KeyPressHelpers::numberPadPrefix() << "separator";
        else if (keyCode == numberPadDecimalPoint)
            desc << KeyPressHelpers::numberPadPrefix() << '.';
        else if (keyCode == numberPadDelete)
            desc << KeyPressHelpers::numberPadPrefix() << "delete";
        else
            desc << '#' << String::toHexString (keyCode);
    }

    return desc;
}


END_JUCE_NAMESPACE
