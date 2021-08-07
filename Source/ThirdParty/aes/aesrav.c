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

// usage: aes_rav /t:[knec] /b:[45678] /k:[45678]
//
// where:
//
// each of the symbols in square brackets can be used in any combination so
// that:
//
//         /t:[knec]  selects the tests to be used
//         /b:[45678] selects the block lengths to be used (in 32-bit words)
//         /k:[45678] selects the key lengths to be used (in 32-bit words)
// and:
//         k: generate ECB Known Answer Test files
//         n: generate ECB Known Answer Test files (new)
//         e: generate ECB Monte Carlo Test files
//         c: generate CBC Monte Carlo Test files
//
// Note that, when compared with the NIST test vector sequences, this
// code implements one additional (all zero) test vector as the first
// vector in each set (test 0).

#if defined( _MSC_VER )
#  if defined( DUAL_CORE ) || defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )
#    define WINDOWS_LEAN_AND_MEAN
#    include <windows.h>
#  endif
#  define ALIGN(x) __declspec(align(x))
#else
#  define ALIGN(x) __attribute__ ((aligned(x)))
#endif

#if defined( __cplusplus )
#  include "aescpp.h"
#else
#  include "aes.h"
#endif
#include "aesaux.h"
#include "aestst.h"

#if defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )
fn_ptrs fn;
#endif

enum test_type { ecb_vk, ecb_vt, ecb_nvk, ecb_nvt, ecb_me, ecb_md, cbc_me, cbc_md };

#if defined( AES_ENCRYPT )

void set_enc_key(f_ectx algd[1], unsigned char key[], unsigned long klen)
{
#if !(defined( AES_128 ) && defined( AES_192 ) && defined( AES_256 ))
    f_enc_key(algd, key, klen);
#else
    if(klen == 16)
        f_enc_key128(algd, key);
    if(klen == 24)
        f_enc_key192(algd, key);
    if(klen == 32)
        f_enc_key256(algd, key);
#endif
}

#endif

#if defined( AES_DECRYPT )

void set_dec_key(f_dctx algd[1], unsigned char key[], unsigned long klen)
{
#if !(defined( AES_128 ) && defined( AES_192 ) && defined( AES_256 ))
    f_dec_key(algd, key, klen);
#else
    if(klen == 16)
        f_dec_key128(algd, key);
    if(klen == 24)
        f_dec_key192(algd, key);
    if(klen == 32)
        f_dec_key256(algd, key);
#endif
}

#endif

