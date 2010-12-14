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

#include "juce_TextEditor.h"
#include "../windows/juce_ComponentPeer.h"
#include "../../graphics/fonts/juce_GlyphArrangement.h"
#include "../../../utilities/juce_SystemClipboard.h"
#include "../../../core/juce_Time.h"
#include "../../../text/juce_LocalisedStrings.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
// a word or space that can't be broken down any further
struct TextAtom
{
    //==============================================================================
    String atomText;
    float width;
    int numChars;

    //==============================================================================
    bool isWhitespace() const       { return CharacterFunctions::isWhitespace (atomText[0]); }
    bool isNewLine() const          { return atomText[0] == '\r' || atomText[0] == '\n'; }

    const String getText (const juce_wchar passwordCharacter) const
    {
        if (passwordCharacter == 0)
            return atomText;
        else
            return String::repeatedString (String::charToString (passwordCharacter),
                                           atomText.length());
    }

    const String getTrimmedText (const juce_wchar passwordCharacter) const
    {
        if (passwordCharacter == 0)
            return atomText.substring (0, numChars);
        else if (isNewLine())
            return String::empty;
        else
            return String::repeatedString (String::charToString (passwordCharacter), numChars);
    }
};

//==============================================================================
// a run of text with a single font and colour
class TextEditor::UniformTextSection
{
public:
    //==============================================================================
    UniformTextSection (const String& text,
                        const Font& font_,
                        const Colour& colour_,
                        const juce_wchar passwordCharacter)
      : font (font_),
        colour (colour_)
    {
        initialiseAtoms (text, passwordCharacter);
    }

    UniformTextSection (const UniformTextSection& other)
      : font (other.font),
        colour (other.colour)
    {
        atoms.ensureStorageAllocated (other.atoms.size());

        for (int i = 0; i < other.atoms.size(); ++i)
            atoms.add (new TextAtom (*other.atoms.getUnchecked(i)));
    }

    ~UniformTextSection()
    {
        // (no need to delete the atoms, as they're explicitly deleted by the caller)
    }

    void clear()
    {
        for (int i = atoms.size(); --i >= 0;)
            delete getAtom(i);

        atoms.clear();
    }

    int getNumAtoms() const
    {
        return atoms.size();
    }

    TextAtom* getAtom (const int index) const throw()
    {
        return atoms.getUnchecked (index);
    }

    void append (const UniformTextSection& other, const juce_wchar passwordCharacter)
    {
        if (other.atoms.size() > 0)
        {
            TextAtom* const lastAtom = atoms.getLast();
            int i = 0;

            if (lastAtom != 0)
            {
                if (! CharacterFunctions::isWhitespace (lastAtom->atomText.getLastCharacter()))
                {
                    TextAtom* const first = other.getAtom(0);

                    if (! CharacterFunctions::isWhitespace (first->atomText[0]))
                    {
                        lastAtom->atomText += first->atomText;
                        lastAtom->numChars = (uint16) (lastAtom->numChars + first->numChars);
                        lastAtom->width = font.getStringWidthFloat (lastAtom->getText (passwordCharacter));
                        delete first;
                        ++i;
                    }
                }
            }

            atoms.ensureStorageAllocated (atoms.size() + other.atoms.size() - i);

            while (i < other.atoms.size())
            {
                atoms.add (other.getAtom(i));
                ++i;
            }
        }
    }

    UniformTextSection* split (const int indexToBreakAt,
                               const juce_wchar passwordCharacter)
    {
        UniformTextSection* const section2 = new UniformTextSection (String::empty,
                                                                     font, colour,
                                                                     passwordCharacter);
        int index = 0;

        for (int i = 0; i < atoms.size(); ++i)
        {
            TextAtom* const atom = getAtom(i);

            const int nextIndex = index + atom->numChars;

            if (index == indexToBreakAt)
            {
                int j;
                for (j = i; j < atoms.size(); ++j)
                    section2->atoms.add (getAtom (j));

                for (j = atoms.size(); --j >= i;)
                    atoms.remove (j);

                break;
            }
            else if (indexToBreakAt >= index && indexToBreakAt < nextIndex)
            {
                TextAtom* const secondAtom = new TextAtom();

                secondAtom->atomText = atom->atomText.substring (indexToBreakAt - index);
                secondAtom->width = font.getStringWidthFloat (secondAtom->getText (passwordCharacter));
                secondAtom->numChars = (uint16) secondAtom->atomText.length();

                section2->atoms.add (secondAtom);

                atom->atomText = atom->atomText.substring (0, indexToBreakAt - index);
                atom->width = font.getStringWidthFloat (atom->getText (passwordCharacter));
                atom->numChars = (uint16) (indexToBreakAt - index);

                int j;
                for (j = i + 1; j < atoms.size(); ++j)
                    section2->atoms.add (getAtom (j));

                for (j = atoms.size(); --j > i;)
                    atoms.remove (j);

                break;
            }

            index = nextIndex;
        }

        return section2;
    }

    void appendAllText (String::Concatenator& concatenator) const
    {
        for (int i = 0; i < atoms.size(); ++i)
            concatenator.append (getAtom(i)->atomText);
    }

    void appendSubstring (String::Concatenator& concatenator,
                          const Range<int>& range) const
    {
        int index = 0;
        for (int i = 0; i < atoms.size(); ++i)
        {
            const TextAtom* const atom = getAtom (i);
            const int nextIndex = index + atom->numChars;

            if (range.getStart() < nextIndex)
            {
                if (range.getEnd() <= index)
                    break;

                const Range<int> r ((range - index).getIntersectionWith (Range<int> (0, (int) atom->numChars)));

                if (! r.isEmpty())
                    concatenator.append (atom->atomText.substring (r.getStart(), r.getEnd()));
            }

            index = nextIndex;
        }
    }

    int getTotalLength() const
    {
        int total = 0;

        for (int i = atoms.size(); --i >= 0;)
            total += getAtom(i)->numChars;

        return total;
    }

    void setFont (const Font& newFont,
                  const juce_wchar passwordCharacter)
    {
        if (font != newFont)
        {
            font = newFont;

            for (int i = atoms.size(); --i >= 0;)
            {
                TextAtom* const atom = atoms.getUnchecked(i);
                atom->width = newFont.getStringWidthFloat (atom->getText (passwordCharacter));
            }
        }
    }

    //==============================================================================
    Font font;
    Colour colour;

private:
    Array <TextAtom*> atoms;

    //==============================================================================
    void initialiseAtoms (const String& textToParse,
                          const juce_wchar passwordCharacter)
    {
        int i = 0;
        const int len = textToParse.length();
        const juce_wchar* const text = textToParse;

        while (i < len)
        {
            int start = i;

            // create a whitespace atom unless it starts with non-ws
            if (CharacterFunctions::isWhitespace (text[i])
                 && text[i] != '\r'
                 && text[i] != '\n')
            {
                while (i < len
                        && CharacterFunctions::isWhitespace (text[i])
                        && text[i] != '\r'
                        && text[i] != '\n')
                {
                    ++i;
                }
            }
            else
            {
                if (text[i] == '\r')
                {
                    ++i;

                    if ((i < len) && (text[i] == '\n'))
                    {
                        ++start;
                        ++i;
                    }
                }
                else if (text[i] == '\n')
                {
                    ++i;
                }
                else
                {
                    while ((i < len) && ! CharacterFunctions::isWhitespace (text[i]))
                        ++i;
                }
            }

            TextAtom* const atom = new TextAtom();
            atom->atomText = String (text + start, i - start);

            atom->width = font.getStringWidthFloat (atom->getText (passwordCharacter));
            atom->numChars = (uint16) (i - start);

            atoms.add (atom);
        }
    }

    UniformTextSection& operator= (const UniformTextSection& other);
    JUCE_LEAK_DETECTOR (UniformTextSection);
};

//==============================================================================
class TextEditor::Iterator
{
public:
    //==============================================================================
    Iterator (const Array <UniformTextSection*>& sections_,
              const float wordWrapWidth_,
              const juce_wchar passwordCharacter_)
      : indexInText (0),
        lineY (0),
        lineHeight (0),
        maxDescent (0),
        atomX (0),
        atomRight (0),
        atom (0),
        currentSection (0),
        sections (sections_),
        sectionIndex (0),
        atomIndex (0),
        wordWrapWidth (wordWrapWidth_),
        passwordCharacter (passwordCharacter_)
    {
        jassert (wordWrapWidth_ > 0);

        if (sections.size() > 0)
        {
            currentSection = sections.getUnchecked (sectionIndex);

            if (currentSection != 0)
                beginNewLine();
        }
    }

