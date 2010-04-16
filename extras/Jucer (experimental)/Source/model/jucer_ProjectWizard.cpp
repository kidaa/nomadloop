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

#include "jucer_ProjectWizard.h"


//==============================================================================
class GUIAppWizard   : public ProjectWizard
{
public:
    GUIAppWizard()  {}
    ~GUIAppWizard() {}

    const String getName()          { return "GUI Application"; }
    const String getDescription()   { return "Creates a standard application"; }

    void addItemsToAlertWindow (AlertWindow& aw)
    {
        const char* fileOptions[] = { "Create a Main.cpp file",
                                      "Create a Main.cpp file and a basic window",
                                      "Don't create any files", 0 };

        aw.addComboBox ("files", StringArray (fileOptions), "Files to Auto-Generate");
    }

    const String processResultsFromAlertWindow (AlertWindow& aw)
    {
        createMainCpp = createWindow = false;

        switch (aw.getComboBoxComponent ("files")->getSelectedItemIndex())
        {
            case 0:     createMainCpp = true;  break;
            case 1:     createMainCpp = createWindow = true;  break;
            case 2:     break;
            default:    jassertfalse; break;
        }

        return String::empty;
    }

    bool initialiseProject (Project& project)
    {
        if (! getSourceFilesFolder().createDirectory())
            failedFiles.add (getSourceFilesFolder().getFullPathName());

        File mainCppFile = getSourceFilesFolder().getChildFile ("Main.cpp");
        File mainWindowCpp = getSourceFilesFolder().getChildFile ("MainWindow.cpp");
        File mainWindowH = mainWindowCpp.withFileExtension (".h");
        String windowClassName = "MainAppWindow";

        project.getProjectType() = (int) Project::application;

        Project::Item group (project.createNewGroup());
        project.getMainGroup().addChild (group, 0);
        group.getName() = "Source";

        for (int i = project.getNumConfigurations(); --i >= 0;)
            project.getConfiguration(i).getTargetBinaryName() = File::createLegalFileName (appTitle);

        String appHeaders (createIncludeStatement (project.getAppIncludeFile(), mainCppFile));
        String initCode, shutdownCode, anotherInstanceStartedCode, privateMembers, memberInitialisers;

        if (createWindow)
        {
            appHeaders << newLine << createIncludeStatement (mainWindowH, mainCppFile);
            memberInitialisers = "   : mainWindow (0)";
            initCode = "mainWindow = new " + windowClassName + "();";
            shutdownCode = "deleteAndZero (mainWindow);";
            privateMembers = windowClassName + "* mainWindow;";

            String windowH = project.getFileTemplate ("jucer_WindowTemplate_h")
                                .replace ("INCLUDES", createIncludeStatement (project.getAppIncludeFile(), mainWindowH), false)
                                .replace ("WINDOWCLASS", windowClassName, false)
                                .replace ("HEADERGUARD", makeHeaderGuardName (mainWindowH), false);

            String windowCpp = project.getFileTemplate ("jucer_WindowTemplate_cpp")
                                .replace ("INCLUDES", createIncludeStatement (mainWindowH, mainWindowCpp), false)
                                .replace ("WINDOWCLASS", windowClassName, false);

            if (! overwriteFileWithNewDataIfDifferent (mainWindowH, windowH))
                failedFiles.add (mainWindowH.getFullPathName());

            if (! overwriteFileWithNewDataIfDifferent (mainWindowCpp, windowCpp))
                failedFiles.add (mainWindowCpp.getFullPathName());

            group.addFile (mainWindowCpp, -1);
            group.addFile (mainWindowH, -1);
        }

        if (createMainCpp)
        {
            String mainCpp = project.getFileTemplate ("jucer_MainTemplate_cpp")
                                .replace ("APPHEADERS", appHeaders, false)
                                .replace ("APPCLASSNAME", makeValidCppIdentifier (appTitle + "Application", false, true, false), false)
                                .replace ("MEMBERINITIALISERS", memberInitialisers, false)
                                .replace ("APPINITCODE", initCode, false)
                                .replace ("APPSHUTDOWNCODE", shutdownCode, false)
                                .replace ("APPNAME", replaceCEscapeChars (appTitle), false)
                                .replace ("APPVERSION", "1.0", false)
                                .replace ("ALLOWMORETHANONEINSTANCE", "true", false)
                                .replace ("ANOTHERINSTANCECODE", anotherInstanceStartedCode, false)
                                .replace ("PRIVATEMEMBERS", privateMembers, false);

            if (! overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
                failedFiles.add (mainCppFile.getFullPathName());

            group.addFile (mainCppFile, -1);
        }

        return true;
    }

private:
    bool createMainCpp, createWindow;
};

//==============================================================================
class ConsoleAppWizard   : public ProjectWizard
{
public:
    ConsoleAppWizard()  {}
    ~ConsoleAppWizard() {}

