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
Issue Date: 26/09/2018
*/

#if defined( _MSC_VER ) && (defined( DUAL_CORE ) || defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD ))
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "aes.h"
#include "aestst.h"
#include "aesaux.h"

#if defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )
fn_ptrs fn;
#endif

#define BLOCK_SIZE       16
#define MAX_TEXT_SIZE   256
#define INPUT_BUF_SIZE 1024
#define MCT_REPEAT     1000

typedef enum { ECB = 0, CBC, CFB, OFB } mode;
char *mode_str[] = { "ECB", "CBC", "CFB128", "OFB" };

typedef enum { GFSbox = 0, KeySbox, MCT, MMT, VarKey, VarTxt } type;
char *type_str[] = { "GFSbox", "KeySbox", "MCT", "MMT", "VarKey", "VarTxt" };

typedef enum { Key128 = 0, Key192, Key256 } key;
char *klen_str[] = { "128",  "192",  "256" };

typedef enum { L_bad = -1, L_count = 0, L_key, L_iv, L_plaintext, L_ciphertext } line_type;
char *hdr_str[] = { "COUNT = ", "KEY = ", "IV = ", "PLAINTEXT = ", "CIPHERTEXT = " };

char *test_path = "..\\testvals\\avs\\";

enum line_type find_line2(char str[])
{   int i;

    for(i = 0 ; i < sizeof(hdr_str) / sizeof(hdr_str[0]) ; ++i) 
        if(find_string(str, hdr_str[i]) >= 0)
        {
            return (line_type)i;
        }
    return L_bad;
}

void do_encrypt(mode mm, const unsigned char key[], unsigned char iv[],
                const unsigned char pt[], unsigned char ct[], int key_len, int block_len)
{   aes_encrypt_ctx ctx[1];

    f_enc_key(ctx, key, key_len);
    switch(mm)
    {
    case ECB:
        f_ecb_enc(ctx, pt, ct, block_len);
        break;
    case CBC:
        f_cbc_enc(ctx, pt, ct, block_len, iv);
        break;
    case CFB:
        f_cfb_enc(ctx, pt, ct, block_len, iv);
        break;
    case OFB:
        f_ofb_cry(ctx, pt, ct, block_len, iv);
        break;
    }
}

void do_decrypt(mode mm, const unsigned char key[], unsigned char iv[],
                const unsigned char ct[], unsigned char pt[], int key_len, int block_len)
{   aes_decrypt_ctx ctx[1];

    switch(mm)
    {
    case ECB:
        f_dec_key(ctx, key, key_len);
        f_ecb_dec(ctx, ct, pt, block_len);
        break;
    case CBC:
        f_dec_key(ctx, key, key_len);
        f_cbc_dec(ctx, ct, pt, block_len, iv);
        break;
    case CFB:
        f_enc_key((aes_encrypt_ctx*)ctx, key, key_len);
        f_cfb_dec((aes_encrypt_ctx*)ctx, ct, pt, block_len, iv);
        break;
    case OFB:
        f_enc_key((aes_encrypt_ctx*)ctx, key, key_len);
        f_ofb_cry((aes_encrypt_ctx*)ctx, ct, pt, block_len, iv);
        break;
    }
}

void do_mct_encrypt(mode mm, const unsigned char key[], unsigned char iv[],
                    unsigned char pt[], unsigned char ct[], int key_len)
{   aes_encrypt_ctx ctx[1];
    unsigned char tmp[BLOCK_SIZE];
    int i;

    f_enc_key(ctx, key, key_len);
    if(mm == ECB)
    {
        for(i = 0; i < MCT_REPEAT / 2; ++i)
        {
            f_ecb_enc(ctx, pt, ct, BLOCK_SIZE);
            f_ecb_enc(ctx, ct, pt, BLOCK_SIZE);
        }
        memcpy(ct, pt, BLOCK_SIZE);
    }
    else
    {
        memcpy(tmp, iv, BLOCK_SIZE);
        for(i = 0; i < MCT_REPEAT; ++i)
        {
            switch(mm)
            {
            case 1:
                f_cbc_enc(ctx, pt, ct, BLOCK_SIZE, iv);
                break;
            case 2:
                f_cfb_enc(ctx, pt, ct, BLOCK_SIZE, iv);
                break;
            case 3:
                f_ofb_cry(ctx, pt, ct, BLOCK_SIZE, iv);
                break;
            }
            memcpy(pt, tmp, BLOCK_SIZE);
            memcpy(tmp, ct, BLOCK_SIZE);
        }
    }
}