    Iterator (const Iterator& other)
      : indexInText (other.indexInText),
        lineY (other.lineY),
        lineHeight (other.lineHeight),
        maxDescent (other.maxDescent),
        atomX (other.atomX),
        atomRight (other.atomRight),
        atom (other.atom),
        currentSection (other.currentSection),
        sections (other.sections),
        sectionIndex (other.sectionIndex),
        atomIndex (other.atomIndex),
        wordWrapWidth (other.wordWrapWidth),
        passwordCharacter (other.passwordCharacter),
        tempAtom (other.tempAtom)
    {
    }

    ~Iterator()
    {
    }

    //==============================================================================
    bool next()
    {
        if (atom == &tempAtom)
        {
            const int numRemaining = tempAtom.atomText.length() - tempAtom.numChars;

            if (numRemaining > 0)
            {
                tempAtom.atomText = tempAtom.atomText.substring (tempAtom.numChars);

                atomX = 0;

                if (tempAtom.numChars > 0)
                    lineY += lineHeight;

                indexInText += tempAtom.numChars;

                GlyphArrangement g;
                g.addLineOfText (currentSection->font, atom->getText (passwordCharacter), 0.0f, 0.0f);

                int split;
                for (split = 0; split < g.getNumGlyphs(); ++split)
                    if (shouldWrap (g.getGlyph (split).getRight()))
                        break;

                if (split > 0 && split <= numRemaining)
                {
                    tempAtom.numChars = (uint16) split;
                    tempAtom.width = g.getGlyph (split - 1).getRight();
                    atomRight = atomX + tempAtom.width;
                    return true;
                }
            }
        }

        bool forceNewLine = false;

        if (sectionIndex >= sections.size())
        {
            moveToEndOfLastAtom();
            return false;
        }
        else if (atomIndex >= currentSection->getNumAtoms() - 1)
        {
            if (atomIndex >= currentSection->getNumAtoms())
            {
                if (++sectionIndex >= sections.size())
                {
                    moveToEndOfLastAtom();
                    return false;
                }

                atomIndex = 0;
                currentSection = sections.getUnchecked (sectionIndex);
            }
            else
            {
                const TextAtom* const lastAtom = currentSection->getAtom (atomIndex);

                if (! lastAtom->isWhitespace())
                {
                    // handle the case where the last atom in a section is actually part of the same
                    // word as the first atom of the next section...
                    float right = atomRight + lastAtom->width;
                    float lineHeight2 = lineHeight;
                    float maxDescent2 = maxDescent;

                    for (int section = sectionIndex + 1; section < sections.size(); ++section)
                    {
                        const UniformTextSection* const s = sections.getUnchecked (section);

                        if (s->getNumAtoms() == 0)
                            break;

                        const TextAtom* const nextAtom = s->getAtom (0);

                        if (nextAtom->isWhitespace())
                            break;

                        right += nextAtom->width;

                        lineHeight2 = jmax (lineHeight2, s->font.getHeight());
                        maxDescent2 = jmax (maxDescent2, s->font.getDescent());

                        if (shouldWrap (right))
                        {
                            lineHeight = lineHeight2;
                            maxDescent = maxDescent2;

                            forceNewLine = true;
                            break;
                        }

                        if (s->getNumAtoms() > 1)
                            break;
                    }
                }
            }
        }

        if (atom != 0)
        {
            atomX = atomRight;
            indexInText += atom->numChars;

            if (atom->isNewLine())
                beginNewLine();
        }

        atom = currentSection->getAtom (atomIndex);
        atomRight = atomX + atom->width;
        ++atomIndex;

        if (shouldWrap (atomRight) || forceNewLine)
        {
            if (atom->isWhitespace())
            {
                // leave whitespace at the end of a line, but truncate it to avoid scrolling
                atomRight = jmin (atomRight, wordWrapWidth);
            }
            else
            {
                atomRight = atom->width;

                if (shouldWrap (atomRight))  // atom too big to fit on a line, so break it up..
                {
                    tempAtom = *atom;
                    tempAtom.width = 0;
                    tempAtom.numChars = 0;
                    atom = &tempAtom;

                    if (atomX > 0)
                        beginNewLine();

                    return next();
                }

                beginNewLine();
                return true;
            }
        }

        return true;
    }

    void beginNewLine()
    {
        atomX = 0;
        lineY += lineHeight;

        int tempSectionIndex = sectionIndex;
        int tempAtomIndex = atomIndex;
        const UniformTextSection* section = sections.getUnchecked (tempSectionIndex);

        lineHeight = section->font.getHeight();
        maxDescent = section->font.getDescent();

        float x = (atom != 0) ? atom->width : 0;

        while (! shouldWrap (x))
        {
            if (tempSectionIndex >= sections.size())
                break;

            bool checkSize = false;

            if (tempAtomIndex >= section->getNumAtoms())
            {
                if (++tempSectionIndex >= sections.size())
                    break;

                tempAtomIndex = 0;
                section = sections.getUnchecked (tempSectionIndex);
                checkSize = true;
            }

            const TextAtom* const nextAtom = section->getAtom (tempAtomIndex);

            if (nextAtom == 0)
                break;

            x += nextAtom->width;

            if (shouldWrap (x) || nextAtom->isNewLine())
                break;

            if (checkSize)
            {
                lineHeight = jmax (lineHeight, section->font.getHeight());
                maxDescent = jmax (maxDescent, section->font.getDescent());
            }

            ++tempAtomIndex;
        }
    }

    //==============================================================================
    void draw (Graphics& g, const UniformTextSection*& lastSection) const
    {
        if (passwordCharacter != 0 || ! atom->isWhitespace())
        {
            if (lastSection != currentSection)
            {
                lastSection = currentSection;
                g.setColour (currentSection->colour);
                g.setFont (currentSection->font);
            }

            jassert (atom->getTrimmedText (passwordCharacter).isNotEmpty());

            GlyphArrangement ga;
            ga.addLineOfText (currentSection->font,
                              atom->getTrimmedText (passwordCharacter),
                              atomX,
                              (float) roundToInt (lineY + lineHeight - maxDescent));
            ga.draw (g);
        }
    }

    void drawSelection (Graphics& g,
                        const Range<int>& selection) const
    {
        const int startX = roundToInt (indexToX (selection.getStart()));
        const int endX   = roundToInt (indexToX (selection.getEnd()));

        const int y = roundToInt (lineY);
        const int nextY = roundToInt (lineY + lineHeight);

        g.fillRect (startX, y, endX - startX, nextY - y);
    }

    void drawSelectedText (Graphics& g,
                           const Range<int>& selection,
                           const Colour& selectedTextColour) const
    {
        if (passwordCharacter != 0 || ! atom->isWhitespace())
        {
            GlyphArrangement ga;
            ga.addLineOfText (currentSection->font,
                              atom->getTrimmedText (passwordCharacter),
                              atomX,
                              (float) roundToInt (lineY + lineHeight - maxDescent));

            if (selection.getEnd() < indexInText + atom->numChars)
            {
                GlyphArrangement ga2 (ga);
                ga2.removeRangeOfGlyphs (0, selection.getEnd() - indexInText);
                ga.removeRangeOfGlyphs (selection.getEnd() - indexInText, -1);

                g.setColour (currentSection->colour);
                ga2.draw (g);
            }

            if (selection.getStart() > indexInText)
            {
                GlyphArrangement ga2 (ga);
                ga2.removeRangeOfGlyphs (selection.getStart() - indexInText, -1);
                ga.removeRangeOfGlyphs (0, selection.getStart() - indexInText);

                g.setColour (currentSection->colour);
                ga2.draw (g);
            }

            g.setColour (selectedTextColour);
            ga.draw (g);
        }
    }

    //==============================================================================
    float indexToX (const int indexToFind) const
    {
        if (indexToFind <= indexInText)
            return atomX;

        if (indexToFind >= indexInText + atom->numChars)
            return atomRight;

        GlyphArrangement g;
        g.addLineOfText (currentSection->font,
                         atom->getText (passwordCharacter),
                         atomX, 0.0f);

        if (indexToFind - indexInText >= g.getNumGlyphs())
            return atomRight;

        return jmin (atomRight, g.getGlyph (indexToFind - indexInText).getLeft());
    }

