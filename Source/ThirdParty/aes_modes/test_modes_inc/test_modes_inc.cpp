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
Issue Date: 21/07/2009
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if defined(__cplusplus)
extern "C"
{
#endif

const char *t_name[] = {
	"KEY", "HDR", "PTX", "CTX", "TAG", "NCE", "IV ", "VEC", "RPT", "END", "REM", "MDE"
};
const size_t t_length = sizeof(t_name) / sizeof(t_name[0]);

typedef int  ret_type;
typedef ret_type t_init_and_key(const unsigned char*, unsigned long, void*);
typedef ret_type t_init_message(const unsigned char*, unsigned long, void*);
typedef ret_type t_auth_header(const unsigned char*, unsigned long, void*);
typedef ret_type t_auth_data(const unsigned char*, unsigned long, void*);
typedef ret_type t_crypt_data(unsigned char*, unsigned long, void*);
typedef ret_type t_compute_tag(unsigned char*, unsigned long, void*);
typedef ret_type t_encrypt(unsigned char*, unsigned long, void*);
typedef ret_type t_decrypt(unsigned char*, unsigned long, void*);
typedef ret_type t_encrypt_message(
	const unsigned char*, unsigned long,
	const unsigned char*, unsigned long,
	unsigned char*, unsigned long,
	unsigned char*, unsigned long,
	void*);
typedef ret_type t_decrypt_message(
	const unsigned char*, unsigned long,
	const unsigned char*, unsigned long,
	unsigned char*, unsigned long,
	unsigned char*, unsigned long,
	void*);
typedef ret_type t_end(void*);

typedef struct                                                                                                       {
   char                *name;
    t_init_and_key      *init_and_key;
    t_init_message      *init_message;
    t_auth_header       *auth_header;
    t_auth_data         *auth_data;
    t_crypt_data        *crypt_data;
    t_compute_tag       *compute_tag;
    t_encrypt           *encrypt;
    t_decrypt           *decrypt;
    t_encrypt_message   *encrypt_message;
    t_decrypt_message   *decrypt_message;
    t_end               *end;
} mode_fns;

#undef mode

#include "ccm.h"
/* special init_message() call for CCM to replace   */
/* t_init_message below when CCM is being used      */
typedef ret_type s_init_message(const unsigned char*,
            unsigned long, length_t, length_t, unsigned long, void*);

#define boiler \
	f->init_and_key =    (t_init_and_key*) mode(init_and_key); \
	f->init_message =    (t_init_message*) mode(init_message); \
	f->auth_header =     (t_auth_header*) mode(auth_header); \
	f->auth_data =       (t_auth_data*) mode(auth_data); \
	f->crypt_data =      (t_crypt_data*) mode(crypt_data); \
	f->compute_tag =     (t_compute_tag*) mode(compute_tag); \
	f->encrypt =         (t_encrypt*) mode(encrypt); \
	f->decrypt =         (t_decrypt*) mode(decrypt); \
	f->encrypt_message = (t_encrypt_message*) mode(encrypt_message); \
	f->decrypt_message = (t_decrypt_message*) mode(decrypt_message); \
	f->end =             (t_end*) mode(end);

#define mode(f) ccm_##f
void mode(functions)(mode_fns f[1])
{
    f->name = "CCM";
	boiler
}
#undef mode

#include "cwc.h"
#define mode(f) cwc_##f
void mode(functions)(mode_fns f[1])
{
    f->name = "CWC";
	boiler
}
#undef mode

#include "eax.h"
#define mode(f) eax_##f
void mode(functions)(mode_fns f[1])
{
    f->name = "EAX";
	boiler
}
#undef mode

#include "gcm.h"
#define mode(f) gcm_##f
void mode(functions)(mode_fns f[1])
{
    f->name = "GCM";
	boiler
}
#undef mode

#define BLOCK_SIZE AES_BLOCK_SIZE

#if defined(__cplusplus)
}
#endif

unsigned int rand32(void)
{   static unsigned int   r4,r_cnt = -1,w = 521288629,z = 362436069;

    z = 36969 * (z & 65535) + (z >> 16);
    w = 18000 * (w & 65535) + (w >> 16);

    r_cnt = 0; r4 = (z << 16) + w; return r4;
}

unsigned char rand8(void)
{   static unsigned int   r4,r_cnt = 4;

    if(r_cnt == 4)
    {
        r4 = rand32(); r_cnt = 0;
    }

    return (char)(r4 >> (8 * r_cnt++));
}

