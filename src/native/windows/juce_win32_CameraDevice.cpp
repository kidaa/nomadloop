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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_USE_CAMERA


//==============================================================================
class DShowCameraDeviceInteral  : public ChangeBroadcaster
{
public:
    DShowCameraDeviceInteral (CameraDevice* const owner_,
                              const ComSmartPtr <ICaptureGraphBuilder2>& captureGraphBuilder_,
                              const ComSmartPtr <IBaseFilter>& filter_,
                              int minWidth, int minHeight,
                              int maxWidth, int maxHeight)
      : owner (owner_),
        captureGraphBuilder (captureGraphBuilder_),
        filter (filter_),
        ok (false),
        imageNeedsFlipping (false),
        width (0),
        height (0),
        activeUsers (0),
        recordNextFrameTime (false),
        activeImage (0),
        loadingImage (0)
    {
        HRESULT hr = graphBuilder.CoCreateInstance (CLSID_FilterGraph, CLSCTX_INPROC);
        if (FAILED (hr))
            return;

        hr = captureGraphBuilder->SetFiltergraph (graphBuilder);
        if (FAILED (hr))
            return;

        hr = graphBuilder->QueryInterface (IID_IMediaControl, (void**) &mediaControl);
        if (FAILED (hr))
            return;

        {
            ComSmartPtr <IAMStreamConfig> streamConfig;

            hr = captureGraphBuilder->FindInterface (&PIN_CATEGORY_CAPTURE, 0, filter,
                                                     IID_IAMStreamConfig, (void**) &streamConfig);

            if (streamConfig != 0)
            {
                getVideoSizes (streamConfig);

                if (! selectVideoSize (streamConfig, minWidth, minHeight, maxWidth, maxHeight))
                    return;
            }
        }

        hr = graphBuilder->AddFilter (filter, _T("Video Capture"));
        if (FAILED (hr))
            return;

        hr = smartTee.CoCreateInstance (CLSID_SmartTee, CLSCTX_INPROC_SERVER);
        if (FAILED (hr))
            return;

        hr = graphBuilder->AddFilter (smartTee, _T("Smart Tee"));
        if (FAILED (hr))
            return;

        if (! connectFilters (filter, smartTee))
            return;

        ComSmartPtr <IBaseFilter> sampleGrabberBase;
        hr = sampleGrabberBase.CoCreateInstance (CLSID_SampleGrabber, CLSCTX_INPROC_SERVER);
        if (FAILED (hr))
            return;

        hr = sampleGrabberBase->QueryInterface (IID_ISampleGrabber, (void**) &sampleGrabber);
        if (FAILED (hr))
            return;

        AM_MEDIA_TYPE mt;
        zerostruct (mt);
        mt.majortype = MEDIATYPE_Video;
        mt.subtype = MEDIASUBTYPE_RGB24;
        mt.formattype = FORMAT_VideoInfo;
        sampleGrabber->SetMediaType (&mt);

        callback = new GrabberCallback (*this);
        sampleGrabber->SetCallback (callback, 1);

        hr = graphBuilder->AddFilter (sampleGrabberBase, _T("Sample Grabber"));
        if (FAILED (hr))
            return;

        ComSmartPtr <IPin> grabberInputPin;
        if (! (getPin (smartTee, PINDIR_OUTPUT, &smartTeeCaptureOutputPin, "capture")
                && getPin (smartTee, PINDIR_OUTPUT, &smartTeePreviewOutputPin, "preview")
                && getPin (sampleGrabberBase, PINDIR_INPUT, &grabberInputPin)))
            return;

        hr = graphBuilder->Connect (smartTeePreviewOutputPin, grabberInputPin);
        if (FAILED (hr))
            return;

        zerostruct (mt);
        hr = sampleGrabber->GetConnectedMediaType (&mt);
        VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*) (mt.pbFormat);
        width = pVih->bmiHeader.biWidth;
        height = pVih->bmiHeader.biHeight;

        ComSmartPtr <IBaseFilter> nullFilter;
        hr = nullFilter.CoCreateInstance (CLSID_NullRenderer, CLSCTX_INPROC_SERVER);
        hr = graphBuilder->AddFilter (nullFilter, _T("Null Renderer"));