    const String getName()          { return "Console Application"; }
    const String getDescription()   { return "Creates a command-line application with no GUI features"; }

    void addItemsToAlertWindow (AlertWindow& aw)
    {
        const char* fileOptions[] = { "Create a Main.cpp file",
                                      "Don't create any files", 0 };

        aw.addComboBox ("files", StringArray (fileOptions), "Files to Auto-Generate");
    }

    const String processResultsFromAlertWindow (AlertWindow& aw)
    {
        createMainCpp = false;

        switch (aw.getComboBoxComponent ("files")->getSelectedItemIndex())
        {
            case 0:     createMainCpp = true;  break;
            case 1:     break;
            default:    jassertfalse; break;
        }

        return String::empty;
    }

    bool initialiseProject (Project& project)
    {
        if (! getSourceFilesFolder().createDirectory())
            failedFiles.add (getSourceFilesFolder().getFullPathName());

        File mainCppFile = getSourceFilesFolder().getChildFile ("Main.cpp");

        project.getProjectType() = (int) Project::commandLineApp;

        Project::Item group (project.createNewGroup());
        project.getMainGroup().addChild (group, 0);
        group.getName() = "Source";

        for (int i = project.getNumConfigurations(); --i >= 0;)
            project.getConfiguration(i).getTargetBinaryName() = File::createLegalFileName (appTitle);

        if (createMainCpp)
        {
            String appHeaders (createIncludeStatement (project.getAppIncludeFile(), mainCppFile));

            String mainCpp = project.getFileTemplate ("jucer_MainConsoleAppTemplate_cpp")
                                .replace ("APPHEADERS", appHeaders, false);

            if (! overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
                failedFiles.add (mainCppFile.getFullPathName());

            group.addFile (mainCppFile, -1);
        }

        return true;
    }

private:
    bool createMainCpp;
};

//==============================================================================
class AudioPluginAppWizard   : public ProjectWizard
{
public:
    AudioPluginAppWizard()  {}
    ~AudioPluginAppWizard() {}

    const String getName()          { return "Audio Plug-In"; }
    const String getDescription()   { return "Creates an audio plugin project"; }

    void addItemsToAlertWindow (AlertWindow& aw)
    {
    }

    const String processResultsFromAlertWindow (AlertWindow& aw)
    {
        return String::empty;
    }

