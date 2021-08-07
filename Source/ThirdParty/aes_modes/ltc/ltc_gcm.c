/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#include <string.h>

#include "ltc_gcm.h"

#define GCM_MODE_IV     0
#define GCM_MODE_AAD    1
#define GCM_MODE_TEXT   2

#define store_64h( x, y ) \
    {   ( y )[0] = ( uint8_t ) ( ((x) >> 56) & 255 ); \
        ( y )[1] = ( uint8_t ) ( ((x) >> 48) & 255 ); \
        ( y )[2] = ( uint8_t ) ( ((x) >> 40) & 255 ); \
        ( y )[3] = ( uint8_t ) ( ((x) >> 32) & 255 ); \
        ( y )[4] = ( uint8_t ) ( ((x) >> 24) & 255 ); \
        ( y )[5] = ( uint8_t ) ( ((x) >> 16) & 255 ); \
        ( y )[6] = ( uint8_t ) ( ((x) >> 8) & 255 ); \
        ( y )[7] = ( uint8_t ) ( (x) & 255 ); \
    }

#if defined( GCM_TABLES )

const uint8_t gcm_shift_table[2 * 256] =
{
    0x00, 0x00, 0x01, 0xc2, 0x03, 0x84, 0x02, 0x46, 0x07, 0x08, 0x06, 0xca, 0x04, 0x8c, 0x05, 0x4e,
    0x0e, 0x10, 0x0f, 0xd2, 0x0d, 0x94, 0x0c, 0x56, 0x09, 0x18, 0x08, 0xda, 0x0a, 0x9c, 0x0b, 0x5e,
    0x1c, 0x20, 0x1d, 0xe2, 0x1f, 0xa4, 0x1e, 0x66, 0x1b, 0x28, 0x1a, 0xea, 0x18, 0xac, 0x19, 0x6e,
    0x12, 0x30, 0x13, 0xf2, 0x11, 0xb4, 0x10, 0x76, 0x15, 0x38, 0x14, 0xfa, 0x16, 0xbc, 0x17, 0x7e,
    0x38, 0x40, 0x39, 0x82, 0x3b, 0xc4, 0x3a, 0x06, 0x3f, 0x48, 0x3e, 0x8a, 0x3c, 0xcc, 0x3d, 0x0e,
    0x36, 0x50, 0x37, 0x92, 0x35, 0xd4, 0x34, 0x16, 0x31, 0x58, 0x30, 0x9a, 0x32, 0xdc, 0x33, 0x1e,
    0x24, 0x60, 0x25, 0xa2, 0x27, 0xe4, 0x26, 0x26, 0x23, 0x68, 0x22, 0xaa, 0x20, 0xec, 0x21, 0x2e,
    0x2a, 0x70, 0x2b, 0xb2, 0x29, 0xf4, 0x28, 0x36, 0x2d, 0x78, 0x2c, 0xba, 0x2e, 0xfc, 0x2f, 0x3e,
    0x70, 0x80, 0x71, 0x42, 0x73, 0x04, 0x72, 0xc6, 0x77, 0x88, 0x76, 0x4a, 0x74, 0x0c, 0x75, 0xce,
    0x7e, 0x90, 0x7f, 0x52, 0x7d, 0x14, 0x7c, 0xd6, 0x79, 0x98, 0x78, 0x5a, 0x7a, 0x1c, 0x7b, 0xde,
    0x6c, 0xa0, 0x6d, 0x62, 0x6f, 0x24, 0x6e, 0xe6, 0x6b, 0xa8, 0x6a, 0x6a, 0x68, 0x2c, 0x69, 0xee,
    0x62, 0xb0, 0x63, 0x72, 0x61, 0x34, 0x60, 0xf6, 0x65, 0xb8, 0x64, 0x7a, 0x66, 0x3c, 0x67, 0xfe,
    0x48, 0xc0, 0x49, 0x02, 0x4b, 0x44, 0x4a, 0x86, 0x4f, 0xc8, 0x4e, 0x0a, 0x4c, 0x4c, 0x4d, 0x8e,
    0x46, 0xd0, 0x47, 0x12, 0x45, 0x54, 0x44, 0x96, 0x41, 0xd8, 0x40, 0x1a, 0x42, 0x5c, 0x43, 0x9e,
    0x54, 0xe0, 0x55, 0x22, 0x57, 0x64, 0x56, 0xa6, 0x53, 0xe8, 0x52, 0x2a, 0x50, 0x6c, 0x51, 0xae,
    0x5a, 0xf0, 0x5b, 0x32, 0x59, 0x74, 0x58, 0xb6, 0x5d, 0xf8, 0x5c, 0x3a, 0x5e, 0x7c, 0x5f, 0xbe,
    0xe1, 0x00, 0xe0, 0xc2, 0xe2, 0x84, 0xe3, 0x46, 0xe6, 0x08, 0xe7, 0xca, 0xe5, 0x8c, 0xe4, 0x4e,
    0xef, 0x10, 0xee, 0xd2, 0xec, 0x94, 0xed, 0x56, 0xe8, 0x18, 0xe9, 0xda, 0xeb, 0x9c, 0xea, 0x5e,
    0xfd, 0x20, 0xfc, 0xe2, 0xfe, 0xa4, 0xff, 0x66, 0xfa, 0x28, 0xfb, 0xea, 0xf9, 0xac, 0xf8, 0x6e,
    0xf3, 0x30, 0xf2, 0xf2, 0xf0, 0xb4, 0xf1, 0x76, 0xf4, 0x38, 0xf5, 0xfa, 0xf7, 0xbc, 0xf6, 0x7e,
    0xd9, 0x40, 0xd8, 0x82, 0xda, 0xc4, 0xdb, 0x06, 0xde, 0x48, 0xdf, 0x8a, 0xdd, 0xcc, 0xdc, 0x0e,
    0xd7, 0x50, 0xd6, 0x92, 0xd4, 0xd4, 0xd5, 0x16, 0xd0, 0x58, 0xd1, 0x9a, 0xd3, 0xdc, 0xd2, 0x1e,
    0xc5, 0x60, 0xc4, 0xa2, 0xc6, 0xe4, 0xc7, 0x26, 0xc2, 0x68, 0xc3, 0xaa, 0xc1, 0xec, 0xc0, 0x2e,
    0xcb, 0x70, 0xca, 0xb2, 0xc8, 0xf4, 0xc9, 0x36, 0xcc, 0x78, 0xcd, 0xba, 0xcf, 0xfc, 0xce, 0x3e,
    0x91, 0x80, 0x90, 0x42, 0x92, 0x04, 0x93, 0xc6, 0x96, 0x88, 0x97, 0x4a, 0x95, 0x0c, 0x94, 0xce,
    0x9f, 0x90, 0x9e, 0x52, 0x9c, 0x14, 0x9d, 0xd6, 0x98, 0x98, 0x99, 0x5a, 0x9b, 0x1c, 0x9a, 0xde,
    0x8d, 0xa0, 0x8c, 0x62, 0x8e, 0x24, 0x8f, 0xe6, 0x8a, 0xa8, 0x8b, 0x6a, 0x89, 0x2c, 0x88, 0xee,
    0x83, 0xb0, 0x82, 0x72, 0x80, 0x34, 0x81, 0xf6, 0x84, 0xb8, 0x85, 0x7a, 0x87, 0x3c, 0x86, 0xfe,
    0xa9, 0xc0, 0xa8, 0x02, 0xaa, 0x44, 0xab, 0x86, 0xae, 0xc8, 0xaf, 0x0a, 0xad, 0x4c, 0xac, 0x8e,
    0xa7, 0xd0, 0xa6, 0x12, 0xa4, 0x54, 0xa5, 0x96, 0xa0, 0xd8, 0xa1, 0x1a, 0xa3, 0x5c, 0xa2, 0x9e,
    0xb5, 0xe0, 0xb4, 0x22, 0xb6, 0x64, 0xb7, 0xa6, 0xb2, 0xe8, 0xb3, 0x2a, 0xb1, 0x6c, 0xb0, 0xae,
    0xbb, 0xf0, 0xba, 0x32, 0xb8, 0x74, 0xb9, 0xb6, 0xbc, 0xf8, 0xbd, 0x3a, 0xbf, 0x7c, 0xbe, 0xbe };