void ref_test(const char *in_file, unsigned int it_cnt, enum test_type t_type, f_ectx alge[1],
                                        f_dctx algd[1], unsigned long blen, unsigned long klen)
{   unsigned long  i, test_cnt, cnt, e_cnt, fe_cnt;
    ALIGN(16)      unsigned char key[32], pt[32], iv[32], ect[32], act[64];
    char           str[128];
    enum line_type ty;
    FILE           *inf;

    if(fopen_s(&inf, in_file, "r"))      // reference test vector file
    {
        printf("Cannot find test vector file (%s)\n", in_file); return;
    }
    else
        printf("Test file %s: ", df_string(in_file));
        
    cnt = 0; e_cnt = test_cnt = 0;

    for(;;)                         // while there are tests
    {
        ty = find_line(inf, str);   // input a line

        if(ty == bad_line)          // until end of file

            break;

        if(ty == block_len)
        {
              if((get_dec(str) >> 3) == blen) continue;

              return;
        }
        else if(ty == key_len)
        {
              if((get_dec(str) >> 3) == klen) continue;

              return;
        }
        else if(ty == test_no)
        {
            test_cnt = get_dec(str); continue;
        }
        else if(ty == iv_val)
        {
            block_in(iv, str); continue;
        }
        else if(ty == key_val)
        {
            block_in(key, str); continue;
        }
        else if(ty == pt_val)
        {
            block_in(pt, str);
            if(t_type != ecb_md && t_type != cbc_md) continue;
        }
        else if(ty == ct_val)
        {
            block_in(ect, str);
            if(t_type == ecb_md || t_type == cbc_md) continue;
        }

#if !defined( AES_VAR )
#if !defined( AES_128 )
        if(klen == 16) continue;
#endif
#if !defined( AES_192 )
        if(klen == 24) continue;
#endif
#if !defined( AES_256 )
        if(klen == 32) continue;
#endif
#endif

#if defined( AES_ENCRYPT )
        if(t_type != ecb_md && t_type != cbc_md)
        {
            set_enc_key(alge, key, klen);           // set the key

            if(t_type == cbc_me)                    // CBC Monte Carlo encryption
            {
                block_copy(act, iv, blen);
                block_copy(act + blen, pt, blen);   // copy IV and plaintext

                for(i = 0; i < it_cnt; i += 2)
                {
                    block_xor(act + blen, act, blen);   // xor low block into high block
                    do_enc(alge, act + blen, act + blen, 1); // encrypt high block
                    block_xor(act, act + blen, blen);   // xor high block into low block
                    do_enc(alge, act, act, 1);       // encrypt low block
                }
            }
            else                                    // ECB Monte Carlo encryption
            {
                block_copy(act, pt, blen);

                for(i = 0; i < it_cnt; ++i)
                    do_enc(alge, act, act, 1);
            }

            if(block_cmp(ect, act, blen) == EXIT_FAILURE)
                if(!e_cnt++)
                    fe_cnt = test_cnt;

#if defined( AES_DECRYPT )
            if(t_type != cbc_me)                    // if ECB mode test decryption
            {
                set_dec_key(algd, key, klen);       // set the key

                for(i = 0; i < it_cnt; ++i)
                    do_dec(algd, act, act, 1);

                if(block_cmp(pt, act, blen) == EXIT_FAILURE)
                    if(!e_cnt++)
                        fe_cnt = test_cnt;
            }
#endif
        }
#endif

#if defined( AES_DECRYPT )

        if(t_type == ecb_md || t_type == cbc_md)
        {
            f_dec_key(algd, key, klen);             // set the key

            block_copy(act, ect, blen);             // encrypted text to low block

            if(t_type == cbc_md)                    // CBC Monte Carlo decryption
            {
                block_copy(act + blen, iv, blen);   // IV to high block

                for(i = 0; i < it_cnt; i += 2)      // do decryptions two at a time
                {
                    do_dec(algd, act, ect, 1);       // decrypt low block
                    block_xor(act + blen, ect, blen);   // xor into high block
                    do_dec(algd, act + blen, ect, 1);// decrypt high block
                    block_xor(act, ect, blen);      // xor into low block
                }
            }
            else    // ECB Monte Carlo decryption
            {
                for(i = 0; i < it_cnt; ++i)
                    do_dec(algd, act, act, 1);
            }

            if(block_cmp(pt, act, blen) == EXIT_FAILURE)
                if(!e_cnt++)
                    fe_cnt = test_cnt;

#if defined( AES_ENCRYPT )

            if(t_type == ecb_md)    // test encryption if ECB mode
            {
                set_enc_key(alge, key, klen);  // set the key

                for(i = 0; i < it_cnt; ++i)
                    do_enc(alge, act, act, 1);

                if(block_cmp(ect, act, blen) == EXIT_FAILURE)
                    if(!e_cnt++)
                        fe_cnt = test_cnt;
            }
#endif
        }
#endif
    }

    fclose(inf);

    if(e_cnt > 0)
        printf("%lu ERRORS during test (first on test %lu)\n", e_cnt, fe_cnt);
    else
        printf("all tests correct\n");
}

