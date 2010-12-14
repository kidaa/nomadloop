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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_WEB_BROWSER


//==============================================================================
class WebBrowserComponentInternal   : public ActiveXControlComponent
{
public:
    //==============================================================================
    WebBrowserComponentInternal()
        : browser (0),
          connectionPoint (0),
          adviseCookie (0)
    {
    }

    ~WebBrowserComponentInternal()
    {
        if (connectionPoint != 0)
            connectionPoint->Unadvise (adviseCookie);

        if (browser != 0)
            browser->Release();
    }

    void createBrowser()
    {
        createControl (&CLSID_WebBrowser);
        browser = (IWebBrowser2*) queryInterface (&IID_IWebBrowser2);

        IConnectionPointContainer* connectionPointContainer = (IConnectionPointContainer*) queryInterface (&IID_IConnectionPointContainer);

        if (connectionPointContainer != 0)
        {
            connectionPointContainer->FindConnectionPoint (DIID_DWebBrowserEvents2,
                                                           &connectionPoint);

            if (connectionPoint != 0)
            {
                WebBrowserComponent* const owner = dynamic_cast <WebBrowserComponent*> (getParentComponent());
                jassert (owner != 0);

                EventHandler* handler = new EventHandler (owner);
                connectionPoint->Advise (handler, &adviseCookie);
                handler->Release();
            }
        }
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData)
    {
        if (browser != 0)
        {
            LPSAFEARRAY sa = 0;

            VARIANT flags, frame, postDataVar, headersVar;  // (_variant_t isn't available in all compilers)
            VariantInit (&flags);
            VariantInit (&frame);
            VariantInit (&postDataVar);
            VariantInit (&headersVar);

            if (headers != 0)
            {
                V_VT (&headersVar) = VT_BSTR;
                V_BSTR (&headersVar) = SysAllocString ((const OLECHAR*) headers->joinIntoString ("\r\n"));
            }

            if (postData != 0 && postData->getSize() > 0)
            {
                LPSAFEARRAY sa = SafeArrayCreateVector (VT_UI1, 0, postData->getSize());

                if (sa != 0)
                {
                    void* data = 0;
                    SafeArrayAccessData (sa, &data);
                    jassert (data != 0);

                    if (data != 0)
                    {
                        postData->copyTo (data, 0, postData->getSize());
                        SafeArrayUnaccessData (sa);

                        VARIANT postDataVar2;
                        VariantInit (&postDataVar2);
                        V_VT (&postDataVar2) = VT_ARRAY | VT_UI1;
                        V_ARRAY (&postDataVar2) = sa;

                        postDataVar = postDataVar2;
                    }
                }
            }

            browser->Navigate ((BSTR) (const OLECHAR*) url,
                               &flags, &frame,
                               &postDataVar, &headersVar);

            if (sa != 0)
                SafeArrayDestroy (sa);

            VariantClear (&flags);
            VariantClear (&frame);
            VariantClear (&postDataVar);
            VariantClear (&headersVar);
        }
    }

    //==============================================================================
    IWebBrowser2* browser;

private:
    IConnectionPoint* connectionPoint;
    DWORD adviseCookie;

    //==============================================================================
    class EventHandler  : public ComBaseClassHelper <IDispatch>,
                          public ComponentMovementWatcher
    {
    public:
        EventHandler (WebBrowserComponent* const owner_)
            : ComponentMovementWatcher (owner_),
              owner (owner_)
        {
        }

        //==============================================================================
        HRESULT __stdcall GetTypeInfoCount (UINT*)                                  { return E_NOTIMPL; }
        HRESULT __stdcall GetTypeInfo (UINT, LCID, ITypeInfo**)                     { return E_NOTIMPL; }
        HRESULT __stdcall GetIDsOfNames (REFIID, LPOLESTR*, UINT, LCID, DISPID*)    { return E_NOTIMPL; }

        HRESULT __stdcall Invoke (DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS* pDispParams,
                                  VARIANT* /*pVarResult*/, EXCEPINFO* /*pExcepInfo*/, UINT* /*puArgErr*/)
        {
            switch (dispIdMember)
            {
                case DISPID_BEFORENAVIGATE2:
                {
                    VARIANT* const vurl = pDispParams->rgvarg[5].pvarVal;

                    String url;

                    if ((vurl->vt & VT_BYREF) != 0)
                        url = *vurl->pbstrVal;
                    else
                        url = vurl->bstrVal;

                    *pDispParams->rgvarg->pboolVal
                        = owner->pageAboutToLoad (url) ? VARIANT_FALSE
                                                       : VARIANT_TRUE;

                    return S_OK;
                }

                default:
                    break;
            }

            return E_NOTIMPL;
        }

        void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) {}
        void componentPeerChanged() {}

        void componentVisibilityChanged (Component&)
        {
            owner->visibilityChanged();
        }

        //==============================================================================
    private:
        WebBrowserComponent* const owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventHandler);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowserComponentInternal);
};


//==============================================================================
WebBrowserComponent::WebBrowserComponent (const bool unloadPageWhenBrowserIsHidden_)
    : browser (0),
      blankPageShown (false),
      unloadPageWhenBrowserIsHidden (unloadPageWhenBrowserIsHidden_)
{
    setOpaque (true);
    addAndMakeVisible (browser = new WebBrowserComponentInternal());
}

WebBrowserComponent::~WebBrowserComponent()
{
    delete browser;
}

//==============================================================================
void WebBrowserComponent::goToURL (const String& url,
                                   const StringArray* headers,
                                   const MemoryBlock* postData)
{
    lastURL = url;

    lastHeaders.clear();
    if (headers != 0)
        lastHeaders = *headers;

    lastPostData.setSize (0);
    if (postData != 0)
        lastPostData = *postData;

    blankPageShown = false;

    browser->goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    if (browser->browser != 0)
        browser->browser->Stop();
}

void WebBrowserComponent::goBack()
{
    lastURL = String::empty;
    blankPageShown = false;

    if (browser->browser != 0)
        browser->browser->GoBack();
}

void WebBrowserComponent::goForward()
{
    lastURL = String::empty;

    if (browser->browser != 0)
        browser->browser->GoForward();
}

void WebBrowserComponent::refresh()
{
    if (browser->browser != 0)
        browser->browser->Refresh();
}

//==============================================================================
void WebBrowserComponent::paint (Graphics& g)
{
    if (browser->browser == 0)
        g.fillAll (Colours::white);
}

void WebBrowserComponent::checkWindowAssociation()
{
    if (isShowing())
    {
        if (browser->browser == 0 && getPeer() != 0)
        {
            browser->createBrowser();
            reloadLastURL();
        }
        else
        {
            if (blankPageShown)
                goBack();
        }
    }
    else
    {
        if (browser != 0 && unloadPageWhenBrowserIsHidden && ! blankPageShown)
        {
            // when the component becomes invisible, some stuff like flash
            // carries on playing audio, so we need to force it onto a blank
            // page to avoid this..

            blankPageShown = true;
            browser->goToURL ("about:blank", 0, 0);
        }
    }
}

void WebBrowserComponent::reloadLastURL()
{
    if (lastURL.isNotEmpty())
    {
        goToURL (lastURL, &lastHeaders, &lastPostData);
        lastURL = String::empty;
    }
}

void WebBrowserComponent::parentHierarchyChanged()
{
    checkWindowAssociation();
}

void WebBrowserComponent::resized()
{
    browser->setSize (getWidth(), getHeight());
}

void WebBrowserComponent::visibilityChanged()
{
    checkWindowAssociation();
}

bool WebBrowserComponent::pageAboutToLoad (const String&)
{
    return true;
}

#endif
