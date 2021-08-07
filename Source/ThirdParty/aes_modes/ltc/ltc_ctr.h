/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#ifndef LTC_CTR_H
#define LTC_CTR_H

#include "aes.h"

#define CTR_COUNTER_LITTLE_ENDIAN    0
#define CTR_COUNTER_BIG_ENDIAN       1

typedef struct
{
    aes_encrypt_ctx ctx[1];
    uint8_t         ctr[AES_BLOCK_SIZE];
    uint8_t         pad[AES_BLOCK_SIZE];
    int             padlen;
    int             mode;
} ctr_ctx;

int ctr_start( const uint8_t iv[],
               const uint8_t key[], int key_len,
               int num_rounds, int ctr_mode, ctr_ctx ctr[1] );

int ctr_encrypt( const uint8_t pt[], uint8_t ct[], unsigned long len, ctr_ctx ctr[1] );

int ctr_decrypt( const uint8_t ct[], uint8_t pt[], unsigned long len, ctr_ctx ctr[1] );

int ctr_setiv( const uint8_t iv[], unsigned long len, ctr_ctx ctr[1] );

int ctr_getiv( uint8_t iv[], unsigned long *len, ctr_ctx ctr[1] );

int ctr_done( ctr_ctx ctr[1] );

#endif
