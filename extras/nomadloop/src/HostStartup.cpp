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

#include "includes.h"
#include "host/MainHostWindow.h"
#include "host/InternalFilters.h"

#if ! JUCE_PLUGINHOST_VST
// #error "If you're building the audio plugin host, you probably want to enable VST support in juce_Config.h"
#endif


ApplicationCommandManager* commandManager = 0;
ApplicationProperties* appProperties = 0;


//==============================================================================
class PluginHostApp  : public JUCEApplication
{
    MainHostWindow* mainWindow;

public:
    //==============================================================================
    PluginHostApp()
        : mainWindow (0)
    {
    }

    ~PluginHostApp()
    {
    }

    void initialise (const String& /*commandLine*/)
    {
        // initialise our settings file..
	PropertiesFile::Options options;
	options.applicationName = "NomadLoop";
	options.filenameSuffix = "settings";
	options.osxLibrarySubFolder = "Preferences";

	appProperties = new ApplicationProperties();
        appProperties->setStorageParameters (options);

        commandManager = new ApplicationCommandManager();

        AudioPluginFormatManager::getInstance()->addDefaultFormats();
        AudioPluginFormatManager::getInstance()->addFormat (new InternalPluginFormat());

        mainWindow = new MainHostWindow();		
        //mainWindow->setUsingNativeTitleBar (true);

        commandManager->registerAllCommandsForTarget (this);
        commandManager->registerAllCommandsForTarget (mainWindow);

        mainWindow->menuItemsChanged();
    }

    void shutdown()
    {
        deleteAndZero (mainWindow);

        appProperties->closeFiles();

        deleteAndZero (commandManager);
	deleteAndZero (appProperties);
    }

    const String getApplicationName()
    {
        return T("NomadLoop");
    }

    const String getApplicationVersion()
    {
        return T("0.1");
    }

    void systemRequestedQuit()
    {
        /*if (mainWindow->getContentComponent()->isValidComponent())
            mainWindow->tryToQuitApplication();
        else*/
            JUCEApplication::quit();
    }

    bool moreThanOneInstanceAllowed()
    {
        return true;
    }
};


// This kicks the whole thing off..
START_JUCE_APPLICATION (PluginHostApp)
