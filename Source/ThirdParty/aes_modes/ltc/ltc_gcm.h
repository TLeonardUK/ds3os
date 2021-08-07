/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#ifndef LTC_GCM_H
#define LTC_GCM_H

#include "aes.h"

#define GCM_TABLES

#define GCM_ENCRYPT 0
#define GCM_DECRYPT 1

typedef struct
{
    aes_encrypt_ctx ctx[1];
    uint8_t hh[16],     /* multiplier                           */
    acc[16],            /* accumulator                          */
    ctr[16],            /* counter                              */
    ctr_0[16],          /* initial counter                      */
    buf[16];            /* buffer for stuff                     */
    int iv_mode,        /* Which mode is the iv in?             */
    mode,               /* mode the GCM code is in              */
    bfr_len;            /* length of data in buf                */
    uint64_t auth_len,  /* 64-bit counter used for iv and AAD   */
    ptxt_len;           /* 64-bit counter for the PT            */
#ifdef GCM_TABLES
    uint8_t gf_tab[16][256][16];    /* 16 tables of 8x128       */
#endif
} gcm_state;

int gcm_init( const unsigned char key[], int key_len, gcm_state gcm[1] );

int gcm_add_iv( const unsigned char iv[], unsigned long iv_len, gcm_state gcm[1] );

int gcm_add_aad( const unsigned char data[], unsigned long data_len, gcm_state gcm[1] );

int gcm_process( unsigned char pt[], unsigned long pt_len, unsigned char ct[], int dir, gcm_state gcm[1] );

int gcm_reset( gcm_state gcm[1] );

int gcm_done( unsigned char tag[], unsigned long *taglen, gcm_state gcm[1] );

int gcm_memory( const unsigned char key[], unsigned long key_len,
                const unsigned char iv[], unsigned long iv_len,
                const unsigned char data[], unsigned long data_len,
                unsigned char pt[], unsigned long pt_len,
                unsigned char ct[],
                unsigned char tag[], unsigned long taglen,
                int dir, int *stat);
#endif
