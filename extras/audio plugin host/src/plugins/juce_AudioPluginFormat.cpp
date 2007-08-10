/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce.h"
#include "juce_AudioPluginFormat.h"
#include "vst/juce_VSTPluginFormat.h"


//==============================================================================
AudioPluginFormatManager::AudioPluginFormatManager() throw()
{
}

AudioPluginFormatManager::~AudioPluginFormatManager() throw()
{
}

juce_ImplementSingleton_SingleThreaded (AudioPluginFormatManager);

//==============================================================================
void AudioPluginFormatManager::addDefaultFormats()
{
#ifdef JUCE_DEBUG
    // you should only call this method once!
    for (int i = formats.size(); --i >= 0;)
        jassert (dynamic_cast <VSTPluginFormat*> (formats[i]) == 0);
#endif

    formats.add (new VSTPluginFormat());
}

int AudioPluginFormatManager::getNumFormats() throw()
{
    return formats.size();
}

AudioPluginFormat* AudioPluginFormatManager::getFormat (const int index) throw()
{
    return formats [index];
}

void AudioPluginFormatManager::addFormat (AudioPluginFormat* const format) throw()
{
    formats.add (format);
}