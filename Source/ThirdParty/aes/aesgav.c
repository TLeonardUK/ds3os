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
Issue Date: 25/09/2018
*/

// AES Algorithm Test - Generate local test files for KAT and Monte Carlo
// tests and optionally compare these files with a  reference set of test
// files
//
// usage: aes_gav /t:[knec] /b:[45678] /k:[45678] [/c]
//
// where:
//
// each of the symbols in square brackets can be used in any combination so
// that:
//
//         /t:[knec]  selects the tests to be used
//         /b:[45678] selects the block lengths to be used (in 32-bit words)
//         /k:[45678] selects the key lengths to be used (in 32-bit words)
//         /c         selects the comparison of output and reference file(s)
// and:
//         k: generate ECB Known Answer Test files
//         n: generate ECB Known Answer Test files (new)
//         e: generate ECB Monte Carlo Test files
//         c: generate CBC Monte Carlo Test files
//
// The reference files have a ".txt" extension while those generated use
// ".txt" if in a different directory or ".dat" otherwise. The directory
// paths for files are set below. Note that, when compared with the NIST
// test vector sequences, this code implements one additional (all zero)
// test vector as the first vector in each set (test 0).

#if defined( _MSC_VER ) && defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )
#include <windows.h>
#endif

#if defined( __cplusplus )
#  include "aescpp.h"
#else
#  include "aes.h"
#endif
#include "aesaux.h"
#include "aestst.h"

#if defined(DLL_IMPORT) && defined( DLL_DYNAMIC_LOAD )
fn_ptrs fn;
#endif

// Outputs a test vector file header

void header(FILE *outf, const int type, const unsigned long blen, const unsigned long klen)
{   char    buf[32];
    unsigned char    dummy;

    fprintf(outf, "==============================================");
    fprintf(outf, "\nAuthor:    Dr B R Gladman (brg@gladman.me.uk)");
    fprintf(outf, "\nTest:      %s", (type < 6 ? "ECB " : "CBC "));
    switch(type)
    {
        case  0:    fprintf(outf, "Variable Key Known Answer Tests"); break;
        case  2:    fprintf(outf, "Variable Key Known Answer Tests"); break;
        case  1:    fprintf(outf, "Variable Text Known Answer Tests"); break;
        case  3:    fprintf(outf, "Variable Text Known Answer Tests"); break;
        case  4:
        case  6:    fprintf(outf, "Monte Carlo (Encryption) Tests"); break;
        case  5:
        case  7:    fprintf(outf, "Monte Carlo (Decryption) Tests"); break;
    }

    fprintf(outf, "\nAlgorithm: Extended Rijndael (an AES Superset)\nFilename:  %s",
         file_name(buf, 32, type, blen, klen));
    fprintf(outf, "\n==============================================\n");

    block_out(block_len, &dummy, outf, 8 * blen);
    block_out(key_len, &dummy, outf, 8 * klen);
    fprintf(outf, "\n");
}

// Test of Electronic Code Book (ECB) mode with Fixed Key and Variable Text

void ecb_vt(FILE *outf, f_ectx alg[1], const unsigned long blen, const unsigned long klen)
{   unsigned long      j;
    unsigned char      pt[32], ct[32], key[32], dummy;

    block_clear(key, klen);                         // all zero key
    block_out(key_val, key, outf, klen);            // output key value
    f_enc_key(alg, key, klen);                      // set key value

    for(j = 0; j <= 8 * blen; ++j)                  // test vectors include
    {                                               // an all zero one
        block_out(test_no, &dummy, outf, j);        // output test number
        block_clear(pt, blen);                      // set all zero plain text
        if(j)                                       // set bit (j-1) if j <> 0
            *(pt + (j - 1) / 8) = 0x80 >> (j - 1) % 8;
        block_out(pt_val, pt, outf, blen);          // output plaintext
        do_enc(alg, pt , ct, 1);                 // do encryption
        block_out(ct_val, ct, outf, blen);          // output ciphertext
    }
}

