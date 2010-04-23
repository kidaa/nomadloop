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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AsyncUpdater.h"


//==============================================================================
AsyncUpdater::AsyncUpdater() throw()
   : asyncMessagePending (false)
{
    internalAsyncHandler.owner = this;
}

AsyncUpdater::~AsyncUpdater()
{
}

void AsyncUpdater::triggerAsyncUpdate() throw()
{
    if (! asyncMessagePending)
    {
        asyncMessagePending = true;
        internalAsyncHandler.postMessage (new Message());
    }
}

void AsyncUpdater::cancelPendingUpdate() throw()
{
    asyncMessagePending = false;
}

void AsyncUpdater::handleUpdateNowIfNeeded()
{
    if (asyncMessagePending)
    {
        asyncMessagePending = false;
        handleAsyncUpdate();
    }
}

void AsyncUpdater::AsyncUpdaterInternal::handleMessage (const Message&)
{
    owner->handleUpdateNowIfNeeded();
}


END_JUCE_NAMESPACE
