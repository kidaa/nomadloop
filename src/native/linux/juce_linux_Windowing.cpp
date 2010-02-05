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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
#define TAKE_FOCUS 0
#define DELETE_WINDOW 1

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

static const int repaintTimerPeriod = 1000 / 100;  // 100 fps maximum

//==============================================================================
static Atom wm_ChangeState = None;
static Atom wm_State = None;
static Atom wm_Protocols = None;
static Atom wm_ProtocolList [2] = { None, None };
static Atom wm_ActiveWin = None;

#define ourDndVersion 3
static Atom XA_XdndAware = None;
static Atom XA_XdndEnter = None;
static Atom XA_XdndLeave = None;
static Atom XA_XdndPosition = None;
static Atom XA_XdndStatus = None;
static Atom XA_XdndDrop = None;
static Atom XA_XdndFinished = None;
static Atom XA_XdndSelection = None;
static Atom XA_XdndProxy = None;

static Atom XA_XdndTypeList = None;
static Atom XA_XdndActionList = None;
static Atom XA_XdndActionDescription = None;
static Atom XA_XdndActionCopy = None;
static Atom XA_XdndActionMove = None;
static Atom XA_XdndActionLink = None;
static Atom XA_XdndActionAsk = None;
static Atom XA_XdndActionPrivate = None;
static Atom XA_JXSelectionWindowProperty = None;

static Atom XA_MimeTextPlain = None;
static Atom XA_MimeTextUriList = None;
static Atom XA_MimeRootDrop = None;

//==============================================================================
static XErrorHandler oldHandler = 0;
static int trappedErrorCode = 0;

extern "C" int errorTrapHandler (Display* dpy, XErrorEvent* err)
{
    trappedErrorCode = err->error_code;
    return 0;
}

static void trapErrors()
{
    trappedErrorCode = 0;
    oldHandler = XSetErrorHandler (errorTrapHandler);
}

static bool untrapErrors()
{
    XSetErrorHandler (oldHandler);
    return (trappedErrorCode == 0);
}


//==============================================================================
static bool isActiveApplication = false;

bool Process::isForegroundProcess()
{
    return isActiveApplication;
}

//==============================================================================
// These are defined in juce_linux_Messaging.cpp
extern Display* display;
extern XContext improbableNumber;

static const int eventMask = NoEventMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask
                             | EnterWindowMask | LeaveWindowMask | PointerMotionMask | KeymapStateMask
                             | ExposureMask | StructureNotifyMask | FocusChangeMask;

//==============================================================================
static int pointerMap[5];
static int lastMousePosX = 0, lastMousePosY = 0;

enum MouseButtons
{
    NoButton = 0,
    LeftButton = 1,
    MiddleButton = 2,
    RightButton = 3,
    WheelUp = 4,
    WheelDown = 5
};

static void getMousePos (int& x, int& y, int& mouseMods) throw()
{
    Window root, child;
    int winx, winy;
    unsigned int mask;

    mouseMods = 0;

    if (XQueryPointer (display,
                       RootWindow (display, DefaultScreen (display)),
                       &root, &child,
                       &x, &y, &winx, &winy, &mask) == False)
    {
        // Pointer not on the default screen
        x = y = -1;
    }
    else
    {
        if ((mask & Button1Mask) != 0)
            mouseMods |= ModifierKeys::leftButtonModifier;

        if ((mask & Button2Mask) != 0)
            mouseMods |= ModifierKeys::middleButtonModifier;

        if ((mask & Button3Mask) != 0)
            mouseMods |= ModifierKeys::rightButtonModifier;
    }
}

//==============================================================================
static int AltMask = 0;
static int NumLockMask = 0;
static bool numLock = 0;
static bool capsLock = 0;
static char keyStates [32];

static void updateKeyStates (const int keycode, const bool press) throw()
{
    const int keybyte = keycode >> 3;
    const int keybit = (1 << (keycode & 7));

    if (press)
        keyStates [keybyte] |= keybit;
    else
        keyStates [keybyte] &= ~keybit;
}

static bool keyDown (const int keycode) throw()
{
    const int keybyte = keycode >> 3;
    const int keybit = (1 << (keycode & 7));

    return (keyStates [keybyte] & keybit) != 0;
}

static const int extendedKeyModifier = 0x10000000;

bool KeyPress::isKeyCurrentlyDown (const int keyCode) throw()
{
    int keysym;

    if (keyCode & extendedKeyModifier)
    {
        keysym = 0xff00 | (keyCode & 0xff);
    }
    else
    {
        keysym = keyCode;

        if (keysym == (XK_Tab & 0xff)
            || keysym == (XK_Return & 0xff)
            || keysym == (XK_Escape & 0xff)
            || keysym == (XK_BackSpace & 0xff))
        {
            keysym |= 0xff00;
        }
    }

    return keyDown (XKeysymToKeycode (display, keysym));
}

//==============================================================================
// Alt and Num lock are not defined by standard X
// modifier constants: check what they're mapped to
static void getModifierMapping() throw()
{
    const int altLeftCode = XKeysymToKeycode (display, XK_Alt_L);
    const int numLockCode = XKeysymToKeycode (display, XK_Num_Lock);

    AltMask = 0;
    NumLockMask = 0;

    XModifierKeymap* mapping = XGetModifierMapping (display);

    if (mapping)
    {
        for (int i = 0; i < 8; i++)
        {
            if (mapping->modifiermap [i << 1] == altLeftCode)
                AltMask = 1 << i;
            else if (mapping->modifiermap [i << 1] == numLockCode)
                NumLockMask = 1 << i;
        }

        XFreeModifiermap (mapping);
    }
}

static int currentModifiers = 0;

void ModifierKeys::updateCurrentModifiers() throw()
{
    currentModifierFlags = currentModifiers;
}

const ModifierKeys ModifierKeys::getCurrentModifiersRealtime() throw()
{
    int x, y, mouseMods;
    getMousePos (x, y, mouseMods);

    currentModifiers &= ~ModifierKeys::allMouseButtonModifiers;
    currentModifiers |= mouseMods;

    return ModifierKeys (currentModifiers);
}

static void updateKeyModifiers (const int status) throw()
{
    currentModifiers &= ~(ModifierKeys::shiftModifier
                           | ModifierKeys::ctrlModifier
                           | ModifierKeys::altModifier);

    if (status & ShiftMask)
        currentModifiers |= ModifierKeys::shiftModifier;

    if (status & ControlMask)
        currentModifiers |= ModifierKeys::ctrlModifier;

    if (status & AltMask)
        currentModifiers |= ModifierKeys::altModifier;

    numLock  = ((status & NumLockMask) != 0);
    capsLock = ((status & LockMask) != 0);
}

static bool updateKeyModifiersFromSym (KeySym sym, const bool press) throw()
{
    int modifier = 0;
    bool isModifier = true;

    switch (sym)
    {
        case XK_Shift_L:
        case XK_Shift_R:
            modifier = ModifierKeys::shiftModifier;
            break;

        case XK_Control_L:
        case XK_Control_R:
            modifier = ModifierKeys::ctrlModifier;
            break;

        case XK_Alt_L:
        case XK_Alt_R:
            modifier = ModifierKeys::altModifier;
            break;

        case XK_Num_Lock:
            if (press)
                numLock = ! numLock;

            break;

        case XK_Caps_Lock:
            if (press)
                capsLock = ! capsLock;

            break;

        case XK_Scroll_Lock:
            break;

        default:
            isModifier = false;
            break;
    }

    if (modifier != 0)
    {
        if (press)
            currentModifiers |= modifier;
        else
            currentModifiers &= ~modifier;
    }

    return isModifier;
}

//==============================================================================
#if JUCE_USE_XSHM
static bool isShmAvailable() throw()
{
    static bool isChecked = false;
    static bool isAvailable = false;

    if (! isChecked)
    {
        isChecked = true;

        int major, minor;
        Bool pixmaps;

        if (XShmQueryVersion (display, &major, &minor, &pixmaps))
        {
            trapErrors();

            XShmSegmentInfo segmentInfo;
            zerostruct (segmentInfo);
            XImage* xImage = XShmCreateImage (display, DefaultVisual (display, DefaultScreen (display)),
                                              24, ZPixmap, 0, &segmentInfo, 50, 50);

            if ((segmentInfo.shmid = shmget (IPC_PRIVATE,
                                             xImage->bytes_per_line * xImage->height,
                                             IPC_CREAT | 0777)) >= 0)
            {
                segmentInfo.shmaddr = (char*) shmat (segmentInfo.shmid, 0, 0);

                if (segmentInfo.shmaddr != (void*) -1)
                {
                    segmentInfo.readOnly = False;
                    xImage->data = segmentInfo.shmaddr;
                    XSync (display, False);

                    if (XShmAttach (display, &segmentInfo) != 0)
                    {
                        XSync (display, False);
                        XShmDetach (display, &segmentInfo);

                        isAvailable = true;
                    }
                }

                XFlush (display);
                XDestroyImage (xImage);

                shmdt (segmentInfo.shmaddr);
            }

            shmctl (segmentInfo.shmid, IPC_RMID, 0);

            isAvailable &= untrapErrors();
        }
    }

    return isAvailable;
}
#endif

//==============================================================================
static Pixmap juce_createColourPixmapFromImage (Display* display, const Image& image)
{
    const int width = image.getWidth();
    const int height = image.getHeight();
    HeapBlock <uint32> colour (width * height);
    int index = 0;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            colour[index++] = image.getPixelAt (x, y).getARGB();

    XImage* ximage = XCreateImage (display, CopyFromParent, 24, ZPixmap,
                                   0, (char*) colour, width, height, 32, 0);

    Pixmap pixmap = XCreatePixmap (display, DefaultRootWindow (display),
                                   width, height, 24);

    GC gc = XCreateGC (display, pixmap, 0, 0);
    XPutImage (display, pixmap, gc, ximage, 0, 0, 0, 0, width, height);
    XFreeGC (display, gc);

    return pixmap;
}

static Pixmap juce_createMaskPixmapFromImage (Display* display, const Image& image)
{
    const int width = image.getWidth();
    const int height = image.getHeight();
    const int stride = (width + 7) >> 3;
    HeapBlock <uint8> mask;
    mask.calloc (stride * height);
    const bool msbfirst = (BitmapBitOrder (display) == MSBFirst);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const uint8 bit = (uint8) (1 << (msbfirst ? (7 - (x & 7)) : (x & 7)));
            const int offset = y * stride + (x >> 3);

            if (image.getPixelAt (x, y).getAlpha() >= 128)
                mask[offset] |= bit;
        }
    }

    return XCreatePixmapFromBitmapData (display, DefaultRootWindow (display),
                                        (char*) mask, width, height, 1, 0, 1);
}


