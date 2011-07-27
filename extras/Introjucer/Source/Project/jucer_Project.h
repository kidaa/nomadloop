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

#ifndef __JUCER_PROJECT_JUCEHEADER__
#define __JUCER_PROJECT_JUCEHEADER__

#include "../jucer_Headers.h"
class ProjectExporter;
class ProjectType;

//==============================================================================
class Project  : public FileBasedDocument,
                 public ValueTree::Listener
{
public:
    //==============================================================================
    Project (const File& file);
    ~Project();

    //==============================================================================
    // FileBasedDocument stuff..
    const String getDocumentTitle();
    const String loadDocument (const File& file);
    const String saveDocument (const File& file);
    const File getLastDocumentOpened();
    void setLastDocumentOpened (const File& file);

    void setTitle (const String& newTitle);

    //==============================================================================
    ValueTree getProjectRoot() const                    { return projectRoot; }
    Value getProjectName()                              { return getMainGroup().getName(); }
    String getProjectFilenameRoot()                     { return File::createLegalFileName (getDocumentTitle()); }
    String getProjectUID() const                        { return projectRoot [Ids::id_]; }

    //==============================================================================
    template <class FileType>
    bool shouldBeAddedToBinaryResourcesByDefault (const FileType& file)
    {
        return ! file.hasFileExtension (sourceOrHeaderFileExtensions);
    }

    File resolveFilename (String filename) const;
    String getRelativePathForFile (const File& file) const;

    //==============================================================================
    // Creates editors for the project settings
    void createPropertyEditors (Array <PropertyComponent*>& properties);

    //==============================================================================
    // project types
    const ProjectType& getProjectType() const;
    Value getProjectTypeValue() const                   { return getProjectValue ("projectType"); }

    Value getVersion() const                            { return getProjectValue ("version"); }
    String getVersionAsHex() const;
    Value getBundleIdentifier() const                   { return getProjectValue ("bundleIdentifier"); }
    void setBundleIdentifierToDefault()                 { getBundleIdentifier() = "com.yourcompany." + CodeHelpers::makeValidIdentifier (getProjectName().toString(), false, true, false); }

    //==============================================================================
    // linkage modes..
    static const char* const notLinkedToJuce;
    static const char* const useLinkedJuce;
    static const char* const useAmalgamatedJuce;
    static const char* const useAmalgamatedJuceViaSingleTemplate;
    static const char* const useAmalgamatedJuceViaMultipleTemplates;

    Value getJuceLinkageModeValue() const               { return getProjectValue ("juceLinkage"); }
    String getJuceLinkageMode() const                   { return getJuceLinkageModeValue().toString(); }

    bool isUsingWrapperFiles() const                    { return isUsingFullyAmalgamatedFile() || isUsingSingleTemplateFile() || isUsingMultipleTemplateFiles(); }
    bool isUsingFullyAmalgamatedFile() const            { return getJuceLinkageMode() == useAmalgamatedJuce; }
    bool isUsingSingleTemplateFile() const              { return getJuceLinkageMode() == useAmalgamatedJuceViaSingleTemplate; }
    bool isUsingMultipleTemplateFiles() const           { return getJuceLinkageMode() == useAmalgamatedJuceViaMultipleTemplates; }

    //==============================================================================
    Value getProjectValue (const Identifier& name) const  { return projectRoot.getPropertyAsValue (name, getUndoManagerFor (projectRoot)); }

    Value getProjectPreprocessorDefs() const            { return getProjectValue (Ids::defines); }
    StringPairArray getPreprocessorDefs() const;

    Value getBigIconImageItemID() const                 { return getProjectValue ("bigIcon"); }
    Value getSmallIconImageItemID() const               { return getProjectValue ("smallIcon"); }
    Image getBigIcon();
    Image getSmallIcon();

    //==============================================================================
    File getAppIncludeFile() const                      { return getGeneratedCodeFolder().getChildFile (getJuceSourceHFilename()); }
    File getGeneratedCodeFolder() const                 { return getFile().getSiblingFile ("JuceLibraryCode"); }
    File getPluginCharacteristicsFile() const           { return getGeneratedCodeFolder().getChildFile (getPluginCharacteristicsFilename()); }
    File getLocalJuceFolder();

    //==============================================================================
    String getAmalgamatedHeaderFileName() const         { return "juce_amalgamated.h"; }
    String getAmalgamatedMMFileName() const             { return "juce_amalgamated.mm"; }
    String getAmalgamatedCppFileName() const            { return "juce_amalgamated.cpp"; }

    String getAppConfigFilename() const                 { return "AppConfig.h"; }
    String getJuceSourceFilenameRoot() const            { return "JuceLibraryCode"; }
    int getNumSeparateAmalgamatedFiles() const          { return 4; }
    String getJuceSourceHFilename() const               { return "JuceHeader.h"; }
    String getJuceCodeGroupName() const                 { return "Juce Library Code"; }
    String getPluginCharacteristicsFilename() const     { return "JucePluginCharacteristics.h"; }

    //==============================================================================
    class Item
    {
    public:
        //==============================================================================
        Item (Project& project, const ValueTree& itemNode);
        Item (const Item& other);
        Item& operator= (const Item& other);
        ~Item();

        static Item createGroup (Project& project, const String& name);
        void initialiseNodeValues();

        //==============================================================================
        bool isValid() const                            { return node.isValid(); }
        const ValueTree& getNode() const noexcept       { return node; }
        ValueTree& getNode() noexcept                   { return node; }
        Project& getProject() const noexcept            { return *project; }
        bool operator== (const Item& other) const       { return node == other.node && project == other.project; }
        bool operator!= (const Item& other) const       { return ! operator== (other); }

        //==============================================================================
        bool isFile() const;
        bool isGroup() const;
        bool isMainGroup() const;
        bool isImageFile() const;

        String getID() const;
        Item findItemWithID (const String& targetId) const; // (recursive search)
        String getImageFileID() const;
        void setID (const String& newID);

        //==============================================================================
        Value getName() const;
        File getFile() const;
        String getFilePath() const;
        void setFile (const File& file);
        void setFile (const RelativePath& file);
        File determineGroupFolder() const;
        bool renameFile (const File& newFile);

        bool shouldBeAddedToTargetProject() const;
        bool shouldBeCompiled() const;
        Value getShouldCompileValue() const;
        bool shouldBeAddedToBinaryResources() const;
        Value getShouldAddToResourceValue() const;
        Value getShouldInhibitWarningsValue() const;
        Value getShouldUseStdCallValue() const;

        //==============================================================================
        bool canContain (const Item& child) const;
        int getNumChildren() const                      { return node.getNumChildren(); }
        Item getChild (int index) const                 { return Item (getProject(), node.getChild (index)); }

        Item addNewSubGroup (const String& name, int insertIndex);
        void addChild (const Item& newChild, int insertIndex);
        bool addFile (const File& file, int insertIndex);
        bool addRelativeFile (const RelativePath& file, int insertIndex, bool shouldCompile);
        void removeItemFromProject();
        void sortAlphabetically();
        Item findItemForFile (const File& file) const;

        Item getParent() const;
        Item createCopy();

        const Drawable* getIcon() const;

    private:
        //==============================================================================
        Project* project;
        ValueTree node;

        UndoManager* getUndoManager() const              { return getProject().getUndoManagerFor (node); }
    };

    Item getMainGroup();

    void findAllImageItems (OwnedArray<Item>& items);

    //==============================================================================
    class BuildConfiguration
    {
    public:
        BuildConfiguration (const BuildConfiguration&);
        const BuildConfiguration& operator= (const BuildConfiguration&);
        ~BuildConfiguration();

        //==============================================================================
        Project& getProject() const                         { return *project; }

        void createPropertyEditors (Array <PropertyComponent*>& properties);

        //==============================================================================
        Value getName() const                               { return getValue (Ids::name); }
        Value isDebug() const                               { return getValue (Ids::isDebug); }
        Value getTargetBinaryName() const                   { return getValue (Ids::targetName); }
        // the path relative to the build folder in which the binary should go
        Value getTargetBinaryRelativePath() const           { return getValue (Ids::binaryPath); }
        Value getOptimisationLevel() const                  { return getValue (Ids::optimisation); }
        String getGCCOptimisationFlag() const;
        Value getBuildConfigPreprocessorDefs() const        { return getValue (Ids::defines); }
        StringPairArray getAllPreprocessorDefs() const; // includes inherited definitions
        Value getHeaderSearchPath() const                   { return getValue (Ids::headerPath); }
        StringArray getHeaderSearchPaths() const;

        static const char* const osxVersionDefault;
        static const char* const osxVersion10_4;
        static const char* const osxVersion10_5;
        static const char* const osxVersion10_6;
        Value getMacSDKVersion() const                      { return getValue (Ids::osxSDK); }
        Value getMacCompatibilityVersion() const            { return getValue (Ids::osxCompatibility); }

        static const char* const osxArch_Default;
        static const char* const osxArch_Native;
        static const char* const osxArch_32BitUniversal;
        static const char* const osxArch_64BitUniversal;
        static const char* const osxArch_64Bit;
        Value getMacArchitecture() const                    { return getValue (Ids::osxArchitecture); }

        //==============================================================================
    private:
        friend class Project;
        Project* project;
        ValueTree config;

        Value getValue (const Identifier& name) const       { return config.getPropertyAsValue (name, getUndoManager()); }
        UndoManager* getUndoManager() const                 { return project->getUndoManagerFor (config); }

        BuildConfiguration (Project* project, const ValueTree& configNode);
    };

    int getNumConfigurations() const;
    BuildConfiguration getConfiguration (int index);
    void addNewConfiguration (BuildConfiguration* configToCopy);
    void deleteConfiguration (int index);
    bool hasConfigurationNamed (const String& name) const;
    String getUniqueConfigName (String name) const;

    //==============================================================================
    ValueTree getExporters();
    int getNumExporters();
    ProjectExporter* createExporter (int index);
    void addNewExporter (int exporterIndex);
    void deleteExporter (int index);
    void createDefaultExporters();

    //==============================================================================
    struct ConfigFlag
    {
        String symbol, description;
        Value value;   // 1 = true, 2 = false, anything else = use default
    };

    void getAllConfigFlags (OwnedArray <ConfigFlag>& flags);

    static const char* const configFlagDefault;
    static const char* const configFlagEnabled;
    static const char* const configFlagDisabled;
    Value getConfigFlag (const String& name);
    bool isConfigFlagEnabled (const String& name) const;

    //==============================================================================
    String getFileTemplate (const String& templateName);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property);
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded);
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved);
    void valueTreeChildOrderChanged (ValueTree& parentTree);
    void valueTreeParentChanged (ValueTree& tree);

    //==============================================================================
    UndoManager* getUndoManagerFor (const ValueTree& node) const             { return 0; }

    //==============================================================================
    static const char* projectFileExtension;

    static void resaveJucerFile (const File& file);

private:
    friend class Item;
    ValueTree projectRoot;
    static File lastDocumentOpened;
    DrawableImage mainProjectIcon;

    void updateProjectSettings();
    void setMissingDefaultValues();
    ValueTree getConfigurations() const;
    void createDefaultConfigs();
    ValueTree getConfigNode();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project);
};


#endif   // __JUCER_PROJECT_JUCEHEADER__
