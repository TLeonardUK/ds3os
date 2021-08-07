
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
 Issue Date: 20/12/2007
*/

#ifndef _XTS_H
#define _XTS_H

#include "aes.h"

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct
{
    aes_encrypt_ctx twk_ctx[1];
    aes_encrypt_ctx enc_ctx[1];
	aes_decrypt_ctx dec_ctx[1];
} xts_ctx;

INT_RETURN xts_key( const unsigned char key[], int key_len, xts_ctx ctx[1] );

INT_RETURN xts_encrypt( unsigned char sector[], unsigned int sector_len_bits,
						unsigned char sector_address[], const xts_ctx ctx[1] );

INT_RETURN xts_decrypt( unsigned char sector[], unsigned int sector_len_bits,
						unsigned char sector_address[], const xts_ctx ctx[1] );

#if defined(__cplusplus)
}
#endif

#endif