//==============================================================================
class XBitmapImage  : public Image
{
public:
    //==============================================================================
    XBitmapImage (const PixelFormat format_, const int w, const int h,
                  const bool clearImage, const bool is16Bit_)
        : Image (format_, w, h),
          is16Bit (is16Bit_)
    {
        jassert (format_ == RGB || format_ == ARGB);

        pixelStride = (format_ == RGB) ? 3 : 4;
        lineStride = ((w * pixelStride + 3) & ~3);

        Visual* const visual = DefaultVisual (display, DefaultScreen (display));

#if JUCE_USE_XSHM
        usingXShm = false;

        if ((! is16Bit) && isShmAvailable())
        {
            zerostruct (segmentInfo);

            xImage = XShmCreateImage (display, visual, 24, ZPixmap, 0, &segmentInfo, w, h);

            if (xImage != 0)
            {
                if ((segmentInfo.shmid = shmget (IPC_PRIVATE,
                                                 xImage->bytes_per_line * xImage->height,
                                                 IPC_CREAT | 0777)) >= 0)
                {
                    segmentInfo.shmaddr = (char*) shmat (segmentInfo.shmid, 0, 0);

                    if (segmentInfo.shmaddr != (void*) -1)
                    {
                        segmentInfo.readOnly = False;

                        xImage->data = segmentInfo.shmaddr;
                        imageData = (uint8*) segmentInfo.shmaddr;

                        XSync (display, False);

                        if (XShmAttach (display, &segmentInfo) != 0)
                        {
                            XSync (display, False);
                            usingXShm = true;
                        }
                        else
                        {
                            jassertfalse
                        }
                    }
                    else
                    {
                        shmctl (segmentInfo.shmid, IPC_RMID, 0);
                    }
                }
            }
        }

        if (! usingXShm)
#endif
        {
            imageDataAllocated.malloc (lineStride * h);
            imageData = imageDataAllocated;

            if (format_ == ARGB && clearImage)
                zeromem (imageData, h * lineStride);

            xImage = (XImage*) juce_calloc (sizeof (XImage));

            xImage->width = w;
            xImage->height = h;
            xImage->xoffset = 0;
            xImage->format = ZPixmap;
            xImage->data = (char*) imageData;
            xImage->byte_order = ImageByteOrder (display);
            xImage->bitmap_unit = BitmapUnit (display);
            xImage->bitmap_bit_order = BitmapBitOrder (display);
            xImage->bitmap_pad = 32;
            xImage->depth = pixelStride * 8;
            xImage->bytes_per_line = lineStride;
            xImage->bits_per_pixel = pixelStride * 8;
            xImage->red_mask   = 0x00FF0000;
            xImage->green_mask = 0x0000FF00;
            xImage->blue_mask  = 0x000000FF;

            if (is16Bit)
            {
                const int pixelStride = 2;
                const int lineStride = ((w * pixelStride + 3) & ~3);

                imageData16Bit.malloc (lineStride * h);
                xImage->data = imageData16Bit;
                xImage->bitmap_pad = 16;
                xImage->depth = pixelStride * 8;
                xImage->bytes_per_line = lineStride;
                xImage->bits_per_pixel = pixelStride * 8;
                xImage->red_mask   = visual->red_mask;
                xImage->green_mask = visual->green_mask;
                xImage->blue_mask  = visual->blue_mask;
            }

            if (! XInitImage (xImage))
            {
                jassertfalse
            }
        }
    }

    ~XBitmapImage()
    {
#if JUCE_USE_XSHM
        if (usingXShm)
        {
            XShmDetach (display, &segmentInfo);

            XFlush (display);
            XDestroyImage (xImage);

            shmdt (segmentInfo.shmaddr);
            shmctl (segmentInfo.shmid, IPC_RMID, 0);
        }
        else
#endif
        {
            xImage->data = 0;
            XDestroyImage (xImage);
        }
    }

    void blitToWindow (Window window, int dx, int dy, int dw, int dh, int sx, int sy)
    {
        static GC gc = 0;

        if (gc == 0)
            gc = DefaultGC (display, DefaultScreen (display));

        if (is16Bit)
        {
            const uint32 rMask = xImage->red_mask;
            const uint32 rShiftL = jmax (0, getShiftNeeded (rMask));
            const uint32 rShiftR = jmax (0, -getShiftNeeded (rMask));
            const uint32 gMask = xImage->green_mask;
            const uint32 gShiftL = jmax (0, getShiftNeeded (gMask));
            const uint32 gShiftR = jmax (0, -getShiftNeeded (gMask));
            const uint32 bMask = xImage->blue_mask;
            const uint32 bShiftL = jmax (0, getShiftNeeded (bMask));
            const uint32 bShiftR = jmax (0, -getShiftNeeded (bMask));

            const Image::BitmapData srcData (*this, 0, 0, getWidth(), getHeight());

            for (int y = sy; y < sy + dh; ++y)
            {
                const uint8* p = srcData.getPixelPointer (sx, y);

                for (int x = sx; x < sx + dw; ++x)
                {
                    const PixelRGB* const pixel = (const PixelRGB*) p;
                    p += srcData.pixelStride;

                    XPutPixel (xImage, x, y,
                               (((((uint32) pixel->getRed()) << rShiftL) >> rShiftR) & rMask)
                                 | (((((uint32) pixel->getGreen()) << gShiftL) >> gShiftR) & gMask)
                                 | (((((uint32) pixel->getBlue()) << bShiftL) >> bShiftR) & bMask));
                }
            }
        }

        // blit results to screen.
#if JUCE_USE_XSHM
        if (usingXShm)
            XShmPutImage (display, (::Drawable) window, gc, xImage, sx, sy, dx, dy, dw, dh, True);
        else
#endif
            XPutImage (display, (::Drawable) window, gc, xImage, sx, sy, dx, dy, dw, dh);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    XImage* xImage;
    const bool is16Bit;
    HeapBlock <char> imageData16Bit;

#if JUCE_USE_XSHM
    XShmSegmentInfo segmentInfo;
    bool usingXShm;
#endif

    static int getShiftNeeded (const uint32 mask) throw()
    {
        for (int i = 32; --i >= 0;)
            if (((mask >> i) & 1) != 0)
                return i - 7;

        jassertfalse
        return 0;
    }
};

#define checkMessageManagerIsLocked     jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

//==============================================================================
class LinuxComponentPeer  : public ComponentPeer
{
public:
    //==============================================================================
    LinuxComponentPeer (Component* const component, const int windowStyleFlags)
        : ComponentPeer (component, windowStyleFlags),
          windowH (0),
          parentWindow (0),
          wx (0),
          wy (0),
          ww (0),
          wh (0),
          taskbarImage (0),
          fullScreen (false),
          entered (false),
          mapped (false)
    {
        // it's dangerous to create a window on a thread other than the message thread..
        checkMessageManagerIsLocked

        repainter = new LinuxRepaintManager (this);

        createWindow();

        setTitle (component->getName());
    }

    ~LinuxComponentPeer()
    {
        // it's dangerous to delete a window on a thread other than the message thread..
        checkMessageManagerIsLocked

        deleteTaskBarIcon();
        deleteIconPixmaps();

        destroyWindow();

        windowH = 0;
    }

    //==============================================================================
    void* getNativeHandle() const
    {
        return (void*) windowH;
    }

    static LinuxComponentPeer* getPeerFor (Window windowHandle) throw()
    {
        XPointer peer = 0;

        if (! XFindContext (display, (XID) windowHandle, improbableNumber, &peer))
        {
            if (peer != 0 && ! ((LinuxComponentPeer*) peer)->isValidMessageListener())
                peer = 0;
        }

        return (LinuxComponentPeer*) peer;
    }

    void setVisible (bool shouldBeVisible)
    {
        if (shouldBeVisible)
            XMapWindow (display, windowH);
        else
            XUnmapWindow (display, windowH);
    }

    void setTitle (const String& title)
    {
        setWindowTitle (windowH, title);
    }

    void setPosition (int x, int y)
    {
        setBounds (x, y, ww, wh, false);
    }

    void setSize (int w, int h)
    {
        setBounds (wx, wy, w, h, false);
    }

    void setBounds (int x, int y, int w, int h, const bool isNowFullScreen)
    {
        fullScreen = isNowFullScreen;

        if (windowH != 0)
        {
            const ComponentDeletionWatcher deletionChecker (component);

            wx = x;
            wy = y;
            ww = jmax (1, w);
            wh = jmax (1, h);

            // Make sure the Window manager does what we want
            XSizeHints* hints = XAllocSizeHints();
            hints->flags = USSize | USPosition;
            hints->width = ww;
            hints->height = wh;
            hints->x = wx;
            hints->y = wy;

            if ((getStyleFlags() & (windowHasTitleBar | windowIsResizable)) == windowHasTitleBar)
            {
                hints->min_width  = hints->max_width  = hints->width;
                hints->min_height = hints->max_height = hints->height;
                hints->flags |= PMinSize | PMaxSize;
            }

            XSetWMNormalHints (display, windowH, hints);
            XFree (hints);

            XMoveResizeWindow (display, windowH,
                               wx - windowBorder.getLeft(),
                               wy - windowBorder.getTop(), ww, wh);

            if (! deletionChecker.hasBeenDeleted())
            {
                updateBorderSize();
                handleMovedOrResized();
            }
        }
    }

    void getBounds (int& x, int& y, int& w, int& h) const
    {
        x = wx;
        y = wy;
        w = ww;
        h = wh;
    }

    int getScreenX() const
    {
        return wx;
    }

    int getScreenY() const
    {
        return wy;
    }

    void relativePositionToGlobal (int& x, int& y)
    {
        x += wx;
        y += wy;
    }

    void globalPositionToRelative (int& x, int& y)
    {
        x -= wx;
        y -= wy;
    }

    void setMinimised (bool shouldBeMinimised)
    {
        if (shouldBeMinimised)
        {
            Window root = RootWindow (display, DefaultScreen (display));

            XClientMessageEvent clientMsg;
            clientMsg.display = display;
            clientMsg.window = windowH;
            clientMsg.type = ClientMessage;
            clientMsg.format = 32;
            clientMsg.message_type = wm_ChangeState;
            clientMsg.data.l[0] = IconicState;

            XSendEvent (display, root, false,
                        SubstructureRedirectMask | SubstructureNotifyMask,
                        (XEvent*) &clientMsg);
        }
        else
        {
            setVisible (true);
        }
    }

    bool isMinimised() const
    {
        bool minimised = false;

        unsigned char* stateProp;
        unsigned long nitems, bytesLeft;
        Atom actualType;
        int actualFormat;

        if (XGetWindowProperty (display, windowH, wm_State, 0, 64, False,
                                wm_State, &actualType, &actualFormat, &nitems, &bytesLeft,
                                &stateProp) == Success
            && actualType == wm_State
            && actualFormat == 32
            && nitems > 0)
        {
            if (((unsigned long*) stateProp)[0] == IconicState)
                minimised = true;

            XFree (stateProp);
        }

        return minimised;
    }

    void setFullScreen (const bool shouldBeFullScreen)
    {
        Rectangle r (lastNonFullscreenBounds); // (get a copy of this before de-minimising)

        setMinimised (false);

        if (fullScreen != shouldBeFullScreen)
        {
            if (shouldBeFullScreen)
                r = Desktop::getInstance().getMainMonitorArea();

            if (! r.isEmpty())
                setBounds (r.getX(), r.getY(), r.getWidth(), r.getHeight(), shouldBeFullScreen);

            getComponent()->repaint();
        }
    }

    bool isFullScreen() const
    {
        return fullScreen;
    }

    bool isChildWindowOf (Window possibleParent) const
    {
        Window* windowList = 0;
        uint32 windowListSize = 0;
        Window parent, root;

        if (XQueryTree (display, windowH, &root, &parent, &windowList, &windowListSize) != 0)
        {
            if (windowList != 0)
                XFree (windowList);

            return parent == possibleParent;
        }

        return false;
    }

    bool isFrontWindow() const
    {
        Window* windowList = 0;
        uint32 windowListSize = 0;
        bool result = false;

        Window parent, root = RootWindow (display, DefaultScreen (display));

        if (XQueryTree (display, root, &root, &parent, &windowList, &windowListSize) != 0)
        {
            for (int i = windowListSize; --i >= 0;)
            {
                LinuxComponentPeer* const peer = LinuxComponentPeer::getPeerFor (windowList[i]);

                if (peer != 0)
                {
                    result = (peer == this);
                    break;
                }
            }
        }

        if (windowList != 0)
            XFree (windowList);

        return result;
    }

    bool contains (int x, int y, bool trueIfInAChildWindow) const
    {
        jassert (x >= 0 && y >= 0 && x < ww && y < wh); // should only be called for points that are actually inside the bounds

        if (((unsigned int) x) >= (unsigned int) ww
             || ((unsigned int) y) >= (unsigned int) wh)
            return false;

        bool inFront = false;

        for (int i = 0; i < Desktop::getInstance().getNumComponents(); ++i)
        {
            Component* const c = Desktop::getInstance().getComponent (i);

            if (inFront)
            {
                if (c->contains (x + wx - c->getScreenX(),
                                 y + wy - c->getScreenY()))
                {
                    return false;
                }
            }
            else if (c == getComponent())
            {
                inFront = true;
            }
        }

        if (trueIfInAChildWindow)
            return true;

        ::Window root, child;
        unsigned int bw, depth;
        int wx, wy, w, h;

        if (! XGetGeometry (display, (::Drawable) windowH, &root,
                            &wx, &wy, (unsigned int*) &w, (unsigned int*) &h,
                            &bw, &depth))
        {
            return false;
        }

        if (! XTranslateCoordinates (display, windowH, windowH, x, y, &wx, &wy, &child))
            return false;

        return child == None;
    }

