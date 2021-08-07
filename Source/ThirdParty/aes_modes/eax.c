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

 This code implements the EAX combined encryption and authentication mode
 specified M. Bellare, P. Rogaway and D. Wagner.

 This is a byte oriented version in which the nonce is of limited length
*/

#include "eax.h"
#include "mode_hdr.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define BLOCK_SIZE      AES_BLOCK_SIZE      /* block length                 */
#define BLK_ADR_MASK    (BLOCK_SIZE - 1)    /* mask for 'in block' address  */

#define inc_ctr(x)  \
    {   int i = BLOCK_SIZE; while(i-- > 0 && !++(UI8_PTR(x)[i])) ; }
#define dec_ctr(x)  \
    {   int i = BLOCK_SIZE; while(i-- > 0 && !(UI8_PTR(x)[i])--) ; }

ret_type eax_init_and_key(                  /* initialise mode and set key  */
            const unsigned char key[],      /* the key value                */
            unsigned long key_len,          /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{   static uint8_t x_t[4] = { 0x00, 0x87, 0x0e, 0x87 ^ 0x0e };
    uint8_t t, *p;
    uint32_t i;

    /* set the context to all zeroes            */
    memset(ctx, 0, sizeof(eax_ctx));

    /* set the AES key                          */
    aes_encrypt_key(key, key_len, ctx->aes);

    /* compute E(0) (needed for the pad values) */
    aes_encrypt(UI8_PTR(ctx->pad_xvv), UI8_PTR(ctx->pad_xvv), ctx->aes);

    /* compute {02} * {E(0)} and {04} * {E(0)}  */
    /* GF(2^128) mod x^128 + x^7 + x^2 + x + 1  */
    for(i = 0, p = UI8_PTR(ctx->pad_xvv), t = *p >> 6; i < EAX_BLOCK_SIZE - 1; ++i, ++p)
    {
        *(p + 16) = (*p << 2) | (*(p + 1) >> 6);
        *p = (*p << 1) | (*(p + 1) >> 7);
    }
    *(p + 16) = (*p << 2) ^ x_t[t];
    *(p + 15) ^= (t >>= 1);
    *p = (*p << 1) ^ x_t[t];

    return RETURN_GOOD;
}

ret_type eax_init_message(                  /* initialise a new message     */
            const unsigned char iv[],       /* the initialisation vector    */
            unsigned long iv_len,           /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{   uint32_t i = 0, n_pos = 0;
    uint8_t *p;

    memset(ctx->nce_cbc, 0, EAX_BLOCK_SIZE);
    memset(ctx->hdr_cbc, 0, EAX_BLOCK_SIZE);
    memset(ctx->txt_cbc, 0, EAX_BLOCK_SIZE);

    /* set the header CBC start value           */
    UI8_PTR(ctx->hdr_cbc)[EAX_BLOCK_SIZE - 1] = 1;
    ctx->hdr_cnt = 0;

    /* set the ciphertext CBC start value       */
    UI8_PTR(ctx->txt_cbc)[EAX_BLOCK_SIZE - 1] = 2;
    ctx->txt_ccnt = 0;  /* encryption count     */
    ctx->txt_acnt = 0;  /* authentication count */

    /* if the nonce length is zero, the OMAC    */
    /* message is a block of zeroes which gives */
    /* the pre-encrypted tag as the Lu value    */
    if(iv_len)
    {
        n_pos = 16;

        /* compile the OMAC value for the nonce     */
        i = 0;
        while(i < iv_len)
        {
            if(n_pos == EAX_BLOCK_SIZE)
            {
                aes_encrypt(UI8_PTR(ctx->nce_cbc), UI8_PTR(ctx->nce_cbc), ctx->aes);
                n_pos = 0;
            }
            UI8_PTR(ctx->nce_cbc)[n_pos++] ^= iv[i++];
        }

        /* do the OMAC padding for the nonce        */
        p = UI8_PTR(ctx->pad_xvv);
        if(n_pos < EAX_BLOCK_SIZE)
        {
            UI8_PTR(ctx->nce_cbc)[n_pos] ^= 0x80;
            p += 16;
        }

        for(i = 0; i < EAX_BLOCK_SIZE; ++i)
            UI8_PTR(ctx->nce_cbc)[i] ^= p[i];
    }
    else
        memcpy(ctx->nce_cbc, ctx->pad_xvv, EAX_BLOCK_SIZE);

    /* compute the OMAC*(nonce) value           */
    aes_encrypt(UI8_PTR(ctx->nce_cbc), UI8_PTR(ctx->nce_cbc), ctx->aes);

    /* copy value into counter for CTR          */
    memcpy(ctx->ctr_val, ctx->nce_cbc, EAX_BLOCK_SIZE);
    return RETURN_GOOD;
}

ret_type eax_auth_header(                   /* authenticate the header      */
            const unsigned char hdr[],      /* the header buffer            */
            unsigned long hdr_len,          /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{   uint32_t cnt = 0, b_pos = ctx->hdr_cnt & BLK_ADR_MASK;

    if(!hdr_len)
        return RETURN_GOOD;

    if(!((hdr - (UI8_PTR(ctx->hdr_cbc) + b_pos)) & BUF_ADRMASK))
    {
	    if(b_pos)
        {
            while(cnt < hdr_len && (b_pos & BUF_ADRMASK))
		        UI8_PTR(ctx->hdr_cbc)[b_pos++] ^= hdr[cnt++];

            while(cnt + BUF_INC <= hdr_len && b_pos <= BLOCK_SIZE - BUF_INC)
            {
                *UNIT_PTR(UI8_PTR(ctx->hdr_cbc) + b_pos) ^= *UNIT_PTR(hdr + cnt);
                cnt += BUF_INC; b_pos += BUF_INC;
            }
        }

        while(cnt + BLOCK_SIZE <= hdr_len)
        {
            aes_encrypt(UI8_PTR(ctx->hdr_cbc), UI8_PTR(ctx->hdr_cbc), ctx->aes);
            xor_block_aligned(ctx->hdr_cbc, ctx->hdr_cbc, hdr + cnt);
            cnt += BLOCK_SIZE;
        }
    }
    else
    {
        if(b_pos)
            while(cnt < hdr_len && b_pos < BLOCK_SIZE)
                UI8_PTR(ctx->hdr_cbc)[b_pos++] ^= hdr[cnt++];

        while(cnt + BLOCK_SIZE <= hdr_len)
        {
            aes_encrypt(UI8_PTR(ctx->hdr_cbc), UI8_PTR(ctx->hdr_cbc), ctx->aes);
            xor_block(ctx->hdr_cbc, ctx->hdr_cbc, hdr + cnt);
            cnt += BLOCK_SIZE;
        }
    }

    while(cnt < hdr_len)
    {
        if(b_pos == BLOCK_SIZE || !b_pos)
        {
            aes_encrypt(UI8_PTR(ctx->hdr_cbc), UI8_PTR(ctx->hdr_cbc), ctx->aes);
            b_pos = 0;
        }
        UI8_PTR(ctx->hdr_cbc)[b_pos++] ^= hdr[cnt++];
    }

    ctx->hdr_cnt += cnt;
    return RETURN_GOOD;
}

ret_type eax_auth_data(                     /* authenticate ciphertext data */
            const unsigned char data[],     /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{   uint32_t cnt = 0, b_pos = ctx->txt_acnt & BLK_ADR_MASK;

    if(!data_len)
        return RETURN_GOOD;

    if(!((data - (UI8_PTR(ctx->txt_cbc) + b_pos)) & BUF_ADRMASK))
    {
        if(b_pos)
        {
	        while(cnt < data_len && (b_pos & BUF_ADRMASK))
		       UI8_PTR(ctx->txt_cbc)[b_pos++] ^= data[cnt++];

            while(cnt + BUF_INC <= data_len && b_pos <= BLOCK_SIZE - BUF_INC)
            {
                *UNIT_PTR(UI8_PTR(ctx->txt_cbc) + b_pos) ^= *UNIT_PTR(data + cnt);
                cnt += BUF_INC; b_pos += BUF_INC;
            }
        }

        while(cnt + BLOCK_SIZE <= data_len)
        {
            aes_encrypt(UI8_PTR(ctx->txt_cbc), UI8_PTR(ctx->txt_cbc), ctx->aes);
            xor_block_aligned(ctx->txt_cbc, ctx->txt_cbc, data + cnt);
            cnt += BLOCK_SIZE;
        }
    }
    else
    {
        if(b_pos)
            while(cnt < data_len && b_pos < BLOCK_SIZE)
               UI8_PTR(ctx->txt_cbc)[b_pos++] ^= data[cnt++];

        while(cnt + BLOCK_SIZE <= data_len)
        {
            aes_encrypt(UI8_PTR(ctx->txt_cbc), UI8_PTR(ctx->txt_cbc), ctx->aes);
            xor_block(ctx->txt_cbc, ctx->txt_cbc, data + cnt);
            cnt += BLOCK_SIZE;
        }
    }

    while(cnt < data_len)
    {
        if(b_pos == BLOCK_SIZE || !b_pos)
        {
            aes_encrypt(UI8_PTR(ctx->txt_cbc), UI8_PTR(ctx->txt_cbc), ctx->aes);
            b_pos = 0;
        }
        UI8_PTR(ctx->txt_cbc)[b_pos++] ^= data[cnt++];
    }

    ctx->txt_acnt += cnt;
    return RETURN_GOOD;
}

ret_type eax_crypt_data(                    /* encrypt or decrypt data      */
            unsigned char data[],           /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{   uint32_t cnt = 0, b_pos = ctx->txt_ccnt & BLK_ADR_MASK;

    if(!data_len)
        return RETURN_GOOD;

    if(!((data - (UI8_PTR(ctx->enc_ctr) + b_pos)) & BUF_ADRMASK))
    {
	    if(b_pos)
        {
            while(cnt < data_len && (b_pos & BUF_ADRMASK))
		        data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];

            while(cnt + BUF_INC <= data_len && b_pos <= BLOCK_SIZE - BUF_INC)
            {
                *UNIT_PTR(data + cnt) ^= *UNIT_PTR(UI8_PTR(ctx->enc_ctr) + b_pos);
                cnt += BUF_INC; b_pos += BUF_INC;
            }
        }

        while(cnt + BLOCK_SIZE <= data_len)
        {
            aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
            inc_ctr(ctx->ctr_val);
            xor_block_aligned(data + cnt, data + cnt, ctx->enc_ctr);
            cnt += BLOCK_SIZE;
        }
    }
    else
    {
        if(b_pos)
            while(cnt < data_len && b_pos < BLOCK_SIZE)
                data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];

        while(cnt + BLOCK_SIZE <= data_len)
        {
            aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
            inc_ctr(ctx->ctr_val);
            xor_block(data + cnt, data + cnt, ctx->enc_ctr);
            cnt += BLOCK_SIZE;
        }
    }

    while(cnt < data_len)
    {
        if(b_pos == BLOCK_SIZE || !b_pos)
        {
            aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
            b_pos = 0;
            inc_ctr(ctx->ctr_val);
        }
        data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
    }

    ctx->txt_ccnt += cnt;
    return RETURN_GOOD;
}

ret_type eax_compute_tag(                   /* compute authentication tag   */
            unsigned char tag[],            /* the buffer for the tag       */
            unsigned long tag_len,          /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{   uint32_t i;
    uint8_t *p;

    if(ctx->txt_acnt != ctx->txt_ccnt && ctx->txt_ccnt > 0)
        return RETURN_ERROR;

    /* complete OMAC* for header value      */
    p = UI8_PTR(ctx->pad_xvv);
    if(i = ctx->hdr_cnt &  BLK_ADR_MASK)
    {
        UI8_PTR(ctx->hdr_cbc)[i] ^= 0x80;
        p += 16;
    }

    xor_block_aligned(ctx->hdr_cbc, ctx->hdr_cbc, p);
    aes_encrypt(UI8_PTR(ctx->hdr_cbc), UI8_PTR(ctx->hdr_cbc), ctx->aes);

    /* complete OMAC* for ciphertext value  */
    p = UI8_PTR(ctx->pad_xvv);
    if(i = ctx->txt_acnt &  BLK_ADR_MASK)
    {
        UI8_PTR(ctx->txt_cbc)[i] ^= 0x80;
        p += 16;
    }

    xor_block_aligned(ctx->txt_cbc, ctx->txt_cbc, p);
    aes_encrypt(UI8_PTR(ctx->txt_cbc), UI8_PTR(ctx->txt_cbc), ctx->aes);

    /* compute final authentication tag     */
    for(i = 0; i < (unsigned int)tag_len; ++i)
        tag[i] = UI8_PTR(ctx->nce_cbc)[i] ^ UI8_PTR(ctx->txt_cbc)[i] ^ UI8_PTR(ctx->hdr_cbc)[i];

    return (ctx->txt_ccnt == ctx->txt_acnt ? RETURN_GOOD : RETURN_WARN);
}

ret_type eax_end(                           /* clean up and end operation   */
            eax_ctx ctx[1])                 /* the mode context             */
{
    memset(ctx, 0, sizeof(eax_ctx));
    return RETURN_GOOD;
}

ret_type eax_encrypt(                       /* encrypt & authenticate data  */
            unsigned char data[],           /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{
    eax_crypt_data(data, data_len, ctx);
    eax_auth_data(data, data_len, ctx);
    return RETURN_GOOD;
}

ret_type eax_decrypt(                       /* authenticate & decrypt data  */
            unsigned char data[],           /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{
    eax_auth_data(data, data_len, ctx);
    eax_crypt_data(data, data_len, ctx);
    return RETURN_GOOD;
}

ret_type eax_encrypt_message(               /* encrypt an entire message    */
            const unsigned char iv[],       /* the initialisation vector    */
            unsigned long iv_len,           /* and its length in bytes      */
            const unsigned char hdr[],      /* the header buffer            */
            unsigned long hdr_len,          /* and its length in bytes      */
            unsigned char msg[],            /* the message buffer           */
            unsigned long msg_len,          /* and its length in bytes      */
            unsigned char tag[],            /* the buffer for the tag       */
            unsigned long tag_len,          /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{
    eax_init_message(iv, iv_len, ctx);
    eax_auth_header(hdr, hdr_len, ctx);
    eax_encrypt(msg, msg_len, ctx);
    return eax_compute_tag(tag, tag_len, ctx) ? RETURN_ERROR : RETURN_GOOD;
}

ret_type eax_decrypt_message(               /* decrypt an entire message    */
            const unsigned char iv[],       /* the initialisation vector    */
            unsigned long iv_len,           /* and its length in bytes      */
            const unsigned char hdr[],      /* the header buffer            */
            unsigned long hdr_len,          /* and its length in bytes      */
            unsigned char msg[],            /* the message buffer           */
            unsigned long msg_len,          /* and its length in bytes      */
            const unsigned char tag[],      /* the buffer for the tag       */
            unsigned long tag_len,          /* and its length in bytes      */
            eax_ctx ctx[1])                 /* the mode context             */
{   uint8_t local_tag[BLOCK_SIZE];
    ret_type rr;

    eax_init_message(iv, iv_len, ctx);
    eax_auth_header(hdr, hdr_len, ctx);
    eax_decrypt(msg, msg_len, ctx);
    rr = eax_compute_tag(local_tag, tag_len, ctx);
    return (rr != RETURN_GOOD || memcmp(tag, local_tag, tag_len)) ? RETURN_ERROR : RETURN_GOOD;
}

#if defined(__cplusplus)
}
#endif