void do_tests(int vkt, int vktn, int ecb, int cbc,
              f_ectx alge[1], f_dctx algd[1], unsigned long blen, unsigned long klen)
{   char    path[128], *sp;

    sp = copy_str(path, ar_path);
    sp = copy_str(sp, ref_path);

    if(vkt)
    {
        file_name(sp, 128 - (sp - path), 0, blen, klen);
        ref_test(path, 1, ecb_vk, alge, algd, blen, klen);
        file_name(sp, 128 - (sp - path), 1, blen, klen);
        ref_test(path, 1, ecb_vt, alge, algd, blen, klen);
    }

    if(vktn)
    {
        file_name(sp, 128 - (sp - path), 2, blen, klen);
        ref_test(path, 1, ecb_nvk, alge, algd, blen, klen);
        file_name(sp, 128 - (sp - path), 3, blen, klen);
        ref_test(path, 1, ecb_nvt, alge, algd, blen, klen);
    }

    if(ecb)
    {
        file_name(sp, 128 - (sp - path), 4, blen, klen);
        ref_test(path, 10000, ecb_me, alge, algd, blen, klen);
        file_name(sp, 128 - (sp - path), 5, blen, klen);
        ref_test(path, 10000, ecb_md, alge, algd, blen, klen);
    }

    if(cbc)
    {
        file_name(sp, 128 - (sp - path), 6, blen, klen);
        ref_test(path, 10000, cbc_me, alge, algd, blen, klen);
        file_name(sp, 128 - (sp - path), 7, blen, klen);
        ref_test(path, 10000, cbc_md, alge, algd, blen, klen);
    }
}

int main(int argc, char *argv[])
{   int      vkt, vktn, ecb, cbc, kf[3], ki;
    f_ectx   alge[1];
    f_dctx   algd[1];

#if defined( DLL_IMPORT ) && defined(  DLL_DYNAMIC_LOAD  )
    HINSTANCE   h_dll;
    if(!(h_dll = init_dll(&fn)))
        return -1;
#else
    aes_init();
#endif

    if(argc == 1)
    {
        printf("\nusage: aes_rav /t:[knec] /k:[468]");
        printf("\n");
        printf("\nwhere the symbols in square brackets can be used in");
        printf("\nany combination (without the brackets) and have the");
        printf("\nfollowing meanings:");
        printf("\n");
        printf("\n        /t:[knec]   selects which tests are used");
        printf("\n        /k:[468]    selects the key lengths used");
        printf("\nwhere:");
        printf("\n        k: generate ECB Known Answer Test files");
        printf("\n        n: generate ECB Known Answer Test files (new)");
        printf("\n        e: generate ECB Monte Carlo Test files");
        printf("\n        c: generate CBC Monte Carlo Test files");
        printf("\n");
        printf("\nand the characters giving block and key lengths are");
        printf("\ndigits representing the lengths in 32-bit units.\n\n");
        exit(0);
    }

    printf("\nRun tests for the AES algorithm%s",
#if defined( DLL_IMPORT )
     " (DLL Version)\n\n");
#elif defined( __cplusplus )
     " (CPP Version)\n\n");
#else
    "\n\n");
#endif

    vkt  = test_args(argc, argv, 't', 'k');
    vktn = test_args(argc, argv, 't', 'n');
    ecb  = test_args(argc, argv, 't', 'e');
    cbc  = test_args(argc, argv, 't', 'c');

    kf[0] = test_args(argc, argv, 'k', '4');
    kf[1] = test_args(argc, argv, 'k', '6');
    kf[2] = test_args(argc, argv, 'k', '8');

    if(!(kf[0] || kf[1] || kf[2]))
    {
        kf[0] = kf[1] = kf[2] = TRUE;       // AES key sizes if not specified
    }

#if !defined( AES_VAR )
#if !defined( AES_128 )
    kf[0] = FALSE;
#endif
#if !defined( AES_192 )
    kf[1] = FALSE;
#endif
#if !defined( AES_256 )
    kf[2] = FALSE;
#endif
#endif

    for(ki = 0; ki < 3; ++ki) if(kf[ki])
    {
        do_tests(vkt, vktn, ecb, cbc, alge, algd, 16, 16 + 8 * ki);
    }

#if defined( DLL_IMPORT ) && defined(  DLL_DYNAMIC_LOAD  )
    if(h_dll) FreeLibrary(h_dll);
#endif
    printf("\n\n");
    return 0;
}