    int xToIndex (const float xToFind) const
    {
        if (xToFind <= atomX || atom->isNewLine())
            return indexInText;

        if (xToFind >= atomRight)
            return indexInText + atom->numChars;

        GlyphArrangement g;
        g.addLineOfText (currentSection->font,
                         atom->getText (passwordCharacter),
                         atomX, 0.0f);

        int j;
        for (j = 0; j < g.getNumGlyphs(); ++j)
            if ((g.getGlyph(j).getLeft() + g.getGlyph(j).getRight()) / 2 > xToFind)
                break;

        return indexInText + j;
    }

    //==============================================================================
    bool getCharPosition (const int index, float& cx, float& cy, float& lineHeight_)
    {
        while (next())
        {
            if (indexInText + atom->numChars > index)
            {
                cx = indexToX (index);
                cy = lineY;
                lineHeight_ = lineHeight;
                return true;
            }
        }

        cx = atomX;
        cy = lineY;
        lineHeight_ = lineHeight;
        return false;
    }

    //==============================================================================
    int indexInText;
    float lineY, lineHeight, maxDescent;
    float atomX, atomRight;
    const TextAtom* atom;
    const UniformTextSection* currentSection;

private:
    const Array <UniformTextSection*>& sections;
    int sectionIndex, atomIndex;
    const float wordWrapWidth;
    const juce_wchar passwordCharacter;
    TextAtom tempAtom;

    Iterator& operator= (const Iterator&);

    void moveToEndOfLastAtom()
    {
        if (atom != 0)
        {
            atomX = atomRight;

            if (atom->isNewLine())
            {
                atomX = 0.0f;
                lineY += lineHeight;
            }
        }
    }

    bool shouldWrap (const float x) const
    {
        return (x - 0.0001f) >= wordWrapWidth;
    }

    JUCE_LEAK_DETECTOR (Iterator);
};


//==============================================================================
class TextEditor::InsertAction  : public UndoableAction
{
public:
    InsertAction (TextEditor& owner_,
                  const String& text_,
                  const int insertIndex_,
                  const Font& font_,
                  const Colour& colour_,
                  const int oldCaretPos_,
                  const int newCaretPos_)
        : owner (owner_),
          text (text_),
          insertIndex (insertIndex_),
          oldCaretPos (oldCaretPos_),
          newCaretPos (newCaretPos_),
          font (font_),
          colour (colour_)
    {
    }

    bool perform()
    {
        owner.insert (text, insertIndex, font, colour, 0, newCaretPos);
        return true;
    }

    bool undo()
    {
        owner.remove (Range<int> (insertIndex, insertIndex + text.length()), 0, oldCaretPos);
        return true;
    }

    int getSizeInUnits()
    {
        return text.length() + 16;
    }

private:
    TextEditor& owner;
    const String text;
    const int insertIndex, oldCaretPos, newCaretPos;
    const Font font;
    const Colour colour;

    JUCE_DECLARE_NON_COPYABLE (InsertAction);
};

//==============================================================================
class TextEditor::RemoveAction  : public UndoableAction
{
public:
    RemoveAction (TextEditor& owner_,
                  const Range<int> range_,
                  const int oldCaretPos_,
                  const int newCaretPos_,
                  const Array <UniformTextSection*>& removedSections_)
        : owner (owner_),
          range (range_),
          oldCaretPos (oldCaretPos_),
          newCaretPos (newCaretPos_),
          removedSections (removedSections_)
    {
    }

    ~RemoveAction()
    {
        for (int i = removedSections.size(); --i >= 0;)
        {
            UniformTextSection* const section = removedSections.getUnchecked (i);
            section->clear();
            delete section;
        }
    }

    bool perform()
    {
        owner.remove (range, 0, newCaretPos);
        return true;
    }

    bool undo()
    {
        owner.reinsert (range.getStart(), removedSections);
        owner.moveCursorTo (oldCaretPos, false);
        return true;
    }

    int getSizeInUnits()
    {
        int n = 0;

        for (int i = removedSections.size(); --i >= 0;)
            n += removedSections.getUnchecked (i)->getTotalLength();

        return n + 16;
    }

private:
    TextEditor& owner;
    const Range<int> range;
    const int oldCaretPos, newCaretPos;
    Array <UniformTextSection*> removedSections;

    JUCE_DECLARE_NON_COPYABLE (RemoveAction);
};

//==============================================================================
class TextEditor::TextHolderComponent  : public Component,
                                         public Timer,
                                         public ValueListener
{
public:
    TextHolderComponent (TextEditor& owner_)
        : owner (owner_)
    {
        setWantsKeyboardFocus (false);
        setInterceptsMouseClicks (false, true);

        owner.getTextValue().addListener (this);
    }

    ~TextHolderComponent()
    {
        owner.getTextValue().removeListener (this);
    }

    void paint (Graphics& g)
    {
        owner.drawContent (g);
    }

    void timerCallback()
    {
        owner.timerCallbackInt();
    }

    const MouseCursor getMouseCursor()
    {
        return owner.getMouseCursor();
    }

    void valueChanged (Value&)
    {
        owner.textWasChangedByValue();
    }

private:
    TextEditor& owner;

    JUCE_DECLARE_NON_COPYABLE (TextHolderComponent);
};

//==============================================================================
class TextEditorViewport  : public Viewport
{
public:
    TextEditorViewport (TextEditor* const owner_)
        : owner (owner_), lastWordWrapWidth (0), rentrant (false)
    {
    }

    ~TextEditorViewport()
    {
    }

    void visibleAreaChanged (int, int, int, int)
    {
        if (! rentrant) // it's rare, but possible to get into a feedback loop as the viewport's scrollbars
                        // appear and disappear, causing the wrap width to change.
        {
            const float wordWrapWidth = owner->getWordWrapWidth();

            if (wordWrapWidth != lastWordWrapWidth)
            {
                lastWordWrapWidth = wordWrapWidth;

                rentrant = true;
                owner->updateTextHolderSize();
                rentrant = false;
            }
        }
    }

private:
    TextEditor* const owner;
    float lastWordWrapWidth;
    bool rentrant;

    JUCE_DECLARE_NON_COPYABLE (TextEditorViewport);
};

//==============================================================================
namespace TextEditorDefs
{
    const int flashSpeedIntervalMs = 380;

    const int textChangeMessageId = 0x10003001;
    const int returnKeyMessageId  = 0x10003002;
    const int escapeKeyMessageId  = 0x10003003;
    const int focusLossMessageId  = 0x10003004;

    const int maxActionsPerTransaction = 100;

    int getCharacterCategory (const juce_wchar character)
    {
        return CharacterFunctions::isLetterOrDigit (character)
                    ? 2 : (CharacterFunctions::isWhitespace (character) ? 0 : 1);
    }
}

//==============================================================================
TextEditor::TextEditor (const String& name,
                        const juce_wchar passwordCharacter_)
    : Component (name),
      borderSize (1, 1, 1, 3),
      readOnly (false),
      multiline (false),
      wordWrap (false),
      returnKeyStartsNewLine (false),
      caretVisible (true),
      popupMenuEnabled (true),
      selectAllTextWhenFocused (false),
      scrollbarVisible (true),
      wasFocused (false),
      caretFlashState (true),
      keepCursorOnScreen (true),
      tabKeyUsed (false),
      menuActive (false),
      valueTextNeedsUpdating (false),
      cursorX (0),
      cursorY (0),
      cursorHeight (0),
      maxTextLength (0),
      leftIndent (4),
      topIndent (4),
      lastTransactionTime (0),
      currentFont (14.0f),
      totalNumChars (0),
      caretPosition (0),
      passwordCharacter (passwordCharacter_),
      dragType (notDragging)
{
    setOpaque (true);

    addAndMakeVisible (viewport = new TextEditorViewport (this));
    viewport->setViewedComponent (textHolder = new TextHolderComponent (*this));
    viewport->setWantsKeyboardFocus (false);
    viewport->setScrollBarsShown (false, false);

    setMouseCursor (MouseCursor::IBeamCursor);
    setWantsKeyboardFocus (true);
}

TextEditor::~TextEditor()
{
    textValue.referTo (Value());
    clearInternal (0);
    viewport = 0;
    textHolder = 0;
}

//==============================================================================
void TextEditor::newTransaction()
{
    lastTransactionTime = Time::getApproximateMillisecondCounter();
    undoManager.beginNewTransaction();
}

