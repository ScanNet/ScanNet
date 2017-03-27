//
//  image/image-codecs.h
//  Uplink
//
//  Copyright (c) 2013 Occipital, Inc. All rights reserved.
//

# pragma once

#include "memory.h"

#include <functional>
# include <stdint.h>
# include <cstdio>
# include <cassert>
# include <cstdlib>
# include <cstdio>
# include <cstring>

# define BITSTREAM_SUCCESS 1
# define BITSTREAM_FAILURE 0

namespace uplinksimple {

	//------------------------------------------------------------------------------

	typedef struct s_bitstream {
		uint8_t *buf;               /* head of bitstream            */
		uint8_t *pos;               /* current byte in bitstream    */
		unsigned int   remain;      /* bits remaining               */
		unsigned int   len;         /* length of bitstream in bytes */
		uint8_t cur_bits;
	} bitstream_t;

	int bs_init(bitstream_t *bp);
	int bs_destroy(bitstream_t **ppb);
	int bs_attach(bitstream_t *b, uint8_t *buf, int blen);
	uint8_t bs_get(bitstream_t * b, uint8_t  nbits);
	int bs_bytes_used(bitstream_t *b);

	static inline int
		bs_put(bitstream_t * b,
		uint8_t       bits,
		uint8_t       nbits)
	{
		assert(nbits != 0 && nbits <= 8);

		if (nbits > b->remain) {
			unsigned int over = nbits - b->remain;
			*(b->pos++) = b->cur_bits | (bits >> over);
			b->remain = 8 - over;
			b->cur_bits = bits << b->remain;
		}
		else {
			b->cur_bits |= bits << (b->remain - nbits);
			b->remain -= nbits;

			// we've exhausted the byte.  move to the next byte.
			if (b->remain == 0) {
				*(b->pos++) = b->cur_bits;
				b->remain = 8;
				b->cur_bits = 0;
			}
		}

		assert((unsigned int)(b->pos - b->buf) <= b->len);
		return BITSTREAM_SUCCESS;
	}

	static inline int
		bs_flush(bitstream_t * b)
	{
		*(b->pos) = b->cur_bits;
		return BITSTREAM_SUCCESS;
	}

	//------------------------------------------------------------------------------

}


namespace uplinksimple {

	//------------------------------------------------------------------------------

	inline int
		bs_init(bitstream_t *bp)
	{
		if (bp) {
			memset(bp, 0, sizeof(bitstream_t));
			return BITSTREAM_SUCCESS;
		}
		return BITSTREAM_FAILURE;
	}

	inline int
		bs_destroy(bitstream_t * b)
	{
		free(b);
		return BITSTREAM_SUCCESS;
	}

	inline int
		bs_attach(bitstream_t *b,
		uint8_t *buf,
		int blen)
	{
		b->buf = b->pos = buf;
		b->remain = 8;
		b->len = blen;
		return BITSTREAM_SUCCESS;
	}

	inline int
		bs_bytes_used(bitstream_t *b)
	{
		unsigned int used = (unsigned int)(b->pos - b->buf);
		return b->remain != 8 ? used + 1 : used;
	}

	inline uint8_t
		bs_get(bitstream_t * b,
		uint8_t  nbits)
	{
		uint8_t out;

		if (b->remain == 0) {
			b->pos++;
			b->remain = 8;
		}

		if (nbits > b->remain) {
			/* Get high bits */
			out = *b->pos;
			out <<= (8 - b->remain);
			out >>= (8 - nbits);
			b->pos++;
			b->remain += 8 - nbits;
			out |= (*b->pos) >> b->remain;
		}
		else {
			out = *b->pos;
			out <<= (8 - b->remain);
			out >>= (8 - nbits);
			b->remain -= nbits;
		}

		assert((unsigned int)(b->pos - b->buf) <= b->len);
		return out;
	}

	//------------------------------------------------------------------------------

}



namespace uplinksimple {

//------------------------------------------------------------------------------

// -----------------------------------------------
// OCCIPITAL DEPTH FRAME COMPRESSION ALGORITHM 0.1
// -----------------------------------------------

// TYPICAL COMPRESSION RATIO ON 640x480 test image:  0.17

// DECODE:
// Step 0. Last value is initialized to 0.  Frame size is known in advance.
// Step 1. Proceed by decoding following bitstream until all pixels are decoded.

// 00 - Next value is same as last value.
// 11 - Next value is last value + 1.
// 10 - Next value is last value - 1.
// 010 - bbbbb - Next N values are same as last value.  (N encoded w/ 5 bits)
// 0111 - bbbbbbbbbbb - Next value is X.  (X encoded w/ 11 bits)
// 01101 - Next value is last value + 2.
// 01100 - Next value is last value - 2.

