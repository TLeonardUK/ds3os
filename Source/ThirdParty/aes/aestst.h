/*
---------------------------------------------------------------------------
Copyright (c) 1998-2013, Brian Gladman, Worcester, UK. All rights reserved.

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
*/

// The following definitions are required for testing only, They are not needed
// for AES (Rijndael) implementation.  They are used to allow C, C++ and DLL
// data access and subroutine calls to be expressed in the same form in the
// testing code.

#ifndef AESTST_H
#define AESTST_H

#define abs_path    ".\\"
#define rel_path    "..\\"
#define ref_path    "testvals\\"                    // path for test vector files
#define out_path    "outvals\\"                     // path for output files
#if 0
#  define ar_path   abs_path
#else
#  define ar_path   rel_path
#endif

#if defined( _WIN64 )
#  define PLFRM     "x64\\"
#else
#  define PLFRM     "win32\\"
#endif

#if defined( _DEBUG )
#  define CNFIG     "debug\\"
#else
#  define CNFIG     "release\\"
#endif

#define dll_path    "..\\dll\\" PLFRM CNFIG "aes.dll"   // path for DLL

#if defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )

#include <windows.h>

#define f_info(x)   (x)->inf.b[2]
#define f_ectx      aes_encrypt_ctx
#define f_dctx      aes_decrypt_ctx

#if defined(__cplusplus)
extern "C"
{
#endif

typedef AES_RETURN g_enc_key(const unsigned char*, aes_encrypt_ctx[]);
typedef AES_RETURN g_dec_key(const unsigned char*, aes_decrypt_ctx[1]);
typedef AES_RETURN g_enc_keyv(const unsigned char*, int, aes_encrypt_ctx[1]);
typedef AES_RETURN g_dec_keyv(const unsigned char*, int, aes_decrypt_ctx[1]);
typedef AES_RETURN g_enc_blk(const unsigned char*, unsigned char*, const aes_encrypt_ctx[1]);
typedef AES_RETURN g_dec_blk(const unsigned char*, unsigned char*, const aes_decrypt_ctx[1]);

typedef AES_RETURN g_talign(unsigned int n);
typedef AES_RETURN g_reset(const aes_encrypt_ctx[1]);
typedef AES_RETURN g_enc1(const unsigned char*, unsigned char*, int, const aes_encrypt_ctx[1]);
typedef AES_RETURN g_dec1(const unsigned char*, unsigned char*, int, const aes_decrypt_ctx[1]);
typedef AES_RETURN g_enc2(const unsigned char*, unsigned char*, int, unsigned char*, const aes_encrypt_ctx[1]);
typedef AES_RETURN g_dec2(const unsigned char*, unsigned char*, int, unsigned char*, const aes_decrypt_ctx[1]);
typedef AES_RETURN g_enc3(const unsigned char*, unsigned char*, int, unsigned char*, aes_encrypt_ctx[1]);
typedef void (*cif)(unsigned char*);
typedef AES_RETURN g_enc4(const unsigned char*, unsigned char*, int, unsigned char*, cif, aes_encrypt_ctx[1]);

typedef struct  // initialised with subroutine addresses when the DLL is loaded
{
    g_enc_key   *fn_enc_key128;
    g_enc_key   *fn_enc_key192;
    g_enc_key   *fn_enc_key256;
    g_enc_keyv  *fn_enc_key;
    g_enc_blk   *fn_enc_blk;

    g_dec_key   *fn_dec_key128;
    g_dec_key   *fn_dec_key192;
    g_dec_key   *fn_dec_key256;
    g_dec_keyv  *fn_dec_key;
    g_dec_blk   *fn_dec_blk;

	g_talign	*fn_test_align;
    g_reset     *fn_mode_reset;
    g_enc1      *fn_ecb_enc;
    g_dec1      *fn_ecb_dec;
    g_enc2      *fn_cbc_enc;
    g_dec2      *fn_cbc_dec;
    g_enc3      *fn_cfb_enc;
    g_enc3      *fn_cfb_dec;
    g_enc3      *fn_ofb_cry;
    g_enc4      *fn_ctr_cry;
} fn_ptrs;

#if defined(__cplusplus)
}
#endif

#define f_dat(a,b)              (a->b)
#define f_enc_key128(a,b)       (fn.fn_enc_key128)((b),(a))
#define f_enc_key192(a,b)       (fn.fn_enc_key192)((b),(a))
#define f_enc_key256(a,b)       (fn.fn_enc_key256)((b),(a))
#define f_enc_key(a,b,c)        (fn.fn_enc_key)((b),(c),(a))
#define f_enc_blk(a,b,c)        (fn.fn_enc_blk)((b),(c),(a))