    bool initialiseProject (Project& project)
    {
        if (! getSourceFilesFolder().createDirectory())
            failedFiles.add (getSourceFilesFolder().getFullPathName());

        String filterClassName = makeValidCppIdentifier (appTitle, true, true, false) + "AudioProcessor";
        filterClassName = filterClassName.substring (0, 1).toUpperCase() + filterClassName.substring (1);
        String editorClassName = filterClassName + "Editor";

        File filterCppFile = getSourceFilesFolder().getChildFile ("PluginProcessor.cpp");
        File filterHFile   = filterCppFile.withFileExtension (".h");
        File editorCppFile = getSourceFilesFolder().getChildFile ("PluginEditor.cpp");
        File editorHFile   = editorCppFile.withFileExtension (".h");

        project.getProjectType() = (int) Project::audioPlugin;

        Project::Item group (project.createNewGroup());
        project.getMainGroup().addChild (group, 0);
        group.getName() = "Source";
        project.getJuceConfigFlag ("JUCE_QUICKTIME") = 2; // disabled because it interferes with RTAS build on PC

        for (int i = project.getNumConfigurations(); --i >= 0;)
            project.getConfiguration(i).getTargetBinaryName() = File::createLegalFileName (appTitle);

        String appHeaders (createIncludeStatement (project.getAppIncludeFile(), filterCppFile));
        appHeaders << newLine << createIncludeStatement (project.getPluginCharacteristicsFile(), filterCppFile);

        String filterCpp = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_cpp")
                            .replace ("FILTERHEADERS", createIncludeStatement (filterHFile, filterCppFile)
                                                            + newLine + createIncludeStatement (editorHFile, filterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("EDITORCLASSNAME", editorClassName, false);

        String filterH = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_h")
                            .replace ("APPHEADERS", appHeaders, false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("HEADERGUARD", makeHeaderGuardName (filterHFile), false);

        String editorCpp = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_cpp")
                            .replace ("EDITORCPPHEADERS", createIncludeStatement (filterHFile, filterCppFile)
                                                               + newLine + createIncludeStatement (editorHFile, filterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("EDITORCLASSNAME", editorClassName, false);

        String editorH = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_h")
                            .replace ("EDITORHEADERS", appHeaders + newLine + createIncludeStatement (filterHFile, filterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("EDITORCLASSNAME", editorClassName, false)
                            .replace ("HEADERGUARD", makeHeaderGuardName (editorHFile), false);

        if (! overwriteFileWithNewDataIfDifferent (filterCppFile, filterCpp))
            failedFiles.add (filterCppFile.getFullPathName());

        if (! overwriteFileWithNewDataIfDifferent (filterHFile, filterH))
            failedFiles.add (filterHFile.getFullPathName());

        if (! overwriteFileWithNewDataIfDifferent (editorCppFile, editorCpp))
            failedFiles.add (editorCppFile.getFullPathName());

        if (! overwriteFileWithNewDataIfDifferent (editorHFile, editorH))
            failedFiles.add (editorHFile.getFullPathName());

        group.addFile (filterCppFile, -1);
        group.addFile (filterHFile, -1);
        group.addFile (editorCppFile, -1);
        group.addFile (editorHFile, -1);

        return true;
    }
};

//==============================================================================
/*class BrowserPluginAppWizard   : public ProjectWizard
{
public:
    BrowserPluginAppWizard()  {}
    ~BrowserPluginAppWizard() {}

    const String getName()          { return "Browser Plug-In"; }
    const String getDescription()   { return "Creates an audio plugin project"; }

    void addItemsToAlertWindow (AlertWindow& aw)
    {
    }

    const String processResultsFromAlertWindow (AlertWindow& aw)
    {
        return String::empty;
    }

    bool initialiseProject (Project& project)
    {
        return true;
    }
};*/

//==============================================================================
//==============================================================================
ProjectWizard::ProjectWizard()
{
}

ProjectWizard::~ProjectWizard()
{
}

const StringArray ProjectWizard::getWizards()
{
    StringArray s;

    for (int i = 0; i < getNumWizards(); ++i)
    {
        ScopedPointer <ProjectWizard> wiz (createWizard (i));
        s.add (wiz->getName());
    }

    return s;
}

int ProjectWizard::getNumWizards()
{
    return 3;
}

ProjectWizard* ProjectWizard::createWizard (int index)
{
    switch (index)
    {
        case 0:     return new GUIAppWizard();
        case 1:     return new ConsoleAppWizard();
        case 2:     return new AudioPluginAppWizard();
        //case 3:     return new BrowserPluginAppWizard();
        default:    jassertfalse; break;
    }

    return 0;
}

//==============================================================================
Project* ProjectWizard::runWizard (Component* ownerWindow_)
{
    ownerWindow = ownerWindow_;

    {
        static File newProjectFolder;
        FileChooser fc ("New Juce Project", newProjectFolder, "*");

        if (! fc.browseForDirectory())
            return 0;

        targetFolder = newProjectFolder = fc.getResult();

        if (! newProjectFolder.exists())
        {
            if (! newProjectFolder.createDirectory())
                failedFiles.add (newProjectFolder.getFullPathName());
        }

        if (containsAnyNonHiddenFiles (newProjectFolder))
        {
            if (! AlertWindow::showOkCancelBox (AlertWindow::InfoIcon, "New Juce Project",
                                                "The folder you chose isn't empty - are you sure you want to create the project there?\n\nAny existing files with the same names may be overwritten by the new files."))
                return 0;
        }
    }

    if (failedFiles.size() == 0)
    {
        AlertWindow aw ("New " + getName(),
                        "Please choose some basic project options...",
                        AlertWindow::NoIcon, ownerWindow);

        aw.addTextEditor ("name", "", "Project Name", false);

        addItemsToAlertWindow (aw);

        aw.addButton ("Create Project", 1, KeyPress (KeyPress::returnKey));
        aw.addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));

        for (;;)
        {
            if (aw.runModalLoop() == 0)
                return 0;

            appTitle = aw.getTextEditorContents ("name").trim();

            String error (processResultsFromAlertWindow (aw));

            if (error.isEmpty() && appTitle.isEmpty())
                error = "Please enter a sensible project title!";

            if (error.isEmpty())
                break;

            aw.setColour (AlertWindow::textColourId, Colours::red);
            aw.setMessage (error);
        }
    }

    projectFile = targetFolder.getChildFile (File::createLegalFileName (appTitle))
                              .withFileExtension (Project::projectFileExtension);

    ScopedPointer <Project> project (new Project (projectFile));

    if (failedFiles.size() == 0)
    {
        project->setFile (projectFile);
        project->setTitle (appTitle);
        project->setBundleIdentifierToDefault();

        if (! initialiseProject (*project))
            return 0;

        if (project->save (false, true) != FileBasedDocument::savedOk)
            return 0;

        project->setChangedFlag (false);
    }

    if (failedFiles.size() > 0)
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                     "Errors in Creating Project!",
                                     "The following files couldn't be written:\n\n"
                                        + failedFiles.joinIntoString ("\n", 0, 10));
        return 0;
    }

    return project.release();
}

Project* ProjectWizard::runNewProjectWizard (Component* ownerWindow)
{
    ScopedPointer <ProjectWizard> wizard;

    {
        AlertWindow aw ("New Juce Project",
                        "Select the type of project to create, and the location of your Juce folder",
                        AlertWindow::NoIcon,
                        ownerWindow);

        aw.addComboBox ("type", getWizards(), "Project Type");

        FilenameComponent juceFolderSelector ("Juce Library Location", StoredSettings::getInstance()->getLastKnownJuceFolder(),
                                              true, true, false, "*", String::empty, "(Please select the folder containing Juce!)");
        juceFolderSelector.setSize (350, 22);

        aw.addCustomComponent (&juceFolderSelector);

        aw.addButton ("Next", 1, KeyPress (KeyPress::returnKey));
        aw.addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));

        for (;;)
        {
            if (aw.runModalLoop() == 0)
                return 0;

            if (isJuceFolder (juceFolderSelector.getCurrentFile()))
            {
                wizard = createWizard (aw.getComboBoxComponent ("type")->getSelectedItemIndex());
                break;
            }

            aw.setColour (AlertWindow::textColourId, Colours::red);
            aw.setMessage ("Please select a valid Juce folder for the project to use!");
        }
    }

    return wizard != 0 ? wizard->runWizard (ownerWindow) : 0;
}