    const BorderSize getFrameSize() const
    {
        return BorderSize();
    }

    bool setAlwaysOnTop (bool alwaysOnTop)
    {
        if (windowH != 0)
        {
            const bool wasVisible = component->isVisible();

            if (wasVisible)
                setVisible (false);  // doesn't always seem to work if the window is visible when this is done..

            XSetWindowAttributes swa;
            swa.override_redirect = alwaysOnTop ? True : False;

            XChangeWindowAttributes (display, windowH, CWOverrideRedirect, &swa);

            if (wasVisible)
                setVisible (true);
        }

        return true;
    }

    void toFront (bool makeActive)
    {
        if (makeActive)
        {
            setVisible (true);
            grabFocus();
        }

        XEvent ev;
        ev.xclient.type = ClientMessage;
        ev.xclient.serial = 0;
        ev.xclient.send_event = True;
        ev.xclient.message_type = wm_ActiveWin;
        ev.xclient.window = windowH;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = 2;
        ev.xclient.data.l[1] = CurrentTime;
        ev.xclient.data.l[2] = 0;
        ev.xclient.data.l[3] = 0;
        ev.xclient.data.l[4] = 0;

        XSendEvent (display, RootWindow (display, DefaultScreen (display)),
                    False,
                    SubstructureRedirectMask | SubstructureNotifyMask,
                    &ev);

        XWindowAttributes attr;
        XGetWindowAttributes (display, windowH, &attr);

        if (attr.override_redirect)
            XRaiseWindow (display, windowH);

        XSync (display, False);

        handleBroughtToFront();
    }

    void toBehind (ComponentPeer* other)
    {
        LinuxComponentPeer* const otherPeer = dynamic_cast <LinuxComponentPeer*> (other);
        jassert (otherPeer != 0); // wrong type of window?

        if (otherPeer != 0)
        {
            setMinimised (false);

            Window newStack[] = { otherPeer->windowH, windowH };

            XRestackWindows (display, newStack, 2);
        }
    }

    bool isFocused() const
    {
        int revert;
        Window focusedWindow = 0;
        XGetInputFocus (display, &focusedWindow, &revert);

        return focusedWindow == windowH;
    }

    void grabFocus()
    {
        XWindowAttributes atts;

        if (windowH != 0
            && XGetWindowAttributes (display, windowH, &atts)
            && atts.map_state == IsViewable
            && ! isFocused())
        {
            XSetInputFocus (display, windowH, RevertToParent, CurrentTime);
            isActiveApplication = true;
        }
    }

    void textInputRequired (int /*x*/, int /*y*/)
    {
    }

    void repaint (int x, int y, int w, int h)
    {
        if (Rectangle::intersectRectangles (x, y, w, h,
                                            0, 0,
                                            getComponent()->getWidth(),
                                            getComponent()->getHeight()))
        {
            repainter->repaint (x, y, w, h);
        }
    }

    void performAnyPendingRepaintsNow()
    {
        repainter->performAnyPendingRepaintsNow();
    }

    void setIcon (const Image& newIcon)
    {
        const int dataSize = newIcon.getWidth() * newIcon.getHeight() + 2;
        HeapBlock <unsigned long> data (dataSize);

        int index = 0;
        data[index++] = newIcon.getWidth();
        data[index++] = newIcon.getHeight();

        for (int y = 0; y < newIcon.getHeight(); ++y)
            for (int x = 0; x < newIcon.getWidth(); ++x)
                data[index++] = newIcon.getPixelAt (x, y).getARGB();

        XChangeProperty (display, windowH,
                         XInternAtom (display, "_NET_WM_ICON", False),
                         XA_CARDINAL, 32, PropModeReplace,
                         (unsigned char*) data, dataSize);

        deleteIconPixmaps();

        XWMHints* wmHints = XGetWMHints (display, windowH);

        if (wmHints == 0)
            wmHints = XAllocWMHints();

        wmHints->flags |= IconPixmapHint | IconMaskHint;
        wmHints->icon_pixmap = juce_createColourPixmapFromImage (display, newIcon);
        wmHints->icon_mask = juce_createMaskPixmapFromImage (display, newIcon);

        XSetWMHints (display, windowH, wmHints);
        XFree (wmHints);

        XSync (display, False);
    }

    void deleteIconPixmaps()
    {
        XWMHints* wmHints = XGetWMHints (display, windowH);

        if (wmHints != 0)
        {
            if ((wmHints->flags & IconPixmapHint) != 0)
            {
                wmHints->flags &= ~IconPixmapHint;
                XFreePixmap (display, wmHints->icon_pixmap);
            }

            if ((wmHints->flags & IconMaskHint) != 0)
            {
                wmHints->flags &= ~IconMaskHint;
                XFreePixmap (display, wmHints->icon_mask);
            }

            XSetWMHints (display, windowH, wmHints);
            XFree (wmHints);
        }
    }

    //==============================================================================
    void handleWindowMessage (XEvent* event)
    {
        switch (event->xany.type)
        {
            case 2: // 'KeyPress'
            {
                XKeyEvent* const keyEvent = (XKeyEvent*) &event->xkey;
                updateKeyStates (keyEvent->keycode, true);

                char utf8 [64];
                zeromem (utf8, sizeof (utf8));
                KeySym sym;
                XLookupString (keyEvent, utf8, sizeof (utf8), &sym, 0);

                const juce_wchar unicodeChar = *(const juce_wchar*) String::fromUTF8 ((const uint8*) utf8, sizeof (utf8) - 1);
                int keyCode = (int) unicodeChar;

                if (keyCode < 0x20)
                    keyCode = XKeycodeToKeysym (display, keyEvent->keycode,
                                                (currentModifiers & ModifierKeys::shiftModifier) != 0 ? 1 : 0);

                const int oldMods = currentModifiers;
                bool keyPressed = false;

                const bool keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, true);

                if ((sym & 0xff00) == 0xff00)
                {
                    // Translate keypad
                    if (sym == XK_KP_Divide)
                        keyCode = XK_slash;
                    else if (sym == XK_KP_Multiply)
                        keyCode = XK_asterisk;
                    else if (sym == XK_KP_Subtract)
                        keyCode = XK_hyphen;
                    else if (sym == XK_KP_Add)
                        keyCode = XK_plus;
                    else if (sym == XK_KP_Enter)
                        keyCode = XK_Return;
                    else if (sym == XK_KP_Decimal)
                        keyCode = numLock ? XK_period : XK_Delete;
                    else if (sym == XK_KP_0)
                        keyCode = numLock ? XK_0 : XK_Insert;
                    else if (sym == XK_KP_1)
                        keyCode = numLock ? XK_1 : XK_End;
                    else if (sym == XK_KP_2)
                        keyCode = numLock ? XK_2 : XK_Down;
                    else if (sym == XK_KP_3)
                        keyCode = numLock ? XK_3 : XK_Page_Down;
                    else if (sym == XK_KP_4)
                        keyCode = numLock ? XK_4 : XK_Left;
                    else if (sym == XK_KP_5)
                        keyCode = XK_5;
                    else if (sym == XK_KP_6)
                        keyCode = numLock ? XK_6 : XK_Right;
                    else if (sym == XK_KP_7)
                        keyCode = numLock ? XK_7 : XK_Home;
                    else if (sym == XK_KP_8)
                        keyCode = numLock ? XK_8 : XK_Up;
                    else if (sym == XK_KP_9)
                        keyCode = numLock ? XK_9 : XK_Page_Up;

                    switch (sym)
                    {
                        case XK_Left:
                        case XK_Right:
                        case XK_Up:
                        case XK_Down:
                        case XK_Page_Up:
                        case XK_Page_Down:
                        case XK_End:
                        case XK_Home:
                        case XK_Delete:
                        case XK_Insert:
                            keyPressed = true;
                            keyCode = (sym & 0xff) | extendedKeyModifier;
                            break;
                        case XK_Tab:
                        case XK_Return:
                        case XK_Escape:
                        case XK_BackSpace:
                            keyPressed = true;
                            keyCode &= 0xff;
                            break;
                        default:
                        {
                            if (sym >= XK_F1 && sym <= XK_F16)
                            {
                                keyPressed = true;
                                keyCode = (sym & 0xff) | extendedKeyModifier;
                            }
                            break;
                        }
                    }
                }

                if (utf8[0] != 0 || ((sym & 0xff00) == 0 && sym >= 8))
                    keyPressed = true;

                if (oldMods != currentModifiers)
                    handleModifierKeysChange();

                if (keyDownChange)
                    handleKeyUpOrDown (true);

                if (keyPressed)
                    handleKeyPress (keyCode, unicodeChar);

                break;
            }

            case KeyRelease:
            {
                const XKeyEvent* const keyEvent = (const XKeyEvent*) &event->xkey;
                updateKeyStates (keyEvent->keycode, false);

                KeySym sym = XKeycodeToKeysym (display, keyEvent->keycode, 0);

                const int oldMods = currentModifiers;
                const bool keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, false);

                if (oldMods != currentModifiers)
                    handleModifierKeysChange();

                if (keyDownChange)
                    handleKeyUpOrDown (false);

                break;
            }

            case ButtonPress:
            {
                const XButtonPressedEvent* const buttonPressEvent = (const XButtonPressedEvent*) &event->xbutton;

                bool buttonMsg = false;
                bool wheelUpMsg = false;
                bool wheelDownMsg = false;

                const int map = pointerMap [buttonPressEvent->button - Button1];

                if (map == LeftButton)
                {
                    currentModifiers |= ModifierKeys::leftButtonModifier;
                    buttonMsg = true;
                }
                else if (map == RightButton)
                {
                    currentModifiers |= ModifierKeys::rightButtonModifier;
                    buttonMsg = true;
                }
                else if (map == MiddleButton)
                {
                    currentModifiers |= ModifierKeys::middleButtonModifier;
                    buttonMsg = true;
                }
                else if (map == WheelUp)
                {
                    wheelUpMsg = true;
                }
                else if (map == WheelDown)
                {
                    wheelDownMsg = true;
                }

                updateKeyModifiers (buttonPressEvent->state);

                if (buttonMsg)
                {
                    toFront (true);
                    handleMouseDown (buttonPressEvent->x, buttonPressEvent->y,
                                     getEventTime (buttonPressEvent->time));
                }
                else if (wheelUpMsg || wheelDownMsg)
                {
                    handleMouseWheel (0, wheelDownMsg ? -84 : 84,
                                      getEventTime (buttonPressEvent->time));
                }

                lastMousePosX = lastMousePosY = 0x100000;
                break;
            }

            case ButtonRelease:
            {
                const XButtonReleasedEvent* const buttonRelEvent = (const XButtonReleasedEvent*) &event->xbutton;

                const int oldModifiers = currentModifiers;
                const int map = pointerMap [buttonRelEvent->button - Button1];

                if (map == LeftButton)
                    currentModifiers &= ~ModifierKeys::leftButtonModifier;
                else if (map == RightButton)
                    currentModifiers &= ~ModifierKeys::rightButtonModifier;
                else if (map == MiddleButton)
                    currentModifiers &= ~ModifierKeys::middleButtonModifier;

                updateKeyModifiers (buttonRelEvent->state);

                handleMouseUp (oldModifiers,
                               buttonRelEvent->x, buttonRelEvent->y,
                               getEventTime (buttonRelEvent->time));

                lastMousePosX = lastMousePosY = 0x100000;
                break;
            }