void TextEditor::doUndoRedo (const bool isRedo)
{
    if (! isReadOnly())
    {
        if (isRedo ? undoManager.redo()
                   : undoManager.undo())
        {
            scrollToMakeSureCursorIsVisible();
            repaint();
            textChanged();
        }
    }
}

//==============================================================================
void TextEditor::setMultiLine (const bool shouldBeMultiLine,
                               const bool shouldWordWrap)
{
    if (multiline != shouldBeMultiLine
         || wordWrap != (shouldWordWrap && shouldBeMultiLine))
    {
        multiline = shouldBeMultiLine;
        wordWrap = shouldWordWrap && shouldBeMultiLine;

        viewport->setScrollBarsShown (scrollbarVisible && multiline,
                                      scrollbarVisible && multiline);
        viewport->setViewPosition (0, 0);
        resized();
        scrollToMakeSureCursorIsVisible();
    }
}

bool TextEditor::isMultiLine() const
{
    return multiline;
}

void TextEditor::setScrollbarsShown (bool shown)
{
    if (scrollbarVisible != shown)
    {
        scrollbarVisible = shown;
        shown = shown && isMultiLine();
        viewport->setScrollBarsShown (shown, shown);
    }
}

void TextEditor::setReadOnly (const bool shouldBeReadOnly)
{
    if (readOnly != shouldBeReadOnly)
    {
        readOnly = shouldBeReadOnly;
        enablementChanged();
    }
}

bool TextEditor::isReadOnly() const
{
    return readOnly || ! isEnabled();
}

bool TextEditor::isTextInputActive() const
{
    return ! isReadOnly();
}

void TextEditor::setReturnKeyStartsNewLine (const bool shouldStartNewLine)
{
    returnKeyStartsNewLine = shouldStartNewLine;
}

void TextEditor::setTabKeyUsedAsCharacter (const bool shouldTabKeyBeUsed)
{
    tabKeyUsed = shouldTabKeyBeUsed;
}

void TextEditor::setPopupMenuEnabled (const bool b)
{
    popupMenuEnabled = b;
}

void TextEditor::setSelectAllWhenFocused (const bool b)
{
    selectAllTextWhenFocused = b;
}

//==============================================================================
const Font TextEditor::getFont() const
{
    return currentFont;
}

void TextEditor::setFont (const Font& newFont)
{
    currentFont = newFont;
    scrollToMakeSureCursorIsVisible();
}

void TextEditor::applyFontToAllText (const Font& newFont)
{
    currentFont = newFont;

    const Colour overallColour (findColour (textColourId));

    for (int i = sections.size(); --i >= 0;)
    {
        UniformTextSection* const uts = sections.getUnchecked (i);
        uts->setFont (newFont, passwordCharacter);
        uts->colour = overallColour;
    }

    coalesceSimilarSections();
    updateTextHolderSize();
    scrollToMakeSureCursorIsVisible();
    repaint();
}

void TextEditor::colourChanged()
{
    setOpaque (findColour (backgroundColourId).isOpaque());
    repaint();
}

void TextEditor::setCaretVisible (const bool shouldCaretBeVisible)
{
    caretVisible = shouldCaretBeVisible;

    if (shouldCaretBeVisible)
        textHolder->startTimer (TextEditorDefs::flashSpeedIntervalMs);

    setMouseCursor (shouldCaretBeVisible ? MouseCursor::IBeamCursor
                                         : MouseCursor::NormalCursor);
}

void TextEditor::setInputRestrictions (const int maxLen,
                                       const String& chars)
{
    maxTextLength = jmax (0, maxLen);
    allowedCharacters = chars;
}

void TextEditor::setTextToShowWhenEmpty (const String& text, const Colour& colourToUse)
{
    textToShowWhenEmpty = text;
    colourForTextWhenEmpty = colourToUse;
}

void TextEditor::setPasswordCharacter (const juce_wchar newPasswordCharacter)
{
    if (passwordCharacter != newPasswordCharacter)
    {
        passwordCharacter = newPasswordCharacter;
        resized();
        repaint();
    }
}

void TextEditor::setScrollBarThickness (const int newThicknessPixels)
{
    viewport->setScrollBarThickness (newThicknessPixels);
}

void TextEditor::setScrollBarButtonVisibility (const bool buttonsVisible)
{
    viewport->setScrollBarButtonVisibility (buttonsVisible);
}

//==============================================================================
void TextEditor::clear()
{
    clearInternal (0);
    updateTextHolderSize();
    undoManager.clearUndoHistory();
}

void TextEditor::setText (const String& newText,
                          const bool sendTextChangeMessage)
{
    const int newLength = newText.length();

    if (newLength != getTotalNumChars() || getText() != newText)
    {
        const int oldCursorPos = caretPosition;
        const bool cursorWasAtEnd = oldCursorPos >= getTotalNumChars();

        clearInternal (0);
        insert (newText, 0, currentFont, findColour (textColourId), 0, caretPosition);

        // if you're adding text with line-feeds to a single-line text editor, it
        // ain't gonna look right!
        jassert (multiline || ! newText.containsAnyOf ("\r\n"));

        if (cursorWasAtEnd && ! isMultiLine())
            moveCursorTo (getTotalNumChars(), false);
        else
            moveCursorTo (oldCursorPos, false);

        if (sendTextChangeMessage)
            textChanged();

        updateTextHolderSize();
        scrollToMakeSureCursorIsVisible();
        undoManager.clearUndoHistory();

        repaint();
    }
}

//==============================================================================
Value& TextEditor::getTextValue()
{
    if (valueTextNeedsUpdating)
    {
        valueTextNeedsUpdating = false;
        textValue = getText();
    }

    return textValue;
}

void TextEditor::textWasChangedByValue()
{
    if (textValue.getValueSource().getReferenceCount() > 1)
        setText (textValue.getValue());
}

//==============================================================================
void TextEditor::textChanged()
{
    updateTextHolderSize();
    postCommandMessage (TextEditorDefs::textChangeMessageId);

    if (textValue.getValueSource().getReferenceCount() > 1)
    {
        valueTextNeedsUpdating = false;
        textValue = getText();
    }
}

void TextEditor::returnPressed()
{
    postCommandMessage (TextEditorDefs::returnKeyMessageId);
}

void TextEditor::escapePressed()
{
    postCommandMessage (TextEditorDefs::escapeKeyMessageId);
}

void TextEditor::addListener (TextEditorListener* const newListener)
{
    listeners.add (newListener);
}

void TextEditor::removeListener (TextEditorListener* const listenerToRemove)
{
    listeners.remove (listenerToRemove);
}

//==============================================================================
void TextEditor::timerCallbackInt()
{
    const bool newState = (! caretFlashState) && ! isCurrentlyBlockedByAnotherModalComponent();

    if (caretFlashState != newState)
    {
        caretFlashState = newState;

        if (caretFlashState)
            wasFocused = true;

        if (caretVisible
             && hasKeyboardFocus (false)
             && ! isReadOnly())
        {
            repaintCaret();
        }
    }

    const unsigned int now = Time::getApproximateMillisecondCounter();

    if (now > lastTransactionTime + 200)
        newTransaction();
}

void TextEditor::repaintCaret()
{
    if (! findColour (caretColourId).isTransparent())
        repaint (borderSize.getLeft() + textHolder->getX() + leftIndent + roundToInt (cursorX) - 1,
                 borderSize.getTop() + textHolder->getY() + topIndent + roundToInt (cursorY) - 1,
                 4,
                 roundToInt (cursorHeight) + 2);
}

void TextEditor::repaintText (const Range<int>& range)
{
    if (! range.isEmpty())
    {
        float x = 0, y = 0, lh = currentFont.getHeight();

        const float wordWrapWidth = getWordWrapWidth();

        if (wordWrapWidth > 0)
        {
            Iterator i (sections, wordWrapWidth, passwordCharacter);

            i.getCharPosition (range.getStart(), x, y, lh);

            const int y1 = (int) y;
            int y2;

            if (range.getEnd() >= getTotalNumChars())
            {
                y2 = textHolder->getHeight();
            }
            else
            {
                i.getCharPosition (range.getEnd(), x, y, lh);
                y2 = (int) (y + lh * 2.0f);
            }

            textHolder->repaint (0, y1, textHolder->getWidth(), y2 - y1);
        }
    }
}

