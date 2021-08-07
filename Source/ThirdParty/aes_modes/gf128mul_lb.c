/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 20/12/2007

 This file provides fast multiplication in GF(128) as required by several
 cryptographic authentication modes. The galois field representation is 
 LB (see gfmul128.h) with the following in memory layout.

    LB (favours BE)
    ===============

    BE-8
      x[0]    x[1]    x[2]    x[3]    x[4]    x[5]    x[6]   x[7]
    ms   ls ms   ls ms   ls ms   ls ms   ls ms   ls ms   ls ms   ls
    ....... ....... ....... ....... ....... ....... ....... .......
    00...07 08...15 16...23 24...31 32...39 40...47 48...55 56...63
      x[8]    x[9]   x[10]   x[11]   x[12]   x[13]   x[14]  x[15]
    ms   ls ms   ls ms   ls ms   ls ms   ls ms   ls ms   ls ms   ls
    ....... ....... ....... ....... ....... ....... ....... .......
    64...71 72...79 80...87 88...95 96..103 104.111 112.119 120.127
    
    BE-16
    ms    x[0]   ls ms    x[1]   ls ms    x[2]   ls ms    x[3]   ls
    ............... ............... ............... ...............
    00...07 08...15 16...23 24...31 32...39 40...47 48...55 56...63
    ms    x[4]   ls ms    x[5]   ls ms    x[6]   ls ms    x[7]   ls
    ............... ............... ............... ...............
    64...71 72...79 80...87 88...95 96..103 104.111 112.119 120.127
    
    BE-32
    ms            x[0]           ls ms            x[1]           ls 
    ............................... ...............................
    00...07 08...15 16...23 24...31 32...39 40...47 48...55 56...63
    ms            x[2]           ls ms            x[3]           ls
    ............................... ...............................
    64...71 72...79 80...87 88...95 96..103 104.111 112.119 120.127

    BE-64
    ms                            x[0]                           ls 
    ...............................................................
    00...07 08...15 16...23 24...31 32...39 40...47 48...55 56...63      
    ms                            x[1]                           ls
    ...............................................................
    64...71 72...79 80...87 88...95 96..103 104.111 112.119 120.127
    
    LE-16
    ms    x[0]   ls ms    x[1]   ls ms    x[2]   ls ms    x[3]   ls
    ............... ............... ............... ...............
    08...15 00...07 24...31 16...23 40...47 32...39 56...63 48...55
    ms    x[4]   ls ms    x[5]   ls ms    x[6]   ls ms    x[7]   ls
    ............... ............... ............... ...............
    72...79 64...71 88...95 80...87 104.111 96..103 120.127 112.119
    
    LE-32
    ms            x[0]           ls ms           x[1]            ls
    ............................... ...............................
    24...31 16...23 08...15 00...07 56...63 48...55 40...47 32...39 
    ms           x[2]            ls ms           x[3]            ls
    ............................... ...............................
    88...95 80...87 72...79 64...71 120.127 112.119 104.111 96..103

    LE-64
    ms                            x[0]                           ls
    ...............................................................
    56...63 48...55 40...47 32...39 24...31 16...23 08...15 00...07 
    ms                            x[1]                           ls
    ...............................................................
    120.127 112.119 104.111 96..103 88...95 80...87 72...79 64...71 
    
    These functions multiply a field element x, by x^4 and by x^8 in 
    the polynomial field representation. It uses 32-bit word operations 
    to gain speed but compensates for machine endianess and hence works 
    correctly on both styles of machine.
*/

#define GF_MODE_LB

/* Change Galois Field representation (for TESTING, not for production) */
#if 0
#  define CHANGE_GF_REPRESENTATION
#  define CONVERT (REVERSE_NONE)
#endif

#include "gf128mul.h"
#include "mode_hdr.h"
#include "gf_mul_lo.c"

/*  Speed critical loops can be unrolled to gain speed but consume more memory */
#if 1
#  define UNROLL_LOOPS
#endif

void gf_mul(gf_t a, const gf_t b)
{   gf_t p[8];
    uint_8t *q, ch;
    int i;
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
    if(a != b)
        convert_representation(b, b, CONVERT);
#endif
    q = (uint_8t*)(a == b ? p[0] : b);
    move_block_aligned(p[0], a);
    for(i = 0; i < 7; ++i)
        gf_mulx1_lb(p[i + 1], p[i]);

    memset(a, 0, GF_BYTE_LEN);
    for(i = 15;  ; )
    {   
        ch = q[i];
        if(ch & X_0)
            xor_block_aligned(a, a, p[0]);
        if(ch & X_1)
            xor_block_aligned(a, a, p[1]);
        if(ch & X_2)
            xor_block_aligned(a, a, p[2]);
        if(ch & X_3)
            xor_block_aligned(a, a, p[3]);
        if(ch & X_4)
            xor_block_aligned(a, a, p[4]);
        if(ch & X_5)
            xor_block_aligned(a, a, p[5]);
        if(ch & X_6)
            xor_block_aligned(a, a, p[6]);
        if(ch & X_7)
            xor_block_aligned(a, a, p[7]);
        if(!i--)
            break;
        gf_mulx8_lb(a);
    }
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
    if(a != b)
        convert_representation(b, b, CONVERT);
#endif
}