            case MotionNotify:
            {
                const XPointerMovedEvent* const movedEvent = (const XPointerMovedEvent*) &event->xmotion;

                updateKeyModifiers (movedEvent->state);

                int x, y, mouseMods;
                getMousePos (x, y, mouseMods);

                if (lastMousePosX != x || lastMousePosY != y)
                {
                    lastMousePosX = x;
                    lastMousePosY = y;

                    if (parentWindow != 0 && (styleFlags & windowHasTitleBar) == 0)
                    {
                        Window wRoot = 0, wParent = 0;
                        Window* wChild = 0;
                        unsigned int numChildren;
                        XQueryTree (display, windowH, &wRoot, &wParent, &wChild, &numChildren);

                        if (wParent != 0
                            && wParent != windowH
                            && wParent != wRoot)
                        {
                            parentWindow = wParent;
                            updateBounds();
                            x -= getScreenX();
                            y -= getScreenY();
                        }
                        else
                        {
                            parentWindow = 0;
                            x -= getScreenX();
                            y -= getScreenY();
                        }
                    }
                    else
                    {
                        x -= getScreenX();
                        y -= getScreenY();
                    }

                    if ((currentModifiers & ModifierKeys::allMouseButtonModifiers) == 0)
                        handleMouseMove (x, y, getEventTime (movedEvent->time));
                    else
                        handleMouseDrag (x, y, getEventTime (movedEvent->time));
                }

                break;
            }

            case EnterNotify:
            {
                lastMousePosX = lastMousePosY = 0x100000;
                const XEnterWindowEvent* const enterEvent = (const XEnterWindowEvent*) &event->xcrossing;

                if ((currentModifiers & ModifierKeys::allMouseButtonModifiers) == 0
                     && ! entered)
                {
                    updateKeyModifiers (enterEvent->state);

                    handleMouseEnter (enterEvent->x, enterEvent->y, getEventTime (enterEvent->time));

                    entered = true;
                }

                break;
            }

            case LeaveNotify:
            {
                const XLeaveWindowEvent* const leaveEvent = (const XLeaveWindowEvent*) &event->xcrossing;

                // Suppress the normal leave if we've got a pointer grab, or if
                // it's a bogus one caused by clicking a mouse button when running
                // in a Window manager
                if (((currentModifiers & ModifierKeys::allMouseButtonModifiers) == 0
                     && leaveEvent->mode == NotifyNormal)
                    || leaveEvent->mode == NotifyUngrab)
                {
                    updateKeyModifiers (leaveEvent->state);

                    handleMouseExit (leaveEvent->x, leaveEvent->y, getEventTime (leaveEvent->time));

                    entered = false;
                }

                break;
            }

            case FocusIn:
            {
                isActiveApplication = true;
                if (isFocused())
                    handleFocusGain();

                break;
            }

            case FocusOut:
            {
                isActiveApplication = false;
                if (! isFocused())
                    handleFocusLoss();

                break;
            }

            case Expose:
            {
                // Batch together all pending expose events
                XExposeEvent* exposeEvent = (XExposeEvent*) &event->xexpose;
                XEvent nextEvent;

                if (exposeEvent->window != windowH)
                {
                    Window child;
                    XTranslateCoordinates (display, exposeEvent->window, windowH,
                                           exposeEvent->x, exposeEvent->y, &exposeEvent->x, &exposeEvent->y,
                                           &child);
                }

                repaint (exposeEvent->x, exposeEvent->y,
                         exposeEvent->width, exposeEvent->height);

                while (XEventsQueued (display, QueuedAfterFlush) > 0)
                {
                    XPeekEvent (display, (XEvent*) &nextEvent);
                    if (nextEvent.type != Expose || nextEvent.xany.window != event->xany.window)
                        break;

                    XNextEvent (display, (XEvent*) &nextEvent);
                    XExposeEvent* nextExposeEvent = (XExposeEvent*) &nextEvent.xexpose;
                    repaint (nextExposeEvent->x, nextExposeEvent->y,
                             nextExposeEvent->width, nextExposeEvent->height);
                }

                break;
            }

            case CirculateNotify:
            case CreateNotify:
            case DestroyNotify:
                // Think we can ignore these
                break;

            case ConfigureNotify:
            {
                updateBounds();
                updateBorderSize();
                handleMovedOrResized();

                // if the native title bar is dragged, need to tell any active menus, etc.
                if ((styleFlags & windowHasTitleBar) != 0
                      && component->isCurrentlyBlockedByAnotherModalComponent())
                {
                    Component* const currentModalComp = Component::getCurrentlyModalComponent();

                    if (currentModalComp != 0)
                        currentModalComp->inputAttemptWhenModal();
                }

                XConfigureEvent* const confEvent = (XConfigureEvent*) &event->xconfigure;

                if (confEvent->window == windowH
                     && confEvent->above != 0
                     && isFrontWindow())
                {
                    handleBroughtToFront();
                }

                break;
            }

            case ReparentNotify:
            case GravityNotify:
            {
                parentWindow = 0;
                Window wRoot = 0;
                Window* wChild = 0;
                unsigned int numChildren;
                XQueryTree (display, windowH, &wRoot, &parentWindow, &wChild, &numChildren);

                if (parentWindow == windowH || parentWindow == wRoot)
                    parentWindow = 0;

                updateBounds();
                updateBorderSize();
                handleMovedOrResized();
                break;
            }

            case MapNotify:
                mapped = true;
                handleBroughtToFront();
                break;

            case UnmapNotify:
                mapped = false;
                break;

            case MappingNotify:
            {
                XMappingEvent* mappingEvent = (XMappingEvent*) &event->xmapping;

                if (mappingEvent->request != MappingPointer)
                {
                    // Deal with modifier/keyboard mapping
                    XRefreshKeyboardMapping (mappingEvent);
                    getModifierMapping();
                }

                break;
            }

            case ClientMessage:
            {
                const XClientMessageEvent* const clientMsg = (const XClientMessageEvent*) &event->xclient;

                if (clientMsg->message_type == wm_Protocols && clientMsg->format == 32)
                {
                    const Atom atom = (Atom) clientMsg->data.l[0];

                    if (atom == wm_ProtocolList [TAKE_FOCUS])
                    {
                        XWindowAttributes atts;

                        if (clientMsg->window != 0
                             && XGetWindowAttributes (display, clientMsg->window, &atts))
                        {
                            if (atts.map_state == IsViewable)
                                XSetInputFocus (display, clientMsg->window, RevertToParent, clientMsg->data.l[1]);
                        }
                    }
                    else if (atom == wm_ProtocolList [DELETE_WINDOW])
                    {
                        handleUserClosingWindow();
                    }
                }
                else if (clientMsg->message_type == XA_XdndEnter)
                {
                    handleDragAndDropEnter (clientMsg);
                }
                else if (clientMsg->message_type == XA_XdndLeave)
                {
                    resetDragAndDrop();
                }
                else if (clientMsg->message_type == XA_XdndPosition)
                {
                    handleDragAndDropPosition (clientMsg);
                }
                else if (clientMsg->message_type == XA_XdndDrop)
                {
                    handleDragAndDropDrop (clientMsg);
                }
                else if (clientMsg->message_type == XA_XdndStatus)
                {
                    handleDragAndDropStatus (clientMsg);
                }
                else if (clientMsg->message_type == XA_XdndFinished)
                {
                    resetDragAndDrop();
                }

                break;
            }

            case SelectionNotify:
                handleDragAndDropSelection (event);
                break;

            case SelectionClear:
            case SelectionRequest:
                break;