//==============================================================================
void TextEditor::moveCaret (int newCaretPos)
{
    if (newCaretPos < 0)
        newCaretPos = 0;
    else if (newCaretPos > getTotalNumChars())
        newCaretPos = getTotalNumChars();

    if (newCaretPos != getCaretPosition())
    {
        repaintCaret();
        caretFlashState = true;
        caretPosition = newCaretPos;
        textHolder->startTimer (TextEditorDefs::flashSpeedIntervalMs);
        scrollToMakeSureCursorIsVisible();
        repaintCaret();
    }
}

void TextEditor::setCaretPosition (const int newIndex)
{
    moveCursorTo (newIndex, false);
}

int TextEditor::getCaretPosition() const
{
    return caretPosition;
}

void TextEditor::scrollEditorToPositionCaret (const int desiredCaretX,
                                              const int desiredCaretY)

{
    updateCaretPosition();

    int vx = roundToInt (cursorX) - desiredCaretX;
    int vy = roundToInt (cursorY) - desiredCaretY;

    if (desiredCaretX < jmax (1, proportionOfWidth (0.05f)))
    {
        vx += desiredCaretX - proportionOfWidth (0.2f);
    }
    else if (desiredCaretX > jmax (0, viewport->getMaximumVisibleWidth() - (wordWrap ? 2 : 10)))
    {
        vx += desiredCaretX + (isMultiLine() ? proportionOfWidth (0.2f) : 10) - viewport->getMaximumVisibleWidth();
    }

    vx = jlimit (0, jmax (0, textHolder->getWidth() + 8 - viewport->getMaximumVisibleWidth()), vx);

    if (! isMultiLine())
    {
        vy = viewport->getViewPositionY();
    }
    else
    {
        vy = jlimit (0, jmax (0, textHolder->getHeight() - viewport->getMaximumVisibleHeight()), vy);

        const int curH = roundToInt (cursorHeight);

        if (desiredCaretY < 0)
        {
            vy = jmax (0, desiredCaretY + vy);
        }
        else if (desiredCaretY > jmax (0, viewport->getMaximumVisibleHeight() - topIndent - curH))
        {
            vy += desiredCaretY + 2 + curH + topIndent - viewport->getMaximumVisibleHeight();
        }
    }

    viewport->setViewPosition (vx, vy);
}

const Rectangle<int> TextEditor::getCaretRectangle()
{
    updateCaretPosition();

    return Rectangle<int> (roundToInt (cursorX) - viewport->getX(),
                           roundToInt (cursorY) - viewport->getY(),
                           1, roundToInt (cursorHeight));
}

//==============================================================================
float TextEditor::getWordWrapWidth() const
{
    return (wordWrap) ? (float) (viewport->getMaximumVisibleWidth() - leftIndent - leftIndent / 2)
                      : 1.0e10f;
}

void TextEditor::updateTextHolderSize()
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        float maxWidth = 0.0f;

        Iterator i (sections, wordWrapWidth, passwordCharacter);

        while (i.next())
            maxWidth = jmax (maxWidth, i.atomRight);

        const int w = leftIndent + roundToInt (maxWidth);
        const int h = topIndent + roundToInt (jmax (i.lineY + i.lineHeight,
                                                    currentFont.getHeight()));

        textHolder->setSize (w + 1, h + 1);
    }
}

int TextEditor::getTextWidth() const
{
    return textHolder->getWidth();
}

int TextEditor::getTextHeight() const
{
    return textHolder->getHeight();
}

void TextEditor::setIndents (const int newLeftIndent,
                             const int newTopIndent)
{
    leftIndent = newLeftIndent;
    topIndent = newTopIndent;
}

void TextEditor::setBorder (const BorderSize& border)
{
    borderSize = border;
    resized();
}

const BorderSize TextEditor::getBorder() const
{
    return borderSize;
}

void TextEditor::setScrollToShowCursor (const bool shouldScrollToShowCursor)
{
    keepCursorOnScreen = shouldScrollToShowCursor;
}

void TextEditor::updateCaretPosition()
{
    cursorHeight = currentFont.getHeight(); // (in case the text is empty and the call below doesn't set this value)
    getCharPosition (caretPosition, cursorX, cursorY, cursorHeight);
}

void TextEditor::scrollToMakeSureCursorIsVisible()
{
    updateCaretPosition();

    if (keepCursorOnScreen)
    {
        int x = viewport->getViewPositionX();
        int y = viewport->getViewPositionY();

        const int relativeCursorX = roundToInt (cursorX) - x;
        const int relativeCursorY = roundToInt (cursorY) - y;

        if (relativeCursorX < jmax (1, proportionOfWidth (0.05f)))
        {
            x += relativeCursorX - proportionOfWidth (0.2f);
        }
        else if (relativeCursorX > jmax (0, viewport->getMaximumVisibleWidth() - (wordWrap ? 2 : 10)))
        {
            x += relativeCursorX + (isMultiLine() ? proportionOfWidth (0.2f) : 10) - viewport->getMaximumVisibleWidth();
        }

        x = jlimit (0, jmax (0, textHolder->getWidth() + 8 - viewport->getMaximumVisibleWidth()), x);

        if (! isMultiLine())
        {
            y = (getHeight() - textHolder->getHeight() - topIndent) / -2;
        }
        else
        {
            const int curH = roundToInt (cursorHeight);

            if (relativeCursorY < 0)
            {
                y = jmax (0, relativeCursorY + y);
            }
            else if (relativeCursorY > jmax (0, viewport->getMaximumVisibleHeight() - topIndent - curH))
            {
                y += relativeCursorY + 2 + curH + topIndent - viewport->getMaximumVisibleHeight();
            }
        }

        viewport->setViewPosition (x, y);
    }
}

void TextEditor::moveCursorTo (const int newPosition,
                               const bool isSelecting)
{
    if (isSelecting)
    {
        moveCaret (newPosition);

        const Range<int> oldSelection (selection);

        if (dragType == notDragging)
        {
            if (abs (getCaretPosition() - selection.getStart()) < abs (getCaretPosition() - selection.getEnd()))
                dragType = draggingSelectionStart;
            else
                dragType = draggingSelectionEnd;
        }

        if (dragType == draggingSelectionStart)
        {
            if (getCaretPosition() >= selection.getEnd())
                dragType = draggingSelectionEnd;

            selection = Range<int>::between (getCaretPosition(), selection.getEnd());
        }
        else
        {
            if (getCaretPosition() < selection.getStart())
                dragType = draggingSelectionStart;

            selection = Range<int>::between (getCaretPosition(), selection.getStart());
        }

        repaintText (selection.getUnionWith (oldSelection));
    }
    else
    {
        dragType = notDragging;

        repaintText (selection);

        moveCaret (newPosition);
        selection = Range<int>::emptyRange (getCaretPosition());
    }
}

int TextEditor::getTextIndexAt (const int x,
                                const int y)
{
    return indexAtPosition ((float) (x + viewport->getViewPositionX() - leftIndent),
                            (float) (y + viewport->getViewPositionY() - topIndent));
}

void TextEditor::insertTextAtCaret (const String& newText_)
{
    String newText (newText_);

    if (allowedCharacters.isNotEmpty())
        newText = newText.retainCharacters (allowedCharacters);

    if (! isMultiLine())
        newText = newText.replaceCharacters ("\r\n", "  ");
    else
        newText = newText.replace ("\r\n", "\n");

    const int newCaretPos = selection.getStart() + newText.length();
    const int insertIndex = selection.getStart();

    remove (selection, getUndoManager(),
            newText.isNotEmpty() ? newCaretPos - 1 : newCaretPos);

    if (maxTextLength > 0)
        newText = newText.substring (0, maxTextLength - getTotalNumChars());

    if (newText.isNotEmpty())
        insert (newText,
                insertIndex,
                currentFont,
                findColour (textColourId),
                getUndoManager(),
                newCaretPos);

    textChanged();
}

void TextEditor::setHighlightedRegion (const Range<int>& newSelection)
{
    moveCursorTo (newSelection.getStart(), false);
    moveCursorTo (newSelection.getEnd(), true);
}

//==============================================================================
void TextEditor::copy()
{
    if (passwordCharacter == 0)
    {
        const String selectedText (getHighlightedText());

        if (selectedText.isNotEmpty())
            SystemClipboard::copyTextToClipboard (selectedText);
    }
}

void TextEditor::paste()
{
    if (! isReadOnly())
    {
        const String clip (SystemClipboard::getTextFromClipboard());

        if (clip.isNotEmpty())
            insertTextAtCaret (clip);
    }
}

void TextEditor::cut()
{
    if (! isReadOnly())
    {
        moveCaret (selection.getEnd());
        insertTextAtCaret (String::empty);
    }
}