#if defined( TABLES_64K )

/*  This version uses 64k bytes of table space on the stack.
    An input field value in a[] has to be multiplied by a
    key value in g[]. To do this a[] is split up into 16
    smaller field values each one byte in length. For the 
    256 values of each of these smaller field values we can
    precompute the result of mulltiplying g by the field
    value in question. So for each of 16 bytes we have a 
    table of 256 field values, each of 16 bytes - 64k bytes
    in total.
*/

void init_64k_table(gf_t g, gf_t64k_t t)
{   int i = 0, j, k;

    /*  
    the byte value 0x80 at the lowest byte position in a[]
    is unity in this field representation g[] goes into 
    this position in the table. 0x40 corresponds to a field 
    value of 2 so we can determine this value by multiplying
    the 0x80 value by x, a process we can repeat for 8 field
    values.
    */
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(t[0][128], g, CONVERT);
#else
    memcpy(t[0][128], g, GF_BYTE_LEN);
#endif
    memset(t[0][0], 0, GF_BYTE_LEN);
    for(j = 64; j > 0; j >>= 1)
        gf_mulx1_lb(t[0][j], t[0][j + j]);

    for( ; ; )
    {
        /*  if { n } stands for the field value represented by
            the integer n, we can express higher multiplies in
            the table as follows:

                1. g * { 3} = g * {2} ^ g * {1}

                2. g * { 5} = g * {4} ^ g * {1}
                   g * { 6} = g * {4} ^ g * {2}
                   g * { 7} = g * {4} ^ g * {3}

                3. g * { 9} = g * {8} ^ g * {1}
                   g * {10} = g * {8} ^ g * {2}
                   ....
           and so on
        */
        for(j = 2; j < 256; j += j)
            for(k = 1; k < j; ++k)
                xor_block_aligned(t[i][j + k], t[i][j], t[i][k]);

        if(++i == GF_BYTE_LEN)  /* all 16 byte positions done */
            return;

        /*  We now move to the next byte up and set up its eight 
            starting values by multiplying the values in the 
            lower table by x^8
        */
        memset(t[i][0], 0, GF_BYTE_LEN);
        for(j = 128; j > 0; j >>= 1)
        {
            memcpy(t[i][j], t[i - 1][j], GF_BYTE_LEN);
            gf_mulx8_lb(t[i][j]);
        }
    }
}

#define xor_64k(i,ap,t,r) xor_block_aligned(r, r, t[i][ap[i]])

#if defined( UNROLL_LOOPS )

void gf_mul_64k(gf_t a, const  gf_t64k_t t, gf_t r)
{   uint_8t *ap = (uint_8t*)a;
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
    memset(r, 0, GF_BYTE_LEN);
    xor_64k(15, ap, t, r); xor_64k(14, ap, t, r); 
    xor_64k(13, ap, t, r); xor_64k(12, ap, t, r); 
    xor_64k(11, ap, t, r); xor_64k(10, ap, t, r); 
    xor_64k( 9, ap, t, r); xor_64k( 8, ap, t, r); 
    xor_64k( 7, ap, t, r); xor_64k( 6, ap, t, r); 
    xor_64k( 5, ap, t, r); xor_64k( 4, ap, t, r); 
    xor_64k( 3, ap, t, r); xor_64k( 2, ap, t, r); 
    xor_64k( 1, ap, t, r); xor_64k( 0, ap, t, r);
    move_block_aligned(a, r);
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
}

#else

void gf_mul_64k(gf_t a, const  gf_t64k_t t, gf_t r)
{   int i;
    uint_8t *ap = (uint_8t*)a;
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
    memset(r, 0, GF_BYTE_LEN);
    for(i = 15; i >= 0; --i)
    {
        xor_64k(i,ap,t,r);
    }
    move_block_aligned(a, r);
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
}

#endif

#endif

#if defined( TABLES_8K )

