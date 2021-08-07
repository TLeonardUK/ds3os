/*
 This code has been derived from LibTomCrypt, the cryptographic library
 authored by Tom St Denis.  His contribution is hence acknowledged. This
 code has been developed to obtain two independent implementations of the
 combined AES encryption/authentication modes (CCM, GCM and EAX) in order
 to be able to check test vectors before their publication.
*/

#ifndef LTC_CCM_H
#define LTC_CCM_H

#include "aes.h"

#define CCM_ENCRYPT 0
#define CCM_DECRYPT 1

int ccm_memory( const unsigned char key[], unsigned long key_len,
                const unsigned char nonce[], unsigned long nonce_len,
                const unsigned char hdr[], unsigned long hdr_len,
                unsigned char pt[], unsigned long pt_len,
                unsigned char ct[],
                unsigned char tag[], unsigned long tag_len,
                int dir, int *stat);
#endif
