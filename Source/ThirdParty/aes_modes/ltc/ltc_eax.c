/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#include <stdlib.h>
#include <string.h>

#include "ltc_eax.h"

int eax_init( const unsigned char key[], unsigned long key_len,
              const unsigned char nonce[], unsigned long nonce_len,
              const unsigned char hdr[], unsigned long header_len,
              eax_state eax[1] )
{
    unsigned char *buf;
    int err = EXIT_FAILURE;
    omac_state *omac;
    unsigned long len;

    if( header_len > 0 && hdr == NULL )
        goto exit3;

    if( ( buf = malloc( AES_BLOCK_SIZE ) ) == NULL )
        goto exit3;
    if( ( omac = malloc( sizeof(*omac) ) ) == NULL )
        goto exit2;

    memset( buf, 0, AES_BLOCK_SIZE );
    if( (err = omac_init(key, key_len, omac)) != EXIT_SUCCESS )
        goto exit1;

    if( (err = omac_process(buf, AES_BLOCK_SIZE, omac)) != EXIT_SUCCESS )
        goto exit1;

    if( (err = omac_process(nonce, nonce_len, omac)) != EXIT_SUCCESS )
        goto exit1;

    len = sizeof( eax->nv );
    if( (err = omac_done(eax->nv, &len, omac)) != EXIT_SUCCESS )
        goto exit1;

    memset( buf, 0, AES_BLOCK_SIZE );
    buf[AES_BLOCK_SIZE - 1] = 1;

    if( (err = omac_init(key, key_len, eax->hdr_omac)) != EXIT_SUCCESS )
        goto exit1;

    if( (err = omac_process(buf, AES_BLOCK_SIZE, eax->hdr_omac)) != EXIT_SUCCESS )
        goto exit1;

    if( header_len != 0 && ( (err = omac_process(hdr, header_len, eax->hdr_omac) ) != EXIT_SUCCESS ) )
            goto exit1;

    if( (err = ctr_start(eax->nv, key, key_len, 0, CTR_COUNTER_BIG_ENDIAN, eax->ctr) ) != EXIT_SUCCESS )
        goto exit1;

    if( (err = omac_init(key, key_len, eax->ctx_omac)) != EXIT_SUCCESS )
        goto exit1;

    memset( buf, 0, AES_BLOCK_SIZE );
    buf[AES_BLOCK_SIZE - 1] = 2;
    if( (err = omac_process(buf, AES_BLOCK_SIZE, eax->ctx_omac)) != EXIT_SUCCESS )
        goto exit1;

    err = EXIT_SUCCESS;
exit1:
    free( omac );
exit2:
    free( buf );
exit3:
    return err;
}
int eax_addheader( const unsigned char hdr[], unsigned long length, eax_state eax[1] )
{
    return omac_process( hdr, length, eax->hdr_omac );
}

int eax_encryptx( const unsigned char pt[], unsigned char ct[], unsigned long length,
                  eax_state eax[1] )
{
    int err;

    if( (err = ctr_encrypt( pt, ct, length, eax->ctr )) != EXIT_SUCCESS )
        return err;

    return omac_process( ct, length, eax->ctx_omac );
}

int eax_decryptx( const unsigned char ct[], unsigned char pt[], unsigned long length,
                  eax_state eax[1] )
{
    int err;

    if( (err = omac_process( ct, length, eax->ctx_omac )) != EXIT_SUCCESS )
        return err;

    return ctr_decrypt( ct, pt, length, eax->ctr );
}

int eax_done( unsigned char tag[], unsigned long tag_len, eax_state eax[1] )
{
    int err = EXIT_FAILURE;
    unsigned char *headermac, *ctmac;
    unsigned long x, len;

    if( ( headermac = malloc( AES_BLOCK_SIZE ) ) == NULL )
        goto exit3;
    if( ( ctmac = malloc( AES_BLOCK_SIZE ) ) == NULL )
        goto exit2;

    len = AES_BLOCK_SIZE;
    if( (err = omac_done( ctmac, &len, eax->ctx_omac )) != EXIT_SUCCESS )
        goto exit1;

    if( (err = omac_done( headermac, &len, eax->hdr_omac )) != EXIT_SUCCESS )
        goto exit1;

    if( (err = ctr_done( eax->ctr )) != 0 )
        goto exit1;

    for( x = 0; x < len && x < tag_len; x++ )
        tag[x] = eax->nv[x] ^ headermac[x] ^ ctmac[x];

    err = EXIT_SUCCESS;
exit1:
    free( ctmac );
exit2:
    free( headermac );
exit3:
    return err;
}

int eax_memory( const unsigned char key[], unsigned long key_len,
                const unsigned char nonce[], unsigned long nonce_len,
                const unsigned char hdr[], unsigned long header_len,
                unsigned char pt[], unsigned long pt_len, unsigned char ct[],
                unsigned char tag[], unsigned long tag_len,
                int dir, int *stat )
{
    int err = EXIT_FAILURE;
    eax_state *eax;

    if( ( eax = malloc( sizeof(eax_state) ) ) == NULL )
        goto exit2;

    if( dir == EAX_ENCRYPT )
    {
        if( ( err = eax_init( key, key_len, nonce, nonce_len, hdr, header_len, eax ) ) == EXIT_SUCCESS )
            if( ( err = eax_encryptx( pt, ct, pt_len, eax ) ) == EXIT_SUCCESS )
                err = eax_done( tag, tag_len, eax );
    }
    else
    {
        uint8_t *buf;
        *stat = EXIT_FAILURE;
        if( ( buf = malloc( tag_len ) ) == NULL )
            goto exit1;

        if( ( err = eax_init( key, key_len, nonce, nonce_len, hdr, header_len, eax ) ) == EXIT_SUCCESS )
            if( ( err = eax_decryptx( ct, pt, pt_len, eax ) ) == EXIT_SUCCESS )
                if( ( err = eax_done( buf, tag_len, eax ) ) == EXIT_SUCCESS )
                    if( memcmp( buf, tag, tag_len ) == 0 )
                        *stat = EXIT_SUCCESS;
        free( buf );
    }
exit1:
    free( eax );
exit2:
    return err;
}
