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

#ifndef __JUCE_INITIALISATION_JUCEHEADER__
#define __JUCE_INITIALISATION_JUCEHEADER__


//==============================================================================
/** Initialises Juce's GUI classes.

    If you're embedding Juce into an application that uses its own event-loop rather
    than using the START_JUCE_APPLICATION macro, call this function before making any
    Juce calls, to make sure things are initialised correctly.

    Note that if you're creating a Juce DLL for Windows, you may also need to call the
    PlatformUtilities::setCurrentModuleInstanceHandle() method.

    @see shutdownJuce_GUI(), initialiseJuce_NonGUI()
*/
void JUCE_PUBLIC_FUNCTION  initialiseJuce_GUI();

/** Clears up any static data being used by Juce's GUI classes.

    If you're embedding Juce into an application that uses its own event-loop rather
    than using the START_JUCE_APPLICATION macro, call this function in your shutdown
    code to clean up any juce objects that might be lying around.

    @see initialiseJuce_GUI(), initialiseJuce_NonGUI()
*/
void JUCE_PUBLIC_FUNCTION  shutdownJuce_GUI();


//==============================================================================
/** Initialises the core parts of Juce.

    If you're embedding Juce into either a command-line program, call this function
    at the start of your main() function to make sure that Juce is initialised correctly.

    Note that if you're creating a Juce DLL for Windows, you may also need to call the
    PlatformUtilities::setCurrentModuleInstanceHandle() method.

    @see shutdownJuce_NonGUI, initialiseJuce_GUI
*/
void JUCE_PUBLIC_FUNCTION  initialiseJuce_NonGUI();

/** Clears up any static data being used by Juce's non-gui core classes.

    If you're embedding Juce into either a command-line program, call this function
    at the end of your main() function if you want to make sure any Juce objects are
    cleaned up correctly.

    @see initialiseJuce_NonGUI, initialiseJuce_GUI
*/
void JUCE_PUBLIC_FUNCTION  shutdownJuce_NonGUI();


//==============================================================================
/** A utility object that helps you initialise and shutdown Juce correctly
    using an RAII pattern.

    When an instance of this class is created, it calls initialiseJuce_NonGUI(),
    and when it's deleted, it calls shutdownJuce_NonGUI(), which lets you easily
    make sure that these functions are matched correctly.

    This class is particularly handy to use at the beginning of a console app's
    main() function, because it'll take care of shutting down whenever you return
    from the main() call.

    @see ScopedJuceInitialiser_GUI
*/
class ScopedJuceInitialiser_NonGUI
{
public:
    /** The constructor simply calls initialiseJuce_NonGUI(). */
    ScopedJuceInitialiser_NonGUI()      { initialiseJuce_NonGUI(); }

    /** The destructor simply calls shutdownJuce_NonGUI(). */
    ~ScopedJuceInitialiser_NonGUI()     { shutdownJuce_NonGUI(); }
};


//==============================================================================
/** A utility object that helps you initialise and shutdown Juce correctly
    using an RAII pattern.

    When an instance of this class is created, it calls initialiseJuce_GUI(),
    and when it's deleted, it calls shutdownJuce_GUI(), which lets you easily
    make sure that these functions are matched correctly.

    This class is particularly handy to use at the beginning of a console app's
    main() function, because it'll take care of shutting down whenever you return
    from the main() call.

    @see ScopedJuceInitialiser_NonGUI
*/
class ScopedJuceInitialiser_GUI
{
public:
    /** The constructor simply calls initialiseJuce_GUI(). */
    ScopedJuceInitialiser_GUI()         { initialiseJuce_GUI(); }

    /** The destructor simply calls shutdownJuce_GUI(). */
    ~ScopedJuceInitialiser_GUI()        { shutdownJuce_GUI(); }
};


//==============================================================================
/*
    To start a JUCE app, use this macro: START_JUCE_APPLICATION (AppSubClass) where
    AppSubClass is the name of a class derived from JUCEApplication.

    See the JUCEApplication class documentation (juce_Application.h) for more details.

*/
#if defined (JUCE_GCC) || defined (__MWERKS__)

  #define START_JUCE_APPLICATION(AppClass) \
    static JUCE_NAMESPACE::JUCEApplication* juce_CreateApplication() { return new AppClass(); } \
    int main (int argc, char* argv[]) \
    { \
        JUCE_NAMESPACE::JUCEApplication::createInstance = &juce_CreateApplication; \
        return JUCE_NAMESPACE::JUCEApplication::main (argc, (const char**) argv); \
    }

#elif JUCE_WINDOWS

  #ifdef _CONSOLE
    #define START_JUCE_APPLICATION(AppClass) \
        static JUCE_NAMESPACE::JUCEApplication* juce_CreateApplication() { return new AppClass(); } \
        int main (int, char* argv[]) \
        { \
            JUCE_NAMESPACE::JUCEApplication::createInstance = &juce_CreateApplication; \
            return JUCE_NAMESPACE::JUCEApplication::main (JUCE_NAMESPACE::PlatformUtilities::getCurrentCommandLineParams()); \
        }
  #elif ! defined (_AFXDLL)
    #ifdef _WINDOWS_
      #define START_JUCE_APPLICATION(AppClass) \
          static JUCE_NAMESPACE::JUCEApplication* juce_CreateApplication() { return new AppClass(); } \
          int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int) \
          { \
              JUCE_NAMESPACE::JUCEApplication::createInstance = &juce_CreateApplication; \
              return JUCE_NAMESPACE::JUCEApplication::main (JUCE_NAMESPACE::PlatformUtilities::getCurrentCommandLineParams()); \
          }
    #else
      #define START_JUCE_APPLICATION(AppClass) \
          static JUCE_NAMESPACE::JUCEApplication* juce_CreateApplication() { return new AppClass(); } \
          int __stdcall WinMain (int, int, const char*, int) \
          { \
              JUCE_NAMESPACE::JUCEApplication::createInstance = &juce_CreateApplication; \
              return JUCE_NAMESPACE::JUCEApplication::main (JUCE_NAMESPACE::PlatformUtilities::getCurrentCommandLineParams()); \
          }
    #endif
  #endif

#endif


#endif   // __JUCE_INITIALISATION_JUCEHEADER__