            default:
#if JUCE_USE_XSHM
                if (event->xany.type == XShmGetEventBase (display))
                    repainter->notifyPaintCompleted();
#endif
                break;
        }
    }

    void showMouseCursor (Cursor cursor) throw()
    {
        XDefineCursor (display, windowH, cursor);
    }

    //==============================================================================
    void setTaskBarIcon (const Image& image)
    {
        deleteTaskBarIcon();
        taskbarImage = image.createCopy();

        Screen* const screen = XDefaultScreenOfDisplay (display);
        const int screenNumber = XScreenNumberOfScreen (screen);

        char screenAtom[32];
        snprintf (screenAtom, sizeof (screenAtom), "_NET_SYSTEM_TRAY_S%d", screenNumber);
        Atom selectionAtom = XInternAtom (display, screenAtom, false);

        XGrabServer (display);
        Window managerWin = XGetSelectionOwner (display, selectionAtom);

        if (managerWin != None)
            XSelectInput (display, managerWin, StructureNotifyMask);

        XUngrabServer (display);
        XFlush (display);

        if (managerWin != None)
        {
            XEvent ev;
            zerostruct (ev);
            ev.xclient.type = ClientMessage;
            ev.xclient.window = managerWin;
            ev.xclient.message_type = XInternAtom (display, "_NET_SYSTEM_TRAY_OPCODE", False);
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = CurrentTime;
            ev.xclient.data.l[1] = SYSTEM_TRAY_REQUEST_DOCK;
            ev.xclient.data.l[2] = windowH;
            ev.xclient.data.l[3] = 0;
            ev.xclient.data.l[4] = 0;

            XSendEvent (display, managerWin, False, NoEventMask, &ev);
            XSync (display, False);
        }

        // For older KDE's ...
        long atomData = 1;
        Atom trayAtom = XInternAtom (display, "KWM_DOCKWINDOW", false);
        XChangeProperty (display, windowH, trayAtom, trayAtom, 32, PropModeReplace, (unsigned char*) &atomData, 1);

        // For more recent KDE's...
        trayAtom = XInternAtom (display, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", false);
        XChangeProperty (display, windowH, trayAtom, XA_WINDOW, 32, PropModeReplace, (unsigned char*) &windowH, 1);

        // a minimum size must be specified for GNOME and Xfce, otherwise the icon is displayed with a width of 1
        XSizeHints* hints = XAllocSizeHints();
        hints->flags = PMinSize;
        hints->min_width = 22;
        hints->min_height = 22;
        XSetWMNormalHints (display, windowH, hints);
        XFree (hints);
    }

    void deleteTaskBarIcon()
    {
        deleteAndZero (taskbarImage);
    }

    const Image* getTaskbarIcon() const throw()           { return taskbarImage; }

    //==============================================================================
    juce_UseDebuggingNewOperator

    bool dontRepaint;

private:
    //==============================================================================
    class LinuxRepaintManager : public Timer
    {
    public:
        LinuxRepaintManager (LinuxComponentPeer* const peer_)
            : peer (peer_),
              lastTimeImageUsed (0)
        {
#if JUCE_USE_XSHM
            shmCompletedDrawing = true;

            useARGBImagesForRendering = isShmAvailable();

            if (useARGBImagesForRendering)
            {
                XShmSegmentInfo segmentinfo;

                XImage* const testImage
                    = XShmCreateImage (display, DefaultVisual (display, DefaultScreen (display)),
                                       24, ZPixmap, 0, &segmentinfo, 64, 64);

                useARGBImagesForRendering = (testImage->bits_per_pixel == 32);
                XDestroyImage (testImage);
            }
#endif
        }

        ~LinuxRepaintManager()
        {
        }

        void timerCallback()
        {
            if (! regionsNeedingRepaint.isEmpty() && shmCompletedDrawing)
            {
                stopTimer();
                performAnyPendingRepaintsNow();
            }
            else if (Time::getApproximateMillisecondCounter() > lastTimeImageUsed + 3000)
            {
                stopTimer();
                image = 0;
            }
        }

        void repaint (int x, int y, int w, int h)
        {
            if (! isTimerRunning())
                startTimer (repaintTimerPeriod);

            regionsNeedingRepaint.add (x, y, w, h);
        }

        void performAnyPendingRepaintsNow()
        {
            peer->clearMaskedRegion();

            const Rectangle totalArea (regionsNeedingRepaint.getBounds());

            if (! totalArea.isEmpty())
            {
                shmCompletedDrawing = false;

                if (image == 0 || image->getWidth() < totalArea.getWidth()
                     || image->getHeight() < totalArea.getHeight())
                {
#if JUCE_USE_XSHM
                    image = new XBitmapImage (useARGBImagesForRendering ? Image::ARGB
                                                                        : Image::RGB,
#else
                    image = new XBitmapImage (Image::RGB,
#endif
                                              (totalArea.getWidth() + 31) & ~31,
                                              (totalArea.getHeight() + 31) & ~31,
                                              false,
                                              peer->depthIs16Bit);
                }

                startTimer (repaintTimerPeriod);

                LowLevelGraphicsSoftwareRenderer context (*image);

                context.setOrigin (-totalArea.getX(), -totalArea.getY());

                if (context.clipToRectangleList (regionsNeedingRepaint))
                    peer->handlePaint (context);

                if (! peer->maskedRegion.isEmpty())
                    regionsNeedingRepaint.subtract (peer->maskedRegion);

                for (RectangleList::Iterator i (regionsNeedingRepaint); i.next();)
                {
                    const Rectangle& r = *i.getRectangle();

                    image->blitToWindow (peer->windowH,
                                         r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                         r.getX() - totalArea.getX(), r.getY() - totalArea.getY());
                }
            }

            regionsNeedingRepaint.clear();

            lastTimeImageUsed = Time::getApproximateMillisecondCounter();
            startTimer (repaintTimerPeriod);
        }

#if JUCE_USE_XSHM
        void notifyPaintCompleted()
        {
            shmCompletedDrawing = true;
        }
#endif

    private:
        LinuxComponentPeer* const peer;
        ScopedPointer <XBitmapImage> image;
        uint32 lastTimeImageUsed;
        RectangleList regionsNeedingRepaint;

#if JUCE_USE_XSHM
        bool useARGBImagesForRendering;
        bool shmCompletedDrawing;
#endif
        LinuxRepaintManager (const LinuxRepaintManager&);
        const LinuxRepaintManager& operator= (const LinuxRepaintManager&);
    };

    ScopedPointer <LinuxRepaintManager> repainter;

    friend class LinuxRepaintManager;
    Window windowH, parentWindow;
    int wx, wy, ww, wh;
    Image* taskbarImage;
    bool fullScreen, entered, mapped, depthIs16Bit;
    BorderSize windowBorder;

    struct MotifWmHints
    {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    };

    //==============================================================================
    void removeWindowDecorations (Window wndH)
    {
        Atom hints = XInternAtom (display, "_MOTIF_WM_HINTS", True);

        if (hints != None)
        {
            MotifWmHints motifHints;
            zerostruct (motifHints);
            motifHints.flags = 2; /* MWM_HINTS_DECORATIONS */
            motifHints.decorations = 0;

            XChangeProperty (display, wndH, hints, hints, 32, PropModeReplace,
                             (unsigned char*) &motifHints, 4);
        }

        hints = XInternAtom (display, "_WIN_HINTS", True);

        if (hints != None)
        {
            long gnomeHints = 0;

            XChangeProperty (display, wndH, hints, hints, 32, PropModeReplace,
                             (unsigned char*) &gnomeHints, 1);
        }

        hints = XInternAtom (display, "KWM_WIN_DECORATION", True);

        if (hints != None)
        {
            long kwmHints = 2; /*KDE_tinyDecoration*/

            XChangeProperty (display, wndH, hints, hints, 32, PropModeReplace,
                             (unsigned char*) &kwmHints, 1);
        }

        hints = XInternAtom (display, "_NET_WM_WINDOW_TYPE", True);

        if (hints != None)
        {
            int netHints [2];
            int numHints = 0;
            if ((styleFlags & windowIsTemporary) != 0)
                netHints [numHints] = XInternAtom (display, "_NET_WM_WINDOW_TYPE_MENU", True);
            else
                netHints [numHints] = XInternAtom (display, "_NET_WM_WINDOW_TYPE_NORMAL", True);

            if (netHints [numHints] != 0)
                ++numHints;

            netHints[numHints] = XInternAtom (display, "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE", True);

            if (netHints [numHints] != 0)
                ++numHints;

            XChangeProperty (display, wndH, hints, XA_ATOM, 32, PropModeReplace,
                             (unsigned char*) &netHints, numHints);
        }
    }

    void addWindowButtons (Window wndH)
    {
        Atom hints = XInternAtom (display, "_MOTIF_WM_HINTS", True);

        if (hints != None)
        {
            MotifWmHints motifHints;
            zerostruct (motifHints);

            motifHints.flags = 1 | 2; /* MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS */
            motifHints.decorations = 2 /* MWM_DECOR_BORDER */ | 8 /* MWM_DECOR_TITLE */ | 16; /* MWM_DECOR_MENU */

            motifHints.functions = 4 /* MWM_FUNC_MOVE */;

            if ((styleFlags & windowHasCloseButton) != 0)
                motifHints.functions |= 32; /* MWM_FUNC_CLOSE */

            if ((styleFlags & windowHasMinimiseButton) != 0)
            {
                motifHints.functions |= 8; /* MWM_FUNC_MINIMIZE */
                motifHints.decorations |= 0x20; /* MWM_DECOR_MINIMIZE */
            }

            if ((styleFlags & windowHasMaximiseButton) != 0)
            {
                motifHints.functions |= 0x10; /* MWM_FUNC_MAXIMIZE */
                motifHints.decorations |= 0x40; /* MWM_DECOR_MAXIMIZE */
            }

            if ((styleFlags & windowIsResizable) != 0)
            {
                motifHints.functions |= 2; /* MWM_FUNC_RESIZE */
                motifHints.decorations |= 0x4; /* MWM_DECOR_RESIZEH */
            }

            XChangeProperty (display, wndH, hints, hints, 32, 0, (unsigned char*) &motifHints, 5);
        }

        hints = XInternAtom (display, "_NET_WM_ALLOWED_ACTIONS", True);

        if (hints != None)
        {
            int netHints [6];
            int num = 0;

            netHints [num++] = XInternAtom (display, "_NET_WM_ACTION_RESIZE", (styleFlags & windowIsResizable) ? True : False);
            netHints [num++] = XInternAtom (display, "_NET_WM_ACTION_FULLSCREEN", (styleFlags & windowHasMaximiseButton) ? True : False);
            netHints [num++] = XInternAtom (display, "_NET_WM_ACTION_MINIMIZE", (styleFlags & windowHasMinimiseButton) ? True : False);
            netHints [num++] = XInternAtom (display, "_NET_WM_ACTION_CLOSE", (styleFlags & windowHasCloseButton) ? True : False);

            XChangeProperty (display, wndH, hints, XA_ATOM, 32, PropModeReplace,
                             (unsigned char*) &netHints, num);
        }
    }

    void createWindow()
    {
        static bool atomsInitialised = false;

        if (! atomsInitialised)
        {
            atomsInitialised = true;

            wm_Protocols                      = XInternAtom (display, "WM_PROTOCOLS", 1);
            wm_ProtocolList [TAKE_FOCUS]      = XInternAtom (display, "WM_TAKE_FOCUS", 1);
            wm_ProtocolList [DELETE_WINDOW]   = XInternAtom (display, "WM_DELETE_WINDOW", 1);
            wm_ChangeState                    = XInternAtom (display, "WM_CHANGE_STATE", 1);
            wm_State                          = XInternAtom (display, "WM_STATE", 1);
            wm_ActiveWin                      = XInternAtom (display, "_NET_ACTIVE_WINDOW", False);

            XA_XdndAware                      = XInternAtom (display, "XdndAware", 0);
            XA_XdndEnter                      = XInternAtom (display, "XdndEnter", 0);
            XA_XdndLeave                      = XInternAtom (display, "XdndLeave", 0);
            XA_XdndPosition                   = XInternAtom (display, "XdndPosition", 0);
            XA_XdndStatus                     = XInternAtom (display, "XdndStatus", 0);
            XA_XdndDrop                       = XInternAtom (display, "XdndDrop", 0);
            XA_XdndFinished                   = XInternAtom (display, "XdndFinished", 0);
            XA_XdndSelection                  = XInternAtom (display, "XdndSelection", 0);
            XA_XdndProxy                      = XInternAtom (display, "XdndProxy", 0);

            XA_XdndTypeList                   = XInternAtom (display, "XdndTypeList", 0);
            XA_XdndActionList                 = XInternAtom (display, "XdndActionList", 0);
            XA_XdndActionCopy                 = XInternAtom (display, "XdndActionCopy", 0);
            XA_XdndActionMove                 = XInternAtom (display, "XdndActionMove", 0);
            XA_XdndActionLink                 = XInternAtom (display, "XdndActionLink", 0);
            XA_XdndActionAsk                  = XInternAtom (display, "XdndActionAsk", 0);
            XA_XdndActionPrivate              = XInternAtom (display, "XdndActionPrivate", 0);
            XA_XdndActionDescription          = XInternAtom (display, "XdndActionDescription", 0);

            XA_JXSelectionWindowProperty      = XInternAtom (display, "JXSelectionWindowProperty", 0);

            XA_MimeTextPlain                  = XInternAtom (display, "text/plain", 0);
            XA_MimeTextUriList                = XInternAtom (display, "text/uri-list", 0);
            XA_MimeRootDrop                   = XInternAtom (display, "application/x-rootwindow-drop", 0);
        }

        resetDragAndDrop();

        XA_OtherMime = XA_MimeTextPlain;  //  xxx why??
        allowedMimeTypeAtoms [0] = XA_MimeTextPlain;
        allowedMimeTypeAtoms [1] = XA_OtherMime;
        allowedMimeTypeAtoms [2] = XA_MimeTextUriList;

        allowedActions [0] = XA_XdndActionMove;
        allowedActions [1] = XA_XdndActionCopy;
        allowedActions [2] = XA_XdndActionLink;
        allowedActions [3] = XA_XdndActionAsk;
        allowedActions [4] = XA_XdndActionPrivate;

        // Get defaults for various properties
        const int screen = DefaultScreen (display);
        Window root = RootWindow (display, screen);

        // Attempt to create a 24-bit window on the default screen.  If this is not
        // possible then exit
        XVisualInfo desiredVisual;
        desiredVisual.screen = screen;
        desiredVisual.depth = 24;
        depthIs16Bit = false;

        int numVisuals;
        XVisualInfo* visuals = XGetVisualInfo (display, VisualScreenMask | VisualDepthMask,
                                               &desiredVisual, &numVisuals);

        if (numVisuals < 1 || visuals == 0)
        {
            XFree (visuals);
            desiredVisual.depth = 16;

            visuals = XGetVisualInfo (display, VisualScreenMask | VisualDepthMask,
                                      &desiredVisual, &numVisuals);

            if (numVisuals < 1 || visuals == 0)
            {
                Logger::outputDebugString ("ERROR: System doesn't support 24 or 16 bit RGB display.\n");
                Process::terminate();
            }

            depthIs16Bit = true;
        }

        XFree (visuals);

        // Set up the window attributes
        XSetWindowAttributes swa;
        swa.border_pixel = 0;
        swa.background_pixmap = None;
        swa.colormap = DefaultColormap (display, screen);
        swa.override_redirect = getComponent()->isAlwaysOnTop() ? True : False;
        swa.event_mask = eventMask;

        Window wndH = XCreateWindow (display, root,
                                     0, 0, 1, 1,
                                     0, 0, InputOutput, (Visual*) CopyFromParent,
                                     CWBorderPixel | CWColormap | CWBackPixmap | CWEventMask | CWOverrideRedirect,
                                     &swa);

        XGrabButton (display, AnyButton, AnyModifier, wndH, False,
                     ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask,
                     GrabModeAsync, GrabModeAsync, None, None);

        // Set the window context to identify the window handle object
        if (XSaveContext (display, (XID) wndH, improbableNumber, (XPointer) this))
        {
            // Failed
            jassertfalse
            Logger::outputDebugString ("Failed to create context information for window.\n");
            XDestroyWindow (display, wndH);
            wndH = 0;
        }

        // Set window manager hints
        XWMHints* wmHints = XAllocWMHints();
        wmHints->flags = InputHint | StateHint;
        wmHints->input = True;      // Locally active input model
        wmHints->initial_state = NormalState;
        XSetWMHints (display, wndH, wmHints);
        XFree (wmHints);

        if ((styleFlags & windowIsSemiTransparent) != 0)
        {
            //xxx
        }

        if ((styleFlags & windowAppearsOnTaskbar) != 0)
        {
            //xxx
        }

        //XSetTransientForHint (display, wndH, RootWindow (display, DefaultScreen (display)));

        if ((styleFlags & windowHasTitleBar) == 0)
            removeWindowDecorations (wndH);
        else
            addWindowButtons (wndH);

        // Set window manager protocols
        XChangeProperty (display, wndH, wm_Protocols, XA_ATOM, 32, PropModeReplace,
                         (unsigned char*) wm_ProtocolList, 2);

        // Set drag and drop flags
        XChangeProperty (display, wndH, XA_XdndTypeList, XA_ATOM, 32, PropModeReplace,
                         (const unsigned char*) allowedMimeTypeAtoms, numElementsInArray (allowedMimeTypeAtoms));

        XChangeProperty (display, wndH, XA_XdndActionList, XA_ATOM, 32, PropModeReplace,
                         (const unsigned char*) allowedActions, numElementsInArray (allowedActions));

        XChangeProperty (display, wndH, XA_XdndActionDescription, XA_STRING, 8, PropModeReplace,
                         (const unsigned char*) "", 0);

        unsigned long dndVersion = ourDndVersion;
        XChangeProperty (display, wndH, XA_XdndAware, XA_ATOM, 32, PropModeReplace,
                         (const unsigned char*) &dndVersion, 1);

        // Set window name
        setWindowTitle (wndH, getComponent()->getName());

        // Initialise the pointer and keyboard mapping
        // This is not the same as the logical pointer mapping the X server uses:
        // we don't mess with this.
        static bool mappingInitialised = false;

        if (! mappingInitialised)
        {
            mappingInitialised = true;

            const int numButtons = XGetPointerMapping (display, 0, 0);

            if (numButtons == 2)
            {
                pointerMap[0] = LeftButton;
                pointerMap[1] = RightButton;
                pointerMap[2] = pointerMap[3] = pointerMap[4] = NoButton;
            }
            else if (numButtons >= 3)
            {
                pointerMap[0] = LeftButton;
                pointerMap[1] = MiddleButton;
                pointerMap[2] = RightButton;

                if (numButtons >= 5)
                {
                    pointerMap[3] = WheelUp;
                    pointerMap[4] = WheelDown;
                }
            }

            getModifierMapping();
        }

        windowH = wndH;
    }

    void destroyWindow()
    {
        XPointer handlePointer;
        if (! XFindContext (display, (XID) windowH, improbableNumber, &handlePointer))
            XDeleteContext (display, (XID) windowH, improbableNumber);

        XDestroyWindow (display, windowH);

        // Wait for it to complete and then remove any events for this
        // window from the event queue.
        XSync (display, false);

        XEvent event;
        while (XCheckWindowEvent (display, windowH, eventMask, &event) == True)
        {}
    }

    static int64 getEventTime (::Time t) throw()
    {
        static int64 eventTimeOffset = 0x12345678;
        const int64 thisMessageTime = t;

        if (eventTimeOffset == 0x12345678)
            eventTimeOffset = Time::currentTimeMillis() - thisMessageTime;

        return eventTimeOffset + thisMessageTime;
    }

    static void setWindowTitle (Window xwin, const char* const title) throw()
    {
        XTextProperty nameProperty;
        char* strings[] = { (char*) title };

        if (XStringListToTextProperty (strings, 1, &nameProperty))
        {
            XSetWMName (display, xwin, &nameProperty);
            XSetWMIconName (display, xwin, &nameProperty);

            XFree (nameProperty.value);
        }
    }

    void updateBorderSize()
    {
        if ((styleFlags & windowHasTitleBar) == 0)
        {
            windowBorder = BorderSize (0);
        }
        else if (windowBorder.getTopAndBottom() == 0 && windowBorder.getLeftAndRight() == 0)
        {
            Atom hints = XInternAtom (display, "_NET_FRAME_EXTENTS", True);

            if (hints != None)
            {
                unsigned char* data = 0;
                unsigned long nitems, bytesLeft;
                Atom actualType;
                int actualFormat;

                if (XGetWindowProperty (display, windowH, hints, 0, 4, False,
                                        XA_CARDINAL, &actualType, &actualFormat, &nitems, &bytesLeft,
                                        &data) == Success)
                {
                    const unsigned long* const sizes = (const unsigned long*) data;

                    if (actualFormat == 32)
                        windowBorder = BorderSize ((int) sizes[2], (int) sizes[0],
                                                   (int) sizes[3], (int) sizes[1]);

                    XFree (data);
                }
            }
        }
    }

    void updateBounds()
    {
        jassert (windowH != 0);
        if (windowH != 0)
        {
            Window root, child;
            unsigned int bw, depth;

            if (! XGetGeometry (display, (::Drawable) windowH, &root,
                                &wx, &wy, (unsigned int*) &ww, (unsigned int*) &wh,
                                &bw, &depth))
            {
                wx = wy = ww = wh = 0;
            }
            else if (! XTranslateCoordinates (display, windowH, root, 0, 0, &wx, &wy, &child))
            {
                wx = wy = 0;
            }
        }
    }

    //==============================================================================
    void resetDragAndDrop()
    {
        dragAndDropFiles.clear();
        lastDropX = lastDropY = -1;
        dragAndDropCurrentMimeType = 0;
        dragAndDropSourceWindow = 0;
        srcMimeTypeAtomList.clear();
    }

    void sendDragAndDropMessage (XClientMessageEvent& msg)
    {
        msg.type = ClientMessage;
        msg.display = display;
        msg.window = dragAndDropSourceWindow;
        msg.format = 32;
        msg.data.l[0] = windowH;

        XSendEvent (display, dragAndDropSourceWindow, False, 0, (XEvent*) &msg);
    }

    void sendDragAndDropStatus (const bool acceptDrop, Atom dropAction)
    {
        XClientMessageEvent msg;
        zerostruct (msg);
        msg.message_type = XA_XdndStatus;
        msg.data.l[1] = (acceptDrop ? 1 : 0) | 2; // 2 indicates that we want to receive position messages
        msg.data.l[4] = dropAction;

        sendDragAndDropMessage (msg);
    }

    void sendDragAndDropLeave()
    {
        XClientMessageEvent msg;
        zerostruct (msg);
        msg.message_type = XA_XdndLeave;
        sendDragAndDropMessage (msg);
    }

    void sendDragAndDropFinish()
    {
        XClientMessageEvent msg;
        zerostruct (msg);
        msg.message_type = XA_XdndFinished;
        sendDragAndDropMessage (msg);
    }

    void handleDragAndDropStatus (const XClientMessageEvent* const clientMsg)
    {
        if ((clientMsg->data.l[1] & 1) == 0)
        {
            sendDragAndDropLeave();

            if (dragAndDropFiles.size() > 0)
                handleFileDragExit (dragAndDropFiles);

            dragAndDropFiles.clear();
        }
    }

    void handleDragAndDropPosition (const XClientMessageEvent* const clientMsg)
    {
        if (dragAndDropSourceWindow == 0)
            return;

        dragAndDropSourceWindow = clientMsg->data.l[0];

        const int dropX = ((int) clientMsg->data.l[2] >> 16) - getScreenX();
        const int dropY = ((int) clientMsg->data.l[2] & 0xffff) - getScreenY();

        if (lastDropX != dropX || lastDropY != dropY)
        {
            lastDropX = dropX;
            lastDropY = dropY;

            dragAndDropTimestamp = clientMsg->data.l[3];

            Atom targetAction = XA_XdndActionCopy;

            for (int i = numElementsInArray (allowedActions); --i >= 0;)
            {
                if ((Atom) clientMsg->data.l[4] == allowedActions[i])
                {
                    targetAction = allowedActions[i];
                    break;
                }
            }

            sendDragAndDropStatus (true, targetAction);

            if (dragAndDropFiles.size() == 0)
                updateDraggedFileList (clientMsg);

            if (dragAndDropFiles.size() > 0)
                handleFileDragMove (dragAndDropFiles, dropX, dropY);
        }
    }

    void handleDragAndDropDrop (const XClientMessageEvent* const clientMsg)
    {
        if (dragAndDropFiles.size() == 0)
            updateDraggedFileList (clientMsg);

        const StringArray files (dragAndDropFiles);
        const int lastX = lastDropX, lastY = lastDropY;

        sendDragAndDropFinish();
        resetDragAndDrop();

        if (files.size() > 0)
            handleFileDragDrop (files, lastX, lastY);
    }

    void handleDragAndDropEnter (const XClientMessageEvent* const clientMsg)
    {
        dragAndDropFiles.clear();
        srcMimeTypeAtomList.clear();

        dragAndDropCurrentMimeType = 0;
        const int dndCurrentVersion = (int) (clientMsg->data.l[1] & 0xff000000) >> 24;

        if (dndCurrentVersion < 3 || dndCurrentVersion > ourDndVersion)
        {
            dragAndDropSourceWindow = 0;
            return;
        }

        dragAndDropSourceWindow = clientMsg->data.l[0];

        if ((clientMsg->data.l[1] & 1) != 0)
        {
            Atom actual;
            int format;
            unsigned long count = 0, remaining = 0;
            unsigned char* data = 0;

            XGetWindowProperty (display, dragAndDropSourceWindow, XA_XdndTypeList,
                                0, 0x8000000L, False, XA_ATOM, &actual, &format,
                                &count, &remaining, &data);

            if (data != 0)
            {
                if (actual == XA_ATOM && format == 32 && count != 0)
                {
                    const unsigned long* const types = (const unsigned long*) data;

                    for (unsigned int i = 0; i < count; ++i)
                        if (types[i] != None)
                            srcMimeTypeAtomList.add (types[i]);
                }

                XFree (data);
            }
        }

        if (srcMimeTypeAtomList.size() == 0)
        {
            for (int i = 2; i < 5; ++i)
                if (clientMsg->data.l[i] != None)
                    srcMimeTypeAtomList.add (clientMsg->data.l[i]);

            if (srcMimeTypeAtomList.size() == 0)
            {
                dragAndDropSourceWindow = 0;
                return;
            }
        }

        for (int i = 0; i < srcMimeTypeAtomList.size() && dragAndDropCurrentMimeType == 0; ++i)
            for (int j = 0; j < numElementsInArray (allowedMimeTypeAtoms); ++j)
                if (srcMimeTypeAtomList[i] == allowedMimeTypeAtoms[j])
                    dragAndDropCurrentMimeType = allowedMimeTypeAtoms[j];

        handleDragAndDropPosition (clientMsg);
    }

    void handleDragAndDropSelection (const XEvent* const evt)
    {
        dragAndDropFiles.clear();

        if (evt->xselection.property != 0)
        {
            StringArray lines;

            {
                MemoryBlock dropData;

                for (;;)
                {
                    Atom actual;
                    uint8* data = 0;
                    unsigned long count = 0, remaining = 0;
                    int format = 0;

                    if (XGetWindowProperty (display, evt->xany.window, evt->xselection.property,
                                            dropData.getSize() / 4, 65536, 1, AnyPropertyType, &actual,
                                            &format, &count, &remaining, &data) == Success)
                    {
                        dropData.append (data, count * format / 8);
                        XFree (data);

                        if (remaining == 0)
                            break;
                    }
                    else
                    {
                        XFree (data);
                        break;
                    }
                }

                lines.addLines (dropData.toString());
            }

            for (int i = 0; i < lines.size(); ++i)
                dragAndDropFiles.add (URL::removeEscapeChars (lines[i].fromFirstOccurrenceOf (T("file://"), false, true)));

            dragAndDropFiles.trim();
            dragAndDropFiles.removeEmptyStrings();
        }
    }

    void updateDraggedFileList (const XClientMessageEvent* const clientMsg)
    {
        dragAndDropFiles.clear();

        if (dragAndDropSourceWindow != None
             && dragAndDropCurrentMimeType != 0)
        {
            dragAndDropTimestamp = clientMsg->data.l[2];

            XConvertSelection (display,
                               XA_XdndSelection,
                               dragAndDropCurrentMimeType,
                               XA_JXSelectionWindowProperty,
                               windowH,
                               dragAndDropTimestamp);
        }
    }

    StringArray dragAndDropFiles;
    int dragAndDropTimestamp, lastDropX, lastDropY;

    Atom XA_OtherMime, dragAndDropCurrentMimeType;
    Window dragAndDropSourceWindow;

    unsigned int allowedActions [5];
    unsigned int allowedMimeTypeAtoms [3];
    Array <Atom> srcMimeTypeAtomList;
};

//==============================================================================
void juce_setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool allowMenusAndBars)
{
    if (enableOrDisable)
        kioskModeComponent->setBounds (Desktop::getInstance().getMainMonitorArea (false));
}

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void* /*nativeWindowToAttachTo*/)
{
    return new LinuxComponentPeer (this, styleFlags);
}


