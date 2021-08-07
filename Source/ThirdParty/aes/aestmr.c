/*
---------------------------------------------------------------------------
Copyright (c) 1998-2013, Brian Gladman, Worcester, UK. All rights reserved.

The redistribution and use of this software (with or without changes)
is allowed without the payment of fees or royalties provided that:

  source code distributions include the above copyright notice, this
  list of conditions and the following disclaimer;

  binary distributions include the above copyright notice, this list
  of conditions and the following disclaimer in their documentation.

This software is provided 'as is' with no explicit or implied warranties
in respect of its operation, including, but not limited to, correctness
and fitness for purpose.
---------------------------------------------------------------------------
Issue Date: 20/12/2007
*/

/* Measure the Encryption, Decryption and Key Setup Times for AES using
   the Pentium Time Stamp Counter  */

#define DUAL_CORE

#if defined( _MSC_VER ) && (defined( DUAL_CORE ) || defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD ))
#  define WINDOWS_LEAN_AND_MEAN
#  include <windows.h>
#elif defined(__GNUC__)
#  define _GNU_SOURCE
#  include <sched.h>
#endif

#include <string.h>
#include <math.h>

#if defined( __cplusplus )
#  include "aescpp.h"
#else
#  include "aes.h"
#endif
#include "aesaux.h"
#include "aestst.h"
#include "rdtsc.h"

#if defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )
fn_ptrs fn;
#endif

#if defined( _M_IX86 )
#  if _M_IX86 == 500
#    define PROCESSOR   "Pentium"
#  elif _M_IX86 == 600
#    define PROCESSOR   "P2/P3/P4"
#  else
#    define PROCESSOR   ""
#  endif
#elif defined( _M_X64 ) || defined( __x86_64__ )
#  define PROCESSOR   "AMD64/EMT64"
#else
#define PROCESSOR   "(Unknown)"
#endif

const int loops = 100; // number of timing loops

#define SAMPLE1  1000
#define SAMPLE2 10000

int time_base(double *av, double *sig)
{   int                 i, tol, lcnt, sam_cnt;
    double              cy, av1, sig1;

    tol = 10; lcnt = sam_cnt = 0;
    while(!sam_cnt)
    {
        av1 = sig1 = 0.0;

        for(i = 0; i < SAMPLE1; ++i)
        {
            cy = (double)read_tsc();
            cy = (double)read_tsc() - cy;

            av1 += cy;
            sig1 += cy * cy;
        }

        av1 /= SAMPLE1;
        sig1 = sqrt((sig1 - av1 * av1 * SAMPLE1) / SAMPLE1);
        sig1 = (sig1 < 0.05 * av1 ? 0.05 * av1 : sig1);

        *av = *sig = 0.0;
        for(i = 0; i < SAMPLE2; ++i)
        {
            cy = (double)read_tsc();
            cy = (double)read_tsc() - cy;

            if(cy > av1 - sig1 && cy < av1 + sig1)
            {
                *av += cy;
                *sig += cy * cy;
                sam_cnt++;
            }
        }

        if(10 * sam_cnt > 9 * SAMPLE2)
        {
            *av /= sam_cnt;
            *sig = sqrt((*sig - *av * *av * sam_cnt) / sam_cnt);
            if(*sig > (tol / 100.0) * *av)
                sam_cnt = 0;
        }
        else
        {
            if(lcnt++ == 10)
            {
                lcnt = 0; tol += 5;
                if(tol > 30)
                    return FALSE;
            }
            sam_cnt = 0;
        }
    }

    return TRUE;
}

typedef unsigned long long (*aes_tmr)(void);

/*  There is a compiler optimisation bug when optimisation is turned on
    in Visual Studio 2005 for the following subroutine
*/
#if defined( _MSC_VER )
#  pragma optimize( "", off )
#endif

