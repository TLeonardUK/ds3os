/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#ifndef LTC_EAX_H
#define LTC_EAX_H

#include "aes.h"
#include "ltc_ctr.h"
#include "ltc_omac.h"

#define EAX_ENCRYPT 0
#define EAX_DECRYPT 1

typedef struct
{
    uint8_t     nv[AES_BLOCK_SIZE];
    ctr_ctx     ctr[1];
    omac_state  hdr_omac[1];
    omac_state  ctx_omac[1];
} eax_state;

int eax_init( const unsigned char key[], unsigned long key_len,
              const unsigned char nonce[], unsigned long nonce_len,
              const unsigned char hdr[], unsigned long header_len,
              eax_state eax[1] );

int eax_addheader( const unsigned char hdr[], unsigned long length, eax_state eax[1] );

int eax_encryptx( const unsigned char pt[], unsigned char ct[], unsigned long length, eax_state eax[1] );

int eax_decryptx( const unsigned char ct[], unsigned char pt[], unsigned long length, eax_state eax[1] );

int eax_done( unsigned char tag[], unsigned long tag_len, eax_state eax[1] );

int eax_memory( const unsigned char key[], unsigned long key_len,
                const unsigned char nonce[], unsigned long nonce_len,
                const unsigned char hdr[], unsigned long header_len,
                unsigned char pt[], unsigned long pt_len,
                unsigned char ct[],
                unsigned char tag[], unsigned long tag_len,
                int dir, int *stat);

#endif