#define f_dec_key128(a,b)       (fn.fn_dec_key128)((b),(a))
#define f_dec_key192(a,b)       (fn.fn_dec_key192)((b),(a))
#define f_dec_key256(a,b)       (fn.fn_dec_key256)((b),(a))
#define f_dec_key(a,b,c)        (fn.fn_dec_key)((b),(c),(a))
#define f_dec_blk(a,b,c)        (fn.fn_dec_blk)((b),(c),(a))

#define f_talign(a,b)			(fn.fn_test_align)((b))
#define f_mode_reset(a)         (fn.fn_mode_reset)((a))
#define f_ecb_enc(a,b,c,d)      (fn.fn_ecb_enc)((b),(c),(d),(a))
#define f_ecb_dec(a,b,c,d)      (fn.fn_ecb_dec)((b),(c),(d),(a))
#define f_cbc_enc(a,b,c,d,e)    (fn.fn_cbc_enc)((b),(c),(d),(e),(a))
#define f_cbc_dec(a,b,c,d,e)    (fn.fn_cbc_dec)((b),(c),(d),(e),(a))
#define f_cfb_enc(a,b,c,d,e)    (fn.fn_cfb_enc)((b),(c),(d),(e),(a))
#define f_cfb_dec(a,b,c,d,e)    (fn.fn_cfb_dec)((b),(c),(d),(e),(a))
#define f_ofb_cry(a,b,c,d,e)    (fn.fn_ofb_cry)((b),(c),(d),(e),(a))
#define f_ctr_cry(a,b,c,d,e,f)  (fn.fn_ctr_cry)((b),(c),(d),(e),(f),(a))

#if defined( _WIN64 )

#define gt_name             "aes_init"
#define ek_name128          "aes_encrypt_key128"
#define ek_name192          "aes_encrypt_key192"
#define ek_name256          "aes_encrypt_key256"
#define ek_name             "aes_encrypt_key"
#define eb_name             "aes_encrypt"
#define dk_name128          "aes_decrypt_key128"
#define dk_name192          "aes_decrypt_key192"
#define dk_name256          "aes_decrypt_key256"
#define dk_name             "aes_decrypt_key"
#define db_name             "aes_decrypt"

#define etad_name           "aes_test_alignment_detection"
#define eres_name           "aes_mode_reset"
#define ecbe_name           "aes_ecb_encrypt"
#define ecbd_name           "aes_ecb_decrypt"
#define cbce_name           "aes_cbc_encrypt"
#define cbcd_name           "aes_cbc_decrypt"
#define cfbe_name           "aes_cfb_encrypt"
#define cfbd_name           "aes_cfb_decrypt"
#define ofb_name            "aes_ofb_crypt"
#define ctr_name            "aes_ctr_crypt"

#else

#define gt_name             "_aes_init@0"
#define ek_name128          "_aes_encrypt_key128@8"
#define ek_name192          "_aes_encrypt_key192@8"
#define ek_name256          "_aes_encrypt_key256@8"
#define ek_name             "_aes_encrypt_key@12"
#define eb_name             "_aes_encrypt@12"
#define dk_name128          "_aes_decrypt_key128@8"
#define dk_name192          "_aes_decrypt_key192@8"
#define dk_name256          "_aes_decrypt_key256@8"
#define dk_name             "_aes_decrypt_key@12"
#define db_name             "_aes_decrypt@12"

#define etad_name           "_aes_test_alignment_detection@4"
#define eres_name           "_aes_mode_reset@4"
#define ecbe_name           "_aes_ecb_encrypt@16"
#define ecbd_name           "_aes_ecb_decrypt@16"
#define cbce_name           "_aes_cbc_encrypt@20"
#define cbcd_name           "_aes_cbc_decrypt@20"
#define cfbe_name           "_aes_cfb_encrypt@20"
#define cfbd_name           "_aes_cfb_decrypt@20"
#define ofb_name            "_aes_ofb_crypt@20"
#define ctr_name            "_aes_ctr_crypt@24"

#endif

#elif defined( __cplusplus )

#define f_info(x)               (x)->cx->inf.b[2]
#define f_ectx                  AESencrypt
#define f_enc_key128(a,b)       (a)->key128((b))
#define f_enc_key192(a,b)       (a)->key192((b))
#define f_enc_key256(a,b)       (a)->key256((b))
#define f_enc_key(a,b,c)        (a)->key((b),(c))
#define f_enc_blk(a,b,c)        (a)->encrypt((b),(c))

#define f_dctx                  AESdecrypt
#define f_dec_key128(a,b)       (a)->key128((b))
#define f_dec_key192(a,b)       (a)->key192((b))
#define f_dec_key256(a,b)       (a)->key256((b))
#define f_dec_key(a,b,c)        (a)->key((b),(c))
#define f_dec_blk(a,b,c)        (a)->decrypt((b),(c))