int time_eks8(unsigned int k_len, double *av, double *sig)
{   int                 i, tol, lcnt, sam_cnt;
    double              cy, av1, sig1;
    unsigned char       key[8][2 * AES_BLOCK_SIZE];
    f_ectx              ec[1];

    block_rndfill(key[0], 16 * AES_BLOCK_SIZE);

    tol = 10; lcnt = sam_cnt = 0;
    while(!sam_cnt)
    {
        av1 = sig1 = 0.0;

        for(i = 0; i < SAMPLE1; ++i)
        {
            cy = (double)read_tsc();
            f_enc_key(ec, key[0], k_len);
            f_enc_key(ec, key[1], k_len);
            f_enc_key(ec, key[2], k_len);
            f_enc_key(ec, key[3], k_len);
            f_enc_key(ec, key[4], k_len);
            f_enc_key(ec, key[5], k_len);
            f_enc_key(ec, key[6], k_len);
            f_enc_key(ec, key[7], k_len);
            cy = (double)read_tsc() - cy;

            av1 += cy;
            sig1 += cy * cy;
        }

        av1 /= SAMPLE1;
        sig1 = sqrt((sig1 - av1 * av1 * SAMPLE1) / SAMPLE1);
        sig1 = (sig1 < 0.05 * av1 ? 0.05 * av1 : sig1);

        f_enc_key(ec, key[0], k_len);
        *av = *sig = 0.0;
        for(i = 0; i < SAMPLE2; ++i)
        {
            cy = (double)read_tsc();
            f_enc_key(ec, key[0], k_len);
            f_enc_key(ec, key[1], k_len);
            f_enc_key(ec, key[2], k_len);
            f_enc_key(ec, key[3], k_len);
            f_enc_key(ec, key[4], k_len);
            f_enc_key(ec, key[5], k_len);
            f_enc_key(ec, key[6], k_len);
            f_enc_key(ec, key[7], k_len);
            cy = (double)read_tsc() - cy;

            if(cy > av1 - sig1 && cy < av1 + sig1)
            {
                *av += cy;
                *sig += cy * cy;
                sam_cnt++;
            }
        }

        if(10 * sam_cnt > 9 * SAMPLE2)
        {
            *av /= sam_cnt;
            *sig = sqrt((*sig - *av * *av * sam_cnt) / sam_cnt);
            if(*sig > (tol / 100.0) * *av)
                sam_cnt = 0;
        }
        else
        {
            if(lcnt++ == 10)
            {
                lcnt = 0; tol += 5;
                if(tol > 30)
                    return FALSE;
            }
            sam_cnt = 0;
        }
    }

    return TRUE;
}

#if defined( _MSC_VER )
#  pragma optimize( "", on )
#endif