//==============================================================================
void TextEditor::drawContent (Graphics& g)
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        g.setOrigin (leftIndent, topIndent);
        const Rectangle<int> clip (g.getClipBounds());
        Colour selectedTextColour;

        Iterator i (sections, wordWrapWidth, passwordCharacter);

        while (i.lineY + 200.0 < clip.getY() && i.next())
        {}

        if (! selection.isEmpty())
        {
            g.setColour (findColour (highlightColourId)
                            .withMultipliedAlpha (hasKeyboardFocus (true) ? 1.0f : 0.5f));

            selectedTextColour = findColour (highlightedTextColourId);

            Iterator i2 (i);

            while (i2.next() && i2.lineY < clip.getBottom())
            {
                if (i2.lineY + i2.lineHeight >= clip.getY()
                     && selection.intersects (Range<int> (i2.indexInText, i2.indexInText + i2.atom->numChars)))
                {
                    i2.drawSelection (g, selection);
                }
            }
        }

        const UniformTextSection* lastSection = 0;

        while (i.next() && i.lineY < clip.getBottom())
        {
            if (i.lineY + i.lineHeight >= clip.getY())
            {
                if (selection.intersects (Range<int> (i.indexInText, i.indexInText + i.atom->numChars)))
                {
                    i.drawSelectedText (g, selection, selectedTextColour);
                    lastSection = 0;
                }
                else
                {
                    i.draw (g, lastSection);
                }
            }
        }
    }
}

void TextEditor::paint (Graphics& g)
{
    getLookAndFeel().fillTextEditorBackground (g, getWidth(), getHeight(), *this);
}

void TextEditor::paintOverChildren (Graphics& g)
{
    if (caretFlashState
         && hasKeyboardFocus (false)
         && caretVisible
         && ! isReadOnly())
    {
        g.setColour (findColour (caretColourId));

        g.fillRect (borderSize.getLeft() + textHolder->getX() + leftIndent + cursorX,
                    borderSize.getTop() + textHolder->getY() + topIndent + cursorY,
                    2.0f, cursorHeight);
    }

    if (textToShowWhenEmpty.isNotEmpty()
         && (! hasKeyboardFocus (false))
         && getTotalNumChars() == 0)
    {
        g.setColour (colourForTextWhenEmpty);
        g.setFont (getFont());

        if (isMultiLine())
        {
            g.drawText (textToShowWhenEmpty,
                        0, 0, getWidth(), getHeight(),
                        Justification::centred, true);
        }
        else
        {
            g.drawText (textToShowWhenEmpty,
                        leftIndent, topIndent,
                        viewport->getWidth() - leftIndent,
                        viewport->getHeight() - topIndent,
                        Justification::centredLeft, true);
        }
    }

    getLookAndFeel().drawTextEditorOutline (g, getWidth(), getHeight(), *this);
}

//==============================================================================
class TextEditorMenuPerformer  : public ModalComponentManager::Callback
{
public:
    TextEditorMenuPerformer (TextEditor* const editor_)
        : editor (editor_)
    {
    }

    void modalStateFinished (int returnValue)
    {
        if (editor != 0 && returnValue != 0)
            editor->performPopupMenuAction (returnValue);
    }

private:
    Component::SafePointer<TextEditor> editor;

    JUCE_DECLARE_NON_COPYABLE (TextEditorMenuPerformer);
};


void TextEditor::mouseDown (const MouseEvent& e)
{
    beginDragAutoRepeat (100);
    newTransaction();

    if (wasFocused || ! selectAllTextWhenFocused)
    {
        if (! (popupMenuEnabled && e.mods.isPopupMenu()))
        {
            moveCursorTo (getTextIndexAt (e.x, e.y),
                          e.mods.isShiftDown());
        }
        else
        {
            PopupMenu m;
            m.setLookAndFeel (&getLookAndFeel());
            addPopupMenuItems (m, &e);

            m.show (0, 0, 0, 0, new TextEditorMenuPerformer (this));
        }
    }
}

void TextEditor::mouseDrag (const MouseEvent& e)
{
    if (wasFocused || ! selectAllTextWhenFocused)
    {
        if (! (popupMenuEnabled && e.mods.isPopupMenu()))
        {
            moveCursorTo (getTextIndexAt (e.x, e.y), true);
        }
    }
}

void TextEditor::mouseUp (const MouseEvent& e)
{
    newTransaction();
    textHolder->startTimer (TextEditorDefs::flashSpeedIntervalMs);

    if (wasFocused || ! selectAllTextWhenFocused)
    {
        if (e.mouseWasClicked() && ! (popupMenuEnabled && e.mods.isPopupMenu()))
        {
            moveCaret (getTextIndexAt (e.x, e.y));
        }
    }

    wasFocused = true;
}

void TextEditor::mouseDoubleClick (const MouseEvent& e)
{
    int tokenEnd = getTextIndexAt (e.x, e.y);
    int tokenStart = tokenEnd;

    if (e.getNumberOfClicks() > 3)
    {
        tokenStart = 0;
        tokenEnd = getTotalNumChars();
    }
    else
    {
        const String t (getText());
        const int totalLength = getTotalNumChars();

        while (tokenEnd < totalLength)
        {
            // (note the slight bodge here - it's because iswalnum only checks for alphabetic chars in the current locale)
            if (CharacterFunctions::isLetterOrDigit (t [tokenEnd]) || t [tokenEnd] > 128)
                ++tokenEnd;
            else
                break;
        }

        tokenStart = tokenEnd;

        while (tokenStart > 0)
        {
            // (note the slight bodge here - it's because iswalnum only checks for alphabetic chars in the current locale)
            if (CharacterFunctions::isLetterOrDigit (t [tokenStart - 1]) || t [tokenStart - 1] > 128)
                --tokenStart;
            else
                break;
        }

        if (e.getNumberOfClicks() > 2)
        {
            while (tokenEnd < totalLength)
            {
                if (t [tokenEnd] != '\r' && t [tokenEnd] != '\n')
                    ++tokenEnd;
                else
                    break;
            }

            while (tokenStart > 0)
            {
                if (t [tokenStart - 1] != '\r' && t [tokenStart - 1] != '\n')
                    --tokenStart;
                else
                    break;
            }
        }
    }

    moveCursorTo (tokenEnd, false);
    moveCursorTo (tokenStart, true);
}

void TextEditor::mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
{
    if (! viewport->useMouseWheelMoveIfNeeded (e, wheelIncrementX, wheelIncrementY))
        Component::mouseWheelMove (e, wheelIncrementX, wheelIncrementY);
}

