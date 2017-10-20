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

// Very simple reader bitstream, note it does not do any overflow checking, etc.
class ReadBitstream
{
public:

    // Construct the bitstream with a fixed byte buffer (which should be padded out to multiples of 8 bytes, as we read in 8 byte chunks).
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

private:

    uint64_t m_bitBuffer;

    const uint8_t* m_buffer;
    const uint8_t* m_cursor;

    size_t m_bufferSize;
    uint32_t m_bitsLeft;

};


RBS_INLINE uint32_t ReadBitstream::Decode( const PrefixCodeTableEntry* table, uint32_t maximumCodeSize )
{
    if ( m_bitsLeft < maximumCodeSize )
    {
#if defined( RBS_LITTLE_ENDIAN_UNALIGNED )

        // We're on x86/x64, so we're little endian and can do an un-aligned read.
        uint64_t intermediateBitBuffer = *(const uint32_t*)m_cursor;

#else

        // other processor architecture, unknown endian/unaligned read support
        uint64_t intermediateBitBuffer = m_cursor[ 0 ];

        intermediateBitBuffer |= static_cast< uint64_t >( m_cursor[ 1 ] ) << 8;
        intermediateBitBuffer |= static_cast< uint64_t >( m_cursor[ 2 ] ) << 16;
        intermediateBitBuffer |= static_cast< uint64_t >( m_cursor[ 3 ] ) << 24;

#endif

        m_bitBuffer           |= intermediateBitBuffer << m_bitsLeft;

        m_bitsLeft            += 32;
        m_cursor              += 4;
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
    m_cursor     =
    m_buffer     = buffer;
    m_bufferSize = bufferSize;

    if ( bufferSize >= 8 )
    {
        m_bitBuffer  = m_cursor[ 0 ];
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 1 ] ) << 8;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 2 ] ) << 16;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 3 ] ) << 24;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 4 ] ) << 32;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 5 ] ) << 40;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 6 ] ) << 48;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 7 ] ) << 56;

        m_cursor  += 8;
        m_bitsLeft = 64;
    }
    else
    {
        m_bitsLeft = 0;
    }
}


RBS_INLINE uint32_t ReadBitstream::Read( uint32_t bitCount )
{
    uint64_t mask   = ( uint64_t( 1 ) << bitCount ) - 1;
    uint32_t result = static_cast< uint32_t >( m_bitBuffer & mask );

    m_bitBuffer >>= bitCount;

    if ( m_bitsLeft < bitCount )
    {
#if defined( RBS_LITTLE_ENDIAN_UNALIGNED )

        // We're on x86/x64, so we're little endian and can do an un-aligned read.
        m_bitBuffer = *(const uint64_t*)m_cursor;

#else

        // other processor architecture, unknown endian/unaligned read support
        m_bitBuffer  = m_cursor[ 0 ];
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 1 ] ) << 8;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 2 ] ) << 16;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 3 ] ) << 24;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 4 ] ) << 32;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 5 ] ) << 40;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 6 ] ) << 48;
        m_bitBuffer |= static_cast< uint64_t >( m_cursor[ 7 ] ) << 56;

#endif

        m_cursor += 8;

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
    while ( readByte & 0x80 );

    return result;
}

#endif // -- READ_BIT_STREAM_H__