int time_dks8(unsigned int k_len, double *av, double *sig)
{   int                 i, tol, lcnt, sam_cnt;
    double              cy, av1, sig1;
    unsigned char       key[8][2 * AES_BLOCK_SIZE];
    f_dctx              dc[1];

    block_rndfill(key[0], 16 * AES_BLOCK_SIZE);

    tol = 10; lcnt = sam_cnt = 0;
    while(!sam_cnt)
    {
        av1 = sig1 = 0.0;

        for(i = 0; i < SAMPLE1; ++i)
        {
            cy = (double)read_tsc();
            f_dec_key(dc, key[ 0], k_len);
            f_dec_key(dc, key[ 1], k_len);
            f_dec_key(dc, key[ 2], k_len);
            f_dec_key(dc, key[ 3], k_len);
            f_dec_key(dc, key[ 4], k_len);
            f_dec_key(dc, key[ 5], k_len);
            f_dec_key(dc, key[ 6], k_len);
            f_dec_key(dc, key[ 7], k_len);
            cy = (double)read_tsc() - cy;

            av1 += cy;
            sig1 += cy * cy;
        }

        av1 /= SAMPLE1;
        sig1 = sqrt((sig1 - av1 * av1 * SAMPLE1) / SAMPLE1);
        sig1 = (sig1 < 0.05 * av1 ? 0.05 * av1 : sig1);

        *av = *sig = 0.0;
        for(i = 0; i < SAMPLE2; ++i)
        {
            cy = (double)read_tsc();
            f_dec_key(dc, key[ 0], k_len);
            f_dec_key(dc, key[ 1], k_len);
            f_dec_key(dc, key[ 2], k_len);
            f_dec_key(dc, key[ 3], k_len);
            f_dec_key(dc, key[ 4], k_len);
            f_dec_key(dc, key[ 5], k_len);
            f_dec_key(dc, key[ 6], k_len);
            f_dec_key(dc, key[ 7], k_len);
            cy = (double)read_tsc() - cy;

            if(cy > av1 - sig1 && cy < av1 + sig1)
            {
                *av += cy;
                *sig += cy * cy;
                sam_cnt++;
            }
        }

        if(10 * sam_cnt > 9 * SAMPLE2)
        {
            *av /= sam_cnt;
            *sig = sqrt((*sig - *av * *av * sam_cnt) / sam_cnt);
            if(*sig > (tol / 100.0) * *av)
                sam_cnt = 0;
        }
        else
        {
            if(lcnt++ == 10)
            {
                lcnt = 0; tol += 5;
                if(tol > 30)
                    return FALSE;
            }
            sam_cnt = 0;
        }
    }

    return TRUE;
}

int time_enc16(unsigned int k_len, double *av, double *sig)
{   int                 i, tol, lcnt, sam_cnt;
    double              cy, av1, sig1;
    unsigned char       key[2 * AES_BLOCK_SIZE], pt[4][4 * AES_BLOCK_SIZE];
    f_ectx              ec[1];

    block_rndfill(key, 2 * AES_BLOCK_SIZE);
    f_enc_key(ec, key, k_len);
    block_rndfill(pt[0], 16 * AES_BLOCK_SIZE);

    tol = 10; lcnt = sam_cnt = 0;
    while(!sam_cnt)
    {
        av1 = sig1 = 0.0;

        for(i = 0; i < SAMPLE1; ++i)
        {
            cy = (double)read_tsc();
            do_enc(ec, pt[0], pt[0], 1);
            do_enc(ec, pt[1], pt[1], 1);
            do_enc(ec, pt[2], pt[2], 1);
            do_enc(ec, pt[3], pt[3], 1);
            do_enc(ec, pt[0], pt[0], 1);
            do_enc(ec, pt[1], pt[1], 1);
            do_enc(ec, pt[2], pt[2], 1);
            do_enc(ec, pt[3], pt[3], 1);
            do_enc(ec, pt[0], pt[0], 1);
            do_enc(ec, pt[1], pt[1], 1);
            do_enc(ec, pt[2], pt[2], 1);
            do_enc(ec, pt[3], pt[3], 1);
            do_enc(ec, pt[0], pt[0], 1);
            do_enc(ec, pt[1], pt[1], 1);
            do_enc(ec, pt[2], pt[2], 1);
            do_enc(ec, pt[3], pt[3], 1);
            cy = (double)read_tsc() - cy;

            av1 += cy;
            sig1 += cy * cy;
        }

        av1 /= SAMPLE1;
        sig1 = sqrt((sig1 - av1 * av1 * SAMPLE1) / SAMPLE1);
        sig1 = (sig1 < 0.05 * av1 ? 0.05 * av1 : sig1);

        *av = *sig = 0.0;
        for(i = 0; i < SAMPLE2; ++i)
        {
            cy = (double)read_tsc();
            do_enc(ec, pt[0], pt[0], 1);
            do_enc(ec, pt[1], pt[1], 1);
            do_enc(ec, pt[2], pt[2], 1);
            do_enc(ec, pt[3], pt[3], 1);
            do_enc(ec, pt[0], pt[0], 1);
            do_enc(ec, pt[1], pt[1], 1);
            do_enc(ec, pt[2], pt[2], 1);
            do_enc(ec, pt[3], pt[3], 1);
            do_enc(ec, pt[0], pt[0], 1);
            do_enc(ec, pt[1], pt[1], 1);
            do_enc(ec, pt[2], pt[2], 1);
            do_enc(ec, pt[3], pt[3], 1);
            do_enc(ec, pt[0], pt[0], 1);
            do_enc(ec, pt[1], pt[1], 1);
            do_enc(ec, pt[2], pt[2], 1);
            do_enc(ec, pt[3], pt[3], 1);
            cy = (double)read_tsc() - cy;

            if(cy > av1 - sig1 && cy < av1 + sig1)
            {
                *av += cy;
                *sig += cy * cy;
                sam_cnt++;
            }
        }

        if(10 * sam_cnt > 9 * SAMPLE2)
        {
            *av /= sam_cnt;
            *sig = sqrt((*sig - *av * *av * sam_cnt) / sam_cnt);
            if(*sig > (tol / 100.0) * *av)
                sam_cnt = 0;
        }
        else
        {
            if(lcnt++ == 10)
            {
                lcnt = 0; tol += 5;
                if(tol > 30)
                    return FALSE;
            }
            sam_cnt = 0;
        }
    }

    return TRUE;
}

