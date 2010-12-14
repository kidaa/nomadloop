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

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
bool File::copyInternal (const File& dest) const
{
    const ScopedAutoReleasePool pool;
    NSFileManager* fm = [NSFileManager defaultManager];

    return [fm fileExistsAtPath: juceStringToNS (fullPath)]
#if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
            && [fm copyItemAtPath: juceStringToNS (fullPath)
                           toPath: juceStringToNS (dest.getFullPathName())
                            error: nil];
#else
            && [fm copyPath: juceStringToNS (fullPath)
                     toPath: juceStringToNS (dest.getFullPathName())
                    handler: nil];
#endif
}

void File::findFileSystemRoots (Array<File>& destArray)
{
    destArray.add (File ("/"));
}


//==============================================================================
namespace FileHelpers
{
    bool isFileOnDriveType (const File& f, const char* const* types)
    {
        struct statfs buf;

        if (juce_doStatFS (f, buf))
        {
            const String type (buf.f_fstypename);

            while (*types != 0)
                if (type.equalsIgnoreCase (*types++))
                    return true;
        }

        return false;
    }

    bool isHiddenFile (const String& path)
    {
      #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        const ScopedAutoReleasePool pool;
        NSNumber* hidden = nil;
        NSError* err = nil;

        return [[NSURL fileURLWithPath: juceStringToNS (path)]
                    getResourceValue: &hidden forKey: NSURLIsHiddenKey error: &err]
                && [hidden boolValue];
      #else
        #if JUCE_IOS
        return File (path).getFileName().startsWithChar ('.');
        #else
        FSRef ref;
        LSItemInfoRecord info;

        return FSPathMakeRefWithOptions ((const UInt8*) path.toUTF8(), kFSPathMakeRefDoNotFollowLeafSymlink, &ref, 0) == noErr
                 && LSCopyItemInfoForRef (&ref, kLSRequestBasicFlagsOnly, &info) == noErr
                 && (info.flags & kLSItemInfoIsInvisible) != 0;
        #endif
      #endif
    }

  #if JUCE_IOS
    const String getIOSSystemLocation (NSSearchPathDirectory type)
    {
        return nsStringToJuce ([NSSearchPathForDirectoriesInDomains (type, NSUserDomainMask, YES)
                                objectAtIndex: 0]);
    }
  #endif

    bool launchExecutable (const String& pathAndArguments)
    {
        const char* const argv[4] = { "/bin/sh", "-c", pathAndArguments.toUTF8(), 0 };

        const int cpid = fork();

        if (cpid == 0)
        {
            // Child process
            if (execve (argv[0], (char**) argv, 0) < 0)
                exit (0);
        }
        else
        {
            if (cpid < 0)
                return false;
        }

        return true;
    }
}

bool File::isOnCDRomDrive() const
{
    const char* const cdTypes[] = { "cd9660", "cdfs", "cddafs", "udf", 0 };

    return FileHelpers::isFileOnDriveType (*this, cdTypes);
}

bool File::isOnHardDisk() const
{
    const char* const nonHDTypes[] = { "nfs", "smbfs", "ramfs", 0 };

    return ! (isOnCDRomDrive() || FileHelpers::isFileOnDriveType (*this, nonHDTypes));
}

bool File::isOnRemovableDrive() const
{
  #if JUCE_IOS
    return false; // xxx is this possible?
  #else
    const ScopedAutoReleasePool pool;
    BOOL removable = false;

    [[NSWorkspace sharedWorkspace]
           getFileSystemInfoForPath: juceStringToNS (getFullPathName())
                        isRemovable: &removable
                         isWritable: nil
                      isUnmountable: nil
                        description: nil
                               type: nil];

    return removable;
  #endif
}

bool File::isHidden() const
{
    return FileHelpers::isHiddenFile (getFullPathName());
}

//==============================================================================
const char* juce_Argv0 = 0;  // referenced from juce_Application.cpp

