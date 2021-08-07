/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#include <string.h>

#include "ltc_ccm.h"

int ccm_memory( const unsigned char key[], unsigned long key_len,
                const unsigned char nonce[], unsigned long nonce_len,
                const unsigned char hdr[], unsigned long hdr_len,
                unsigned char pt[], unsigned long pt_len,
                unsigned char ct[],
                unsigned char tag[], unsigned long tag_len,
                int dir, int *stat )
{
    unsigned char pad[16], ctr[16], ctr_pad[16], b;
    unsigned long len, lval, x, y, z, ctr_len;
    aes_encrypt_ctx ctx[1];

    if( aes_encrypt_key(key, key_len, ctx) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    tag_len &= ~1;
    if( tag_len > 16 )
        tag_len = 16;

    if( tag_len < 4 )
        return EXIT_FAILURE;

    len = pt_len;
    lval = 0;
    while( len )
    {
        ++lval;
        len >>= 8;
    }
    if( lval <= 1 )
        lval = 2;

    nonce_len = ( nonce_len > 13 ) ? 13 : nonce_len;
    if( 15 - nonce_len > lval )
        lval = 15 - nonce_len;
    if( nonce_len + lval > 15 )
        nonce_len = 15 - lval;

    x = 0;
    pad[x++] = (unsigned char)(( (hdr_len > 0) ? (1 << 6) : 0 ) | ( ((tag_len - 2) >> 1) << 3 ) | ( lval - 1 ));
    for( y = 0; y < (16 - (lval + 1)); y++ )
        pad[x++] = nonce[y];

    len = pt_len;

    for( y = lval; y < 4; y++ )
        len <<= 8;

    for( y = 0; lval > 4 && (lval - y) > 4; y++ )
        pad[x++] = 0;

    for( ; y < lval; y++ )
    {
        pad[x++] = ( len >> 24 ) & 255;
        len <<= 8;
    }
    if( aes_encrypt(pad, pad, ctx) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    if( hdr_len > 0 )
    {
        x = 0;
        if( hdr_len < ((1UL << 16) - (1UL << 8)) )
        {
            pad[x++] ^= ( hdr_len >> 8 ) & 255;
            pad[x++] ^= hdr_len & 255;
        }
        else
        {
            pad[x++] ^= 0xFF;
            pad[x++] ^= 0xFE;
            pad[x++] ^= ( hdr_len >> 24 ) & 255;
            pad[x++] ^= ( hdr_len >> 16 ) & 255;
            pad[x++] ^= ( hdr_len >> 8 ) & 255;
            pad[x++] ^= hdr_len & 255;
        }
        for( y = 0; y < hdr_len; y++ )
        {
            if( x == 16 )
            {
                if( aes_encrypt(pad, pad, ctx) != EXIT_SUCCESS )
                    return EXIT_FAILURE;
                x = 0;
            }
            pad[x++] ^= hdr[y];
        }
        if( x != 0 )
            if( aes_encrypt(pad, pad, ctx) != EXIT_SUCCESS )
                return EXIT_FAILURE;
    }
    x = 0;
    ctr[x++] = (unsigned char)(lval - 1);
    for( y = 0; y < (16 - (lval + 1)); ++y )
        ctr[x++] = nonce[y];

    while( x < 16 )
        ctr[x++] = 0;

    x = 0;
    ctr_len = 16;
    if( pt_len > 0 )
    {
        y = 0;
        for( ; y < pt_len; y++ )
        {
            if( ctr_len == 16 )
            {
                for( z = 15; z > 15 - lval; z-- )
                {
                    ctr[z] = ( ctr[z] + 1 ) & 255;
                    if( ctr[z] )
                        break;
                }
                if( aes_encrypt(ctr, ctr_pad, ctx) != EXIT_SUCCESS )
                    return EXIT_FAILURE;
                ctr_len = 0;
            }
            if( dir == CCM_ENCRYPT )
            {
                b = pt[y];
                ct[y] = b ^ ctr_pad[ctr_len++];
            }
            else
            {
                b = ct[y] ^ ctr_pad[ctr_len++];
                pt[y] = b;
            }
            if( x == 16 )
            {
                if( aes_encrypt(pad, pad, ctx) != EXIT_SUCCESS )
                    return EXIT_FAILURE;
                x = 0;
            }
            pad[x++] ^= b;
        }
        if( x != 0 )
            if( aes_encrypt(pad, pad, ctx) != EXIT_SUCCESS )
                return EXIT_FAILURE;
    }
    for( y = 15; y > 15 - lval; y-- )
        ctr[y] = 0;

    if( aes_encrypt(ctr, ctr_pad, ctx) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    if( dir == CCM_ENCRYPT )
        for( x = 0; x < 16 && x < tag_len; x++ )
            tag[x] = pad[x] ^ ctr_pad[x];
    else
    {
        for( x = 0; x < 16 && x < tag_len; x++ )
            pad[x] ^= ctr_pad[x];
        *stat = ( memcmp(pad, tag, tag_len) == 0 ? EXIT_SUCCESS : EXIT_FAILURE );
    }
    return EXIT_SUCCESS;
}