	inline uint16_t * decode(const uint8_t * bitstream_data, unsigned int bitstream_length_bytes, int numelements, uint16_t* output)
{

    uint16_t lastVal = 0;
    uint16_t curVal = 0;

    bitstream_t bs;
    bs_init(&bs);
    bs_attach(&bs, const_cast<uint8_t*>(bitstream_data), bitstream_length_bytes);

    uint16_t * depthimage = 0 == output
        ? (uint16_t*)malloc(numelements * sizeof(uint16_t))
        : output
        ;

    uint16_t * depth_ptr = depthimage;

    while(numelements > 0)
    {
        uint8_t bit0 = bs_get(&bs, 1);
        uint8_t bit1 = bs_get(&bs, 1);

        if(bit0 == 0 && bit1 == 0) // 00
        {
            curVal = lastVal;

            *(depth_ptr++) = curVal; lastVal = curVal;

            numelements-=1;
        }
        else if(bit0 == 1) // 1 prefix
        {

            if(bit1 == 0) {
                curVal = lastVal - 1;
            }
            else {
                curVal = lastVal + 1;
            }

            *(depth_ptr++) = curVal; lastVal = curVal;

            numelements-=1;

        }
        else  // must be 01 prefix
        {
            uint8_t bit2 = bs_get(&bs, 1);

            if(bit2 == 0) // 010 --> multiple zeros!
            {
                uint16_t numZeros = bs_get(&bs, 5);

                numZeros += 5; // We never encode less than 5.

                for(int i = 0; i < numZeros; i++) {
                    *(depth_ptr++) = curVal;
                }

                numelements-=numZeros;
            }
            else
            {
                uint8_t bit3 = bs_get(&bs, 1);

                if(bit3 == 0) // 0110 -- DELTA!
                {
                    uint8_t delta_bit = bs_get(&bs, 1);

                    if(delta_bit == 0) {
                        curVal = lastVal - 2;
                    }
                    else {
                        curVal = lastVal + 2;
                    }

                    *(depth_ptr++) = curVal; lastVal = curVal;

                    numelements-=1;

                }
                else // 0111 -- RESET!
                {
                    uint16_t value = (bs_get(&bs, 3) << 8) | bs_get(&bs, 8); // 11 bits total.

                    curVal = value;

                    *(depth_ptr++) = curVal; lastVal = curVal;
                    numelements-=1;
                }

            }

        }

    }

    return depthimage;

}

//------------------------------------------------------------------------------

inline uint32_t encode(const uint16_t * data_in, int numelements,
                                  uint8_t* out_buffer, uint32_t out_buffer_size)
{
    int numZeros = 0;
    int lastVal = 0;
    
    bitstream_t bs;
    bs_init(&bs);
    bs_attach(&bs, out_buffer, out_buffer_size);

    // Loop over pixels.
    while (numelements > 0) {

        int curVal = *(data_in++);
        int delta = curVal - lastVal;

        if(delta == 0)
        {
            numZeros++;
        }
        else
        {
            if(numZeros > 0)
            {
                // MUST BURN ZEROS!
                while( numZeros > 0 )
                {
                    if(numZeros <= 4)
                    {
                        // Ternary is fastest way of deciding how many zeros to encode (2 * numZeros)
                        bs_put(&bs, 0x0000, numZeros == 1 ? 2 : numZeros == 2 ? 4 : numZeros == 3 ? 6 : 8);
                        numZeros = 0;
                    }
                    else
                    {
                        bs_put(&bs, 0x2, 3); // 010bbbbb

                        // We never encode less than 5 because in that case
                        //  we'll just use multiple 2-bit single zeros.
                        unsigned int numberToEncode = numZeros - 5;

                        // We're only using 5 bits, so we can't encode greater than 31.
                        if(numberToEncode > 31) numberToEncode = 31;

                        bs_put(&bs, numberToEncode, 5); // 0b 010

                        numZeros -= (numberToEncode+5);
                    }
                }

                // numZeros is now zero.
            }

            if(delta == 1 || delta == -1)
            {
                bs_put(&bs, delta == 1 ? 0x3 : 0x2, 2); // 0b 11
            }
            else if (delta >= -2 && delta <= 2)
            {
                bs_put(&bs, delta == 2 ? 0xD : 0xC, 5);
            }
            else // Reset == 1111 bbbbbbbbbbb
            {
                bs_put(&bs, 0x7, 4); // 0111
                bs_put(&bs, curVal >> 8, 3);
                bs_put(&bs, curVal , 8);
            }

        } // end else block of if (delta == 0)

        lastVal = curVal;

        numelements--;
    }

    // FINISH Up -- repeat zeros check.

    if(numZeros > 0)
    {
        // MUST BURN ZEROS!
        while(numZeros > 0)
        {
            if(numZeros <= 4)
            {
                // Ternary is fastest way of deciding how many zeros to encode (2 * numZeros)
                bs_put(&bs, 0x0000, numZeros == 1 ? 2 : numZeros == 2 ? 4 : numZeros == 3 ? 6 : 8);
                numZeros = 0;
            }
            else
            {
                bs_put(&bs, 0x2, 3); // 010bbbbb

                // We never encode less than 5 because in that case
                //  we'll just use multiple 2-bit single zeros.
                unsigned int numberToEncode = numZeros - 5;

                // We're only using 5 bits, so we can't encode greater than 31.
                if(numberToEncode > 31) numberToEncode = 31;

                bs_put(&bs, numberToEncode, 5); // 0b 010

                numZeros -= (numberToEncode+5);
            }
        }
    }

    // numZeros is now zero.

    // END FINISH UP


    bs_flush(&bs);
    return bs_bytes_used(&bs);
}

} // uplink namespace