int time_dec16(unsigned int k_len, double *av, double *sig)
{   int                 i, tol, lcnt, sam_cnt;
    double              cy, av1, sig1;
    unsigned char       key[2 * AES_BLOCK_SIZE], pt[4][4 * AES_BLOCK_SIZE];
    f_dctx              dc[1];

    block_rndfill(key, 2 * AES_BLOCK_SIZE);
    f_dec_key(dc, key, k_len);
    block_rndfill(pt[0], 16 * AES_BLOCK_SIZE);

    tol = 10; lcnt = sam_cnt = 0;
    while(!sam_cnt)
    {
        av1 = sig1 = 0.0;

        for(i = 0; i < SAMPLE1; ++i)
        {
            cy = (double)read_tsc();
            do_dec(dc, pt[0], pt[0], 1);
            do_dec(dc, pt[1], pt[1], 1);
            do_dec(dc, pt[2], pt[2], 1);
            do_dec(dc, pt[3], pt[3], 1);
            do_dec(dc, pt[0], pt[0], 1);
            do_dec(dc, pt[1], pt[1], 1);
            do_dec(dc, pt[2], pt[2], 1);
            do_dec(dc, pt[3], pt[3], 1);
            do_dec(dc, pt[0], pt[0], 1);
            do_dec(dc, pt[1], pt[1], 1);
            do_dec(dc, pt[2], pt[2], 1);
            do_dec(dc, pt[3], pt[3], 1);
            do_dec(dc, pt[0], pt[0], 1);
            do_dec(dc, pt[1], pt[1], 1);
            do_dec(dc, pt[2], pt[2], 1);
            do_dec(dc, pt[3], pt[3], 1);
            cy = (double)read_tsc() - cy;

            av1 += cy;
            sig1 += cy * cy;
        }

        av1 /= SAMPLE1;
        sig1 = sqrt((sig1 - av1 * av1 * SAMPLE1) / SAMPLE1);
        sig1 = (sig1 < 0.05 * av1 ? 0.05 * av1 : sig1);

        *av = *sig = 0.0;
        for(i = 0; i < SAMPLE2; ++i)
        {
            cy = (double)read_tsc();
            do_dec(dc, pt[0], pt[0], 1);
            do_dec(dc, pt[1], pt[1], 1);
            do_dec(dc, pt[2], pt[2], 1);
            do_dec(dc, pt[3], pt[3], 1);
            do_dec(dc, pt[0], pt[0], 1);
            do_dec(dc, pt[1], pt[1], 1);
            do_dec(dc, pt[2], pt[2], 1);
            do_dec(dc, pt[3], pt[3], 1);
            do_dec(dc, pt[0], pt[0], 1);
            do_dec(dc, pt[1], pt[1], 1);
            do_dec(dc, pt[2], pt[2], 1);
            do_dec(dc, pt[3], pt[3], 1);
            do_dec(dc, pt[0], pt[0], 1);
            do_dec(dc, pt[1], pt[1], 1);
            do_dec(dc, pt[2], pt[2], 1);
            do_dec(dc, pt[3], pt[3], 1);
            cy = (double)read_tsc() - cy;

            if(cy > av1 - sig1 && cy < av1 + sig1)
            {
                *av += cy;
                *sig += cy * cy;
                sam_cnt++;
            }
        }

        if(10 * sam_cnt > 9 * SAMPLE2)
        {
            *av /= sam_cnt;
            *sig = sqrt((*sig - *av * *av * sam_cnt) / sam_cnt);
            if(*sig > (tol / 100.0) * *av)
                sam_cnt = 0;
        }
        else
        {
            if(lcnt++ == 10)
            {
                lcnt = 0; tol += 5;
                if(tol > 30)
                    return FALSE;
            }
            sam_cnt = 0;
        }
    }

    return TRUE;
}

