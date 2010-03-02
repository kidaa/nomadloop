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

#ifndef __JUCE_FILESEARCHPATH_JUCEHEADER__
#define __JUCE_FILESEARCHPATH_JUCEHEADER__

#include "juce_File.h"
#include "../../text/juce_StringArray.h"


//==============================================================================
/**
    Encapsulates a set of folders that make up a search path.

    @see File
*/
class JUCE_API  FileSearchPath
{
public:
    //==============================================================================
    /** Creates an empty search path. */
    FileSearchPath();

    /** Creates a search path from a string of pathnames.

        The path can be semicolon- or comma-separated, e.g.
        "/foo/bar;/foo/moose;/fish/moose"

        The separate folders are tokenised and added to the search path.
    */
    FileSearchPath (const String& path);

    /** Creates a copy of another search path. */
    FileSearchPath (const FileSearchPath& other);

    /** Destructor. */
    ~FileSearchPath();

    /** Uses a string containing a list of pathnames to re-initialise this list.

        This search path is cleared and the semicolon- or comma-separated folders
        in this string are added instead. e.g. "/foo/bar;/foo/moose;/fish/moose"
    */
    FileSearchPath& operator= (const String& path);

    //==============================================================================
    /** Returns the number of folders in this search path.

        @see operator[]
    */
    int getNumPaths() const;

    /** Returns one of the folders in this search path.

        The file returned isn't guaranteed to actually be a valid directory.

        @see getNumPaths
    */
    const File operator[] (const int index) const;

    /** Returns the search path as a semicolon-separated list of directories. */
    const String toString() const;

    //==============================================================================
    /** Adds a new directory to the search path.

        The new directory is added to the end of the list if the insertIndex parameter is
        less than zero, otherwise it is inserted at the given index.
    */
    void add (const File& directoryToAdd,
              const int insertIndex = -1);

    /** Adds a new directory to the search path if it's not already in there. */
    void addIfNotAlreadyThere (const File& directoryToAdd);

    /** Removes a directory from the search path. */
    void remove (const int indexToRemove);

    /** Merges another search path into this one.

        This will remove any duplicate directories.
    */
    void addPath (const FileSearchPath& other);

    /** Removes any directories that are actually subdirectories of one of the other directories in the search path.

        If the search is intended to be recursive, there's no point having nested folders in the search
        path, because they'll just get searched twice and you'll get duplicate results.

        e.g. if the path is "c:\abc\de;c:\abc", this method will simplify it to "c:\abc"
    */
    void removeRedundantPaths();

    /** Removes any directories that don't actually exist. */
    void removeNonExistentPaths();

    //==============================================================================
    /** Searches the path for a wildcard.

        This will search all the directories in the search path in order, adding any
        matching files to the results array.

        @param results                  an array to append the results to
        @param whatToLookFor            a value from the File::TypesOfFileToFind enum, specifying whether to
                                        return files, directories, or both.
        @param searchRecursively        whether to recursively search the subdirectories too
        @param wildCardPattern          a pattern to match against the filenames
        @returns the number of files added to the array
        @see File::findChildFiles
    */
    int findChildFiles (Array<File>& results,
                        const int whatToLookFor,
                        const bool searchRecursively,
                        const String& wildCardPattern = JUCE_T("*")) const;

    //==============================================================================
    /** Finds out whether a file is inside one of the path's directories.

        This will return true if the specified file is a child of one of the
        directories specified by this path. Note that this doesn't actually do any
        searching or check that the files exist - it just looks at the pathnames
        to work out whether the file would be inside a directory.

        @param fileToCheck      the file to look for
        @param checkRecursively if true, then this will return true if the file is inside a
                                subfolder of one of the path's directories (at any depth). If false
                                it will only return true if the file is actually a direct child
                                of one of the directories.
        @see File::isAChildOf

    */
    bool isFileInPath (const File& fileToCheck,
                       const bool checkRecursively) const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    StringArray directories;

    void init (const String& path);
};

#endif   // __JUCE_FILESEARCHPATH_JUCEHEADER__