const File File::getSpecialLocation (const SpecialLocationType type)
{
    const ScopedAutoReleasePool pool;

    String resultPath;

    switch (type)
    {
        case userHomeDirectory:                 resultPath = nsStringToJuce (NSHomeDirectory()); break;

      #if JUCE_IOS
        case userDocumentsDirectory:            resultPath = getIOSSystemLocation (NSDocumentDirectory); break;
        case userDesktopDirectory:              resultPath = getIOSSystemLocation (NSDesktopDirectory); break;

        case tempDirectory:
        {
            File tmp (getIOSSystemLocation (NSCachesDirectory));
            tmp = tmp.getChildFile (juce_getExecutableFile().getFileNameWithoutExtension());
            tmp.createDirectory();
            return tmp.getFullPathName();
        }

      #else
        case userDocumentsDirectory:            resultPath = "~/Documents"; break;
        case userDesktopDirectory:              resultPath = "~/Desktop"; break;

        case tempDirectory:
        {
            File tmp ("~/Library/Caches/" + juce_getExecutableFile().getFileNameWithoutExtension());
            tmp.createDirectory();
            return tmp.getFullPathName();
        }
      #endif
        case userMusicDirectory:                resultPath = "~/Music"; break;
        case userMoviesDirectory:               resultPath = "~/Movies"; break;
        case userApplicationDataDirectory:      resultPath = "~/Library"; break;
        case commonApplicationDataDirectory:    resultPath = "/Library"; break;
        case globalApplicationsDirectory:       resultPath = "/Applications"; break;

        case invokedExecutableFile:
            if (juce_Argv0 != 0)
                return File (String::fromUTF8 (juce_Argv0));
            // deliberate fall-through...

        case currentExecutableFile:
            return juce_getExecutableFile();

        case currentApplicationFile:
        {
            const File exe (juce_getExecutableFile());
            const File parent (exe.getParentDirectory());

          #if JUCE_IOS
            return parent;
          #else
            return parent.getFullPathName().endsWithIgnoreCase ("Contents/MacOS")
                    ? parent.getParentDirectory().getParentDirectory()
                    : exe;
          #endif
        }

        case hostApplicationPath:
        {
            unsigned int size = 8192;
            HeapBlock<char> buffer;
            buffer.calloc (size + 8);

            _NSGetExecutablePath (buffer.getData(), &size);
            return String::fromUTF8 (buffer, size);
        }

        default:
            jassertfalse; // unknown type?
            break;
    }

    if (resultPath.isNotEmpty())
        return File (PlatformUtilities::convertToPrecomposedUnicode (resultPath));

    return File::nonexistent;
}

//==============================================================================
const String File::getVersion() const
{
    const ScopedAutoReleasePool pool;
    String result;

    NSBundle* bundle = [NSBundle bundleWithPath: juceStringToNS (getFullPathName())];

    if (bundle != 0)
    {
        NSDictionary* info = [bundle infoDictionary];

        if (info != 0)
        {
            NSString* name = [info valueForKey: @"CFBundleShortVersionString"];

            if (name != nil)
                result = nsStringToJuce (name);
        }
    }

    return result;
}

//==============================================================================
const File File::getLinkedTarget() const
{
  #if JUCE_IOS || (defined (MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MIN_ALLOWED >= MAC_OS_X_VERSION_10_5)
    NSString* dest = [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath: juceStringToNS (getFullPathName()) error: nil];

  #else
    // (the cast here avoids a deprecation warning)
    NSString* dest = [((id) [NSFileManager defaultManager]) pathContentOfSymbolicLinkAtPath: juceStringToNS (getFullPathName())];
  #endif

    if (dest != nil)
        return File (nsStringToJuce (dest));

    return *this;
}

//==============================================================================
bool File::moveToTrash() const
{
    if (! exists())
        return true;

  #if JUCE_IOS
    return deleteFile(); //xxx is there a trashcan on the iPhone?
  #else
    const ScopedAutoReleasePool pool;

    NSString* p = juceStringToNS (getFullPathName());

    return [[NSWorkspace sharedWorkspace]
                performFileOperation: NSWorkspaceRecycleOperation
                              source: [p stringByDeletingLastPathComponent]
                         destination: @""
                               files: [NSArray arrayWithObject: [p lastPathComponent]]
                                 tag: nil ];
  #endif
}

//==============================================================================
class DirectoryIterator::NativeIterator::Pimpl
{
public:
    Pimpl (const File& directory, const String& wildCard_)
        : parentDir (File::addTrailingSeparator (directory.getFullPathName())),
          wildCard (wildCard_),
          enumerator (0)
    {
        const ScopedAutoReleasePool pool;

        enumerator = [[[NSFileManager defaultManager] enumeratorAtPath: juceStringToNS (directory.getFullPathName())] retain];

        wildcardUTF8 = wildCard.toUTF8();
    }

    ~Pimpl()
    {
        [enumerator release];
    }