// measure cycles for an encryption call

double e_cycles(const unsigned long klen, f_ectx alg[1])
{   unsigned char  pt[16], ct[16], key[32];
    double cy1, cy2, c1 = -1, c2 = -1;
    int i;

    // set up a random key of 256 bits

    block_rndfill(key, 32);

    // set up a random plain text

    block_rndfill(pt, 16);

    // do a set_key in case it is necessary

    f_enc_key(alg, key, klen); c1 = c2 = 0xffffffff;

    // do an encrypt to remove any 'first time through' effects

    do_enc(alg, pt, ct, 1);

    for(i = 0; i < loops; ++i)
    {
        block_rndfill(pt, 16);

        // time 1 and 9 encryptions - subtract to get cost of 8
        cy1 = (double)read_tsc();
        do_enc(alg, ct, ct, 1);
        cy1 = (double)read_tsc() - cy1;

        cy2 = (double)read_tsc();
        do_enc(alg, ct, ct, 1);
        do_enc(alg, ct, ct, 1);
        do_enc(alg, ct, ct, 1);
        do_enc(alg, ct, ct, 1);
        do_enc(alg, ct, ct, 1);
        do_enc(alg, ct, ct, 1);
        do_enc(alg, ct, ct, 1);
        do_enc(alg, ct, ct, 1);
        do_enc(alg, ct, ct, 1);
        cy2 = (double)read_tsc() - cy2;

        if(i > loops / 10)
        {
            c1 = (c1 < cy1 ? c1 : cy1); // find minimum values over the loops
            c2 = (c2 < cy2 ? c2 : cy2);
        }
    }

    return ((c2 - c1) + 4.0) / 8.0;
}

// measure cycles for a decryption call

double d_cycles(const unsigned long klen, f_dctx alg[1])
{   unsigned char  pt[16], ct[16], key[32];
    double cy1, cy2, c1 = -1, c2 = -1;
    int i;

    // set up a random key of 256 bits

    block_rndfill(key, 32);

    // set up a random plain text

    block_rndfill(pt, 16);

    // do a set_key in case it is necessary

    f_dec_key(alg, key, klen); c1 = c2 = 0xffffffff;

    // do an decrypt to remove any 'first time through' effects

    do_dec(alg, pt, ct, 1);

    for(i = 0; i < loops; ++i)
    {
        block_rndfill(pt, 16);

        // time 1 and 9 decryptions - subtract to get cost of 8
        cy1 = (double)read_tsc();
        do_dec(alg, ct, ct, 1);
        cy1 = (double)read_tsc() - cy1;

        cy2 = (double)read_tsc();
        do_dec(alg, ct, ct, 1);
        do_dec(alg, ct, ct, 1);
        do_dec(alg, ct, ct, 1);
        do_dec(alg, ct, ct, 1);
        do_dec(alg, ct, ct, 1);
        do_dec(alg, ct, ct, 1);
        do_dec(alg, ct, ct, 1);
        do_dec(alg, ct, ct, 1);
        do_dec(alg, ct, ct, 1);
        cy2 = (double)read_tsc() - cy2;

        if(i > loops / 10)
        {
            c1 = (c1 < cy1 ? c1 : cy1); // find minimum values over the loops
            c2 = (c2 < cy2 ? c2 : cy2);
        }
    }

    return ((c2 - c1) + 4.0) / 8.0;
}