// Test of Electronic Code Book (ECB) mode with Fixed Text and Variable Key

void ecb_vk(FILE *outf, f_ectx alg[1], const unsigned long blen, const unsigned long klen)
{   unsigned long      j;
    unsigned char      pt[32], ct[32], key[32], dummy;

    block_clear(pt, blen);                          // all zero plaintext
    block_out(pt_val, pt, outf, blen);              // output plaintext

    for(j = 0; j <= 8 * klen; ++j)                  // 129, 193 or 257 tests
    {
        block_out(test_no, &dummy, outf, j);        // output test number
        block_clear(key, klen);                     // set all zero key
        if(j)                                       // set bit (j-1) if j <> 0
            *(key + (j - 1) / 8) = 0x80 >> (j - 1) % 8;
        block_out(key_val, key, outf, klen);        // output key value
        f_enc_key(alg, key, klen);                  // set key value
        do_enc(alg, pt , ct, 1);                 // alg.encrypt
        block_out(ct_val, ct, outf, blen);          // output ciphertext
    }
}

// Test of Electronic Code Book (ECB) mode with Fixed Key and Variable Text

void ecb_vtn(FILE *outf, f_ectx alg[1], const unsigned long blen, const unsigned long klen)
{   unsigned long      j;
    unsigned char      pt[32], ct[32], key[32], *bp, dummy;

    block_clear(key, klen);                         // all zero key
    block_out(key_val, key, outf, klen);            // output key value
    f_enc_key(alg, key, klen);                      // set key value
    block_clear(pt, blen);                          // set all zero plain text

    for(j = 0; j < 16 * blen; ++j)
    {
        block_out(test_no, &dummy, outf, j);        // output test number
        block_out(pt_val, pt, outf, blen);          // output plaintext
        do_enc(alg, pt , ct, 1);                 // do encryption
        block_out(ct_val, ct, outf, blen);          // output ciphertext
        bp = pt + blen - 1 - j / 8;
        if(j < 8 * blen)
            *bp |= (*bp << 1) | 1;
        else
            *(bp + blen) = *(bp + blen) << 1;
    }
}

// Test of Electronic Code Book (ECB) mode with Fixed Text and Variable Key

void ecb_vkn(FILE *outf, f_ectx alg[1], const unsigned long blen, const unsigned long klen)
{   unsigned long      j;
    unsigned char      pt[32], ct[32], key[32], *bp, dummy;

    block_clear(pt, blen);                          // all zero plaintext
    block_out(pt_val, pt, outf, blen);              // output plaintext
    block_clear(key, klen);

    for(j = 0; j < 16 * klen; ++j)
    {
        block_out(test_no, &dummy, outf, j);        // output test number
        block_out(key_val, key, outf, klen);        // output key value
        f_enc_key(alg, key, klen);                  // set key value
        do_enc(alg, pt , ct, 1);                 // alg.encrypt
        block_out(ct_val, ct, outf, blen);          // output ciphertext
        bp = key + klen - 1 - j / 8;
        if(j < 8 * klen)
            *bp |= (*bp << 1) | 1;
        else
            *(bp + klen) = *(bp + klen) << 1;
    }
}

// Monte Carlo Encryption Test of Electronic Code Book (ECB) mode

void ecb_me(FILE *outf, f_ectx alg[1], const unsigned long blen, const unsigned long klen)
{   unsigned long  j, k;
    unsigned char  pt[32], ct[64], key[32], dummy;

    block_clear(pt, blen);                          // clear initial plaintext
    block_clear(key, klen);                         // and key blocks
    block_copy(ct + blen, pt, blen);                // put plaintext in upper half
                                                    // of double length buffer
    for(j = 0; j < 400; j++)                        // 400 Monte Carlo tests
    {
        block_out(test_no, &dummy, outf, j);        // output test number
        block_out(key_val, key, outf, klen);        // output key
        block_out(pt_val, pt, outf, blen);          // output plaintext
        f_enc_key(alg, key, klen);                  // set the key

        for(k = 0; k < 5000; ++k)                   // 10000 encryptions alternating
        {                                           // upper and lower blocks in ct
            do_enc(alg, ct + blen, ct, 1);
            do_enc(alg, ct, ct + blen, 1);
        }

        // compile next key as defined by NIST

        block_xor(key, ct + 2 * blen - klen, klen);
        block_out(ct_val, ct + blen, outf, blen);   // output ciphertext
        block_copy(pt, ct + blen, blen);            // copy ciphertext as next plaintext
    }
}