#define f_talign(a,b)			(a)->fn_test_align(b)
#define f_mode_reset(a)         (a)->mode_reset()
#define f_ecb_enc(a,b,c,d)      (a)->ecb_encrypt((b),(c),(d))
#define f_ecb_dec(a,b,c,d)      (a)->ecb_decrypt((b),(c),(d))
#define f_cbc_enc(a,b,c,d,e)    (a)->cbc_encrypt((b),(c),(d),(e))
#define f_cbc_dec(a,b,c,d,e)    (a)->cbc_decrypt((b),(c),(d),(e))
#define f_cfb_enc(a,b,c,d,e)    (a)->cfb_encrypt((b),(c),(d),(e))
#define f_cfb_dec(a,b,c,d,e)    (a)->cfb_decrypt((b),(c),(d),(e))
#define f_ofb_cry(a,b,c,d,e)    (a)->ofb_crypt((b),(c),(d),(e))
#define f_ctr_cry(a,b,c,d,e,f)  (a)->ctr_crypt((b),(c),(d),(e),(f))

#else

#define f_info(x)               (x)->inf.b[2]
#define f_ectx                  aes_encrypt_ctx
#define f_enc_key128(a,b)       aes_encrypt_key128((b),(a))
#define f_enc_key192(a,b)       aes_encrypt_key192((b),(a))
#define f_enc_key256(a,b)       aes_encrypt_key256((b),(a))
#define f_enc_key(a,b,c)        aes_encrypt_key((b),(c),(a))
#define f_enc_blk(a,b,c)        aes_encrypt((b),(c),(a))

#define f_dctx                  aes_decrypt_ctx
#define f_dec_key128(a,b)       aes_decrypt_key128((b),(a))
#define f_dec_key192(a,b)       aes_decrypt_key192((b),(a))
#define f_dec_key256(a,b)       aes_decrypt_key256((b),(a))
#define f_dec_key(a,b,c)        aes_decrypt_key((b),(c),(a))
#define f_dec_blk(a,b,c)        aes_decrypt((b),(c),(a))

#define f_talign(a,b)			aes_test_alignment_detection(b)
#define f_mode_reset(a)         aes_mode_reset(a)
#define f_ecb_enc(a,b,c,d)      aes_ecb_encrypt((b),(c),(d),(a))
#define f_ecb_dec(a,b,c,d)      aes_ecb_decrypt((b),(c),(d),(a))
#define f_cbc_enc(a,b,c,d,e)    aes_cbc_encrypt((b),(c),(d),(e),(a))
#define f_cbc_dec(a,b,c,d,e)    aes_cbc_decrypt((b),(c),(d),(e),(a))
#define f_cfb_enc(a,b,c,d,e)    aes_cfb_encrypt((b),(c),(d),(e),(a))
#define f_cfb_dec(a,b,c,d,e)    aes_cfb_decrypt((b),(c),(d),(e),(a))
#define f_ofb_cry(a,b,c,d,e)    aes_ofb_crypt((b),(c),(d),(e),(a))
#define f_ctr_cry(a,b,c,d,e,f)  aes_ctr_crypt((b),(c),(d),(e),(f),(a))

#define ek_name128          "aes_encrypt_key128"
#define ek_name192          "aes_encrypt_key192"
#define ek_name256          "aes_encrypt_key256"
#define ek_name             "aes_encrypt_key"
#define eb_name             "aes_encrypt"

#define dk_name128          "aes_decrypt_key128"
#define dk_name192          "aes_decrypt_key192"
#define dk_name256          "aes_decrypt_key256"
#define dk_name             "aes_decrypt_key"
#define db_name             "aes_decrypt"

#define eres_name           "aes_mode_reset"
#define ecbe_name           "aes_ecb_encrypt"
#define ecbd_name           "aes_ecb_decrypt"
#define cbce_name           "aes_cbc_encrypt"
#define cbcd_name           "aes_cbc_decrypt"
#define cfbe_name           "aes_cfb_encrypt"
#define cfbd_name           "aes_cfb_decrypt"
#define ofb_name            "aes_ofb_crypt"
#define ctr_name            "aes_ctr_crypt"

#endif

#ifndef AES_N_BLOCK
#define do_enc(a,b,c,d) f_enc_blk(a, b, c)
#define do_dec(a,b,c,d) f_dec_blk(a, b, c)
#else
#define do_enc(a,b,c,d) f_ecb_enc(a, b, c, 1)
#define do_dec(a,b,c,d) f_ecb_dec(a, b, c, 1)
#endif

#endif