// measure cycles for an encryption key setup

double ke_cycles(const unsigned long klen, f_ectx alg[1])
{   unsigned char  key[32];
    double cy1, cy2, c1 = -1, c2 = -1;
    int i;

    // set up a random key of 256 bits

    block_rndfill(key, 32);

    // do an set_key to remove any 'first time through' effects

    f_enc_key(alg, key, klen); c1 = c2 = 0xffffffff;

    switch(klen)
    {
    case 16:
        for(i = 0; i < loops; ++i)
        {
            // time 1 and 5 encryption key schedules - subtract to get cost of 4
            cy1 = (double)read_tsc();
            f_enc_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            cy1 = (double)read_tsc() - cy1;

            cy2 = (double)read_tsc();
            f_enc_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            f_enc_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            f_enc_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            f_enc_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            f_enc_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            cy2 = (double)read_tsc() - cy2;

            if(i > loops / 10)
            {
                c1 = (c1 < cy1 ? c1 : cy1); // find minimum values over the loops
                c2 = (c2 < cy2 ? c2 : cy2);
            }
        }
        break;
    case 24:
        for(i = 0; i < loops; ++i)
        {
            // time 1 and 5 encryption key schedules - subtract to get cost of 4
            cy1 = (double)read_tsc();
            f_enc_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            cy1 = (double)read_tsc() - cy1;

            cy2 = (double)read_tsc();
            f_enc_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            f_enc_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            f_enc_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            f_enc_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            f_enc_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            cy2 = (double)read_tsc() - cy2;

            if(i > loops / 10)
            {
                c1 = (c1 < cy1 ? c1 : cy1); // find minimum values over the loops
                c2 = (c2 < cy2 ? c2 : cy2);
            }
        }
        break;
    case 32:
        for(i = 0; i < loops; ++i)
        {
            // time 1 and 5 encryption key schedules - subtract to get cost of 4
            cy1 = (double)read_tsc();
            f_enc_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            cy1 = (double)read_tsc() - cy1;

            cy2 = (double)read_tsc();
            f_enc_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            f_enc_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            f_enc_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            f_enc_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            f_enc_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            cy2 = (double)read_tsc() - cy2;

            if(i > loops / 10)
            {
                c1 = (c1 < cy1 ? c1 : cy1); // find minimum values over the loops
                c2 = (c2 < cy2 ? c2 : cy2);
            }
        }
        break;
    }

    return ((c2 - c1) + 2.0) / 4.0;
}

// measure cycles for an encryption key setup