// Monte Carlo Decryption Test of Electronic Code Book (ECB) mode

void ecb_md(FILE *outf, f_dctx alg[1], const unsigned long blen, const unsigned long klen)
{   unsigned long  j, k;
    unsigned char  pt[32], ct[64], key[32], dummy;

    block_clear(pt, blen);                          // clear initial plaintext and key
    block_clear(key, klen);
    block_copy(ct + blen, pt, blen);                // copy plaintext into upper half
                                                    // of double length ciphertext block
    for(j = 0; j < 400; j++)                        // 400 Monte Carlo tests
    {
        block_out(test_no, &dummy, outf, j);        // output test number
        block_out(key_val, key, outf, klen);        // output key
        block_out(ct_val, pt, outf, blen);          // output plaintext
        f_dec_key(alg, key, klen);                  // set key

        for(k = 0; k < 5000; ++k)                   // 10000 decryptions alternating
        {                                           // upper and lower blocks in ct
            do_dec(alg, ct + blen, ct, 1);
            do_dec(alg, ct, ct + blen, 1);
        }

        // compile next key as defined by NIST

        block_xor(key, ct + 2 * blen - klen, klen);
        block_out(pt_val, ct + blen, outf, blen);   // output ciphertext
        block_copy(pt, ct + blen, blen);            // set ciphertext as next plaintext
    }
}

// Monte Carlo Encryption Test of Cipher Block Chaining (CBC) mode

void cbc_me(FILE *outf, f_ectx alg[1], const unsigned long blen, const unsigned long klen)
{   unsigned long  j, k;
    unsigned char  ct[64], key[32], dummy;

    block_clear(key, klen);                         // clear key: KEY[0]
    block_clear(ct, 2 * blen);                      // clear ct:  PT[0], ct + blen: IV[0]

    for(j = 0; j < 400; j++)                        // 400 Monte Carlo tests
    {
        block_out(test_no, &dummy, outf, j);        // output test number
        block_out(key_val, key, outf, klen);        // output key
        block_out(iv_val, ct + blen, outf, blen);   // output initialisation vector
        block_out(pt_val, ct, outf, blen);          // output plaintext
        f_enc_key(alg, key, klen);                  // set key

        for(k = 0; k < 5000; ++k)                   // 10000 encryptions, two at a time
        {
            block_xor(ct, ct + blen, blen);         // do CBC chaining
            do_enc(alg, ct, ct, 1);              // do block encryption
            block_xor(ct + blen, ct, blen);         // do CBC chaining
            do_enc(alg, ct + blen, ct + blen, 1);// do block encryption

        }

        block_out(ct_val, ct + blen, outf, blen);   // output ciphertext
        // compile next key as defined by NIST
        block_xor(key, ct + 2 * blen - klen, klen);
    }
}

// Monte Carlo Encryption Test of Cipher Block Chaining (CBC) mode