// fill a block with random charactrers

void block_rndfill(unsigned char l[], unsigned int len)
{   unsigned int  i;

    for(i = 0; i < len; ++i)

        l[i] = rand8();
}

void do_test(mode_fns f[1], int key_len, int iv_len, int hdr_len, int ptx_len, void* ctxa, void* ctxb, int flag)
{   unsigned char   key[32], iv[256], tag1[BLOCK_SIZE], tag2[BLOCK_SIZE], tag3[BLOCK_SIZE], 
                    tag4[BLOCK_SIZE], *hdr, *hdrx, *ptx, *ptxx, *ptx1, *ptx1x, *ptx2, *ptx2x,
                    *ctx1, *ctx1x, *ctx2, *ctx2x, *tp;
    int             len, inc, tag_len = BLOCK_SIZE, ctx_len = ptx_len, h_len, p_len, err, i;
    static int      lths[] = { 0, 1, 2, 3, 5, 7, 11, 15 };

    hdrx = hdr = (unsigned char*)malloc(hdr_len + 2 * BLOCK_SIZE);
	ptxx = ptx = (unsigned char*)malloc(ptx_len + 2 * BLOCK_SIZE);
	ctx1x = ctx1 = (unsigned char*)malloc(ptx_len + 2 * BLOCK_SIZE);
	ctx2x = ctx2 = (unsigned char*)malloc(ptx_len + 2 * BLOCK_SIZE);
	ptx1x = ptx1 = (unsigned char*)malloc(ptx_len + 2 * BLOCK_SIZE);
	ptx2x = ptx2 = (unsigned char*)malloc(ptx_len + 2 * BLOCK_SIZE);

    if(flag & 2)
    {
        hdr += 3;
        ptx += 3;
        ctx1 += 3;
        ctx2 += 3;
        ptx1 += 3;
        ptx2 += 3;
    }

    block_rndfill(key, key_len);
    block_rndfill(iv, iv_len);
    block_rndfill(hdr, hdr_len + BLOCK_SIZE);
    block_rndfill(ptx, ptx_len + BLOCK_SIZE);

    for( i = 0 ; i < 64 ; ++i )
    {   
        h_len = hdr_len + lths[i & 7];
        p_len = ptx_len + lths[i >> 3];

        f->init_and_key(key, key_len, ctxa);
        f->init_and_key(key, key_len, ctxb);
        if(strcmp(f->name, "CCM") == 0)
        {
            ((s_init_message*)f->init_message)(iv, iv_len, 
                            h_len, p_len, tag_len, ctxa);

            ((s_init_message*)f->init_message)(iv, iv_len, 
                            h_len, p_len, tag_len, ctxb);
        }
        else
        {
            f->init_message(iv, iv_len, ctxa);
            f->init_message(iv, iv_len, ctxb);
        }

        f->auth_header(hdr, h_len, ctxa);
        memcpy(ctx1, ptx, p_len);
        f->encrypt(ctx1, p_len, ctxa);
        f->compute_tag(tag1, tag_len, ctxa);
        f->end(ctxa);

        len = h_len;
        tp = hdr;
        while(len)
        {
            inc = rand32() & 31;
            inc = len < inc ? len : inc;
            f->auth_header(tp, inc, ctxb);
            tp += inc;
            len -= inc;
        }

        memcpy(ctx2, ptx, p_len);
        len = p_len;
        tp = ctx2;
        while(len)
        {
            inc = rand32() & 31;
            inc = len < inc ? len : inc;
            f->encrypt(tp, inc, ctxb);
            tp += inc;
            len -= inc;
        }
        
        f->compute_tag(tag2, tag_len, ctxb);    
        f->end(ctxb);

        if(memcmp(ctx1, ctx2, p_len))
            printf("\n%s encrypt ciphertext error", f->name), err++;
        if(memcmp(tag1, tag2, tag_len))
            printf("\n%s encrypt tag error", f->name), err++;

        f->init_and_key(key, key_len, ctxa);
        f->init_and_key(key, key_len, ctxb);
        if(strcmp(f->name, "CCM") == 0)
        {
            ((s_init_message*)f->init_message)(iv, iv_len, h_len, p_len, tag_len, ctxa);
            ((s_init_message*)f->init_message)(iv, iv_len, h_len, p_len, tag_len, ctxb);
        }
        else
        {
            f->init_message(iv, iv_len, ctxa);
            f->init_message(iv, iv_len, ctxb);
        }

        f->auth_header(hdr, h_len, ctxa);
        memcpy(ptx1, ctx1, p_len);
        f->decrypt(ptx1, p_len, ctxa);
        f->compute_tag(tag3, tag_len, ctxa);
        f->end(ctxa);

        len = h_len;
        tp = hdr;
        while(len)
        {
            inc = rand32() & 31;
            inc = len < inc ? len : inc;
            f->auth_header(tp, inc, ctxb);
            tp += inc;
            len -= inc;
        }

        memcpy(ptx2, ctx1, p_len);
        if(strcmp(f->name, "CCM") != 0)
        {
            len = p_len;
            tp = ptx2;
            while(len)
            {
                inc = rand32() & 31;
                inc = len < inc ? len : inc;
                f->auth_data(tp, inc, ctxb);
                tp += inc;
                len -= inc;
            }

            if(flag)
                f->compute_tag(tag4, tag_len, ctxb);

            len = p_len;
            tp = ptx2;
            while(len)
            {
                inc = rand32() & 31;
                inc = len < inc ? len : inc;
                f->crypt_data(tp, inc, ctxb);
                tp += inc;
                len -= inc;
            }

            if(!flag)
                f->compute_tag(tag4, tag_len, ctxb);
        }
        else
        {
            len = p_len;
            tp = ptx2;
            while(len)
            {
                inc = rand32() & 31;
                inc = len < inc ? len : inc;
                f->crypt_data(tp, inc, ctxb);
                f->auth_data(tp, inc, ctxb);
                tp += inc;
                len -= inc;
            }
            f->compute_tag(tag4, tag_len, ctxb);
        }

        f->end(ctxb);

        if(memcmp(ptx1, ptx, p_len))
            printf("\n%s decrypt plaintext error on single block decrypt", f->name), err++;
        if(memcmp(ptx2, ptx, p_len))
            printf("\n%s decrypt plaintext error on incremenatal decrypt", f->name), err++;

        if(memcmp(tag1, tag3, tag_len))
            printf("\n%s decrypt tag error on single block decrypt", f->name), err++;
        if(memcmp(tag1, tag4, tag_len))
            printf("\n%s decrypt tag error on incremenatal decrypt", f->name), err++;
    }

    free(ptx2x);
    free(ptx1x);
    free(ctx2x);
    free(ctx1x);
    free(ptxx);
    free(hdrx);
}

