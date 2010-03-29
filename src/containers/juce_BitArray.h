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

#ifndef __JUCE_BITARRAY_JUCEHEADER__
#define __JUCE_BITARRAY_JUCEHEADER__

#include "../text/juce_String.h"
#include "juce_HeapBlock.h"
class MemoryBlock;


//==============================================================================
/**
    An arbitrarily large integer class.

    A BigInteger can be used in a similar way to a normal integer, but has no size
    limit (except for memory and performance constraints).

    Negative values are possible, but the value isn't stored as 2s-complement, so
    be careful if you use negative values and look at the values of individual bits.
*/
class JUCE_API  BigInteger
{
public:
    //==============================================================================
    /** Creates an empty BigInteger */
    BigInteger();

    /** Creates a BigInteger containing an integer value in its low bits.

        The low 32 bits of the number are initialised with this value.
    */
    BigInteger (unsigned int value);

    /** Creates a BigInteger containing an integer value in its low bits.

        The low 32 bits of the number are initialised with the absolute value
        passed in, and its sign is set to reflect the sign of the number.
    */
    BigInteger (int value);

    /** Creates a BigInteger containing an integer value in its low bits.

        The low 64 bits of the number are initialised with the absolute value
        passed in, and its sign is set to reflect the sign of the number.
    */
    BigInteger (int64 value);

    /** Creates a copy of another BigInteger. */
    BigInteger (const BigInteger& other);

    /** Destructor. */
    ~BigInteger();

    //==============================================================================
    /** Copies another BigInteger onto this one. */
    BigInteger& operator= (const BigInteger& other);

    /** Swaps the internal contents of this with another object. */
    void swapWith (BigInteger& other) throw();

    //==============================================================================
    /** Returns the value of a specified bit in the number.
        If the index is out-of-range, the result will be false.
    */
    bool operator[] (int bit) const throw();

    /** Returns true if no bits are set. */
    bool isZero() const throw();

    /** Returns true if the value is 1. */
    bool isOne() const throw();

    /** Attempts to get the lowest bits of the value as an integer.
        If the value is bigger than the integer limits, this will return only the lower bits.
    */
    int toInteger() const throw();

    //==============================================================================
    /** Resets the value to 0. */
    void clear();

    /** Clears a particular bit in the number. */
    void clearBit (int bitNumber) throw();

    /** Sets a specified bit to 1. */
    void setBit (int bitNumber);

    /** Sets or clears a specified bit. */
    void setBit (int bitNumber, bool shouldBeSet);

    /** Sets a range of bits to be either on or off.

        @param startBit     the first bit to change
        @param numBits      the number of bits to change
        @param shouldBeSet  whether to turn these bits on or off
    */
    void setRange (int startBit, int numBits, bool shouldBeSet);

    /** Inserts a bit an a given position, shifting up any bits above it. */
    void insertBit (int bitNumber, bool shouldBeSet);

    /** Returns a range of bits as a new BigInteger.

        e.g. getBitRangeAsInt (0, 64) would return the lowest 64 bits.
        @see getBitRangeAsInt
    */
    const BigInteger getBitRange (int startBit, int numBits) const;

    /** Returns a range of bits as an integer value.

        e.g. getBitRangeAsInt (0, 32) would return the lowest 32 bits.

        Asking for more than 32 bits isn't allowed (obviously) - for that, use
        getBitRange().
    */
    int getBitRangeAsInt (int startBit, int numBits) const throw();

    /** Sets a range of bits to an integer value.

        Copies the given integer onto a range of bits, starting at startBit,
        and using up to numBits of the available bits.
    */
    void setBitRangeAsInt (int startBit, int numBits, unsigned int valueToSet);

    /** Shifts a section of bits left or right.

        @param howManyBitsLeft  how far to move the bits (+ve numbers shift it left, -ve numbers shift it right).
        @param startBit         the first bit to affect - if this is > 0, only bits above that index will be affected.
    */
    void shiftBits (int howManyBitsLeft, int startBit);

    /** Returns the total number of set bits in the value. */
    int countNumberOfSetBits() const throw();

    /** Looks for the index of the next set bit after a given starting point.

        This searches from startIndex (inclusive) upwards for the first set bit,
        and returns its index. If no set bits are found, it returns -1.
    */
    int findNextSetBit (int startIndex = 0) const throw();

    /** Looks for the index of the next clear bit after a given starting point.

        This searches from startIndex (inclusive) upwards for the first clear bit,
        and returns its index.
    */
    int findNextClearBit (int startIndex = 0) const throw();

    /** Returns the index of the highest set bit in the number.
        If the value is zero, this will return -1.
    */
    int getHighestBit() const throw();

    //==============================================================================
    // All the standard arithmetic ops...