//==============================================================================
// (this callback is hooked up in the messaging code)
void juce_windowMessageReceive (XEvent* event)
{
    if (event->xany.window != None)
    {
        LinuxComponentPeer* const peer = LinuxComponentPeer::getPeerFor (event->xany.window);

        if (ComponentPeer::isValidPeer (peer))
            peer->handleWindowMessage (event);
    }
    else
    {
        switch (event->xany.type)
        {
            case KeymapNotify:
            {
                const XKeymapEvent* const keymapEvent = (const XKeymapEvent*) &event->xkeymap;
                memcpy (keyStates, keymapEvent->key_vector, 32);
                break;
            }

            default:
                break;
        }
    }
}

//==============================================================================
void juce_updateMultiMonitorInfo (Array <Rectangle>& monitorCoords, const bool /*clipToWorkArea*/)
{
    if (display == 0)
        return;

#if JUCE_USE_XINERAMA
    int major_opcode, first_event, first_error;

    if (XQueryExtension (display, "XINERAMA", &major_opcode, &first_event, &first_error))
    {
        typedef Bool (*tXineramaIsActive) (Display*);
        typedef XineramaScreenInfo* (*tXineramaQueryScreens) (Display*, int*);

        static tXineramaIsActive xXineramaIsActive = 0;
        static tXineramaQueryScreens xXineramaQueryScreens = 0;

        if (xXineramaIsActive == 0 || xXineramaQueryScreens == 0)
        {
            void* h = dlopen ("libXinerama.so", RTLD_GLOBAL | RTLD_NOW);

            if (h != 0)
            {
                xXineramaIsActive = (tXineramaIsActive) dlsym (h, "XineramaIsActive");
                xXineramaQueryScreens = (tXineramaQueryScreens) dlsym (h, "XineramaQueryScreens");
            }
        }

        if (xXineramaIsActive != 0
            && xXineramaQueryScreens != 0
            && xXineramaIsActive (display))
        {
            int numMonitors = 0;
            XineramaScreenInfo* const screens = xXineramaQueryScreens (display, &numMonitors);

            if (screens != 0)
            {
                for (int i = numMonitors; --i >= 0;)
                {
                    int index = screens[i].screen_number;

                    if (index >= 0)
                    {
                        while (monitorCoords.size() < index)
                            monitorCoords.add (Rectangle (0, 0, 0, 0));

                        monitorCoords.set (index, Rectangle (screens[i].x_org,
                                                             screens[i].y_org,
                                                             screens[i].width,
                                                             screens[i].height));
                    }
                }

                XFree (screens);
            }
        }
    }

    if (monitorCoords.size() == 0)
#endif
    {
        Atom hints = XInternAtom (display, "_NET_WORKAREA", True);

        if (hints != None)
        {
            const int numMonitors = ScreenCount (display);

            for (int i = 0; i < numMonitors; ++i)
            {
                Window root = RootWindow (display, i);

                unsigned long nitems, bytesLeft;
                Atom actualType;
                int actualFormat;
                unsigned char* data = 0;

                if (XGetWindowProperty (display, root, hints, 0, 4, False,
                                        XA_CARDINAL, &actualType, &actualFormat, &nitems, &bytesLeft,
                                        &data) == Success)
                {
                    const long* const position = (const long*) data;

                    if (actualType == XA_CARDINAL && actualFormat == 32 && nitems == 4)
                        monitorCoords.add (Rectangle (position[0], position[1],
                                                      position[2], position[3]));

                    XFree (data);
                }
            }
        }

        if (monitorCoords.size() == 0)
        {
            monitorCoords.add (Rectangle (0, 0,
                                          DisplayWidth (display, DefaultScreen (display)),
                                          DisplayHeight (display, DefaultScreen (display))));
        }
    }
}

