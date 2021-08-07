/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#ifndef LTC_OMAC_H
#define LTC_OMAC_H

#include "aes.h"

typedef struct
{
    aes_encrypt_ctx ctx[1];
    uint8_t         block[AES_BLOCK_SIZE];
    uint8_t         prev[AES_BLOCK_SIZE];
    uint8_t         lu[2][AES_BLOCK_SIZE];
    uint32_t        buflen;
} omac_state;

int omac_init( const unsigned char key[], unsigned long key_len, omac_state omac[1] );
int omac_process( const unsigned char in[], unsigned long in_len, omac_state omac[1] );
int omac_done( unsigned char out[], unsigned long *out_len, omac_state omac[1] );
int omac_memory( const unsigned char key[], unsigned long key_len,
                 const unsigned char in[], unsigned long in_len,
                 unsigned char out[], unsigned long *out_len );

#endif
