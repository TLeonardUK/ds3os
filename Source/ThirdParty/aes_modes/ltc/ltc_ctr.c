/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#include <stdlib.h>
#include <string.h>

#include "ltc_ctr.h"

int ctr_start( const uint8_t iv[], const uint8_t key[], int key_len,
               int num_rounds, int ctr_mode, ctr_ctx ctr[1] )
{
    if( aes_encrypt_key(key, key_len, ctr->ctx) == EXIT_SUCCESS )
    {
        int x;
        ctr->padlen = 0;
        ctr->mode = ctr_mode;
        for( x = 0; x < AES_BLOCK_SIZE; x++ )
            ctr->ctr[x] = iv[x];
        return aes_encrypt( ctr->ctr, ctr->pad, ctr->ctx );
    }
    else
        return EXIT_FAILURE;
}
int ctr_encrypt( const uint8_t pt[], uint8_t ct[], unsigned long len, ctr_ctx ctr[1] )
{
    int x;

    if( ctr->padlen < 0 || ctr->padlen > AES_BLOCK_SIZE )
        return EXIT_FAILURE;

    while( len )
    {
        if( ctr->padlen == AES_BLOCK_SIZE )
        {
            if( ctr->mode == CTR_COUNTER_LITTLE_ENDIAN )
            {
                for( x = 0; x < AES_BLOCK_SIZE; x++ )
                    if( ctr->ctr[x] = (ctr->ctr[x] + (uint8_t) 1) & (uint8_t) 0xff )
                        break;
            }
            else
            {
                for( x = AES_BLOCK_SIZE - 1; x >= 0; x-- )
                    if( ctr->ctr[x] = (ctr->ctr[x] + (uint8_t) 1) & (uint8_t) 0xff )
                        break;
            }

            if( aes_encrypt( ctr->ctr, ctr->pad, ctr->ctx ) != EXIT_SUCCESS )
                return EXIT_FAILURE;
            ctr->padlen = 0;
        }

        *ct++ = *pt++ ^ ctr->pad[ctr->padlen++];
        --len;
    }

    return EXIT_SUCCESS;
}

int ctr_decrypt( const uint8_t ct[], uint8_t pt[], unsigned long len, ctr_ctx ctr[1] )
{
    return ctr_encrypt( ct, pt, len, ctr );
}

int ctr_setiv( const uint8_t iv[], unsigned long len, ctr_ctx ctr[1] )
{
    if( len != AES_BLOCK_SIZE )
        return EXIT_FAILURE;
    memcpy( ctr->ctr, iv, len );
    ctr->padlen = 0;
    return aes_encrypt( iv, ctr->pad, ctr->ctx );
}

int ctr_getiv( uint8_t iv[], unsigned long *len, ctr_ctx ctr[1] )
{
    if( AES_BLOCK_SIZE > *len )
    {
        *len = AES_BLOCK_SIZE;
        return EXIT_FAILURE;
    }

    memcpy( iv, ctr->ctr, AES_BLOCK_SIZE );
    *len = AES_BLOCK_SIZE;
    return EXIT_SUCCESS;
}

int ctr_done( ctr_ctx ctr[1] )
{
    return EXIT_SUCCESS;
}