//==============================================================================
bool Desktop::canUseSemiTransparentWindows() throw()
{
    return false;
}

void Desktop::getMousePosition (int& x, int& y) throw()
{
    int mouseMods;
    getMousePos (x, y, mouseMods);
}

void Desktop::setMousePosition (int x, int y) throw()
{
    Window root = RootWindow (display, DefaultScreen (display));
    XWarpPointer (display, None, root, 0, 0, 0, 0, x, y);
}


//==============================================================================
static bool screenSaverAllowed = true;

void Desktop::setScreenSaverEnabled (const bool isEnabled) throw()
{
    if (screenSaverAllowed != isEnabled)
    {
        screenSaverAllowed = isEnabled;

        typedef void (*tXScreenSaverSuspend) (Display*, Bool);
        static tXScreenSaverSuspend xScreenSaverSuspend = 0;

        if (xScreenSaverSuspend == 0)
        {
            void* h = dlopen ("libXss.so", RTLD_GLOBAL | RTLD_NOW);

            if (h != 0)
                xScreenSaverSuspend = (tXScreenSaverSuspend) dlsym (h, "XScreenSaverSuspend");
        }

        if (xScreenSaverSuspend != 0)
            xScreenSaverSuspend (display, ! isEnabled);
    }
}

bool Desktop::isScreenSaverEnabled() throw()
{
    return screenSaverAllowed;
}

//==============================================================================
void* juce_createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY) throw()
{
    Window root = RootWindow (display, DefaultScreen (display));
    const unsigned int imageW = image.getWidth();
    const unsigned int imageH = image.getHeight();
    unsigned int cursorW, cursorH;

    if (! XQueryBestCursor (display, root, imageW, imageH, &cursorW, &cursorH))
        return 0;

    Image im (Image::ARGB, cursorW, cursorH, true);
    Graphics g (im);

    if (imageW > cursorW || imageH > cursorH)
    {
        hotspotX = (hotspotX * cursorW) / imageW;
        hotspotY = (hotspotY * cursorH) / imageH;

        g.drawImageWithin (&image, 0, 0, imageW, imageH,
                           RectanglePlacement::xLeft | RectanglePlacement::yTop | RectanglePlacement::onlyReduceInSize,
                           false);
    }
    else
    {
        g.drawImageAt (&image, 0, 0);
    }

    const int stride = (cursorW + 7) >> 3;
    HeapBlock <uint8> maskPlane, sourcePlane;
    maskPlane.calloc (stride * cursorH);
    sourcePlane.calloc (stride * cursorH);

    const bool msbfirst = (BitmapBitOrder (display) == MSBFirst);

    for (int y = cursorH; --y >= 0;)
    {
        for (int x = cursorW; --x >= 0;)
        {
            const uint8 mask = (uint8) (1 << (msbfirst ? (7 - (x & 7)) : (x & 7)));
            const int offset = y * stride + (x >> 3);

            const Colour c (im.getPixelAt (x, y));

            if (c.getAlpha() >= 128)
                maskPlane[offset] |= mask;

            if (c.getBrightness() >= 0.5f)
                sourcePlane[offset] |= mask;
        }
    }

    Pixmap sourcePixmap = XCreatePixmapFromBitmapData (display, root, (char*) sourcePlane, cursorW, cursorH, 0xffff, 0, 1);
    Pixmap maskPixmap = XCreatePixmapFromBitmapData (display, root, (char*) maskPlane, cursorW, cursorH, 0xffff, 0, 1);

    XColor white, black;
    black.red = black.green = black.blue = 0;
    white.red = white.green = white.blue = 0xffff;

    void* result = (void*) XCreatePixmapCursor (display, sourcePixmap, maskPixmap, &white, &black, hotspotX, hotspotY);

    XFreePixmap (display, sourcePixmap);
    XFreePixmap (display, maskPixmap);

    return result;
}

void juce_deleteMouseCursor (void* const cursorHandle, const bool) throw()
{
    if (cursorHandle != None)
        XFreeCursor (display, (Cursor) cursorHandle);
}

void* juce_createStandardMouseCursor (MouseCursor::StandardCursorType type) throw()
{
    unsigned int shape;

    switch (type)
    {
        case MouseCursor::NoCursor:
        {
            const Image im (Image::ARGB, 16, 16, true);
            return juce_createMouseCursorFromImage (im, 0, 0);
        }

        case MouseCursor::NormalCursor:
            return (void*) None; // Use parent cursor

        case MouseCursor::DraggingHandCursor:
        {
            static unsigned char dragHandData[] = {71,73,70,56,57,97,16,0,16,0,145,2,0,0,0,0,255,255,255,0,
              0,0,0,0,0,33,249,4,1,0,0,2,0,44,0,0,0,0,16,0,
              16,0,0,2,52,148,47,0,200,185,16,130,90,12,74,139,107,84,123,39,
              132,117,151,116,132,146,248,60,209,138,98,22,203,114,34,236,37,52,77,217,
              247,154,191,119,110,240,193,128,193,95,163,56,60,234,98,135,2,0,59 };
            const int dragHandDataSize = 99;

            const ScopedPointer <Image> im (ImageFileFormat::loadFrom ((const char*) dragHandData, dragHandDataSize));
            return juce_createMouseCursorFromImage (*im, 8, 7);
        }

        case MouseCursor::CopyingCursor:
        {
            static unsigned char copyCursorData[] = {71,73,70,56,57,97,21,0,21,0,145,0,0,0,0,0,255,255,255,0,
              128,128,255,255,255,33,249,4,1,0,0,3,0,44,0,0,0,0,21,0,
              21,0,0,2,72,4,134,169,171,16,199,98,11,79,90,71,161,93,56,111,
              78,133,218,215,137,31,82,154,100,200,86,91,202,142,12,108,212,87,235,174,
              15,54,214,126,237,226,37,96,59,141,16,37,18,201,142,157,230,204,51,112,
              252,114,147,74,83,5,50,68,147,208,217,16,71,149,252,124,5,0,59,0,0 };
            const int copyCursorSize = 119;

            const ScopedPointer <Image> im (ImageFileFormat::loadFrom ((const char*) copyCursorData, copyCursorSize));
            return juce_createMouseCursorFromImage (*im, 1, 3);
        }

        case MouseCursor::WaitCursor:
            shape = XC_watch;
            break;

        case MouseCursor::IBeamCursor:
            shape = XC_xterm;
            break;

        case MouseCursor::PointingHandCursor:
            shape = XC_hand2;
            break;

        case MouseCursor::LeftRightResizeCursor:
            shape = XC_sb_h_double_arrow;
            break;

        case MouseCursor::UpDownResizeCursor:
            shape = XC_sb_v_double_arrow;
            break;

        case MouseCursor::UpDownLeftRightResizeCursor:
            shape = XC_fleur;
            break;

        case MouseCursor::TopEdgeResizeCursor:
            shape = XC_top_side;
            break;

        case MouseCursor::BottomEdgeResizeCursor:
            shape = XC_bottom_side;
            break;

        case MouseCursor::LeftEdgeResizeCursor:
            shape = XC_left_side;
            break;

        case MouseCursor::RightEdgeResizeCursor:
            shape = XC_right_side;
            break;

        case MouseCursor::TopLeftCornerResizeCursor:
            shape = XC_top_left_corner;
            break;

        case MouseCursor::TopRightCornerResizeCursor:
            shape = XC_top_right_corner;
            break;

        case MouseCursor::BottomLeftCornerResizeCursor:
            shape = XC_bottom_left_corner;
            break;

        case MouseCursor::BottomRightCornerResizeCursor:
            shape = XC_bottom_right_corner;
            break;

        case MouseCursor::CrosshairCursor:
            shape = XC_crosshair;
            break;

        default:
            return (void*) None; // Use parent cursor
    }

    return (void*) XCreateFontCursor (display, shape);
}

void MouseCursor::showInWindow (ComponentPeer* peer) const throw()
{
    LinuxComponentPeer* const lp = dynamic_cast <LinuxComponentPeer*> (peer);

    if (lp != 0)
        lp->showMouseCursor ((Cursor) getHandle());
}

