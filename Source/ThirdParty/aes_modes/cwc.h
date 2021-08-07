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
Issue Date: 20/12/2007

 This file contains the code for implementing encryption and authentication
 using AES with a Carter-Wegamn hash function
*/

#ifndef _CWC_H
#define _CWC_H

/*  This define sets the memory alignment that will be used for fast move
    and xor operations on buffers when the alignment matches this value. 
*/
#if !defined( UNIT_BITS )
#  if 1
#    define UNIT_BITS 32
#  else
#    define UNIT_BITS  8
#  endif
#endif

#include "aes.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#if 1 || _WIN64
#  define USE_LONGS
#else
#  define USE_FLOATS
#endif

/*  After encryption or decryption operations the return value of
    'compute tag' will be one of the values RETURN_GOOD, RETURN_WARN
    or RETURN_ERROR, the latter indicating an error. A return value
    RETURN_GOOD indicates that both encryption and authentication
    have taken place and resulted in the returned tag value. If
    the returned value is RETURN_WARN, the tag value is the result
    of authentication alone without encryption (CCM) or decryption
    (GCM and EAX).
*/
#ifndef RETURN_GOOD
# define RETURN_WARN      1
# define RETURN_GOOD      0
# define RETURN_ERROR    -1
#endif

#ifndef RET_TYPE_DEFINED
  typedef int  ret_type;
#endif
UNIT_TYPEDEF(cwc_unit_t, UNIT_BITS);
BUFR_TYPEDEF(cwc_buf_t, UNIT_BITS, AES_BLOCK_SIZE);

#define CWC_ABLK_SIZE   12
#define CWC_CBLK_SIZE   AES_BLOCK_SIZE

typedef struct
{   cwc_buf_t        ctr_val;               /* counter (iv) buffer          */
    cwc_buf_t        enc_ctr;               /* encrypted data buffer        */
    cwc_buf_t        cwc_buf;               /* auth data buffer             */
    aes_encrypt_ctx enc_ctx[1];             /* AES context                  */
#if defined( USE_LONGS )
    uint32_t   zval[CWC_CBLK_SIZE >> 2];    /* cwc z value                  */
    uint32_t   hash[CWC_CBLK_SIZE >> 1];    /* running hash value           */
#endif
#if defined( USE_FLOATS )
    double          zval[6];                /* cwc z value                  */
    double          hash[6];                /* running hash value           */
#endif
    uint32_t   hdr_cnt;                     /* header bytes so far          */
    uint32_t   txt_ccnt;                    /* text bytes so far (encrypt)  */
    uint32_t   txt_acnt;                    /* text bytes so far (auth)     */
} cwc_ctx;

/* The following calls handle mode initialisation, keying and completion    */

ret_type cwc_init_and_key(                  /* initialise mode and set key  */
            const unsigned char key[],      /* the key value                */
            unsigned long key_len,          /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

ret_type cwc_end(                           /* clean up and end operation   */
            cwc_ctx ctx[1]);                /* the mode context             */

/* The following calls handle complete messages in memory as one operation  */

ret_type cwc_encrypt_message(               /* encrypt an entire message    */
            const unsigned char iv[],       /* the initialisation vector    */
            unsigned long iv_len,           /* and its length in bytes      */
            const unsigned char hdr[],      /* the header buffer            */
            unsigned long hdr_len,          /* and its length in bytes      */
            unsigned char msg[],            /* the message buffer           */
            unsigned long msg_len,          /* and its length in bytes      */
            unsigned char tag[],            /* the buffer for the tag       */
            unsigned long tag_len,          /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

                                /* RETURN_GOOD is returned if the input tag */
                                /* matches that for the decrypted message   */
ret_type cwc_decrypt_message(               /* decrypt an entire message    */
            const unsigned char iv[],       /* the initialisation vector    */
            unsigned long iv_len,           /* and its length in bytes      */
            const unsigned char hdr[],      /* the header buffer            */
            unsigned long hdr_len,          /* and its length in bytes      */
            unsigned char msg[],            /* the message buffer           */
            unsigned long msg_len,          /* and its length in bytes      */
            const unsigned char tag[],      /* the buffer for the tag       */
            unsigned long tag_len,          /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

/* The following calls handle messages in a sequence of operations followed */
/* by tag computation after the sequence has been completed. In these calls */
/* the user is responsible for verfiying the computed tag on decryption     */

ret_type cwc_init_message(                  /* initialise a new message     */
            const unsigned char iv[],       /* the initialisation vector    */
            unsigned long iv_len,           /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

ret_type cwc_auth_header(                   /* authenticate the header      */
            const unsigned char hdr[],      /* the header buffer            */
            unsigned long hdr_len,          /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

ret_type cwc_encrypt(                       /* encrypt & authenticate data  */
            unsigned char data[],           /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

ret_type cwc_decrypt(                       /* authenticate & decrypt data  */
            unsigned char data[],           /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

ret_type cwc_compute_tag(                   /* compute authentication tag   */
            unsigned char tag[],            /* the buffer for the tag       */
            unsigned long tag_len,          /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

/*  The use of the following calls should be avoided if possible because 
    their use requires a very good understanding of the way this encryption 
    mode works and the way in which this code implements it in order to use
    them correctly.

    The cwc_auth_data routine is used to authenticate encrypted message data.
    In message encryption cwc_crypt_data must be called before cwc_auth_data
    is called since it is encrypted data that is authenticated.  In message
    decryption authentication must occur before decryption and data can be
    authenticated without being decrypted if necessary.

    If these calls are used it is up to the user to ensure that these routines
    are called in the correct order and that the correct data is passed to 
    them.

    When cwc_compute_tag is called it is assumed that an error in use has
    occurred if both encryption (or decryption) and authentication have taken
    place but the total lengths of the message data respectively authenticated
    and encrypted are not the same. If authentication has taken place but
    there has been no corresponding encryption or decryption operations (none 
    at all) only a warning is issued. This should be treated as an error if it
    occurs during encryption but it is only signalled as a warning as it might 
    be intentional when decryption operations are involved (this avoids having
    different compute tag functions for encryption and decryption). Decryption
    operations can be undertaken freely after authetication but if the tag is
    computed after such operations an error will be signalled if the lengths
    of the data authenticated and decrypted don't match.
*/

ret_type cwc_auth_data(                     /* authenticate ciphertext      */
            const unsigned char data[],     /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

ret_type cwc_crypt_data(                    /* encrypt or decrypt data      */
            unsigned char data[],           /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            cwc_ctx ctx[1]);                /* the mode context             */

#if defined(__cplusplus)
}
#endif

#endif