    bool next (String& filenameFound,
               bool* const isDir, bool* const isHidden, int64* const fileSize,
               Time* const modTime, Time* const creationTime, bool* const isReadOnly)
    {
        const ScopedAutoReleasePool pool;

        for (;;)
        {
            NSString* file;
            if (enumerator == 0 || (file = [enumerator nextObject]) == 0)
                return false;

            [enumerator skipDescendents];
            filenameFound = nsStringToJuce (file);

            if (fnmatch (wildcardUTF8, filenameFound.toUTF8(), FNM_CASEFOLD) != 0)
                continue;

            const String path (parentDir + filenameFound);

            if (isDir != 0 || fileSize != 0 || modTime != 0 || creationTime != 0)
            {
                juce_statStruct info;
                const bool statOk = juce_stat (path, info);

                if (isDir != 0)         *isDir = statOk && ((info.st_mode & S_IFDIR) != 0);
                if (fileSize != 0)      *fileSize = statOk ? info.st_size : 0;
                if (modTime != 0)       *modTime = statOk ? (int64) info.st_mtime * 1000 : 0;
                if (creationTime != 0)  *creationTime = statOk ? (int64) info.st_ctime * 1000 : 0;
            }

            if (isHidden != 0)
                *isHidden = FileHelpers::isHiddenFile (path);

            if (isReadOnly != 0)
                *isReadOnly = access (path.toUTF8(), W_OK) != 0;

            return true;
        }
    }

private:
    String parentDir, wildCard;
    const char* wildcardUTF8;
    NSDirectoryEnumerator* enumerator;

    JUCE_DECLARE_NON_COPYABLE (Pimpl);
};

DirectoryIterator::NativeIterator::NativeIterator (const File& directory, const String& wildCard)
    : pimpl (new DirectoryIterator::NativeIterator::Pimpl (directory, wildCard))
{
}

DirectoryIterator::NativeIterator::~NativeIterator()
{
}

bool DirectoryIterator::NativeIterator::next (String& filenameFound,
                                              bool* const isDir, bool* const isHidden, int64* const fileSize,
                                              Time* const modTime, Time* const creationTime, bool* const isReadOnly)
{
    return pimpl->next (filenameFound, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
}


//==============================================================================
bool PlatformUtilities::openDocument (const String& fileName, const String& parameters)
{
  #if JUCE_IOS
    return [[UIApplication sharedApplication] openURL: [NSURL fileURLWithPath: juceStringToNS (fileName)]];
  #else
    const ScopedAutoReleasePool pool;

    if (parameters.isEmpty())
    {
        return [[NSWorkspace sharedWorkspace] openFile: juceStringToNS (fileName)]
            || [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: juceStringToNS (fileName)]];
    }

    bool ok = false;

    if (PlatformUtilities::isBundle (fileName))
    {
        NSMutableArray* urls = [NSMutableArray array];

        StringArray docs;
        docs.addTokens (parameters, true);
        for (int i = 0; i < docs.size(); ++i)
            [urls addObject: juceStringToNS (docs[i])];

        ok = [[NSWorkspace sharedWorkspace] openURLs: urls
                             withAppBundleIdentifier: [[NSBundle bundleWithPath: juceStringToNS (fileName)] bundleIdentifier]
                                             options: 0
                      additionalEventParamDescriptor: nil
                                   launchIdentifiers: nil];
    }
    else if (File (fileName).exists())
    {
        ok = FileHelpers::launchExecutable ("\"" + fileName + "\" " + parameters);
    }

    return ok;
  #endif
}

void File::revealToUser() const
{
  #if ! JUCE_IOS
    if (exists())
        [[NSWorkspace sharedWorkspace] selectFile: juceStringToNS (getFullPathName()) inFileViewerRootedAtPath: @""];
    else if (getParentDirectory().exists())
        getParentDirectory().revealToUser();
  #endif
}

//==============================================================================
#if ! JUCE_IOS
bool PlatformUtilities::makeFSRefFromPath (FSRef* destFSRef, const String& path)
{
    return FSPathMakeRef ((const UInt8*) path.toUTF8(), destFSRef, 0) == noErr;
}

const String PlatformUtilities::makePathFromFSRef (FSRef* file)
{
    char path [2048];
    zerostruct (path);

    if (FSRefMakePath (file, (UInt8*) path, sizeof (path) - 1) == noErr)
        return PlatformUtilities::convertToPrecomposedUnicode (String::fromUTF8 (path));

    return String::empty;
}
#endif

//==============================================================================
OSType PlatformUtilities::getTypeOfFile (const String& filename)
{
    const ScopedAutoReleasePool pool;

  #if JUCE_IOS || (defined (MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MIN_ALLOWED >= MAC_OS_X_VERSION_10_5)
    NSDictionary* fileDict = [[NSFileManager defaultManager] attributesOfItemAtPath: juceStringToNS (filename) error: nil];
  #else
    // (the cast here avoids a deprecation warning)
    NSDictionary* fileDict = [((id) [NSFileManager defaultManager]) fileAttributesAtPath: juceStringToNS (filename) traverseLink: NO];
  #endif

    return [fileDict fileHFSTypeCode];
}

bool PlatformUtilities::isBundle (const String& filename)
{
  #if JUCE_IOS
    return false; // xxx can't find a sensible way to do this without trying to open the bundle..
  #else
    const ScopedAutoReleasePool pool;
    return [[NSWorkspace sharedWorkspace] isFilePackageAtPath: juceStringToNS (filename)];
  #endif
}

#endif