void MouseCursor::showInAllWindows() const throw()
{
    for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        showInWindow (ComponentPeer::getPeer (i));
}

//==============================================================================
Image* juce_createIconForFile (const File& file)
{
    return 0;
}

Image* Image::createNativeImage (const PixelFormat format, const int imageWidth, const int imageHeight, const bool clearImage)
{
    return new Image (format, imageWidth, imageHeight, clearImage);
}


//==============================================================================
#if JUCE_OPENGL


//==============================================================================
class WindowedGLContext     : public OpenGLContext
{
public:
    WindowedGLContext (Component* const component,
                       const OpenGLPixelFormat& pixelFormat_,
                       GLXContext sharedContext)
        : renderContext (0),
          embeddedWindow (0),
          pixelFormat (pixelFormat_)
    {
        jassert (component != 0);
        LinuxComponentPeer* const peer = dynamic_cast <LinuxComponentPeer*> (component->getTopLevelComponent()->getPeer());
        if (peer == 0)
            return;

        XSync (display, False);

        GLint attribs [64];
        int n = 0;
        attribs[n++] = GLX_RGBA;
        attribs[n++] = GLX_DOUBLEBUFFER;
        attribs[n++] = GLX_RED_SIZE;
        attribs[n++] = pixelFormat.redBits;
        attribs[n++] = GLX_GREEN_SIZE;
        attribs[n++] = pixelFormat.greenBits;
        attribs[n++] = GLX_BLUE_SIZE;
        attribs[n++] = pixelFormat.blueBits;
        attribs[n++] = GLX_ALPHA_SIZE;
        attribs[n++] = pixelFormat.alphaBits;
        attribs[n++] = GLX_DEPTH_SIZE;
        attribs[n++] = pixelFormat.depthBufferBits;
        attribs[n++] = GLX_STENCIL_SIZE;
        attribs[n++] = pixelFormat.stencilBufferBits;
        attribs[n++] = GLX_ACCUM_RED_SIZE;
        attribs[n++] = pixelFormat.accumulationBufferRedBits;
        attribs[n++] = GLX_ACCUM_GREEN_SIZE;
        attribs[n++] = pixelFormat.accumulationBufferGreenBits;
        attribs[n++] = GLX_ACCUM_BLUE_SIZE;
        attribs[n++] = pixelFormat.accumulationBufferBlueBits;
        attribs[n++] = GLX_ACCUM_ALPHA_SIZE;
        attribs[n++] = pixelFormat.accumulationBufferAlphaBits;

        // xxx not sure how to do fullSceneAntiAliasingNumSamples on linux..

        attribs[n++] = None;

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

        XSaveContext (display, (XID) embeddedWindow, improbableNumber, (XPointer) peer);

        XMapWindow (display, embeddedWindow);
        XFreeColormap (display, colourMap);

        XFree (bestVisual);
        XSync (display, False);
    }

    ~WindowedGLContext()
    {
        makeInactive();

        glXDestroyContext (display, renderContext);

        XUnmapWindow (display, embeddedWindow);
        XDestroyWindow (display, embeddedWindow);
    }

    bool makeActive() const throw()
    {
        jassert (renderContext != 0);

        return glXMakeCurrent (display, embeddedWindow, renderContext)
                && XSync (display, False);
    }

    bool makeInactive() const throw()
    {
        return (! isActive()) || glXMakeCurrent (display, None, 0);
    }

    bool isActive() const throw()
    {
        return glXGetCurrentContext() == renderContext;
    }

    const OpenGLPixelFormat getPixelFormat() const
    {
        return pixelFormat;
    }

    void* getRawContext() const throw()
    {
        return renderContext;
    }

    void updateWindowPosition (int x, int y, int w, int h, int)
    {
        XMoveResizeWindow (display, embeddedWindow,
                           x, y, jmax (1, w), jmax (1, h));
    }

    void swapBuffers()
    {
        glXSwapBuffers (display, embeddedWindow);
    }

    bool setSwapInterval (const int numFramesPerSwap)
    {
        // xxx needs doing..
        return false;
    }

    int getSwapInterval() const
    {
        // xxx needs doing..
        return 0;
    }

    void repaint()
    {
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    GLXContext renderContext;

private:
    Window embeddedWindow;
    OpenGLPixelFormat pixelFormat;

    //==============================================================================
    WindowedGLContext (const WindowedGLContext&);
    const WindowedGLContext& operator= (const WindowedGLContext&);
};

//==============================================================================
OpenGLContext* OpenGLContext::createContextForWindow (Component* const component,
                                                      const OpenGLPixelFormat& pixelFormat,
                                                      const OpenGLContext* const contextToShareWith)
{
    WindowedGLContext* c = new WindowedGLContext (component, pixelFormat,
                                                  contextToShareWith != 0 ? (GLXContext) contextToShareWith->getRawContext() : 0);

    if (c->renderContext == 0)
        deleteAndZero (c);

    return c;
}

void juce_glViewport (const int w, const int h)
{
    glViewport (0, 0, w, h);
}

void OpenGLPixelFormat::getAvailablePixelFormats (Component* component,
                                                  OwnedArray <OpenGLPixelFormat>& results)
{
    results.add (new OpenGLPixelFormat()); // xxx
}

#endif


//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMoveFiles)
{
    jassertfalse    // not implemented!
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    jassertfalse    // not implemented!
    return false;
}

//==============================================================================
void SystemTrayIconComponent::setIconImage (const Image& newImage)
{
    if (! isOnDesktop ())
        addToDesktop (0);

    LinuxComponentPeer* const wp = dynamic_cast <LinuxComponentPeer*> (getPeer());

    if (wp != 0)
    {
        wp->setTaskBarIcon (newImage);

        setVisible (true);
        toFront (false);
        repaint();
    }
}

void SystemTrayIconComponent::paint (Graphics& g)
{
    LinuxComponentPeer* const wp = dynamic_cast <LinuxComponentPeer*> (getPeer());

    if (wp != 0)
    {
        const Image* const image = wp->getTaskbarIcon();

        if (image != 0)
        {
            g.drawImageWithin (image, 0, 0, getWidth(), getHeight(),
                               RectanglePlacement::xLeft | RectanglePlacement::yTop | RectanglePlacement::onlyReduceInSize,
                               false);
        }
    }
}

void SystemTrayIconComponent::setIconTooltip (const String& tooltip)
{
    // xxx not yet implemented!
}


//==============================================================================
void PlatformUtilities::beep()
{
    fprintf (stdout, "\a");
    fflush (stdout);
}


//==============================================================================
bool AlertWindow::showNativeDialogBox (const String& title,
                                       const String& bodyText,
                                       bool isOkCancel)
{
    // xxx this is supposed to pop up an alert!
    Logger::outputDebugString (title + ": " + bodyText);

    // use a non-native one for the time being..
    if (isOkCancel)
        return AlertWindow::showOkCancelBox (AlertWindow::NoIcon, title, bodyText);
    else
        AlertWindow::showMessageBox (AlertWindow::NoIcon, title, bodyText);

    return true;
}

//==============================================================================
const int KeyPress::spaceKey            = XK_space & 0xff;
const int KeyPress::returnKey           = XK_Return & 0xff;
const int KeyPress::escapeKey           = XK_Escape & 0xff;
const int KeyPress::backspaceKey        = XK_BackSpace & 0xff;
const int KeyPress::leftKey             = (XK_Left & 0xff) | extendedKeyModifier;
const int KeyPress::rightKey            = (XK_Right & 0xff) | extendedKeyModifier;
const int KeyPress::upKey               = (XK_Up & 0xff) | extendedKeyModifier;
const int KeyPress::downKey             = (XK_Down & 0xff) | extendedKeyModifier;
const int KeyPress::pageUpKey           = (XK_Page_Up & 0xff) | extendedKeyModifier;
const int KeyPress::pageDownKey         = (XK_Page_Down & 0xff) | extendedKeyModifier;
const int KeyPress::endKey              = (XK_End & 0xff) | extendedKeyModifier;
const int KeyPress::homeKey             = (XK_Home & 0xff) | extendedKeyModifier;
const int KeyPress::insertKey           = (XK_Insert & 0xff) | extendedKeyModifier;
const int KeyPress::deleteKey           = (XK_Delete & 0xff) | extendedKeyModifier;
const int KeyPress::tabKey              = XK_Tab & 0xff;
const int KeyPress::F1Key               = (XK_F1 & 0xff) | extendedKeyModifier;
const int KeyPress::F2Key               = (XK_F2 & 0xff) | extendedKeyModifier;
const int KeyPress::F3Key               = (XK_F3 & 0xff) | extendedKeyModifier;
const int KeyPress::F4Key               = (XK_F4 & 0xff) | extendedKeyModifier;
const int KeyPress::F5Key               = (XK_F5 & 0xff) | extendedKeyModifier;
const int KeyPress::F6Key               = (XK_F6 & 0xff) | extendedKeyModifier;
const int KeyPress::F7Key               = (XK_F7 & 0xff) | extendedKeyModifier;
const int KeyPress::F8Key               = (XK_F8 & 0xff) | extendedKeyModifier;
const int KeyPress::F9Key               = (XK_F9 & 0xff) | extendedKeyModifier;
const int KeyPress::F10Key              = (XK_F10 & 0xff) | extendedKeyModifier;
const int KeyPress::F11Key              = (XK_F11 & 0xff) | extendedKeyModifier;
const int KeyPress::F12Key              = (XK_F12 & 0xff) | extendedKeyModifier;
const int KeyPress::F13Key              = (XK_F13 & 0xff) | extendedKeyModifier;
const int KeyPress::F14Key              = (XK_F14 & 0xff) | extendedKeyModifier;
const int KeyPress::F15Key              = (XK_F15 & 0xff) | extendedKeyModifier;
const int KeyPress::F16Key              = (XK_F16 & 0xff) | extendedKeyModifier;
const int KeyPress::numberPad0          = (XK_KP_0 & 0xff) | extendedKeyModifier;
const int KeyPress::numberPad1          = (XK_KP_1 & 0xff) | extendedKeyModifier;
const int KeyPress::numberPad2          = (XK_KP_2 & 0xff) | extendedKeyModifier;
const int KeyPress::numberPad3          = (XK_KP_3 & 0xff) | extendedKeyModifier;
const int KeyPress::numberPad4          = (XK_KP_4 & 0xff) | extendedKeyModifier;
const int KeyPress::numberPad5          = (XK_KP_5 & 0xff) | extendedKeyModifier;
const int KeyPress::numberPad6          = (XK_KP_6 & 0xff) | extendedKeyModifier;
const int KeyPress::numberPad7          = (XK_KP_7 & 0xff)| extendedKeyModifier;
const int KeyPress::numberPad8          = (XK_KP_8 & 0xff)| extendedKeyModifier;
const int KeyPress::numberPad9          = (XK_KP_9 & 0xff)| extendedKeyModifier;
const int KeyPress::numberPadAdd            = (XK_KP_Add & 0xff)| extendedKeyModifier;
const int KeyPress::numberPadSubtract       = (XK_KP_Subtract & 0xff)| extendedKeyModifier;
const int KeyPress::numberPadMultiply       = (XK_KP_Multiply & 0xff)| extendedKeyModifier;
const int KeyPress::numberPadDivide         = (XK_KP_Divide & 0xff)| extendedKeyModifier;
const int KeyPress::numberPadSeparator      = (XK_KP_Separator & 0xff)| extendedKeyModifier;
const int KeyPress::numberPadDecimalPoint   = (XK_KP_Decimal & 0xff)| extendedKeyModifier;
const int KeyPress::numberPadEquals         = (XK_KP_Equal & 0xff)| extendedKeyModifier;
const int KeyPress::numberPadDelete         = (XK_KP_Delete & 0xff)| extendedKeyModifier;
const int KeyPress::playKey             = (0xffeeff00) | extendedKeyModifier;
const int KeyPress::stopKey             = (0xffeeff01) | extendedKeyModifier;
const int KeyPress::fastForwardKey      = (0xffeeff02) | extendedKeyModifier;
const int KeyPress::rewindKey           = (0xffeeff03) | extendedKeyModifier;


#endif
