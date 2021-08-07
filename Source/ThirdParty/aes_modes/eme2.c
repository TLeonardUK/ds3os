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
 Issue Date: 04/11/2008
*/

#include "mode_hdr.h"
#include "eme2.h"
#include "gf_mulx.h"

#define BLOCK_PWR2      4
#define BLOCK_SIZE      (1 << BLOCK_PWR2)
#define BLOCK_LEN_MASK  (BLOCK_SIZE - 1)
#define N_BLOCKS(x)     (((x) - 1) >> BLOCK_PWR2) + 1

UNIT_TYPEDEF(buf_unit, UNIT_BITS);
BUFR_TYPEDEF(buf_type, UNIT_BITS, BLOCK_SIZE);


INT_RETURN eme2_encrypt( const unsigned char pt_buf[], unsigned char ct_buf[], unsigned int pt_len,
                         const unsigned char ad_buf[], unsigned int ad_len,
                         const eme2_key key[1], unsigned int key_len )
{   buf_type            t_buf, t_star, mp, mm, mc, mc1, m, m1;
    aes_encrypt_ctx     ctx[1];
    unsigned int        len, tt;
    
    if( pt_len < BLOCK_SIZE )
        return EXIT_FAILURE;

    if( key_len == 48 || key_len == 8 * 48 )
        aes_encrypt_key128( key->aes_key, ctx );
    else if( key_len == 64 || key_len == 8 * 64 )
        aes_encrypt_key256( key->aes_key, ctx );
    else
        return EXIT_FAILURE;

    if(ad_len > 0)
    {
        memset( t_star, 0, BLOCK_SIZE );
        copy_block( mm, key->adt_key );
        gf_mulx( mm );
        len = 0;
        while(len < ad_len)
        {
            tt = ( ad_len - len < BLOCK_SIZE ? ad_len - len : BLOCK_SIZE );
            memcpy( t_buf, ad_buf + len, tt );
            if( tt < BLOCK_SIZE )
            {
                memset( UI8_PTR(t_buf) + tt, 0, BLOCK_SIZE - tt );
                UI8_PTR(t_buf)[tt] = 0x80;
                gf_mulx( mm );
            }
            xor_block_aligned( t_buf, t_buf, mm ); 
            aes_encrypt( UI8_PTR(t_buf), UI8_PTR(t_buf), ctx );
            xor_block_aligned( t_buf, t_buf, mm ); 
            gf_mulx( mm );
            xor_block_aligned( t_star, t_star, t_buf ); 
            len += BLOCK_SIZE;
        }
    }
    else
        aes_encrypt( key->adt_key, UI8_PTR(t_star), ctx );

    copy_block( mm, key->ecb_key );
    copy_block_aligned( mp, t_star );
    len = 0;
    while( pt_len >= len + BLOCK_SIZE )
    {
        copy_block( t_buf, pt_buf + len );
        xor_block_aligned( t_buf, t_buf, mm );
        aes_encrypt( UI8_PTR(t_buf), UI8_PTR(t_buf), ctx );
        xor_block_aligned( mp, mp, t_buf );
        copy_block( ct_buf + len, t_buf );
        gf_mulx( mm );
        len += BLOCK_SIZE;
    }

    if( pt_len & BLOCK_LEN_MASK )
    {
        tt = len - 1; 
        while( ++tt < pt_len )
        {
            ct_buf[tt] = pt_buf[tt];
            UI8_PTR(mp)[tt - len] ^= pt_buf[tt];
        }
        UI8_PTR(mp)[tt - len] ^= 0x80;

        aes_encrypt( UI8_PTR(mp), UI8_PTR(mm), ctx );
        aes_encrypt( UI8_PTR(mm), UI8_PTR(mc), ctx );
    }
    else
        aes_encrypt( UI8_PTR(mp), UI8_PTR(mc), ctx );

    copy_block_aligned( mc1, mc );
    xor_block_aligned( m, mp, mc );
    copy_block_aligned( m1, m );

    len = BLOCK_SIZE;
    while( pt_len >= len + BLOCK_SIZE )
    {
        copy_block( t_buf,  ct_buf + len );
        if( len & 0x07f0 )
        {
            gf_mulx( m );
            xor_block_aligned( t_buf, t_buf, m ); 
        }
        else
        {
            xor_block_aligned( mp, t_buf, m1 );
            aes_encrypt( UI8_PTR(mp), UI8_PTR(mc), ctx );
            xor_block_aligned( m, mp, mc );
            xor_block_aligned( t_buf, mc, m1 );
        }
        xor_block_aligned( mc1, mc1, t_buf);
        copy_block( ct_buf + len, t_buf );
        len += BLOCK_SIZE;
    }

    if( pt_len & BLOCK_LEN_MASK )
    {
        tt = len - 1;
        while( ++tt < pt_len )
        {
            ct_buf[tt] = pt_buf[tt] ^ UI8_PTR(mm)[tt - len];
            UI8_PTR(mc1)[tt - len] ^= ct_buf[tt];
        }
        UI8_PTR(mc1)[tt - len] ^= 0x80; 
    }
    
    xor_block_aligned( mc1, mc1, t_star );
    copy_block( ct_buf, mc1 );

    copy_block( mm, key->ecb_key );
    len = 0;
    while( pt_len >= len + BLOCK_SIZE )
    {
        aes_encrypt( ct_buf + len, UI8_PTR(t_buf), ctx );
        xor_block( t_buf, t_buf , mm );
        copy_block( ct_buf + len, t_buf );
        gf_mulx( mm );
        len += BLOCK_SIZE;
    }
    return EXIT_SUCCESS;
}