#endif

static void gcm_rightshift( uint8_t a[] )
{
    int x;
    for( x = 15; x > 0; x-- )
    {
        a[x] = ( a[x] >> 1 ) | ( (a[x - 1] << 7) & 0x80 );
    }

    a[0] >>= 1;
}

static const uint8_t mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static const uint8_t poly[] = { 0x00, 0xe1 };

static void gcm_gf_mult( const uint8_t a[], const uint8_t b[], uint8_t c[] )
{
    uint8_t zb[16], vb[16];
    unsigned int x, y, z;

    memset( zb, 0, 16 );
    memcpy( vb, a, 16 );
    for( x = 0; x < 128; x++ )
    {
        if( b[x >> 3] & mask[x & 7] )
            for( y = 0; y < 16; y++ )
                zb[y] ^= vb[y];

        z = vb[15] & 0x01;
        gcm_rightshift( vb );
        vb[0] ^= poly[z];
    }

    memcpy( c, zb, 16 );
}

static void gcm_mult_h( gcm_state gcm[1], uint8_t ix[] )
{
    uint8_t tb[16];
#ifdef GCM_TABLES
    int x, y;
    memcpy( tb, &gcm->gf_tab[0][ix[0]][0], 16 );
    for( x = 1; x < 16; x++ )
        for( y = 0; y < 16; y++ )
            tb[y] ^= gcm->gf_tab[x][ix[x]][y];
#else
    gcm_gf_mult( gcm->hh, ix, tb );
#endif
    memcpy( ix, tb, 16 );
}

