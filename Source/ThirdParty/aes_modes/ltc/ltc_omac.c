/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#include <stdlib.h>
#include <string.h>

#include "ltc_omac.h"

int omac_init( const unsigned char key[], unsigned long key_len, omac_state omac[1] )
{
    int x, y, msb;

    if(aes_encrypt_key( key, key_len, omac->ctx ) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    memset( omac->lu[0], 0, AES_BLOCK_SIZE );
    if(aes_encrypt( omac->lu[0], omac->lu[0], omac->ctx ) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    for( x = 0; x < 2; x++ )
    {
        msb = omac->lu[x][0] >> 7;

        for( y = 0; y < (AES_BLOCK_SIZE - 1); y++ )
            omac->lu[x][y] = ( (omac->lu[x][y] << 1) | (omac->lu[x][y + 1] >> 7) ) & 0xff;

        omac->lu[x][AES_BLOCK_SIZE - 1] = ( (omac->lu[x][AES_BLOCK_SIZE - 1] << 1) ^ (msb ? 0x87 : 0x00) ) & 0xff;

        if( x == 0 )
            memcpy( omac->lu[1], omac->lu[0], sizeof(omac->lu[0]) );
    }

    omac->buflen = 0;
    memset( omac->prev, 0, sizeof(omac->prev) );
    memset( omac->block, 0, sizeof(omac->block) );
    return EXIT_SUCCESS;
}

int omac_process( const unsigned char in[], unsigned long in_len, omac_state omac[1] )
{
    unsigned long n, x;

    if( omac->buflen < 0 || omac->buflen > AES_BLOCK_SIZE )
        return EXIT_FAILURE;
    while( in_len != 0 )
    {
        if( omac->buflen == AES_BLOCK_SIZE )
        {
            for( x = 0; x < AES_BLOCK_SIZE; x++ )
                omac->block[x] ^= omac->prev[x];

            if(aes_encrypt( omac->block, omac->prev, omac->ctx ) != EXIT_SUCCESS)
                return EXIT_FAILURE;

            omac->buflen = 0;
        }

        n = ( in_len < AES_BLOCK_SIZE - omac->buflen ? in_len : AES_BLOCK_SIZE - omac->buflen );
        memcpy( omac->block + omac->buflen, in, n );
        omac->buflen += n;
        in_len -= n;
        in += n;
    }

    return EXIT_SUCCESS;
}

int omac_done( unsigned char out[], unsigned long *out_len, omac_state omac[1] )
{
    int mode;
    unsigned x;

    if( omac->buflen < 0 || omac->buflen > AES_BLOCK_SIZE )
        return EXIT_FAILURE;

    if( omac->buflen != AES_BLOCK_SIZE )
    {
        omac->block[omac->buflen++] = 0x80;

        while( omac->buflen < AES_BLOCK_SIZE )
            omac->block[omac->buflen++] = 0;
        mode = 1;
    }
    else
        mode = 0;

    for( x = 0; x < AES_BLOCK_SIZE; x++ )
        omac->block[x] ^= omac->prev[x] ^ omac->lu[mode][x];

    if(aes_encrypt( omac->block, omac->block, omac->ctx ) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    for( x = 0; x < AES_BLOCK_SIZE && x < *out_len; x++ )
        out[x] = omac->block[x];
    *out_len = x;
    return EXIT_SUCCESS;
}

int omac_memory( const unsigned char key[], unsigned long key_len,
                 const unsigned char in[], unsigned long in_len,
                 unsigned char out[], unsigned long *out_len )
{
    int err;
    omac_state *omac;

    omac = malloc( sizeof(omac_state) );
    if( omac == NULL )
        return EXIT_FAILURE;

    if( (err = omac_init( key, key_len, omac )) != EXIT_SUCCESS )
        goto error;

    if( (err = omac_process( in, in_len, omac )) != EXIT_SUCCESS )
        goto error;

    if( (err = omac_done( out, out_len, omac )) != EXIT_SUCCESS )
        goto error;

    err = 0;
error:
    free( omac );
    return err;
}