double kd_cycles(const unsigned long klen, f_dctx alg[1])
{   unsigned char  key[32];
    double cy1, cy2, c1 = -1, c2 = -1;
    int i;

    // set up a random key of 256 bits

    block_rndfill(key, 32);

    // do an set_key to remove any 'first time through' effects

    f_dec_key(alg, key, klen); c1 = c2 = 0xffffffff;

    switch(klen)
    {
    case 16:
        for(i = 0; i < loops; ++i)
        {
            // time 1 and 5 decryption key schedules - subtract to get cost of 4
            cy1 = (double)read_tsc();
            f_dec_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            cy1 = (double)read_tsc() - cy1;

            cy2 = (double)read_tsc();
            f_dec_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            f_dec_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            f_dec_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            f_dec_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            f_dec_key(alg, key + 10 * AES_BLOCK_SIZE, 128);
            cy2 = (double)read_tsc() - cy2;

            if(i > loops / 10)
            {
                c1 = (c1 < cy1 ? c1 : cy1); // find minimum values over the loops
                c2 = (c2 < cy2 ? c2 : cy2);
            }
        }
        break;
    case 24:
        for(i = 0; i < loops; ++i)
        {
            // time 1 and 5 decryption key schedules - subtract to get cost of 4
            cy1 = (double)read_tsc();
            f_dec_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            cy1 = (double)read_tsc() - cy1;

            cy2 = (double)read_tsc();
            f_dec_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            f_dec_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            f_dec_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            f_dec_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            f_dec_key(alg, key + 23 * (AES_BLOCK_SIZE / 2), 192);
            cy2 = (double)read_tsc() - cy2;

            if(i > loops / 10)
            {
                c1 = (c1 < cy1 ? c1 : cy1); // find minimum values over the loops
                c2 = (c2 < cy2 ? c2 : cy2);
            }
        }
        break;
    case 32:
        for(i = 0; i < loops; ++i)
        {
            // time 1 and 5 decryption key schedules - subtract to get cost of 4
            cy1 = (double)read_tsc();
            f_dec_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            cy1 = (double)read_tsc() - cy1;

            cy2 = (double)read_tsc();
            f_dec_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            f_dec_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            f_dec_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            f_dec_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            f_dec_key(alg, key + 13 * AES_BLOCK_SIZE, 256);
            cy2 = (double)read_tsc() - cy2;

            if(i > loops / 10)
            {
                c1 = (c1 < cy1 ? c1 : cy1); // find minimum values over the loops
                c2 = (c2 < cy2 ? c2 : cy2);
            }
        }
        break;
    }

    return ((c2 - c1) + 2.0) / 4.0;
}

static unsigned long kl[3] = { 16, 24, 32 };
static double ekt[3], dkt[3], et[3], dt[3];

void output(FILE *outf, const unsigned long inx, const unsigned long bits)
{   unsigned long  t;
    unsigned char  c0, c1, c2;

    fprintf(outf, "\n// %lu Bit: ", 8 * kl[inx]);
    fprintf(outf, "   Key Setup: %f/%f cycles", ekt[inx], dkt[inx]);
    t = (unsigned long)((1000 * bits + et[inx] / 2) / et[inx]);
    c0 = (unsigned char)('0' + t / 100);
    c1 = (unsigned char)('0' + (t / 10) % 10);
    c2 = (unsigned char)('0' + t % 10);
    fprintf(outf, "\n// Encrypt:   %f  cycles = 0.%c%c%c bits/cycle", et[inx], c0, c1 , c2);
    t = (unsigned long)((1000 * bits + dt[inx] / 2) / dt[inx]);
    c0 = (unsigned char)('0' + t / 100);
    c1 = (unsigned char)('0' + (t / 10) % 10);
    c2 = (unsigned char)('0' + t % 10);
    fprintf(outf, "\n// Decrypt:   %f  cycles = 0.%c%c%c bits/cycle", dt[inx], c0, c1 , c2);
}

#if defined( _WIN64 )

#define CurrentProcessorNumber GetCurrentProcessorNumber

#elif defined( _WIN32 )

unsigned long CurrentProcessorNumber(void)
{
    __asm
    {
        mov     eax,1
        cpuid
        shr     ebx,24
        mov     eax, ebx
    }
}

#endif

