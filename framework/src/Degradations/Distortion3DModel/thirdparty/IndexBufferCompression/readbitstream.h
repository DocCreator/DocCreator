/*
Copyright (c) 2014-2015, Conor Stokes
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef READ_BIT_STREAM_H__
#define READ_BIT_STREAM_H__
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER

#define RBS_INLINE __forceinline

#else

#define RBS_INLINE inline

#endif

// Detect if our processor is x86/x64 (supports unaligned reads and is little endian)
// Note that other processors could be added
#if __X86_64__ || \
    _M_X64 || \
    _M_AMD64 || \
   defined(__x86_64__) || \
   __386__ || \
   _M_I386 || \
   (defined(__DMC__) && defined(_M_IX86)) || \
   (defined(_MSC_VER) && _M_IX86) || \
   defined(__i386__)

#define RBS_LITTLE_ENDIAN_UNALIGNED

#endif


// Used for representing an entry in the decoding table for prefix coding.
struct PrefixCodeTableEntry
{
    uint8_t original;
    uint8_t codeLength;
};

/*
  Simple reader bitstream.

  DocCreator: the original did no overflow checking: it refilled its bit
  buffer with 8 (or 4) byte reads at a cursor it never compared with the
  end of the buffer, so a stream asking for more bits than it holds -- a
  corrupt or crafted file -- read past the buffer for as long as the
  decoder kept asking. The bytes past the end now read as zeros, the
  position is an offset rather than a pointer, so that no pointer past the
  buffer is ever formed, and Overrun() tells whether more bits were
  consumed than the buffer holds. ReadVInt() also stops after the five
  bytes a 32 bit value takes, instead of shifting by 35 bits and more.
  Reading a valid stream gives the same values as before.
*/
class ReadBitstream
{
public:

    // Construct the bitstream with a fixed byte buffer.
    ReadBitstream( const uint8_t* buffer, size_t bufferSize );

    ~ReadBitstream() {}

    // Read a number of bits
    uint32_t Read( uint32_t bitcount );

    // Get the buffer size of this in bytes
    size_t Size() const { return m_bufferSize; }

    // Read a variable encoded int (use MSB of each byte to signal another byte
    uint32_t ReadVInt();

    // Decode prefix code using table (least significant bits lookup).
    // Note, maximum code length should be 32 or less (practically much lower, as you need a table to match).
    // Also note, this uses 4 byte reads/only partially refills the bit-buffer.
    uint32_t Decode( const PrefixCodeTableEntry* table, const uint32_t maximumCodeLength );

    // true if more bits have been read than the buffer holds
    bool Overrun() const { return m_offset * 8 - m_bitsLeft > m_bufferSize * 8; }

private:

    // @return the @a count bytes (at most 8) at the current offset, little
    // endian, those past the end of the buffer as zeros; and advance.
    uint64_t Fetch( uint32_t count );

    uint64_t m_bitBuffer;

    const uint8_t* m_buffer;
    size_t m_offset;

    size_t m_bufferSize;
    uint32_t m_bitsLeft;

};


RBS_INLINE uint64_t ReadBitstream::Fetch( uint32_t count )
{
    uint64_t value = 0;

    if ( m_offset <= m_bufferSize && count <= m_bufferSize - m_offset )
    {
#if defined( RBS_LITTLE_ENDIAN_UNALIGNED )
        memcpy( &value, m_buffer + m_offset, count );
#else
        for ( uint32_t i = 0; i < count; ++i )
        {
            value |= static_cast< uint64_t >( m_buffer[ m_offset + i ] ) << ( 8 * i );
        }
#endif
    }
    else
    {
        for ( uint32_t i = 0; i < count; ++i )
        {
            const size_t index = m_offset + i;
            if ( index < m_bufferSize )
            {
                value |= static_cast< uint64_t >( m_buffer[ index ] ) << ( 8 * i );
            }
        }
    }

    m_offset += count;

    return value;
}


RBS_INLINE uint32_t ReadBitstream::Decode( const PrefixCodeTableEntry* table, uint32_t maximumCodeSize )
{
    if ( m_bitsLeft < maximumCodeSize )
    {
        uint64_t intermediateBitBuffer = Fetch( 4 );

        m_bitBuffer           |= intermediateBitBuffer << m_bitsLeft;

        m_bitsLeft            += 32;
    }

    // mask should be constant collasped due to maximumCodeSize being fixed and this being inlined.
    const uint64_t              mask       = ( uint64_t( 1 ) << maximumCodeSize ) - 1;
    const PrefixCodeTableEntry& codeEntry  = table[ m_bitBuffer & mask ];
    const uint32_t              codeLength = codeEntry.codeLength;

    m_bitBuffer >>= codeLength;
    m_bitsLeft   -= codeLength;

    return codeEntry.original;
}


inline ReadBitstream::ReadBitstream( const uint8_t* buffer, size_t bufferSize )
{
    m_buffer     = buffer;
    m_offset     = 0;
    m_bufferSize = bufferSize;

    m_bitBuffer  = Fetch( 8 );
    m_bitsLeft   = 64;
}


RBS_INLINE uint32_t ReadBitstream::Read( uint32_t bitCount )
{
    uint64_t mask   = ( uint64_t( 1 ) << bitCount ) - 1;
    uint32_t result = static_cast< uint32_t >( m_bitBuffer & mask );

    m_bitBuffer >>= bitCount;

    if ( m_bitsLeft < bitCount )
    {
        m_bitBuffer = Fetch( 8 );

        uint32_t leftOverBits = bitCount - m_bitsLeft;

        result       |= static_cast< uint32_t >( m_bitBuffer << m_bitsLeft ) & mask;
        m_bitBuffer >>= leftOverBits;
        m_bitsLeft    = 64 - leftOverBits;
    }
    else
    {
        m_bitsLeft -= bitCount;
    }

    return result;
}


RBS_INLINE uint32_t ReadBitstream::ReadVInt()
{
    uint32_t bitsToShift = 0;
    uint32_t result      = 0;
    uint32_t readByte;

    do
    {

        readByte      = Read( 8 );
        result       |= ( readByte & 0x7F ) << bitsToShift;
        bitsToShift  += 7;

    }
    while ( ( readByte & 0x80 ) && bitsToShift < 32 );

    return result;
}

#endif // -- READ_BIT_STREAM_H__