void cbc_md(FILE *outf, f_dctx alg[1], const unsigned long blen, const unsigned long klen)
{   unsigned long  j, k;
    unsigned char  pt[32], ct[64], key[32], dummy;

    block_clear(key, klen);                         // clear key: KEY[0]
    block_clear(ct, 2 * blen);                      // clear ct: IV[0] ct + blen: CT[0]

    for(j = 0; j < 400; j++)                        // 400 Monte Carlo tests
    {
        block_out(test_no, &dummy, outf, j);        // output test number
        block_out(key_val, key, outf, klen);        // output key
        block_out(iv_val, ct, outf, blen);          // output initialisation vector
        block_out(ct_val, ct + blen, outf, blen);   // output ciphertext
        f_dec_key(alg, key, klen);                  // set key

        for(k = 0; k < 5000; ++k)                   // 10000 encryptions, two at a time
        {
            do_dec(alg, ct + blen, pt, 1);       // do block decryption
            block_xor(ct, pt, blen);                // do CBC chaining
            do_dec(alg, ct, pt, 1);              // do block decryption
            block_xor(ct + blen, pt, blen);         // do CBC chaining
        }

        block_out(pt_val, ct + blen, outf, blen);   // output plaintext
        // compile next key as defined by NIST
        block_xor(key, ct + 2 * blen - klen, klen);
    }
}

// Synchronise two comparison files if they get out of step

int sync(int nbr, FILE *inf, char str[], int outp)
{   enum line_type  ty;
    int             nn;

    for(;;)
    {
        ty = find_line(inf, str);

        if(ty == bad_line) return -1;

        if(ty == test_no)
        {
            nn = get_dec(str + 2);

            if(nn >= nbr) return nn;
        }

        if(outp)
            printf("\n  %s", str);
    }
}

// Compare two test vector files

void comp_vecs(const char *fn1, const char *fn2)
{   char            str1[128], str2[128];
    enum line_type  ty1, ty2;
    int             no1, no2, err_cnt, np_cnt, req;
    FILE            *if1, *if2;

    err_cnt = np_cnt = 0; req = TRUE;

    if(fopen_s(&if1, fn1, "r"))
    {
        printf("\n*** 1st file (%s) not found ***", fn1); return;
    }

    if(fopen_s(&if2, fn2, "r"))
    {
        printf("\n*** 2nd file (%s) not found ***", fn2); return;
    }

    for(;;)         // while there is still input
    {
        if(req)     // if another line needs to be input
        {
            ty1 = find_line(if1, str1); ty2 = find_line(if2, str2);
        }

        if(ty1 == bad_line && ty2 == bad_line)      // if end of file on both files
            break;

        if(ty1 == bad_line || ty2 == bad_line)      // if end of file on one file
        {
            printf("\n%s%s%s%s" , fn1, (ty1 == bad_line ? " short" : " long"), "er than ", fn2);
            break;
        }

        if(ty1 == test_no)          // if 'test number' line in file 1

            no1 = get_dec(str1);

        if(ty2 == test_no)          // if 'test number' line in file 2

            no2 = get_dec(str2);

        if(cmp_nocase(str1, str2) == 0)
        {
            req = TRUE; continue;   // if lines are the same continue
        }

        if(ty1 == test_no && ty2 == test_no)    // if not the same but both are at a
        {                                       // 'test number' line
            np_cnt += abs(no2 - no1); req = FALSE;

            if(no2 < no1)   // extra tests in file 2
            {
                printf("\nextra test(s) in %s:\n  %s", fn2, str2);
                no2 = sync(no1, if2, str2, np_cnt < 10); // skip tests in file 2
            }

            if(no1 < no2)   // extra test in file 1
            {
                printf("\nextra test(s) in %s:\n  %s", fn1, str1);
                no1 = sync(no2, if1, str1, np_cnt < 10);// skip tests in file 1
            }
        }
        else if(ty1 != ty2) // cannot synchronise test vector files
        {
            printf("\n*** synchronisation error tests %i and %i ***", no1, no2);
            fflush(stdout); return;
        }
        else if(ty1 != bad_line)   // test vector mismatch
        {
            err_cnt++;

            printf("\r*** mismatch error test %i ***", no1);
        }

        fflush(stdout);
    }

    if(np_cnt && !err_cnt)  // all tests present match

        printf("\nother tests match\n");

    else
    {
        if(err_cnt)
            printf("\r%s doesn't match %s (%i errors)\n", fn2, fn1, err_cnt);
        else
            printf("\r%s matches %s\n", df_string(fn2), df_string(fn1));
    }

    fclose(if1); fclose(if2);
}

