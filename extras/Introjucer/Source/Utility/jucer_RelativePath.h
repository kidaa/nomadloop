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

#ifndef __JUCER_RELATIVEPATH_JUCEHEADER__
#define __JUCER_RELATIVEPATH_JUCEHEADER__


//==============================================================================
/** Manipulates a cross-platform partial file path. (Needed because File is designed
    for absolute paths on the active OS)
*/
class RelativePath
{
public:
    //==============================================================================
    enum RootFolder
    {
        unknown,
        projectFolder,
        buildTargetFolder
    };

    //==============================================================================
    RelativePath()
        : root (unknown)
    {}

    RelativePath (const String& path_, const RootFolder root_)
        : path (path_.replaceCharacter ('\\', '/')), root (root_)
    {
    }

    RelativePath (const File& file, const File& rootFolder, const RootFolder root_)
        : path (file.getRelativePathFrom (rootFolder).replaceCharacter ('\\', '/')), root (root_)
    {
    }

    RootFolder getRoot() const                              { return root; }

    String toUnixStyle() const                              { return FileHelpers::unixStylePath (path); }
    String toWindowsStyle() const                           { return FileHelpers::windowsStylePath (path); }

    String getFileName() const                              { return getFakeFile().getFileName(); }
    String getFileNameWithoutExtension() const              { return getFakeFile().getFileNameWithoutExtension(); }

    String getFileExtension() const                         { return getFakeFile().getFileExtension(); }
    bool hasFileExtension (const String& extension) const   { return getFakeFile().hasFileExtension (extension); }
    bool isAbsolute() const                                 { return isAbsolute (path); }

    RelativePath withFileExtension (const String& extension) const
    {
        return RelativePath (path.upToLastOccurrenceOf (".", ! extension.startsWithChar ('.'), false) + extension, root);
    }

    RelativePath getParentDirectory() const
    {
        String p (path);
        if (path.endsWithChar ('/'))
            p = p.dropLastCharacters (1);

        return RelativePath (p.upToLastOccurrenceOf ("/", false, false), root);
    }

    RelativePath getChildFile (const String& subpath) const
    {
        if (isAbsolute (subpath))
            return RelativePath (subpath, root);

        String p (toUnixStyle());
        if (! p.endsWithChar ('/'))
            p << '/';

        return RelativePath (p + subpath, root);
    }

    RelativePath rebased (const File& originalRoot, const File& newRoot, const RootFolder newRootType) const
    {
        if (isAbsolute())
            return RelativePath (path, newRootType);

        return RelativePath (originalRoot.getChildFile (toUnixStyle()).getRelativePathFrom (newRoot), newRootType);
    }

private:
    //==============================================================================
    String path;
    RootFolder root;

    File getFakeFile() const
    {
        return File::getCurrentWorkingDirectory().getChildFile (path);
    }

    static bool isAbsolute (const String& path)
    {
        return File::isAbsolutePath (path)
                || path.startsWithChar ('/') // (needed because File::isAbsolutePath will ignore forward-slashes on win32)
                || path.startsWithChar ('$')
                || path.startsWithChar ('~')
                || (CharacterFunctions::isLetter (path[0]) && path[1] == ':')
                || path.startsWithIgnoreCase ("smb:");
    }
};


#endif   // __JUCER_RELATIVEPATH_JUCEHEADER__