int gcm_init( const unsigned char key[], int key_len, gcm_state gcm[1] )
{
    uint8_t bb[16];
#ifdef GCM_TABLES
    int x, y, z, t;
#endif

    if(aes_encrypt_key( key, key_len, gcm->ctx ) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    memset( bb, 0, 16 );
    aes_encrypt( bb, gcm->hh, gcm->ctx );

    memset( gcm->buf, 0, sizeof(gcm->buf) );
    memset( gcm->acc, 0, sizeof(gcm->acc) );
    gcm->mode = GCM_MODE_IV;
    gcm->iv_mode = 0;
    gcm->bfr_len = 0;
    gcm->auth_len = 0;
    gcm->ptxt_len = 0;

#ifdef GCM_TABLES
    memset( bb, 0, 16 );
    for( y = 0; y < 256; y++ )
    {
        bb[0] = y;
        gcm_gf_mult( gcm->hh, bb, &gcm->gf_tab[0][y][0] );
    }

    for( x = 1; x < 16; x++ )
    {
        for( y = 0; y < 256; y++ )
        {
            t = gcm->gf_tab[x - 1][y][15];
            for( z = 15; z > 0; z-- )
                gcm->gf_tab[x][y][z] = gcm->gf_tab[x - 1][y][z - 1];

            gcm->gf_tab[x][y][0] = gcm_shift_table[t << 1];
            gcm->gf_tab[x][y][1] ^= gcm_shift_table[( t << 1 ) + 1];
        }
    }
#endif
    return EXIT_SUCCESS;
}

int gcm_add_iv( const unsigned char iv[], unsigned long iv_len, gcm_state gcm[1] )
{
    uint32_t x, y;

    if( gcm->mode != GCM_MODE_IV )
        return EXIT_FAILURE;

    if( gcm->bfr_len >= 16 || gcm->bfr_len < 0 )
        return EXIT_FAILURE;

    if( iv_len + gcm->bfr_len > 12 )
        gcm->iv_mode |= 1;

    for( x = 0; x < iv_len; x++ )
    {
        gcm->buf[gcm->bfr_len++] = *iv++;

        if( gcm->bfr_len == 16 )
        {
            for( y = 0; y < 16; y++ )
                gcm->acc[y] ^= gcm->buf[y];

            gcm_mult_h( gcm, gcm->acc );
            gcm->bfr_len = 0;
            gcm->auth_len += 128;
        }
    }

    return EXIT_SUCCESS;
}

int gcm_add_aad( const unsigned char data[], unsigned long data_len, gcm_state gcm[1] )
{
    uint32_t x;

    if( gcm->bfr_len > 16 || gcm->bfr_len < 0 )
        return EXIT_FAILURE;

    if( gcm->mode == GCM_MODE_IV )
    {
        if( gcm->iv_mode || gcm->bfr_len != 12 )
        {
            for( x = 0; x < (uint32_t) gcm->bfr_len; x++ )
                gcm->acc[x] ^= gcm->buf[x];

            if( gcm->bfr_len )
            {
                gcm->auth_len += gcm->bfr_len * 8;
                gcm_mult_h( gcm, gcm->acc );
            }

            memset( gcm->buf, 0, 8 );
            store_64h( gcm->auth_len, gcm->buf + 8 );
            for( x = 0; x < 16; x++ )
                gcm->acc[x] ^= gcm->buf[x];

            gcm_mult_h( gcm, gcm->acc );

            memcpy( gcm->ctr, gcm->acc, 16 );
            memset( gcm->acc, 0, 16 );
        }
        else
        {
            memcpy( gcm->ctr, gcm->buf, 12 );
            gcm->ctr[12] = 0;
            gcm->ctr[13] = 0;
            gcm->ctr[14] = 0;
            gcm->ctr[15] = 1;
        }

        memcpy( gcm->ctr_0, gcm->ctr, 16 );
        memset( gcm->buf, 0, 16 );
        gcm->bfr_len = 0;
        gcm->auth_len = 0;
        gcm->mode = GCM_MODE_AAD;
    }

    if( gcm->mode != GCM_MODE_AAD || gcm->bfr_len >= 16 )
        return EXIT_FAILURE;

    for( x = 0; x < data_len; x++ )
    {
        gcm->acc[gcm->bfr_len++] ^= *data++;

        if( gcm->bfr_len == 16 )
        {
            gcm_mult_h( gcm, gcm->acc );
            gcm->bfr_len = 0;
            gcm->auth_len += 128;
        }
    }

    return EXIT_SUCCESS;
}

int gcm_process( unsigned char pt[], unsigned long pt_len, unsigned char ct[], int dir, gcm_state gcm[1] )
{
    uint32_t x, y;
    uint8_t b;

    if( gcm->bfr_len > 16 || gcm->bfr_len < 0 )
        return EXIT_FAILURE;
    if( gcm->mode == GCM_MODE_AAD )
    {
        if( gcm->bfr_len )
        {
            gcm->auth_len += gcm->bfr_len * 8;
            gcm_mult_h( gcm, gcm->acc );
        }

        for( y = 15; y >= 12; y-- )
            if( ++gcm->ctr[y] & 255 )
                break;

        if( aes_encrypt( gcm->ctr, gcm->buf, gcm->ctx ) != EXIT_SUCCESS )
            return EXIT_FAILURE;

        gcm->bfr_len = 0;
        gcm->mode = GCM_MODE_TEXT;
    }

    if( gcm->mode != GCM_MODE_TEXT )
        return EXIT_FAILURE;

    for( x = 0; x < pt_len; x++ )
    {
        if( gcm->bfr_len == 16 )
        {
            gcm->ptxt_len += 128;
            gcm_mult_h( gcm, gcm->acc );

            for( y = 15; y >= 12; y-- )
                if( ++gcm->ctr[y] & 255 )
                    break;

            if( aes_encrypt( gcm->ctr, gcm->buf, gcm->ctx ) != EXIT_SUCCESS )
                return EXIT_FAILURE;

            gcm->bfr_len = 0;
        }

        if( dir == GCM_ENCRYPT )
            b = ct[x] = pt[x] ^ gcm->buf[gcm->bfr_len];
        else
        {
            b = ct[x];
            pt[x] = ct[x] ^ gcm->buf[gcm->bfr_len];
        }

        gcm->acc[gcm->bfr_len++] ^= b;
    }

    return EXIT_SUCCESS;
}

int gcm_reset( gcm_state gcm[1] )
{
    memset( gcm->buf, 0, sizeof(gcm->buf) );
    memset( gcm->acc, 0, sizeof(gcm->acc) );
    gcm->mode = GCM_MODE_IV;
    gcm->iv_mode = 0;
    gcm->bfr_len = 0;
    gcm->auth_len = 0;
    gcm->ptxt_len = 0;

    return EXIT_SUCCESS;
}

int gcm_done( unsigned char tag[], unsigned long *taglen, gcm_state gcm[1] )
{
    uint32_t x;
    if( gcm->bfr_len > 16 || gcm->bfr_len < 0 )
        return EXIT_FAILURE;

    if( gcm->mode != GCM_MODE_TEXT )
        return EXIT_FAILURE;

    if( gcm->bfr_len )
    {
        gcm->ptxt_len += gcm->bfr_len * 8;
        gcm_mult_h( gcm, gcm->acc );
    }

    store_64h( gcm->auth_len, gcm->buf );
    store_64h( gcm->ptxt_len, gcm->buf + 8 );

    for( x = 0; x < 16; x++ )
        gcm->acc[x] ^= gcm->buf[x];

    gcm_mult_h( gcm, gcm->acc );
    if( aes_encrypt( gcm->ctr_0, gcm->buf, gcm->ctx ) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    for( x = 0; x < 16 && x < *taglen; x++ )
        tag[x] = gcm->buf[x] ^ gcm->acc[x];
    *taglen = x;

    return EXIT_SUCCESS;
}

int gcm_memory( const unsigned char key[], unsigned long key_len,
                const unsigned char iv[], unsigned long iv_len,
                const unsigned char data[], unsigned long data_len,
                unsigned char pt[], unsigned long pt_len,
                unsigned char ct[],
                unsigned char tag[], unsigned long taglen,
                int dir, int *stat )
{
    void *orig;
    gcm_state *gcm;
    int err, t_len = taglen;

    orig = gcm = malloc( sizeof(*gcm) );

    if( (err = gcm_init( key, key_len, gcm )) != EXIT_SUCCESS )
        goto exit1;

    if( (err = gcm_add_iv( iv, iv_len, gcm )) != EXIT_SUCCESS )
        goto exit1;

    if( (err = gcm_add_aad( data, data_len, gcm )) != EXIT_SUCCESS )
        goto exit1;

    if( (err = gcm_process( pt, pt_len, ct, dir, gcm )) != EXIT_SUCCESS )
        goto exit1;

    if(dir == 0)
       err = gcm_done( tag, &t_len, gcm );
    else
    {   unsigned char *buf = malloc( taglen );
        if(buf)
        {
            err = gcm_done( buf, &t_len, gcm );
            *stat = ( memcmp( buf, tag, taglen ) == 0 ? EXIT_SUCCESS : EXIT_FAILURE );
            free( buf );
        }
        else
            err = EXIT_FAILURE;
    }
exit1:
    free( orig );
    return err;
}
