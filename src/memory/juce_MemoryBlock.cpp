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

#include "juce_MemoryBlock.h"


//==============================================================================
MemoryBlock::MemoryBlock() throw()
    : size (0)
{
}

MemoryBlock::MemoryBlock (const size_t initialSize, const bool initialiseToZero)
{
    if (initialSize > 0)
    {
        size = initialSize;
        data.allocate (initialSize, initialiseToZero);
    }
    else
    {
        size = 0;
    }
}

MemoryBlock::MemoryBlock (const MemoryBlock& other)
    : size (other.size)
{
    if (size > 0)
    {
        jassert (other.data != 0);
        data.malloc (size);
        memcpy (data, other.data, size);
    }
}

MemoryBlock::MemoryBlock (const void* const dataToInitialiseFrom, const size_t sizeInBytes)
    : size (jmax ((size_t) 0, sizeInBytes))
{
    jassert (sizeInBytes >= 0);

    if (size > 0)
    {
        jassert (dataToInitialiseFrom != 0); // non-zero size, but a zero pointer passed-in?

        data.malloc (size);

        if (dataToInitialiseFrom != 0)
            memcpy (data, dataToInitialiseFrom, size);
    }
}

MemoryBlock::~MemoryBlock() throw()
{
    jassert (size >= 0);    // should never happen
    jassert (size == 0 || data != 0); // non-zero size but no data allocated?
}

MemoryBlock& MemoryBlock::operator= (const MemoryBlock& other)
{
    if (this != &other)
    {
        setSize (other.size, false);
        memcpy (data, other.data, size);
    }

    return *this;
}

//==============================================================================
bool MemoryBlock::operator== (const MemoryBlock& other) const throw()
{
    return matches (other.data, other.size);
}

bool MemoryBlock::operator!= (const MemoryBlock& other) const throw()
{
    return ! operator== (other);
}

bool MemoryBlock::matches (const void* dataToCompare, size_t dataSize) const throw()
{
    return size == dataSize
            && memcmp (data, dataToCompare, size) == 0;
}

//==============================================================================
// this will resize the block to this size
void MemoryBlock::setSize (const size_t newSize, const bool initialiseToZero)
{
    if (size != newSize)
    {
        if (newSize <= 0)
        {
            data.free();
            size = 0;
        }
        else
        {
            if (data != 0)
            {
                data.realloc (newSize);

                if (initialiseToZero && (newSize > size))
                    zeromem (data + size, newSize - size);
            }
            else
            {
                data.allocate (newSize, initialiseToZero);
            }

            size = newSize;
        }
    }
}

void MemoryBlock::ensureSize (const size_t minimumSize, const bool initialiseToZero)
{
    if (size < minimumSize)
        setSize (minimumSize, initialiseToZero);
}

void MemoryBlock::swapWith (MemoryBlock& other) throw()
{
    swapVariables (size, other.size);
    data.swapWith (other.data);
}

//==============================================================================
void MemoryBlock::fillWith (const uint8 value) throw()
{
    memset (data, (int) value, size);
}

void MemoryBlock::append (const void* const srcData, const size_t numBytes)
{
    if (numBytes > 0)
    {
        const size_t oldSize = size;
        setSize (size + numBytes);
        memcpy (data + oldSize, srcData, numBytes);
    }
}

void MemoryBlock::copyFrom (const void* const src, int offset, size_t num) throw()
{
    const char* d = static_cast<const char*> (src);

    if (offset < 0)
    {
        d -= offset;
        num -= offset;
        offset = 0;
    }

    if (offset + num > size)
        num = size - offset;

    if (num > 0)
        memcpy (data + offset, d, num);
}

void MemoryBlock::copyTo (void* const dst, int offset, size_t num) const throw()
{
    char* d = static_cast<char*> (dst);

    if (offset < 0)
    {
        zeromem (d, -offset);
        d -= offset;

        num += offset;
        offset = 0;
    }

    if (offset + num > size)
    {
        const size_t newNum = size - offset;
        zeromem (d + newNum, num - newNum);
        num = newNum;
    }

    if (num > 0)
        memcpy (d, data + offset, num);
}

void MemoryBlock::removeSection (size_t startByte, size_t numBytesToRemove)
{
    if (startByte < 0)
    {
        numBytesToRemove += startByte;
        startByte = 0;
    }

    if (startByte + numBytesToRemove >= size)
    {
        setSize (startByte);
    }
    else if (numBytesToRemove > 0)
    {
        memmove (data + startByte,
                 data + startByte + numBytesToRemove,
                 size - (startByte + numBytesToRemove));

        setSize (size - numBytesToRemove);
    }
}