        if (connectFilters (sampleGrabberBase, nullFilter)
              && addGraphToRot())
        {
            activeImage = new Image (Image::RGB, width, height, true);
            loadingImage = new Image (Image::RGB, width, height, true);

            ok = true;
        }
    }

    ~DShowCameraDeviceInteral()
    {
        if (mediaControl != 0)
            mediaControl->Stop();

        removeGraphFromRot();

        for (int i = viewerComps.size(); --i >= 0;)
            ((DShowCaptureViewerComp*) viewerComps.getUnchecked(i))->ownerDeleted();

        callback = 0;
        graphBuilder = 0;
        sampleGrabber = 0;
        mediaControl = 0;
        filter = 0;
        captureGraphBuilder = 0;
        smartTee = 0;
        smartTeePreviewOutputPin = 0;
        smartTeeCaptureOutputPin = 0;
        asfWriter = 0;

        delete activeImage;
        delete loadingImage;
    }

    void addUser()
    {
        if (ok && activeUsers++ == 0)
            mediaControl->Run();
    }

    void removeUser()
    {
        if (ok && --activeUsers == 0)
            mediaControl->Stop();
    }

    void handleFrame (double /*time*/, BYTE* buffer, long /*bufferSize*/)
    {
        if (recordNextFrameTime)
        {
            const double defaultCameraLatency = 0.1;

            firstRecordedTime = Time::getCurrentTime() - RelativeTime (defaultCameraLatency);
            recordNextFrameTime = false;

            ComSmartPtr <IPin> pin;
            if (getPin (filter, PINDIR_OUTPUT, &pin))
            {
                ComSmartPtr <IAMPushSource> pushSource;
                HRESULT hr = pin->QueryInterface (IID_IAMPushSource, (void**) &pushSource);

                if (pushSource != 0)
                {
                    REFERENCE_TIME latency = 0;
                    hr = pushSource->GetLatency (&latency);

                    firstRecordedTime = firstRecordedTime - RelativeTime ((double) latency);
                }
            }
        }

        {
            const int lineStride = width * 3;
            const ScopedLock sl (imageSwapLock);

            {
                const Image::BitmapData destData (*loadingImage, 0, 0, width, height, true);

                for (int i = 0; i < height; ++i)
                    memcpy (destData.getLinePointer ((height - 1) - i),
                            buffer + lineStride * i,
                            lineStride);
            }

            imageNeedsFlipping = true;
        }

        if (listeners.size() > 0)
            callListeners (*loadingImage);

        sendChangeMessage (this);
    }

    void drawCurrentImage (Graphics& g, int x, int y, int w, int h)
    {
        if (imageNeedsFlipping)
        {
            const ScopedLock sl (imageSwapLock);
            swapVariables (loadingImage, activeImage);
            imageNeedsFlipping = false;
        }

        RectanglePlacement rp (RectanglePlacement::centred);
        double dx = 0, dy = 0, dw = width, dh = height;
        rp.applyTo (dx, dy, dw, dh, x, y, w, h);
        const int rx = roundToInt (dx), ry = roundToInt (dy);
        const int rw = roundToInt (dw), rh = roundToInt (dh);

        g.saveState();
        g.excludeClipRegion (Rectangle<int> (rx, ry, rw, rh));
        g.fillAll (Colours::black);
        g.restoreState();

        g.drawImage (activeImage, rx, ry, rw, rh, 0, 0, width, height);
    }

    bool createFileCaptureFilter (const File& file)
    {
        removeFileCaptureFilter();
        file.deleteFile();
        mediaControl->Stop();
        firstRecordedTime = Time();
        recordNextFrameTime = true;

        HRESULT hr = asfWriter.CoCreateInstance (CLSID_WMAsfWriter, CLSCTX_INPROC_SERVER);

        if (SUCCEEDED (hr))
        {
            ComSmartPtr <IFileSinkFilter> fileSink;
            hr = asfWriter->QueryInterface (IID_IFileSinkFilter, (void**) &fileSink);

            if (SUCCEEDED (hr))
            {
                hr = fileSink->SetFileName (file.getFullPathName(), 0);

                if (SUCCEEDED (hr))
                {
                    hr = graphBuilder->AddFilter (asfWriter, _T("AsfWriter"));

                    if (SUCCEEDED (hr))
                    {
                        ComSmartPtr <IConfigAsfWriter> asfConfig;
                        hr = asfWriter->QueryInterface (IID_IConfigAsfWriter, (void**) &asfConfig);
                        asfConfig->SetIndexMode (true);
                        ComSmartPtr <IWMProfileManager> profileManager;
                        hr = WMCreateProfileManager (&profileManager);

                        // This gibberish is the DirectShow profile for a video-only wmv file.
                        String prof ("<profile version=\"589824\" storageformat=\"1\" name=\"Quality\" description=\"Quality type for output.\"><streamconfig "
                            "majortype=\"{73646976-0000-0010-8000-00AA00389B71}\" streamnumber=\"1\" streamname=\"Video Stream\" inputname=\"Video409\" bitrate=\"894960\" "
                            "bufferwindow=\"0\" reliabletransport=\"1\" decodercomplexity=\"AU\" rfc1766langid=\"en-us\"><videomediaprops maxkeyframespacing=\"50000000\" quality=\"90\"/>"
                            "<wmmediatype subtype=\"{33564D57-0000-0010-8000-00AA00389B71}\" bfixedsizesamples=\"0\" btemporalcompression=\"1\" lsamplesize=\"0\"> <videoinfoheader "
                            "dwbitrate=\"894960\" dwbiterrorrate=\"0\" avgtimeperframe=\"100000\"><rcsource left=\"0\" top=\"0\" right=\"$WIDTH\" bottom=\"$HEIGHT\"/> <rctarget "
                            "left=\"0\" top=\"0\" right=\"$WIDTH\" bottom=\"$HEIGHT\"/> <bitmapinfoheader biwidth=\"$WIDTH\" biheight=\"$HEIGHT\" biplanes=\"1\" bibitcount=\"24\" "
                            "bicompression=\"WMV3\" bisizeimage=\"0\" bixpelspermeter=\"0\" biypelspermeter=\"0\" biclrused=\"0\" biclrimportant=\"0\"/> "
                            "</videoinfoheader></wmmediatype></streamconfig></profile>");

                        prof = prof.replace ("$WIDTH", String (width))
                                   .replace ("$HEIGHT", String (height));

                        ComSmartPtr <IWMProfile> currentProfile;
                        hr = profileManager->LoadProfileByData ((const WCHAR*) prof, &currentProfile);
                        hr = asfConfig->ConfigureFilterUsingProfile (currentProfile);

                        if (SUCCEEDED (hr))
                        {
                            ComSmartPtr <IPin> asfWriterInputPin;

                            if (getPin (asfWriter, PINDIR_INPUT, &asfWriterInputPin, "Video Input 01"))
                            {
                                hr = graphBuilder->Connect (smartTeeCaptureOutputPin, asfWriterInputPin);

                                if (SUCCEEDED (hr)
                                     && ok && activeUsers > 0
                                     && SUCCEEDED (mediaControl->Run()))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }

        removeFileCaptureFilter();

        if (ok && activeUsers > 0)
            mediaControl->Run();

        return false;
    }

    void removeFileCaptureFilter()
    {
        mediaControl->Stop();

        if (asfWriter != 0)
        {
            graphBuilder->RemoveFilter (asfWriter);
            asfWriter = 0;
        }

        if (ok && activeUsers > 0)
            mediaControl->Run();
    }

    //==============================================================================
    void addListener (CameraImageListener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);

        if (listeners.size() == 0)
            addUser();

        listeners.addIfNotAlreadyThere (listenerToAdd);
    }

    void removeListener (CameraImageListener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.removeValue (listenerToRemove);

        if (listeners.size() == 0)
            removeUser();
    }

    void callListeners (Image& image)
    {
        const ScopedLock sl (listenerLock);

        for (int i = listeners.size(); --i >= 0;)
        {
            CameraImageListener* l = (CameraImageListener*) listeners[i];

            if (l != 0)
                l->imageReceived (image);
        }
    }


    //==============================================================================
    class DShowCaptureViewerComp   : public Component,
                                     public ChangeListener
    {
    public:
        DShowCaptureViewerComp (DShowCameraDeviceInteral* const owner_)
            : owner (owner_)
        {
            setOpaque (true);
            owner->addChangeListener (this);
            owner->addUser();
            owner->viewerComps.add (this);
            setSize (owner_->width, owner_->height);
        }

        ~DShowCaptureViewerComp()
        {
            if (owner != 0)
            {
                owner->viewerComps.removeValue (this);
                owner->removeUser();
                owner->removeChangeListener (this);
            }
        }

        void ownerDeleted()
        {
            owner = 0;
        }

        void paint (Graphics& g)
        {
            g.setColour (Colours::black);
            g.setImageResamplingQuality (Graphics::lowResamplingQuality);

            if (owner != 0)
                owner->drawCurrentImage (g, 0, 0, getWidth(), getHeight());
            else
                g.fillAll (Colours::black);
        }

        void changeListenerCallback (void*)
        {
            repaint();
        }

    private:
        DShowCameraDeviceInteral* owner;
    };

    //==============================================================================
    bool ok;
    int width, height;
    Time firstRecordedTime;

    VoidArray viewerComps;

private:
    CameraDevice* const owner;
    ComSmartPtr <ICaptureGraphBuilder2> captureGraphBuilder;
    ComSmartPtr <IBaseFilter> filter;
    ComSmartPtr <IBaseFilter> smartTee;
    ComSmartPtr <IGraphBuilder> graphBuilder;
    ComSmartPtr <ISampleGrabber> sampleGrabber;
    ComSmartPtr <IMediaControl> mediaControl;
    ComSmartPtr <IPin> smartTeePreviewOutputPin;
    ComSmartPtr <IPin> smartTeeCaptureOutputPin;
    ComSmartPtr <IBaseFilter> asfWriter;
    int activeUsers;
    Array <int> widths, heights;
    DWORD graphRegistrationID;

    CriticalSection imageSwapLock;
    bool imageNeedsFlipping;
    Image* loadingImage;
    Image* activeImage;

    bool recordNextFrameTime;

    void getVideoSizes (IAMStreamConfig* const streamConfig)
    {
        widths.clear();
        heights.clear();

        int count = 0, size = 0;
        streamConfig->GetNumberOfCapabilities (&count, &size);

        if (size == sizeof (VIDEO_STREAM_CONFIG_CAPS))
        {
            for (int i = 0; i < count; ++i)
            {
                VIDEO_STREAM_CONFIG_CAPS scc;
                AM_MEDIA_TYPE* config;

                HRESULT hr = streamConfig->GetStreamCaps (i, &config, (BYTE*) &scc);

                if (SUCCEEDED (hr))
                {
                    const int w = scc.InputSize.cx;
                    const int h = scc.InputSize.cy;

                    bool duplicate = false;

                    for (int j = widths.size(); --j >= 0;)
                    {
                        if (w == widths.getUnchecked (j) && h == heights.getUnchecked (j))
                        {
                            duplicate = true;
                            break;
                        }
                    }

                    if (! duplicate)
                    {
                        DBG ("Camera capture size: " + String (w) + ", " + String (h));
                        widths.add (w);
                        heights.add (h);
                    }

                    deleteMediaType (config);
                }
            }
        }
    }

    bool selectVideoSize (IAMStreamConfig* const streamConfig,
                          const int minWidth, const int minHeight,
                          const int maxWidth, const int maxHeight)
    {
        int count = 0, size = 0, bestArea = 0, bestIndex = -1;
        streamConfig->GetNumberOfCapabilities (&count, &size);

        if (size == sizeof (VIDEO_STREAM_CONFIG_CAPS))
        {
            AM_MEDIA_TYPE* config;
            VIDEO_STREAM_CONFIG_CAPS scc;

            for (int i = 0; i < count; ++i)
            {
                HRESULT hr = streamConfig->GetStreamCaps (i, &config, (BYTE*) &scc);

                if (SUCCEEDED (hr))
                {
                    if (scc.InputSize.cx >= minWidth
                         && scc.InputSize.cy >= minHeight
                         && scc.InputSize.cx <= maxWidth
                         && scc.InputSize.cy <= maxHeight)
                    {
                        int area = scc.InputSize.cx * scc.InputSize.cy;
                        if (area > bestArea)
                        {
                            bestIndex = i;
                            bestArea = area;
                        }
                    }

                    deleteMediaType (config);
                }
            }

            if (bestIndex >= 0)
            {
                HRESULT hr = streamConfig->GetStreamCaps (bestIndex, &config, (BYTE*) &scc);

                hr = streamConfig->SetFormat (config);
                deleteMediaType (config);
                return SUCCEEDED (hr);
            }
        }

        return false;
    }

    static bool getPin (IBaseFilter* filter, const PIN_DIRECTION wantedDirection, IPin** result, const char* pinName = 0)
    {
        ComSmartPtr <IEnumPins> enumerator;
        ComSmartPtr <IPin> pin;

        filter->EnumPins (&enumerator);

        while (enumerator->Next (1, &pin, 0) == S_OK)
        {
            PIN_DIRECTION dir;
            pin->QueryDirection (&dir);

            if (wantedDirection == dir)
            {
                PIN_INFO info;
                zerostruct (info);
                pin->QueryPinInfo (&info);

                if (pinName == 0 || String (pinName).equalsIgnoreCase (String (info.achName)))
                {
                    pin.p->AddRef();
                    *result = pin;
                    return true;
                }
            }
        }

        return false;
    }

    bool connectFilters (IBaseFilter* const first, IBaseFilter* const second) const
    {
        ComSmartPtr <IPin> in, out;

        return getPin (first, PINDIR_OUTPUT, &out)
                && getPin (second, PINDIR_INPUT, &in)
                && SUCCEEDED (graphBuilder->Connect (out, in));
    }

    bool addGraphToRot()
    {
        ComSmartPtr <IRunningObjectTable> rot;
        if (FAILED (GetRunningObjectTable (0, &rot)))
            return false;

        ComSmartPtr <IMoniker> moniker;
        WCHAR buffer[128];
        HRESULT hr = CreateItemMoniker (_T("!"), buffer, &moniker);
        if (FAILED (hr))
            return false;

        graphRegistrationID = 0;
        return SUCCEEDED (rot->Register (0, graphBuilder, moniker, &graphRegistrationID));
    }

    void removeGraphFromRot()
    {
        ComSmartPtr <IRunningObjectTable> rot;

        if (SUCCEEDED (GetRunningObjectTable (0, &rot)))
            rot->Revoke (graphRegistrationID);
    }

    static void deleteMediaType (AM_MEDIA_TYPE* const pmt)
    {
        if (pmt->cbFormat != 0)
            CoTaskMemFree ((PVOID) pmt->pbFormat);

        if (pmt->pUnk != 0)
            pmt->pUnk->Release();

        CoTaskMemFree (pmt);
    }

    //==============================================================================
    class GrabberCallback   : public ISampleGrabberCB
    {
    public:
        GrabberCallback (DShowCameraDeviceInteral& owner_)
            : owner (owner_)
        {
        }

        HRESULT __stdcall QueryInterface (REFIID id, void** result)
        {
            if (id == IID_IUnknown)
                *result = dynamic_cast <IUnknown*> (this);
            else if (id == IID_ISampleGrabberCB)
                *result = dynamic_cast <ISampleGrabberCB*> (this);
            else
            {
                *result = 0;
                return E_NOINTERFACE;
            }

            AddRef();
            return S_OK;
        }

        ULONG __stdcall AddRef()    { return ++refCount; }
        ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

        //==============================================================================
        STDMETHODIMP SampleCB (double /*SampleTime*/, IMediaSample* /*pSample*/)
        {
            return E_FAIL;
        }

        STDMETHODIMP BufferCB (double time, BYTE* buffer, long bufferSize)
        {
            owner.handleFrame (time, buffer, bufferSize);
            return S_OK;
        }

    private:
        int refCount;
        DShowCameraDeviceInteral& owner;

        GrabberCallback (const GrabberCallback&);
        GrabberCallback& operator= (const GrabberCallback&);
    };

    ComSmartPtr <GrabberCallback> callback;
    VoidArray listeners;
    CriticalSection listenerLock;

    //==============================================================================
    DShowCameraDeviceInteral (const DShowCameraDeviceInteral&);
    DShowCameraDeviceInteral& operator= (const DShowCameraDeviceInteral&);
};


//==============================================================================
CameraDevice::CameraDevice (const String& name_, int /*index*/)
    : name (name_)
{
    isRecording = false;
}

CameraDevice::~CameraDevice()
{
    stopRecording();
    delete (DShowCameraDeviceInteral*) internal;
    internal = 0;
}

Component* CameraDevice::createViewerComponent()
{
    return new DShowCameraDeviceInteral::DShowCaptureViewerComp ((DShowCameraDeviceInteral*) internal);
}

const String CameraDevice::getFileExtension()
{
    return ".wmv";
}

void CameraDevice::startRecordingToFile (const File& file, int quality)
{
    stopRecording();

    DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;
    d->addUser();
    isRecording = d->createFileCaptureFilter (file);
}

const Time CameraDevice::getTimeOfFirstRecordedFrame() const
{
    DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;
    return d->firstRecordedTime;
}

void CameraDevice::stopRecording()
{
    if (isRecording)
    {
        DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;
        d->removeFileCaptureFilter();
        d->removeUser();
        isRecording = false;
    }
}

void CameraDevice::addListener (CameraImageListener* listenerToAdd)
{
    DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;

    if (listenerToAdd != 0)
        d->addListener (listenerToAdd);
}

void CameraDevice::removeListener (CameraImageListener* listenerToRemove)
{
    DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;

    if (listenerToRemove != 0)
        d->removeListener (listenerToRemove);
}


//==============================================================================
static ComSmartPtr <IBaseFilter> enumerateCameras (StringArray* const names,
                                                   const int deviceIndexToOpen,
                                                   String& name)
{
    int index = 0;
    ComSmartPtr <IBaseFilter> result;

    ComSmartPtr <ICreateDevEnum> pDevEnum;
    HRESULT hr = pDevEnum.CoCreateInstance (CLSID_SystemDeviceEnum, CLSCTX_INPROC);

    if (SUCCEEDED (hr))
    {
        ComSmartPtr <IEnumMoniker> enumerator;
        hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &enumerator, 0);

        if (SUCCEEDED (hr) && enumerator != 0)
        {
            ComSmartPtr <IBaseFilter> captureFilter;
            ComSmartPtr <IMoniker> moniker;
            ULONG fetched;

            while (enumerator->Next (1, &moniker, &fetched) == S_OK)
            {
                hr = moniker->BindToObject (0, 0, IID_IBaseFilter, (void**) &captureFilter);

                if (SUCCEEDED (hr))
                {
                    ComSmartPtr <IPropertyBag> propertyBag;
                    hr = moniker->BindToStorage (0, 0, IID_IPropertyBag, (void**) &propertyBag);

                    if (SUCCEEDED (hr))
                    {
                        VARIANT var;
                        var.vt = VT_BSTR;

                        hr = propertyBag->Read (_T("FriendlyName"), &var, 0);
                        propertyBag = 0;

                        if (SUCCEEDED (hr))
                        {
                            if (names != 0)
                                names->add (var.bstrVal);

                            if (index == deviceIndexToOpen)
                            {
                                name = var.bstrVal;
                                result = captureFilter;
                                captureFilter = 0;
                                break;
                            }

                            ++index;
                        }

                        moniker = 0;
                    }

                    captureFilter = 0;
                }
            }
        }
    }

    return result;
}

const StringArray CameraDevice::getAvailableDevices()
{
    StringArray devs;
    String dummy;
    enumerateCameras (&devs, -1, dummy);
    return devs;
}

CameraDevice* CameraDevice::openDevice (int index,
                                        int minWidth, int minHeight,
                                        int maxWidth, int maxHeight)
{
    ComSmartPtr <ICaptureGraphBuilder2> captureGraphBuilder;
    HRESULT hr = captureGraphBuilder.CoCreateInstance (CLSID_CaptureGraphBuilder2, CLSCTX_INPROC);

    if (SUCCEEDED (hr))
    {
        String name;
        const ComSmartPtr <IBaseFilter> filter (enumerateCameras (0, index, name));

        if (filter != 0)
        {
            CameraDevice* const cam = new CameraDevice (name, index);
            DShowCameraDeviceInteral* const intern
                = new DShowCameraDeviceInteral (cam, captureGraphBuilder, filter,
                                                minWidth, minHeight, maxWidth, maxHeight);
            cam->internal = intern;

            if (intern->ok)
                return cam;
            else
                delete cam;
        }
    }

    return 0;
}


#endif
