/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_OPENGL


//==============================================================================
class WindowedGLContext   : public OpenGLContext
{
public:
    WindowedGLContext (Component* const component,
                       const OpenGLPixelFormat& pixelFormat_,
                       GLXContext sharedContext)
        : renderContext (0),
          embeddedWindow (0),
          pixelFormat (pixelFormat_),
          swapInterval (0)
    {
        jassert (component != nullptr);
        LinuxComponentPeer* const peer = dynamic_cast <LinuxComponentPeer*> (component->getTopLevelComponent()->getPeer());
        if (peer == nullptr)
            return;

        ScopedXLock xlock;
        XSync (display, False);

        GLint attribs[] =
        {
            GLX_RGBA, GLX_DOUBLEBUFFER,
            GLX_RED_SIZE,   pixelFormat.redBits,
            GLX_GREEN_SIZE, pixelFormat.greenBits,
            GLX_BLUE_SIZE,  pixelFormat.blueBits,
            GLX_ALPHA_SIZE, pixelFormat.alphaBits,
            GLX_DEPTH_SIZE,   pixelFormat.depthBufferBits,
            GLX_STENCIL_SIZE, pixelFormat.stencilBufferBits,
            GLX_ACCUM_RED_SIZE,   pixelFormat.accumulationBufferRedBits,
            GLX_ACCUM_GREEN_SIZE, pixelFormat.accumulationBufferGreenBits,
            GLX_ACCUM_BLUE_SIZE,  pixelFormat.accumulationBufferBlueBits,
            GLX_ACCUM_ALPHA_SIZE, pixelFormat.accumulationBufferAlphaBits,
            None
        };

        XVisualInfo* const bestVisual = glXChooseVisual (display, DefaultScreen (display), attribs);

        if (bestVisual == 0)
            return;

        renderContext = glXCreateContext (display, bestVisual, sharedContext, GL_TRUE);

        Window windowH = (Window) peer->getNativeHandle();

        Colormap colourMap = XCreateColormap (display, windowH, bestVisual->visual, AllocNone);
        XSetWindowAttributes swa;
        swa.colormap = colourMap;
        swa.border_pixel = 0;
        swa.event_mask = ExposureMask | StructureNotifyMask;

        embeddedWindow = XCreateWindow (display, windowH,
                                        0, 0, 1, 1, 0,
                                        bestVisual->depth,
                                        InputOutput,
                                        bestVisual->visual,
                                        CWBorderPixel | CWColormap | CWEventMask,
                                        &swa);

        XSaveContext (display, (XID) embeddedWindow, windowHandleXContext, (XPointer) peer);

        XMapWindow (display, embeddedWindow);
        XFreeColormap (display, colourMap);

        XFree (bestVisual);
        XSync (display, False);
    }

    ~WindowedGLContext()
    {
        ScopedXLock xlock;
        deleteContext();

        XUnmapWindow (display, embeddedWindow);
        XDestroyWindow (display, embeddedWindow);
    }

    void deleteContext()
    {
        makeInactive();

        if (renderContext != 0)
        {
            ScopedXLock xlock;
            glXDestroyContext (display, renderContext);
            renderContext = nullptr;
        }
    }

    bool makeActive() const noexcept
    {
        jassert (renderContext != 0);

        ScopedXLock xlock;
        return glXMakeCurrent (display, embeddedWindow, renderContext)
                && XSync (display, False);
    }

    bool makeInactive() const noexcept
    {
        ScopedXLock xlock;
        return (! isActive()) || glXMakeCurrent (display, None, 0);
    }

    bool isActive() const noexcept
    {
        ScopedXLock xlock;
        return glXGetCurrentContext() == renderContext;
    }

    const OpenGLPixelFormat getPixelFormat() const
    {
        return pixelFormat;
    }

    void* getRawContext() const noexcept
    {
        return renderContext;
    }

    void updateWindowPosition (const Rectangle<int>& bounds)
    {
        ScopedXLock xlock;
        XMoveResizeWindow (display, embeddedWindow,
                           bounds.getX(), bounds.getY(), jmax (1, bounds.getWidth()), jmax (1, bounds.getHeight()));
    }

    void swapBuffers()
    {
        ScopedXLock xlock;
        glXSwapBuffers (display, embeddedWindow);
    }

    bool setSwapInterval (const int numFramesPerSwap)
    {
        static PFNGLXSWAPINTERVALSGIPROC GLXSwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC) glXGetProcAddress ((const GLubyte*) "glXSwapIntervalSGI");

        if (GLXSwapIntervalSGI != 0)
        {
            swapInterval = numFramesPerSwap;
            GLXSwapIntervalSGI (numFramesPerSwap);
            return true;
        }

        return false;
    }

    int getSwapInterval() const    { return swapInterval; }
    void repaint() {}

    //==============================================================================
    GLXContext renderContext;

private:
    Window embeddedWindow;
    OpenGLPixelFormat pixelFormat;
    int swapInterval;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowedGLContext);
};

//==============================================================================
OpenGLContext* OpenGLComponent::createContext()
{
    ScopedPointer<WindowedGLContext> c (new WindowedGLContext (this, preferredPixelFormat,
                                                               contextToShareListsWith != 0 ? (GLXContext) contextToShareListsWith->getRawContext() : 0));

    return (c->renderContext != 0) ? c.release() : nullptr;
}

void juce_glViewport (const int w, const int h)
{
    glViewport (0, 0, w, h);
}

void OpenGLPixelFormat::getAvailablePixelFormats (Component* component, OwnedArray <OpenGLPixelFormat>& results)
{
    results.add (new OpenGLPixelFormat()); // xxx
}

#endif