/*  This version uses 8k bytes of table space on the stack.
    An input field value in a[] has to be multiplied by a
    key value in g[]. To do this a[] is split up into 32
    smaller field values each 4-bits in length. For the 
    16 values of each of these smaller field values we can
    precompute the result of mulltiplying g[] by the field
    value in question. So for each of 32 nibbles we have a 
    table of 16 field values, each of 16 bytes - 8k bytes
    in total.
*/

void init_8k_table(gf_t g, gf_t8k_t t)
{   int i = 0, j, k;

    /*  do the high 4-bit nibble first - t[1][16] - and note
        that the unit multiplier sits at 0x80 - t[1][8] in 
        the table. Then multiplies by x go at 4, 2, 1
    */
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(t[1][8], g, CONVERT);
#else
    memcpy(t[1][8], g, GF_BYTE_LEN);
#endif
    memset(t[0][0], 0, GF_BYTE_LEN);
    memset(t[1][0], 0, GF_BYTE_LEN);
    for(j = 4; j > 0; j >>= 1)
        gf_mulx1_lb(t[1][j], t[1][j + j]);

    /*  now do the low nibble: g * {x^4} = x * g * {x^3} */
    gf_mulx1_lb(t[0][8], t[1][1]);
    for(j = 4; j > 0; j >>= 1)
        gf_mulx1_lb(t[0][j], t[0][j + j]);

    for( ; ; )
    {
        for(j = 2; j < 16; j += j)
            for(k = 1; k < j; ++k)
                xor_block_aligned(t[i][j + k], t[i][j], t[i][k]);

        if(++i == 2 * GF_BYTE_LEN)
            return;

        if(i > 1)
        {
            memset(t[i][0], 0, GF_BYTE_LEN);
            for(j = 8; j > 0; j >>= 1)
            {
                memcpy(t[i][j], t[i - 2][j], GF_BYTE_LEN);
                gf_mulx8_lb(t[i][j]);
            }
        }

    }
}

#define xor_8k(i,ap,t,r)   \
    xor_block_aligned(r, r, t[i + i][ap[i] & 15]); \
    xor_block_aligned(r, r, t[i + i + 1][ap[i] >> 4])

#if defined( UNROLL_LOOPS )

void gf_mul_8k(gf_t a, const gf_t8k_t t, gf_t r)
{   uint_8t *ap = (uint_8t*)a; 
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
    memset(r, 0, GF_BYTE_LEN);
    xor_8k(15, ap, t, r); xor_8k(14, ap, t, r); 
    xor_8k(13, ap, t, r); xor_8k(12, ap, t, r);
    xor_8k(11, ap, t, r); xor_8k(10, ap, t, r); 
    xor_8k( 9, ap, t, r); xor_8k( 8, ap, t, r);
    xor_8k( 7, ap, t, r); xor_8k( 6, ap, t, r); 
    xor_8k( 5, ap, t, r); xor_8k( 4, ap, t, r);
    xor_8k( 3, ap, t, r); xor_8k( 2, ap, t, r); 
    xor_8k( 1, ap, t, r); xor_8k( 0, ap, t, r);
    move_block_aligned(a, r);
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
}

#else

void gf_mul_8k(gf_t a, const gf_t8k_t t, gf_t r)
{   int i;
    uint_8t *ap = (uint_8t*)a; 
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
    memset(r, 0, GF_BYTE_LEN);
    for(i = 15; i >= 0; --i)
    {
        xor_8k(i,ap,t,r);
    }
    memcpy(a, r, GF_BYTE_LEN);
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
}

#endif

#endif

#if defined( TABLES_4K )

/*  This version uses 4k bytes of table space on the stack.
    A 16 byte buffer has to be multiplied by a 16 byte key
    value in GF(128).  If we consider a GF(128) value in a
    single byte, we can construct a table of the 256 16 byte
    values that result from the 256 values of this byte.
    This requires 4096 bytes. If we take the highest byte in
    the buffer and use this table to get the result, we then
    have to multiply by x^120 to get the final value. For the
    next highest byte the result has to be multiplied by x^112
    and so on. But we can do this by accumulating the result
    in an accumulator starting with the result for the top
    byte.  We repeatedly multiply the accumulator value by
    x^8 and then add in (i.e. xor) the 16 bytes of the next
    lower byte in the buffer, stopping when we reach the
    lowest byte. This requires a 4096 byte table.
*/

void init_4k_table(gf_t g, gf_t4k_t t)
{   int j, k;

#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(t[128], g, CONVERT);
#else
    memcpy(t[128], g, GF_BYTE_LEN);
#endif
    memset(t[0], 0, GF_BYTE_LEN);

    for(j = 64; j > 0; j >>= 1)
        gf_mulx1_lb(t[j], t[j + j]);

    for(j = 2; j < 256; j += j)
        for(k = 1; k < j; ++k)
            xor_block_aligned(t[j + k], t[j], t[k]);
}