// array of functions to call for each test

typedef void (*f_ep)(FILE *outf, f_ectx alg[1], const unsigned long blen, const unsigned long klen);
typedef void (*f_dp)(FILE *outf, f_dctx alg[1], const unsigned long blen, const unsigned long klen);

f_ep f_ptr[8] = { ecb_vk, ecb_vt, ecb_vkn, ecb_vtn, ecb_me, (f_ep)ecb_md, cbc_me, (f_ep)cbc_md };

// do the tests for each algorithm

void do_tests(int do_cmp, int ttype[3], f_ectx alg[1], const unsigned long blen, const unsigned long klen)
{   char       name1[128], name2[128], *sp1, *sp2;
    int        i;
    FILE       *outf;

    printf("\nGenerate%s tests for aes (AES_BLOCK_SIZE = %lu, key size = %lu)\n",
            (do_cmp ? " and verify" : ""), 8 * blen, 8 * klen);

    for(i = 0; i < 8; ++i)  // for each type of test /k /x /e /c (2 tests each)
        if(ttype[i / 2])    // if this test required
        {
            // name of file for output of generated test vectors
            sp1 = copy_str(name1, ar_path);
            sp1 = copy_str(sp1, out_path);
            sp2 = copy_str(name2, ar_path);
            sp2 = copy_str(sp2, ref_path);
            file_name(sp1, 128, i, blen, klen);
            copy_str(sp2, sp1);

            if(!fopen_s(&outf, name1, "w"))
            {
                header(outf, i, blen, klen);
                f_ptr[i](outf, alg, blen, klen);
                fprintf(outf, "\n"); fclose(outf);

                if(do_cmp)  // compare it with reference if required
                    comp_vecs(name2, name1);
            } 
            else 
            {
                printf("ERROR: failed to open %s for writing\n", name1);
            }
       }
}

int main(int argc, char *argv[])
{   int do_cmp, tyf[4], kf[3], ki;
    f_ectx alg[1];

#if defined(DLL_IMPORT) && defined(DLL_DYNAMIC_LOAD)
    HINSTANCE   h_dll;
    if(!(h_dll = init_dll(&fn)))
        return -1;
#else
    aes_init();
#endif

    if(argc == 1)
    {
        printf("\nusage: aes_gav /t:[knec] /k:[468] [/c]");
        printf("\n");
        printf("\nwhere the symbols in square brackets can be used in");
        printf("\nany combination (without the brackets) and have the");
        printf("\nfollowing meanings:");
        printf("\n");
        printf("\n        /t:[knec]   selects which tests are used");
        printf("\n        /k:[468]    selects the key lengths used");
        printf("\n        /c          compares output with reference");
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

    printf("\nRun tests for the AES algorithm %s",

#if defined(DLL_IMPORT)
    " (DLL Version)\n");
#elif defined(AES_CPP)
    " (CPP Version)\n");
#else
    "");
#endif

    do_cmp = test_args(argc, argv, 'c', '\0');

    tyf[0] = test_args(argc, argv, 't', 'k');
    tyf[1] = test_args(argc, argv, 't', 'n');
    tyf[2] = test_args(argc, argv, 't', 'e');
    tyf[3] = test_args(argc, argv, 't', 'c');

    kf[0] = test_args(argc, argv, 'k', '4');
    kf[1] = test_args(argc, argv, 'k', '6');
    kf[2] = test_args(argc, argv, 'k', '8');

    if(!(kf[0] || kf[1] || kf[2]))
    {
        kf[0] = kf[1] = kf[2] = TRUE;   // AES key sizes if not specified
    }

    for(ki = 0; ki < 3; ++ki) if(kf[ki])
    {
        do_tests(do_cmp, tyf, alg, 16, 16 + 8 * ki);
    }

#if defined(DLL_IMPORT) && defined(DLL_DYNAMIC_LOAD)
    if(h_dll) FreeLibrary(h_dll);
#endif
    printf("\n\n");
    return 0;
}

