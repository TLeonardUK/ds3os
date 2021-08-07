/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 04/11/2008
*/

#ifndef _EME2_H
#define _EME2_H

#include "aes.h"

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct
{   unsigned char adt_key[AES_BLOCK_SIZE];
    unsigned char ecb_key[AES_BLOCK_SIZE];
    unsigned char aes_key[2 * AES_BLOCK_SIZE];
} eme2_key;

INT_RETURN eme2_encrypt( const unsigned char pt_buf[], unsigned char ct_buf[], unsigned int pt_len,
                         const unsigned char ad_buf[], unsigned int ad_len,
                         const eme2_key key[1], unsigned int key_len);

INT_RETURN eme2_decrypt( const unsigned char ct_buf[], unsigned char pt_buf[], unsigned int pt_len,
                         const unsigned char ad_buf[], unsigned int ad_len,
                         const eme2_key key[1], unsigned int key_len);

#if defined(__cplusplus)
}
#endif

#endif