int main(int argc, char *argv[])
{   FILE     *outf;
    f_ectx   alge[1];
    f_dctx   algd[1];
    double   a0, av, sig;
    int ki, i, w;

#if defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )
    HINSTANCE   h_dll;
#endif

#if defined( DUAL_CORE ) && defined( _WIN32 )
    // we need to constrain the process to one core in order to
    // obtain meaningful timing data
    HANDLE ph;
    DWORD_PTR afp;
    DWORD_PTR afs;
    ph = GetCurrentProcess();
    if(GetProcessAffinityMask(ph, &afp, &afs))
    {
        afp &= (1 << CurrentProcessorNumber());
        if(!SetProcessAffinityMask(ph, afp))
        {
            printf("Couldn't set Process Affinity Mask\n\n"); return -1;
        }
    }
    else
    {
        printf("Couldn't get Process Affinity Mask\n\n"); return -1;
    }
#elif defined( DUAL_CORE ) && defined( __GNUC__ )
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(0, &cpu_set);
	if(sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set) == -1){
		perror("Unable to set CPU affinity mask"); return -1;
}
#endif

#if defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )
    if(!(h_dll = init_dll(&fn)))
        return -1;
#else 
    aes_init();
#endif

    memset(&alge, 0, sizeof(aes_encrypt_ctx));
    memset(&algd, 0, sizeof(aes_decrypt_ctx));

	if(argc == 2)		
	{
		if(fopen_s(&outf, argv[1], "w"))
		{
			printf("\nCannot open %s for output\n", argc == 2 ? argv[1] : "CON");
			return -1;
		}
	}
	else
		   outf = stdout;

    printf("\n// AES%s Timing For The %s Processor",
#if defined( DLL_IMPORT ) && defined( __cplusplus )
        " (DLL & CPP)",
#elif defined(  DLL_IMPORT )
        " (DLL)",
#elif defined( __cplusplus )
        " (CPP)",
#else
        "",
#endif
    PROCESSOR);

    for(ki = 0; ki < 3; ++ki)
    {
        ekt[ki] = ke_cycles(kl[ki], alge);
        dkt[ki] = kd_cycles(kl[ki], algd);
        et[ki]  =  e_cycles(kl[ki], alge);
        dt[ki]  =  d_cycles(kl[ki], algd);
    }

    fprintf(outf, "\n// KeySize     EKS(cycles)  ENC(cycles/byte)    DKS(cycles)  DEC(cycles/byte)");

    for(i = 0; i < 3; ++i)
    {
        av = et[i] / 16.0; w = (int)(10.0 * av + 0.5);
        fprintf(outf, "\n// min(%03i) %14.1f%16i.%1i", 128 + 64 * i, ekt[i], w / 10, w % 10);
        av = dt[i] / 16.0; w = (int)(10.0 * av + 0.5);;
        fprintf(outf, "%15.1f%16i.%1i", dkt[i], w / 10, w % 10);
    }

    time_base(&a0, &sig);
    for(i = 128; i <= 256; i += 64)
    {
        fprintf(outf, "\n// avg(%3i) ", i);
        time_eks8(i, &av, &sig);
        sig *= 100.0 / av;
        av = ((av - a0 + 4.0) / 8.0);
        fprintf(outf, "%6.0f (%4.1f%%)", av, sig);

        time_enc16(i, &av, &sig);
        sig *= 100.0 / av;
        av = (int)(10.0 * (av - a0) / 256.0) / 10.0;
        sig = (int)(10 * sig) / 10.0;
        fprintf(outf, "%10.1f (%4.1f%%)", av, sig);

        time_dks8(i, &av, &sig);
        sig *= 100.0 / av;
        av = (int)((av - a0 + 4.0) / 8.0);
        fprintf(outf, "%7.0f (%4.1f%%)", av, sig);

        time_dec16(i, &av, &sig);
        sig *= 100.0 / av;
        av = (int)(10.0 * (av - a0) / 256.0) / 10.0;
        sig = (int)(10 * sig) / 10.0;
        fprintf(outf, "%10.1f (%4.1f%%)", av, sig);
    }

#if defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )
    if(h_dll) FreeLibrary(h_dll);
#endif
    fprintf(outf, "\n\n");
	if(argc == 2)
		fclose(outf);
    return 0;
}