const String MemoryBlock::toString() const
{
    return String (static_cast <const char*> (getData()), size);
}

//==============================================================================
int MemoryBlock::getBitRange (const size_t bitRangeStart, size_t numBits) const throw()
{
    int res = 0;

    size_t byte = bitRangeStart >> 3;
    int offsetInByte = (int) bitRangeStart & 7;
    size_t bitsSoFar = 0;

    while (numBits > 0 && (size_t) byte < size)
    {
        const int bitsThisTime = jmin ((int) numBits, 8 - offsetInByte);
        const int mask = (0xff >> (8 - bitsThisTime)) << offsetInByte;

        res |= (((data[byte] & mask) >> offsetInByte) << bitsSoFar);

        bitsSoFar += bitsThisTime;
        numBits -= bitsThisTime;
        ++byte;
        offsetInByte = 0;
    }

    return res;
}

void MemoryBlock::setBitRange (const size_t bitRangeStart, size_t numBits, int bitsToSet) throw()
{
    size_t byte = bitRangeStart >> 3;
    int offsetInByte = (int) bitRangeStart & 7;
    unsigned int mask = ~((((unsigned int) 0xffffffff) << (32 - numBits)) >> (32 - numBits));

    while (numBits > 0 && (size_t) byte < size)
    {
        const int bitsThisTime = jmin ((int) numBits, 8 - offsetInByte);

        const unsigned int tempMask = (mask << offsetInByte) | ~((((unsigned int) 0xffffffff) >> offsetInByte) << offsetInByte);
        const unsigned int tempBits = bitsToSet << offsetInByte;

        data[byte] = (char) ((data[byte] & tempMask) | tempBits);

        ++byte;
        numBits -= bitsThisTime;
        bitsToSet >>= bitsThisTime;
        mask >>= bitsThisTime;
        offsetInByte = 0;
    }
}

//==============================================================================
void MemoryBlock::loadFromHexString (const String& hex)
{
    ensureSize (hex.length() >> 1);
    char* dest = data;
    int i = 0;

    for (;;)
    {
        int byte = 0;

        for (int loop = 2; --loop >= 0;)
        {
            byte <<= 4;

            for (;;)
            {
                const juce_wchar c = hex [i++];

                if (c >= '0' && c <= '9')
                {
                    byte |= c - '0';
                    break;
                }
                else if (c >= 'a' && c <= 'z')
                {
                    byte |= c - ('a' - 10);
                    break;
                }
                else if (c >= 'A' && c <= 'Z')
                {
                    byte |= c - ('A' - 10);
                    break;
                }
                else if (c == 0)
                {
                    setSize (static_cast <size_t> (dest - data));
                    return;
                }
            }
        }

        *dest++ = (char) byte;
    }
}

//==============================================================================
const char* const MemoryBlock::encodingTable = ".ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+";

const String MemoryBlock::toBase64Encoding() const
{
    const size_t numChars = ((size << 3) + 5) / 6;

    String destString ((unsigned int) size); // store the length, followed by a '.', and then the data.
    const int initialLen = destString.length();
    destString.preallocateStorage (initialLen + 2 + numChars);

    juce_wchar* d = destString;
    d += initialLen;
    *d++ = '.';

    for (size_t i = 0; i < numChars; ++i)
        *d++ = encodingTable [getBitRange (i * 6, 6)];

    *d++ = 0;

    return destString;
}

bool MemoryBlock::fromBase64Encoding (const String& s)
{
    const int startPos = s.indexOfChar ('.') + 1;

    if (startPos <= 0)
        return false;

    const int numBytesNeeded = s.substring (0, startPos - 1).getIntValue();

    setSize (numBytesNeeded, true);

    const int numChars = s.length() - startPos;
    const juce_wchar* srcChars = s;
    srcChars += startPos;
    int pos = 0;

    for (int i = 0; i < numChars; ++i)
    {
        const char c = (char) srcChars[i];

        for (int j = 0; j < 64; ++j)
        {
            if (encodingTable[j] == c)
            {
                setBitRange (pos, 6, j);
                pos += 6;
                break;
            }
        }
    }

    return true;
}


END_JUCE_NAMESPACE