//==============================================================================
bool TextEditor::keyPressed (const KeyPress& key)
{
    if (isReadOnly() && key != KeyPress ('c', ModifierKeys::commandModifier, 0))
        return false;

    const bool moveInWholeWordSteps = key.getModifiers().isCtrlDown() || key.getModifiers().isAltDown();

    if (key.isKeyCode (KeyPress::leftKey)
         || key.isKeyCode (KeyPress::upKey))
    {
        newTransaction();

        int newPos;

        if (isMultiLine() && key.isKeyCode (KeyPress::upKey))
            newPos = indexAtPosition (cursorX, cursorY - 1);
        else if (moveInWholeWordSteps)
            newPos = findWordBreakBefore (getCaretPosition());
        else
            newPos = getCaretPosition() - 1;

        moveCursorTo (newPos, key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::rightKey)
              || key.isKeyCode (KeyPress::downKey))
    {
        newTransaction();

        int newPos;

        if (isMultiLine() && key.isKeyCode (KeyPress::downKey))
            newPos = indexAtPosition (cursorX, cursorY + cursorHeight + 1);
        else if (moveInWholeWordSteps)
            newPos = findWordBreakAfter (getCaretPosition());
        else
            newPos = getCaretPosition() + 1;

        moveCursorTo (newPos, key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::pageDownKey) && isMultiLine())
    {
        newTransaction();

        moveCursorTo (indexAtPosition (cursorX, cursorY + cursorHeight + viewport->getViewHeight()),
                      key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::pageUpKey) && isMultiLine())
    {
        newTransaction();

        moveCursorTo (indexAtPosition (cursorX, cursorY - viewport->getViewHeight()),
                      key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::homeKey))
    {
        newTransaction();

        if (isMultiLine() && ! moveInWholeWordSteps)
            moveCursorTo (indexAtPosition (0.0f, cursorY),
                          key.getModifiers().isShiftDown());
        else
            moveCursorTo (0, key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::endKey))
    {
        newTransaction();

        if (isMultiLine() && ! moveInWholeWordSteps)
            moveCursorTo (indexAtPosition ((float) textHolder->getWidth(), cursorY),
                          key.getModifiers().isShiftDown());
        else
            moveCursorTo (getTotalNumChars(), key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::backspaceKey))
    {
        if (moveInWholeWordSteps)
        {
            moveCursorTo (findWordBreakBefore (getCaretPosition()), true);
        }
        else
        {
            if (selection.isEmpty() && selection.getStart() > 0)
                selection.setStart (selection.getEnd() - 1);
        }

        cut();
    }
    else if (key.isKeyCode (KeyPress::deleteKey))
    {
        if (key.getModifiers().isShiftDown())
            copy();

        if (selection.isEmpty() && selection.getStart() < getTotalNumChars())
            selection.setEnd (selection.getStart() + 1);

        cut();
    }
    else if (key == KeyPress ('c', ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::insertKey, ModifierKeys::ctrlModifier, 0))
    {
        newTransaction();
        copy();
    }
    else if (key == KeyPress ('x', ModifierKeys::commandModifier, 0))
    {
        newTransaction();
        copy();
        cut();
    }
    else if (key == KeyPress ('v', ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::insertKey, ModifierKeys::shiftModifier, 0))
    {
        newTransaction();
        paste();
    }
    else if (key == KeyPress ('z', ModifierKeys::commandModifier, 0))
    {
        newTransaction();
        doUndoRedo (false);
    }
    else if (key == KeyPress ('y', ModifierKeys::commandModifier, 0))
    {
        newTransaction();
        doUndoRedo (true);
    }
    else if (key == KeyPress ('a', ModifierKeys::commandModifier, 0))
    {
        newTransaction();
        moveCursorTo (getTotalNumChars(), false);
        moveCursorTo (0, true);
    }
    else if (key == KeyPress::returnKey)
    {
        newTransaction();

        if (returnKeyStartsNewLine)
            insertTextAtCaret ("\n");
        else
            returnPressed();
    }
    else if (key.isKeyCode (KeyPress::escapeKey))
    {
        newTransaction();
        moveCursorTo (getCaretPosition(), false);
        escapePressed();
    }
    else if (key.getTextCharacter() >= ' '
              || (tabKeyUsed && (key.getTextCharacter() == '\t')))
    {
        insertTextAtCaret (String::charToString (key.getTextCharacter()));

        lastTransactionTime = Time::getApproximateMillisecondCounter();
    }
    else
    {
        return false;
    }

    return true;
}

bool TextEditor::keyStateChanged (const bool isKeyDown)
{
    if (! isKeyDown)
        return false;

#if JUCE_WINDOWS
    if (KeyPress (KeyPress::F4Key, ModifierKeys::altModifier, 0).isCurrentlyDown())
        return false;  // We need to explicitly allow alt-F4 to pass through on Windows
#endif

    // (overridden to avoid forwarding key events to the parent)
    return ! ModifierKeys::getCurrentModifiers().isCommandDown();
}

//==============================================================================
const int baseMenuItemID = 0x7fff0000;

void TextEditor::addPopupMenuItems (PopupMenu& m, const MouseEvent*)
{
    const bool writable = ! isReadOnly();

    if (passwordCharacter == 0)
    {
        m.addItem (baseMenuItemID + 1, TRANS("cut"), writable);
        m.addItem (baseMenuItemID + 2, TRANS("copy"), ! selection.isEmpty());
        m.addItem (baseMenuItemID + 3, TRANS("paste"), writable);
    }

    m.addItem (baseMenuItemID + 4, TRANS("delete"), writable);
    m.addSeparator();
    m.addItem (baseMenuItemID + 5, TRANS("select all"));
    m.addSeparator();

    if (getUndoManager() != 0)
    {
        m.addItem (baseMenuItemID + 6, TRANS("undo"), undoManager.canUndo());
        m.addItem (baseMenuItemID + 7, TRANS("redo"), undoManager.canRedo());
    }
}

void TextEditor::performPopupMenuAction (const int menuItemID)
{
    switch (menuItemID)
    {
    case baseMenuItemID + 1:
        copy();
        cut();
        break;

    case baseMenuItemID + 2:
        copy();
        break;

    case baseMenuItemID + 3:
        paste();
        break;

    case baseMenuItemID + 4:
        cut();
        break;

    case baseMenuItemID + 5:
        moveCursorTo (getTotalNumChars(), false);
        moveCursorTo (0, true);
        break;

    case baseMenuItemID + 6:
        doUndoRedo (false);
        break;

    case baseMenuItemID + 7:
        doUndoRedo (true);
        break;

    default:
        break;
    }
}

//==============================================================================
void TextEditor::focusGained (FocusChangeType)
{
    newTransaction();

    caretFlashState = true;

    if (selectAllTextWhenFocused)
    {
        moveCursorTo (0, false);
        moveCursorTo (getTotalNumChars(), true);
    }

    repaint();

    if (caretVisible)
        textHolder->startTimer (TextEditorDefs::flashSpeedIntervalMs);

    ComponentPeer* const peer = getPeer();
    if (peer != 0 && ! isReadOnly())
        peer->textInputRequired (getScreenPosition() - peer->getScreenPosition());
}

void TextEditor::focusLost (FocusChangeType)
{
    newTransaction();

    wasFocused = false;
    textHolder->stopTimer();
    caretFlashState = false;

    postCommandMessage (TextEditorDefs::focusLossMessageId);
    repaint();
}

//==============================================================================
void TextEditor::resized()
{
    viewport->setBoundsInset (borderSize);
    viewport->setSingleStepSizes (16, roundToInt (currentFont.getHeight()));

    updateTextHolderSize();

    if (! isMultiLine())
    {
        scrollToMakeSureCursorIsVisible();
    }
    else
    {
        updateCaretPosition();
    }
}

void TextEditor::handleCommandMessage (const int commandId)
{
    Component::BailOutChecker checker (this);

    switch (commandId)
    {
    case TextEditorDefs::textChangeMessageId:
        listeners.callChecked (checker, &TextEditorListener::textEditorTextChanged, (TextEditor&) *this);
        break;

    case TextEditorDefs::returnKeyMessageId:
        listeners.callChecked (checker, &TextEditorListener::textEditorReturnKeyPressed, (TextEditor&) *this);
        break;

    case TextEditorDefs::escapeKeyMessageId:
        listeners.callChecked (checker, &TextEditorListener::textEditorEscapeKeyPressed, (TextEditor&) *this);
        break;

    case TextEditorDefs::focusLossMessageId:
        listeners.callChecked (checker, &TextEditorListener::textEditorFocusLost, (TextEditor&) *this);
        break;

    default:
        jassertfalse;
        break;
    }
}

void TextEditor::enablementChanged()
{
    setMouseCursor (isReadOnly() ? MouseCursor::NormalCursor
                                 : MouseCursor::IBeamCursor);
    repaint();
}

//==============================================================================
UndoManager* TextEditor::getUndoManager() throw()
{
    return isReadOnly() ? 0 : &undoManager;
}

void TextEditor::clearInternal (UndoManager* const um)
{
    remove (Range<int> (0, getTotalNumChars()), um, caretPosition);
}

void TextEditor::insert (const String& text,
                         const int insertIndex,
                         const Font& font,
                         const Colour& colour,
                         UndoManager* const um,
                         const int caretPositionToMoveTo)
{
    if (text.isNotEmpty())
    {
        if (um != 0)
        {
            if (um->getNumActionsInCurrentTransaction() > TextEditorDefs::maxActionsPerTransaction)
                newTransaction();

            um->perform (new InsertAction (*this, text, insertIndex, font, colour,
                                           caretPosition, caretPositionToMoveTo));
        }
        else
        {
            repaintText (Range<int> (insertIndex, getTotalNumChars())); // must do this before and after changing the data, in case
                                                                        // a line gets moved due to word wrap

            int index = 0;
            int nextIndex = 0;

            for (int i = 0; i < sections.size(); ++i)
            {
                nextIndex = index + sections.getUnchecked (i)->getTotalLength();

                if (insertIndex == index)
                {
                    sections.insert (i, new UniformTextSection (text,
                                                                font, colour,
                                                                passwordCharacter));
                    break;
                }
                else if (insertIndex > index && insertIndex < nextIndex)
                {
                    splitSection (i, insertIndex - index);
                    sections.insert (i + 1, new UniformTextSection (text,
                                                                    font, colour,
                                                                    passwordCharacter));
                    break;
                }

                index = nextIndex;
            }

            if (nextIndex == insertIndex)
                sections.add (new UniformTextSection (text,
                                                      font, colour,
                                                      passwordCharacter));

            coalesceSimilarSections();
            totalNumChars = -1;
            valueTextNeedsUpdating = true;

            moveCursorTo (caretPositionToMoveTo, false);

            repaintText (Range<int> (insertIndex, getTotalNumChars()));
        }
    }
}

