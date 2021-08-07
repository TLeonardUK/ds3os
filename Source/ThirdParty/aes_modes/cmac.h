/*
---------------------------------------------------------------------------
Copyright (c) 1998-2010, Brian Gladman, Worcester, UK. All rights reserved.

The redistribution and use of this software (with or without changes)
is allowed without the payment of fees or royalties provided that:

  source code distributions include the above copyright notice, this
  list of conditions and the following disclaimer;

  binary distributions include the above copyright notice, this list
  of conditions and the following disclaimer in their documentation.

This software is provided 'as is' with no explicit or implied warranties
in respect of its operation, including, but not limited to, correctness
and fitness for purpose.
---------------------------------------------------------------------------
Issue Date: 6/10/2008
*/

#ifndef CMAC_AES_H
#define CMAC_AES_H

#if !defined( UNIT_BITS )
#  if 1
#    define UNIT_BITS 64
#  elif 0
#    define UNIT_BITS 32
#  else
#    define UNIT_BITS  8
#  endif
#endif

#include <string.h>
#include "aes.h"
#include "mode_hdr.h"

UNIT_TYPEDEF(buf_unit, UNIT_BITS);
BUFR_TYPEDEF(buf_type, UNIT_BITS, AES_BLOCK_SIZE);

#if defined(__cplusplus)
extern "C"
{
#endif

#define BLOCK_SIZE  AES_BLOCK_SIZE

typedef struct
{
    buf_type        txt_cbc;
    aes_encrypt_ctx aes[1];                 /* AES encryption context           */
    uint32_t        txt_cnt;
} cmac_ctx;

void cmac_init( const unsigned char key[],  /* the encryption key               */
                unsigned long key_len,      /* key length (bytes)               */
                cmac_ctx ctx[1] );          /* the OMAC context                 */

void cmac_data( unsigned char buf[],        /* the data buffer                  */
                unsigned long len,          /* the length of this block (bytes) */
                cmac_ctx ctx[1] );          /* the OMAC context                 */

void cmac_end( unsigned char auth_tag[],    /* the encryption key               */
               cmac_ctx ctx[1] );           /* the OMAC context                 */

#if defined(__cplusplus)
}
#endif

#endif