void do_mct_decrypt(mode mm, const unsigned char key[], unsigned char iv[],
                    unsigned char ct[], unsigned char pt[], int key_len)
{   aes_decrypt_ctx ctx[1];
    unsigned char tmp[BLOCK_SIZE];
    int i;
    if(mm == ECB)
    {
        f_dec_key(ctx, key, key_len);
        memcpy(tmp, ct, BLOCK_SIZE);
        for(i = 0; i < MCT_REPEAT / 2; ++i)
        {
            f_ecb_dec(ctx, ct, pt, BLOCK_SIZE);
            f_ecb_dec(ctx, pt, ct, BLOCK_SIZE);
        }
        memcpy(pt, ct, BLOCK_SIZE);
    }
    else
    {
        if(mm == CBC)
            f_dec_key(ctx, key, key_len);
        else
            f_enc_key((aes_encrypt_ctx*)ctx, key, key_len);
        memcpy(tmp, iv, BLOCK_SIZE);
        for(i = 0; i < MCT_REPEAT; ++i)
        {
            switch(mm)
            {
            case 1:
                f_cbc_dec(ctx, ct, pt, BLOCK_SIZE, iv);
                break;
            case 2:
                f_cfb_dec((aes_encrypt_ctx*)ctx, ct, pt, BLOCK_SIZE, iv);
                break;
            case 3:
                f_ofb_cry((aes_encrypt_ctx*)ctx, ct, pt, BLOCK_SIZE, iv);
                break;
            }
            memcpy(ct, tmp, BLOCK_SIZE);
            memcpy(tmp, pt, BLOCK_SIZE);
        }
    }
}

void run_aes_avs_test(mode mm, type tt)
{
    char  path[128], inbuf[1024];
    unsigned char key[2 * BLOCK_SIZE], iv[BLOCK_SIZE], pt[MAX_TEXT_SIZE], ct[MAX_TEXT_SIZE], rt[MAX_TEXT_SIZE];
    int i, err, cnt, key_len, iv_len, pt_len, ct_len;
    FILE *f;
    line_type ty;

    for(i = 0; i < 3; ++i)
    {
        err = 0;
        strcpy(path, test_path);
        strcat(path, mode_str[mm]);
        strcat(path, type_str[tt]);
        strcat(path, klen_str[i]);
        strcat(path, ".rsp");
        if(fopen_s(&f, path, "r"))
        {
            printf("\nUnable to open %s for reading", path);
            return;
        }
        while(get_line(f, inbuf, 1024) == EXIT_SUCCESS)
        {
            if((ty = find_line2(inbuf)) != L_bad)
            {
                switch(ty)
                {
                case L_count:
                    key_len = iv_len = pt_len = ct_len = 0;
                    cnt = get_dec(inbuf);
                    break;
                case L_key:
                    key_len = block_in(key, inbuf);
                    break;
                case L_iv:
                    iv_len = block_in(iv, inbuf);
                    break;
                case L_plaintext:
                    pt_len = block_in(pt, inbuf);
                    if(pt_len == ct_len)
                    {
                        if(tt == MCT)
                            do_mct_decrypt(mm, key, iv, ct, rt, key_len);
                        else
                            do_decrypt(mm, key, iv, ct, rt, key_len, pt_len);
                        if(memcmp(pt, rt, pt_len)){
                            printf("\nError on file %s, on test %i", path, cnt);
                            ++err;
                        }
                    }
                    break;
                case L_ciphertext:
                    ct_len = block_in(ct, inbuf);
                    if(ct_len == pt_len)
                    {
                        if(tt == MCT)
                            do_mct_encrypt(mm, key, iv, pt, rt, key_len);
                        else
                            do_encrypt(mm, key, iv, pt, rt, key_len, pt_len);
                        if(memcmp(ct, rt, pt_len)){
                            printf("\nError on file %s, on test %i", path, cnt);
                            ++err;
                        }
                    }
                    break;
                }
                if(err > 5)                  
                {
                    printf("\nAborted - too many error on file %s", path);
                    break;
                }
            }
        }
        fclose(f);
        if(!err)
            printf("\nCorrect results for %s", path);
    }
}

int main(void)
{   int i, j;

#if defined( DLL_IMPORT ) && defined(  DLL_DYNAMIC_LOAD  )
    HINSTANCE   h_dll;
    printf("Testing with the AES DLL (with dynamic loading)");
    if(!(h_dll = init_dll(&fn)))
           return -1;
#else
    printf("Testing with the AES static Library");
    aes_init();
#endif

    for( i = 0 ; i < 4 ; ++i )
        for( j = 0 ; j < 6 ; ++j)
            run_aes_avs_test((mode)i, (type)j);

#if defined( DLL_IMPORT ) && defined(  DLL_DYNAMIC_LOAD  )
    if(h_dll) FreeLibrary(h_dll);
#endif
    printf("\n\n");
}
