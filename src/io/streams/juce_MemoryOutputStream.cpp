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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MemoryOutputStream.h"


//==============================================================================
MemoryOutputStream::MemoryOutputStream (const size_t initialSize)
  : data (internalBlock),
    position (0),
    size (0)
{
    internalBlock.setSize (initialSize, false);
}

MemoryOutputStream::MemoryOutputStream (MemoryBlock& memoryBlockToWriteTo,
                                        const bool appendToExistingBlockContent)
  : data (memoryBlockToWriteTo),
    position (0),
    size (0)
{
    if (appendToExistingBlockContent)
        position = size = memoryBlockToWriteTo.getSize();
}

MemoryOutputStream::~MemoryOutputStream()
{
    flush();
}

void MemoryOutputStream::flush()
{
    if (&data != &internalBlock)
        data.setSize (size, false);
}

void MemoryOutputStream::preallocate (const size_t bytesToPreallocate)
{
    data.ensureSize (bytesToPreallocate + 1);
}

void MemoryOutputStream::reset() throw()
{
    position = 0;
    size = 0;
}

bool MemoryOutputStream::write (const void* const buffer, int howMany)
{
    if (howMany > 0)
    {
        const size_t storageNeeded = position + howMany;

        if (storageNeeded >= data.getSize())
            data.ensureSize ((storageNeeded + jmin ((int) (storageNeeded / 2), 1024 * 1024) + 32) & ~31);

        memcpy (static_cast<char*> (data.getData()) + position, buffer, howMany);
        position += howMany;
        size = jmax (size, position);
    }

    return true;
}

const void* MemoryOutputStream::getData() const throw()
{
    void* const d = data.getData();

    if (data.getSize() > size)
        static_cast <char*> (d) [size] = 0;

    return d;
}

bool MemoryOutputStream::setPosition (int64 newPosition)
{
    if (newPosition <= (int64) size)
    {
        // ok to seek backwards
        position = jlimit ((size_t) 0, size, (size_t) newPosition);
        return true;
    }
    else
    {
        // trying to make it bigger isn't a good thing to do..
        return false;
    }
}

int MemoryOutputStream::writeFromInputStream (InputStream& source, int64 maxNumBytesToWrite)
{
    // before writing from an input, see if we can preallocate to make it more efficient..
    int64 availableData = source.getTotalLength() - source.getPosition();

    if (availableData > 0)
    {
        if (maxNumBytesToWrite > 0 && maxNumBytesToWrite < availableData)
            availableData = maxNumBytesToWrite;

        preallocate (data.getSize() + (size_t) maxNumBytesToWrite);
    }

    return OutputStream::writeFromInputStream (source, maxNumBytesToWrite);
}

const String MemoryOutputStream::toUTF8() const
{
    const char* const d = static_cast <const char*> (getData());
    return String (CharPointer_UTF8 (d), CharPointer_UTF8 (d + getDataSize()));
}

const String MemoryOutputStream::toString() const
{
    return String::createStringFromData (getData(), (int) getDataSize());
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const MemoryOutputStream& streamToRead)
{
    stream.write (streamToRead.getData(), (int) streamToRead.getDataSize());
    return stream;
}

END_JUCE_NAMESPACE
