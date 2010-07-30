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

#ifndef __JUCE_PLATFORMDEFS_JUCEHEADER__
#define __JUCE_PLATFORMDEFS_JUCEHEADER__

//==============================================================================
/*  This file defines miscellaneous macros for debugging, assertions, etc.
*/

//==============================================================================
#ifdef JUCE_FORCE_DEBUG
  #undef JUCE_DEBUG

  #if JUCE_FORCE_DEBUG
    #define JUCE_DEBUG 1
  #endif
#endif

/** This macro defines the C calling convention used as the standard for Juce calls. */
#if JUCE_MSVC
  #define JUCE_CALLTYPE             __stdcall
#else
  #define JUCE_CALLTYPE
#endif

//==============================================================================
// Debugging and assertion macros

// (For info about JUCE_LOG_ASSERTIONS, have a look in juce_Config.h)
#if JUCE_LOG_ASSERTIONS
  #define juce_LogCurrentAssertion    juce_LogAssertion (__FILE__, __LINE__);
#elif JUCE_DEBUG
  #define juce_LogCurrentAssertion    std::cerr << "JUCE Assertion failure in " << __FILE__ << ", line " << __LINE__ << std::endl;
#else
  #define juce_LogCurrentAssertion
#endif

#if JUCE_DEBUG
  //==============================================================================
  // If debugging is enabled..

  /** Writes a string to the standard error stream.

    This is only compiled in a debug build.

    @see Logger::outputDebugString
  */
  #define DBG(dbgtext)                  JUCE_NAMESPACE::Logger::outputDebugString (dbgtext);

  //==============================================================================
  // Assertions..

  #if JUCE_WINDOWS || DOXYGEN

    #if JUCE_USE_INTRINSICS
      #pragma intrinsic (__debugbreak)

      /** This will try to break the debugger if one is currently hosting this app.
          @see jassert()
      */
      #define juce_breakDebugger            __debugbreak();

    #elif JUCE_GCC
      /** This will try to break the debugger if one is currently hosting this app.
          @see jassert()
      */
      #define juce_breakDebugger            asm("int $3");
    #else
      /** This will try to break the debugger if one is currently hosting this app.
          @see jassert()
      */
      #define juce_breakDebugger            { __asm int 3 }
    #endif
  #elif JUCE_MAC
    #define juce_breakDebugger              Debugger();
  #elif JUCE_IOS
    #define juce_breakDebugger              kill (0, SIGTRAP);
  #elif JUCE_LINUX
    #define juce_breakDebugger              kill (0, SIGTRAP);
  #endif

  //==============================================================================
  /** This will always cause an assertion failure.

      It is only compiled in a debug build, (unless JUCE_LOG_ASSERTIONS is enabled
      in juce_Config.h).

      @see jassert()
  */
  #define jassertfalse                  { juce_LogCurrentAssertion; if (JUCE_NAMESPACE::juce_isRunningUnderDebugger()) juce_breakDebugger; }

  //==============================================================================
  /** Platform-independent assertion macro.

      This gets optimised out when not being built with debugging turned on.

      Be careful not to call any functions within its arguments that are vital to
      the behaviour of the program, because these won't get called in the release
      build.

      @see jassertfalse
  */
  #define jassert(expression)           { if (! (expression)) jassertfalse; }

#else
  //==============================================================================
  // If debugging is disabled, these dummy debug and assertion macros are used..

  #define DBG(dbgtext)

  #define jassertfalse                  { juce_LogCurrentAssertion }

  #if JUCE_LOG_ASSERTIONS
    #define jassert(expression)         { if (! (expression)) jassertfalse; }
  #else
    #define jassert(a)                  { }
  #endif

#endif

//==============================================================================
#ifndef DOXYGEN
  template <bool b> struct JuceStaticAssert;
  template <> struct JuceStaticAssert <true> { static void dummy() {} };
#endif

/** A compile-time assertion macro.

    If the expression parameter is false, the macro will cause a compile error.
*/
#define static_jassert(expression)      JuceStaticAssert<expression>::dummy();


//==============================================================================
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS

  #define JUCE_TRY try

  #define JUCE_CATCH_ALL            catch (...) {}
  #define JUCE_CATCH_ALL_ASSERT     catch (...) { jassertfalse; }

  #if JUCE_ONLY_BUILD_CORE_LIBRARY
    #define JUCE_CATCH_EXCEPTION    JUCE_CATCH_ALL
  #else
    /** Used in try-catch blocks, this macro will send exceptions to the JUCEApplication
        object so they can be logged by the application if it wants to.
    */
    #define JUCE_CATCH_EXCEPTION \
      catch (const std::exception& e)  \
      { \
          JUCEApplication::sendUnhandledException (&e, __FILE__, __LINE__); \
      } \
      catch (...) \
      { \
          JUCEApplication::sendUnhandledException (0, __FILE__, __LINE__); \
      }
  #endif

#else

  #define JUCE_TRY
  #define JUCE_CATCH_EXCEPTION
  #define JUCE_CATCH_ALL
  #define JUCE_CATCH_ALL_ASSERT

#endif

//==============================================================================
// Macros for inlining.

#if JUCE_MSVC
  /** A platform-independent way of forcing an inline function.

      Use the syntax: @code
      forcedinline void myfunction (int x)
      @endcode
  */
  #ifndef JUCE_DEBUG
    #define forcedinline  __forceinline
  #else
    #define forcedinline  inline
  #endif

  #define JUCE_ALIGN(bytes) __declspec (align (bytes))

#else
  /** A platform-independent way of forcing an inline function.

      Use the syntax: @code
      forcedinline void myfunction (int x)
      @endcode
  */
  #ifndef JUCE_DEBUG
    #define forcedinline  inline __attribute__((always_inline))
  #else
    #define forcedinline  inline
  #endif

  #define JUCE_ALIGN(bytes) __attribute__ ((aligned (bytes)))

#endif

#endif   // __JUCE_PLATFORMDEFS_JUCEHEADER__