INT_RETURN eme2_decrypt( const unsigned char ct_buf[], unsigned char pt_buf[], unsigned int pt_len,
                         const unsigned char ad_buf[], unsigned int ad_len,
                         const eme2_key key[1], unsigned int key_len )
{   buf_type            t_buf, t_star, mc, mm, mp, mp1, m, m1;
    unsigned int        len, tt;
    union
    {   aes_encrypt_ctx e[1];
        aes_decrypt_ctx d[1];
    } ctx;
    
    if( pt_len < BLOCK_SIZE )
        return EXIT_FAILURE;

    if( key_len == 48 || key_len == 8 * 48 )
        aes_encrypt_key128( key->aes_key, ctx.e );
    else if( key_len == 64 || key_len == 8 * 64 )
        aes_encrypt_key256( key->aes_key, ctx.e );
    else
        return EXIT_FAILURE;

    if(ad_len > 0)
    {
        memset( t_star, 0, BLOCK_SIZE );
        copy_block( mm, key->adt_key );
        gf_mulx( mm );
        len = 0;
        while(len < ad_len)
        {
            tt = ( ad_len - len < BLOCK_SIZE ? ad_len - len : BLOCK_SIZE );
            memcpy( t_buf, ad_buf + len, tt );
            if( tt < BLOCK_SIZE )
            {
                memset( UI8_PTR(t_buf) + tt, 0, BLOCK_SIZE - tt );
                UI8_PTR(t_buf)[tt] = 0x80;
                gf_mulx( mm );
            }
            xor_block_aligned( t_buf, t_buf, mm ); 
            aes_encrypt( UI8_PTR(t_buf), UI8_PTR(t_buf), ctx.e );
            xor_block_aligned( t_buf, t_buf, mm ); 
            gf_mulx( mm );
            xor_block_aligned( t_star, t_star, t_buf ); 
            len += BLOCK_SIZE;
        }
    }
    else
        aes_encrypt( key->adt_key, UI8_PTR(t_star), ctx.e );

    if( key_len == 48 || key_len == 8 * 48 )
        aes_decrypt_key128( key->aes_key, ctx.d );
    else if( key_len == 64 || key_len == 8 * 64 )
        aes_decrypt_key256( key->aes_key, ctx.d );

    copy_block( mm, key->ecb_key );
    copy_block_aligned( mc, t_star );
    len = 0;
    while( pt_len >= len + BLOCK_SIZE )
    {
        copy_block( t_buf, ct_buf + len );
        xor_block_aligned( t_buf, t_buf, mm );
        aes_decrypt( UI8_PTR(t_buf), UI8_PTR(t_buf), ctx.d );
        xor_block_aligned( mc, mc, t_buf );
        copy_block( pt_buf + len, t_buf );
        gf_mulx( mm );
        len += BLOCK_SIZE;
    }

    if( pt_len & BLOCK_LEN_MASK )
    {
        tt = len - 1; 
        while( ++tt < pt_len )
        {
            pt_buf[tt] = ct_buf[tt];
            UI8_PTR(mc)[tt - len] ^= pt_buf[tt];
        }
        UI8_PTR(mc)[tt - len] ^= 0x80;

        aes_decrypt( UI8_PTR(mc), UI8_PTR(mm), ctx.d );
        aes_decrypt( UI8_PTR(mm), UI8_PTR(mp), ctx.d );
    }
    else
        aes_decrypt( UI8_PTR(mc), UI8_PTR(mp), ctx.d );

    copy_block_aligned( mp1, mp );
    xor_block_aligned(m, mp, mc);
    copy_block_aligned( m1, m );

    len = BLOCK_SIZE;
    while( pt_len >= len + BLOCK_SIZE )
    {
        copy_block( t_buf, pt_buf + len );
        if( len & 0x07f0 )
        {
            gf_mulx( m );
            xor_block_aligned( t_buf, t_buf, m ); 
        }
        else
        {
            xor_block_aligned( mc, t_buf, m1 );
            aes_decrypt( UI8_PTR(mc), UI8_PTR(mp), ctx.d );
            xor_block_aligned( m, mp, mc );
            xor_block_aligned( t_buf, mp, m1 );
        }
        xor_block_aligned( mp1, mp1, t_buf );
        copy_block( pt_buf + len, t_buf );
        len += BLOCK_SIZE;
    }

    if( pt_len & BLOCK_LEN_MASK )
    {
        tt = len - 1;
        while( ++tt < pt_len )
        {
            pt_buf[tt] = ct_buf[tt] ^ UI8_PTR(mm)[tt - len];
            UI8_PTR(mp1)[tt - len] ^= pt_buf[tt];
        }
        UI8_PTR(mp1)[tt - len] ^= 0x80; 
    }
    xor_block_aligned( mp1, mp1, t_star );
    copy_block( pt_buf, mp1 );

    copy_block( mm, key->ecb_key );
    len = 0;
    while( pt_len >= len + BLOCK_SIZE )
    {
        aes_decrypt( pt_buf + len, UI8_PTR(t_buf), ctx.d );
        xor_block_aligned( t_buf, t_buf, mm );
        copy_block( pt_buf + len, t_buf );
        gf_mulx( mm );
        len += BLOCK_SIZE;
    }
    return EXIT_SUCCESS;
}
