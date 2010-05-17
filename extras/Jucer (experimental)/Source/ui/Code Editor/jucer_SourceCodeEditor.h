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

#ifndef __JUCER_SOURCECODEEDITOR_JUCEHEADER__
#define __JUCER_SOURCECODEEDITOR_JUCEHEADER__

#include "../../model/Project/jucer_Project.h"
#include "../jucer_DocumentEditorComponent.h"


//==============================================================================
/**
*/
class SourceCodeEditor  : public DocumentEditorComponent
{
public:
    //==============================================================================
    SourceCodeEditor (OpenDocumentManager::Document* document,
                      CodeDocument& codeDocument,
                      CodeTokeniser* const codeTokeniser);

    ~SourceCodeEditor();

    static bool isTextFile (const File& file);
    static bool isCppFile (const File& file);

    void resized();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    CodeEditorComponent* editor;
};


#endif   // __JUCER_SOURCECODEEDITOR_JUCEHEADER__