void ccm_tests(unsigned long key_len, unsigned long iv_len,
                    unsigned long hdr_len, unsigned long ptx_len, int flag)
{   ccm_ctx ctx1[1], ctx2[1];
    mode_fns f[1];

    ccm_functions(f);
    do_test(f, key_len, iv_len, hdr_len, ptx_len, ctx1, ctx2, flag);
}

void cwc_tests(unsigned long key_len, unsigned long iv_len,
                    unsigned long hdr_len, unsigned long ptx_len, int flag)
{   cwc_ctx ctx1[1], ctx2[1];
    mode_fns f[1];

    cwc_functions(f);
    do_test(f, key_len, iv_len, hdr_len, ptx_len, ctx1, ctx2, flag);
}

void eax_tests(unsigned long key_len, unsigned long iv_len,
                    unsigned long hdr_len, unsigned long ptx_len, int flag)
{   eax_ctx ctx1[1], ctx2[1];
    mode_fns f[1];

    eax_functions(f);
    do_test(f, key_len, iv_len, hdr_len, ptx_len, ctx1, ctx2, flag);
}

void gcm_tests(unsigned long key_len, unsigned long iv_len,
                    unsigned long hdr_len, unsigned long ptx_len, int flag)
{   gcm_ctx ctx1[1], ctx2[1];
    mode_fns f[1];

    gcm_functions(f);
    do_test(f, key_len, iv_len, hdr_len, ptx_len, ctx1, ctx2, flag);
}

int main(void)
{
    unsigned long key_len = BLOCK_SIZE, iv_len = 12, hdr_len = 100000, ptx_len = 1000000;
    int     flag = 0;  /* 0..3, bit 0 = tag compute posn, bit 1 = memory misalignment */

    ccm_tests(key_len, iv_len, hdr_len, ptx_len, flag);
    cwc_tests(key_len, iv_len, hdr_len, ptx_len, flag);
    eax_tests(key_len, iv_len, hdr_len, ptx_len, flag);
    gcm_tests(key_len, iv_len, hdr_len, ptx_len, flag);

    printf("\n\n");
    return 0;
}