#define xor_4k(i,ap,t,r) gf_mulx8_lb(r); xor_block_aligned(r, r, t[ap[i]])

#if defined( UNROLL_LOOPS )

void gf_mul_4k(gf_t a, const gf_t4k_t t, gf_t r)
{   uint_8t *ap = (uint_8t*)a; 
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
    memset(r, 0, GF_BYTE_LEN);
    xor_4k(15, ap, t, r); xor_4k(14, ap, t, r); 
    xor_4k(13, ap, t, r); xor_4k(12, ap, t, r);
    xor_4k(11, ap, t, r); xor_4k(10, ap, t, r); 
    xor_4k( 9, ap, t, r); xor_4k( 8, ap, t, r); 
    xor_4k( 7, ap, t, r); xor_4k( 6, ap, t, r);
    xor_4k( 5, ap, t, r); xor_4k( 4, ap, t, r); 
    xor_4k( 3, ap, t, r); xor_4k( 2, ap, t, r); 
    xor_4k( 1, ap, t, r); xor_4k( 0, ap, t, r);
    move_block_aligned(a, r);
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
}

#else

void gf_mul_4k(gf_t a, const gf_t4k_t t, gf_t r)
{   int i = 15;
    uint_8t *ap = (uint_8t*)a;     
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
    memset(r, 0, GF_BYTE_LEN);
    for(i = 15; i >=0; --i)
    {
        xor_4k(i, ap, t, r);
    }
    move_block_aligned(a, r);
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
}

#endif

#endif

#if defined( TABLES_256 )

/*  This version uses 256 bytes of table space on the stack.
    A 16 byte buffer has to be multiplied by a 16 byte key
    value in GF(128).  If we consider a GF(128) value in a
    single 4-bit nibble, we can construct a table of the 16
    16 byte  values that result from the 16 values of this
    byte.  This requires 256 bytes. If we take the highest
    4-bit nibble in the buffer and use this table to get the
    result, we then have to multiply by x^124 to get the
    final value. For the next highest byte the result has to
    be multiplied by x^120 and so on. But we can do this by
    accumulating the result in an accumulator starting with
    the result for the top nibble.  We repeatedly multiply
    the accumulator value by x^4 and then add in (i.e. xor)
    the 16 bytes of the next lower nibble in the buffer,
    stopping when we reach the lowest nibblebyte. This uses
    a 256 byte table.
*/

void init_256_table(gf_t g, gf_t256_t t)
{   int j, k;

#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(t[8], g, CONVERT);
#else
    memcpy(t[8], g, GF_BYTE_LEN);
#endif
    memset(t[0], 0, GF_BYTE_LEN);
    for(j = 4; j > 0; j >>= 1)
        gf_mulx1_lb(t[j], t[j + j]);

    for(j = 2; j < 16; j += j)
        for(k = 1; k < j; ++k)
            xor_block_aligned(t[j + k], t[j], t[k]);
}

#define xor_256(i,ap,t,r)    \
    gf_mulx4_lb(r); xor_block_aligned(r, r, t[ap[i] & 15]);  \
    gf_mulx4_lb(r); xor_block_aligned(r, r, t[ap[i] >> 4])

#if defined( UNROLL_LOOPS )

void gf_mul_256(gf_t a, const gf_t256_t t, gf_t r)
{   uint_8t *ap = (uint_8t*)a;
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
    memset(r, 0, GF_BYTE_LEN);
    xor_256(15, ap, t, r); xor_256(14, ap, t, r); 
    xor_256(13, ap, t, r); xor_256(12, ap, t, r); 
    xor_256(11, ap, t, r); xor_256(10, ap, t, r); 
    xor_256( 9, ap, t, r); xor_256( 8, ap, t, r); 
    xor_256( 7, ap, t, r); xor_256( 6, ap, t, r); 
    xor_256( 5, ap, t, r); xor_256( 4, ap, t, r); 
    xor_256( 3, ap, t, r); xor_256( 2, ap, t, r); 
    xor_256( 1, ap, t, r); xor_256( 0, ap, t, r); 
    move_block_aligned(a, r);
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
}

#else

void gf_mul_256(gf_t a, const gf_t256_t t, gf_t r)
{   int i;
    uint_8t *ap = (uint_8t*)a;
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
    memset(r, 0, GF_BYTE_LEN);
    for(i = 15; i >= 0; --i)
    {
        xor_256(i, ap, t, r);
    }
    move_block_aligned(a, r);
#ifdef CHANGE_GF_REPRESENTATION
    convert_representation(a, a, CONVERT);
#endif
}

#endif

#endif