    BigInteger& operator+= (const BigInteger& other);
    BigInteger& operator-= (const BigInteger& other);
    BigInteger& operator*= (const BigInteger& other);
    BigInteger& operator/= (const BigInteger& other);
    BigInteger& operator|= (const BigInteger& other);
    BigInteger& operator&= (const BigInteger& other);
    BigInteger& operator^= (const BigInteger& other);
    BigInteger& operator%= (const BigInteger& other);
    BigInteger& operator<<= (int numBitsToShift);
    BigInteger& operator>>= (int numBitsToShift);
    BigInteger& operator++();
    BigInteger& operator--();
    const BigInteger operator++ (int);
    const BigInteger operator-- (int);

    const BigInteger operator-() const;
    const BigInteger operator+ (const BigInteger& other) const;
    const BigInteger operator- (const BigInteger& other) const;
    const BigInteger operator* (const BigInteger& other) const;
    const BigInteger operator/ (const BigInteger& other) const;
    const BigInteger operator| (const BigInteger& other) const;
    const BigInteger operator& (const BigInteger& other) const;
    const BigInteger operator^ (const BigInteger& other) const;
    const BigInteger operator% (const BigInteger& other) const;
    const BigInteger operator<< (int numBitsToShift) const;
    const BigInteger operator>> (int numBitsToShift) const;

    bool operator== (const BigInteger& other) const throw();
    bool operator!= (const BigInteger& other) const throw();
    bool operator<  (const BigInteger& other) const throw();
    bool operator<= (const BigInteger& other) const throw();
    bool operator>  (const BigInteger& other) const throw();
    bool operator>= (const BigInteger& other) const throw();

    //==============================================================================
    /** Does a signed comparison of two BigIntegers.

        Return values are:
            - 0 if the numbers are the same
            - < 0 if this number is smaller than the other
            - > 0 if this number is bigger than the other
    */
    int compare (const BigInteger& other) const throw();

    /** Compares the magnitudes of two BigIntegers, ignoring their signs.

        Return values are:
            - 0 if the numbers are the same
            - < 0 if this number is smaller than the other
            - > 0 if this number is bigger than the other
    */
    int compareAbsolute (const BigInteger& other) const throw();

    /** Divides this value by another one and returns the remainder.

        This number is divided by other, leaving the quotient in this number,
        with the remainder being copied to the other BigInteger passed in.
    */
    void divideBy (const BigInteger& divisor, BigInteger& remainder);

    /** Returns the largest value that will divide both this value and the one passed-in.
    */
    const BigInteger findGreatestCommonDivisor (BigInteger other) const;

    /** Performs a combined exponent and modulo operation.

        This BigInteger's value becomes (this ^ exponent) % modulus.
    */
    void exponentModulo (const BigInteger& exponent, const BigInteger& modulus);

    /** Performs an inverse modulo on the value.

        i.e. the result is (this ^ -1) mod (modulus).
    */
    void inverseModulo (const BigInteger& modulus);

    //==============================================================================
    /** Returns true if the value is less than zero.
        @see setNegative, negate
    */
    bool isNegative() const throw();

    /** Changes the sign of the number to be positive or negative.
        @see isNegative, negate
    */
    void setNegative (const bool shouldBeNegative) throw();

    /** Inverts the sign of the number.
        @see isNegative, setNegative
    */
    void negate() throw();

    //==============================================================================
    /** Converts the number to a string.

        Specify a base such as 2 (binary), 8 (octal), 10 (decimal), 16 (hex).
        If minimumNumCharacters is greater than 0, the returned string will be
        padded with leading zeros to reach at least that length.
    */
    const String toString (int base, int minimumNumCharacters = 1) const;

    /** Reads the numeric value from a string.

        Specify a base such as 2 (binary), 8 (octal), 10 (decimal), 16 (hex).
        Any invalid characters will be ignored.
    */
    void parseString (const String& text, int base);

    //==============================================================================
    /** Turns the number into a block of binary data.

        The data is arranged as little-endian, so the first byte of data is the low 8 bits
        of the number, and so on.

        @see loadFromMemoryBlock
    */
    const MemoryBlock toMemoryBlock() const;

    /** Converts a block of raw data into a number.

        The data is arranged as little-endian, so the first byte of data is the low 8 bits
        of the number, and so on.

        @see toMemoryBlock
    */
    void loadFromMemoryBlock (const MemoryBlock& data);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    HeapBlock <unsigned int> values;
    int numValues, highestBit;
    bool negative;

    void ensureSize (int numVals);
    static const BigInteger simpleGCD (BigInteger* m, BigInteger* n);
};

/** Writes a BigInteger to an OutputStream as a UTF8 decimal string. */
OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const BigInteger& value);

//==============================================================================
/** For backwards compatibility, BitArray is defined to be an alias for BigInteger.
*/
typedef BigInteger BitArray;


#endif   // __JUCE_BITARRAY_JUCEHEADER__