void TextEditor::reinsert (const int insertIndex,
                           const Array <UniformTextSection*>& sectionsToInsert)
{
    int index = 0;
    int nextIndex = 0;

    for (int i = 0; i < sections.size(); ++i)
    {
        nextIndex = index + sections.getUnchecked (i)->getTotalLength();

        if (insertIndex == index)
        {
            for (int j = sectionsToInsert.size(); --j >= 0;)
                sections.insert (i, new UniformTextSection (*sectionsToInsert.getUnchecked(j)));

            break;
        }
        else if (insertIndex > index && insertIndex < nextIndex)
        {
            splitSection (i, insertIndex - index);

            for (int j = sectionsToInsert.size(); --j >= 0;)
                sections.insert (i + 1, new UniformTextSection (*sectionsToInsert.getUnchecked(j)));

            break;
        }

        index = nextIndex;
    }

    if (nextIndex == insertIndex)
    {
        for (int j = 0; j < sectionsToInsert.size(); ++j)
            sections.add (new UniformTextSection (*sectionsToInsert.getUnchecked(j)));
    }

    coalesceSimilarSections();
    totalNumChars = -1;
    valueTextNeedsUpdating = true;
}

void TextEditor::remove (const Range<int>& range,
                         UndoManager* const um,
                         const int caretPositionToMoveTo)
{
    if (! range.isEmpty())
    {
        int index = 0;

        for (int i = 0; i < sections.size(); ++i)
        {
            const int nextIndex = index + sections.getUnchecked(i)->getTotalLength();

            if (range.getStart() > index && range.getStart() < nextIndex)
            {
                splitSection (i, range.getStart() - index);
                --i;
            }
            else if (range.getEnd() > index && range.getEnd() < nextIndex)
            {
                splitSection (i, range.getEnd() - index);
                --i;
            }
            else
            {
                index = nextIndex;

                if (index > range.getEnd())
                    break;
            }
        }

        index = 0;

        if (um != 0)
        {
            Array <UniformTextSection*> removedSections;

            for (int i = 0; i < sections.size(); ++i)
            {
                if (range.getEnd() <= range.getStart())
                    break;

                UniformTextSection* const section = sections.getUnchecked (i);

                const int nextIndex = index + section->getTotalLength();

                if (range.getStart() <= index && range.getEnd() >= nextIndex)
                    removedSections.add (new UniformTextSection (*section));

                index = nextIndex;
            }

            if (um->getNumActionsInCurrentTransaction() > TextEditorDefs::maxActionsPerTransaction)
                newTransaction();

            um->perform (new RemoveAction (*this, range, caretPosition,
                                           caretPositionToMoveTo, removedSections));
        }
        else
        {
            Range<int> remainingRange (range);

            for (int i = 0; i < sections.size(); ++i)
            {
                UniformTextSection* const section = sections.getUnchecked (i);

                const int nextIndex = index + section->getTotalLength();

                if (remainingRange.getStart() <= index && remainingRange.getEnd() >= nextIndex)
                {
                    sections.remove(i);
                    section->clear();
                    delete section;

                    remainingRange.setEnd (remainingRange.getEnd() - (nextIndex - index));
                    if (remainingRange.isEmpty())
                        break;

                    --i;
                }
                else
                {
                    index = nextIndex;
                }
            }

            coalesceSimilarSections();
            totalNumChars = -1;
            valueTextNeedsUpdating = true;

            moveCursorTo (caretPositionToMoveTo, false);

            repaintText (Range<int> (range.getStart(), getTotalNumChars()));
        }
    }
}

//==============================================================================
const String TextEditor::getText() const
{
    String t;
    t.preallocateStorage (getTotalNumChars());
    String::Concatenator concatenator (t);

    for (int i = 0; i < sections.size(); ++i)
        sections.getUnchecked (i)->appendAllText (concatenator);

    return t;
}

const String TextEditor::getTextInRange (const Range<int>& range) const
{
    String t;

    if (! range.isEmpty())
    {
        t.preallocateStorage (jmin (getTotalNumChars(), range.getLength()));
        String::Concatenator concatenator (t);
        int index = 0;

        for (int i = 0; i < sections.size(); ++i)
        {
            const UniformTextSection* const s = sections.getUnchecked (i);
            const int nextIndex = index + s->getTotalLength();

            if (range.getStart() < nextIndex)
            {
                if (range.getEnd() <= index)
                    break;

                s->appendSubstring (concatenator, range - index);
            }

            index = nextIndex;
        }
    }

    return t;
}

const String TextEditor::getHighlightedText() const
{
    return getTextInRange (selection);
}

int TextEditor::getTotalNumChars() const
{
    if (totalNumChars < 0)
    {
        totalNumChars = 0;

        for (int i = sections.size(); --i >= 0;)
            totalNumChars += sections.getUnchecked (i)->getTotalLength();
    }

    return totalNumChars;
}

bool TextEditor::isEmpty() const
{
    return getTotalNumChars() == 0;
}

void TextEditor::getCharPosition (const int index, float& cx, float& cy, float& lineHeight) const
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0 && sections.size() > 0)
    {
        Iterator i (sections, wordWrapWidth, passwordCharacter);

        i.getCharPosition (index, cx, cy, lineHeight);
    }
    else
    {
        cx = cy = 0;
        lineHeight = currentFont.getHeight();
    }
}

int TextEditor::indexAtPosition (const float x, const float y)
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        Iterator i (sections, wordWrapWidth, passwordCharacter);

        while (i.next())
        {
            if (i.lineY + i.lineHeight > y)
            {
                if (i.lineY > y)
                    return jmax (0, i.indexInText - 1);

                if (i.atomX >= x)
                    return i.indexInText;

                if (x < i.atomRight)
                    return i.xToIndex (x);
            }
        }
    }

    return getTotalNumChars();
}

//==============================================================================
int TextEditor::findWordBreakAfter (const int position) const
{
    const String t (getTextInRange (Range<int> (position, position + 512)));
    const int totalLength = t.length();
    int i = 0;

    while (i < totalLength && CharacterFunctions::isWhitespace (t[i]))
        ++i;

    const int type = TextEditorDefs::getCharacterCategory (t[i]);

    while (i < totalLength && type == TextEditorDefs::getCharacterCategory (t[i]))
        ++i;

    while (i < totalLength && CharacterFunctions::isWhitespace (t[i]))
        ++i;

    return position + i;
}

int TextEditor::findWordBreakBefore (const int position) const
{
    if (position <= 0)
        return 0;

    const int startOfBuffer = jmax (0, position - 512);
    const String t (getTextInRange (Range<int> (startOfBuffer, position)));

    int i = position - startOfBuffer;

    while (i > 0 && CharacterFunctions::isWhitespace (t [i - 1]))
        --i;

    if (i > 0)
    {
        const int type = TextEditorDefs::getCharacterCategory (t [i - 1]);

        while (i > 0 && type == TextEditorDefs::getCharacterCategory (t [i - 1]))
            --i;
    }

    jassert (startOfBuffer + i >= 0);
    return startOfBuffer + i;
}


//==============================================================================
void TextEditor::splitSection (const int sectionIndex,
                               const int charToSplitAt)
{
    jassert (sections[sectionIndex] != 0);

    sections.insert (sectionIndex + 1,
                     sections.getUnchecked (sectionIndex)->split (charToSplitAt, passwordCharacter));
}

void TextEditor::coalesceSimilarSections()
{
    for (int i = 0; i < sections.size() - 1; ++i)
    {
        UniformTextSection* const s1 = sections.getUnchecked (i);
        UniformTextSection* const s2 = sections.getUnchecked (i + 1);

        if (s1->font == s2->font
             && s1->colour == s2->colour)
        {
            s1->append (*s2, passwordCharacter);
            sections.remove (i + 1);
            delete s2;
            --i;
        }
    }
}


END_JUCE_NAMESPACE
