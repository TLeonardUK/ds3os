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

My thanks to Colin Sinclair for finding an error and suggesting a number
of improvements to this code
*/

#include "ccm.h"
#include "mode_hdr.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define BLOCK_SIZE      AES_BLOCK_SIZE      /* block length                 */
#define BLK_ADR_MASK    (BLOCK_SIZE - 1)    /* mask for 'in block' address  */
#define INIT_STATE      10
#define AUTH_STATE      11
#define MSG_STATE       12

#define ctr_len(x)      (((*(uint8_t*)(x)) & 0x07) + 1)

#define clr_ctr(x,l)    memset((uint8_t*)(x) + BLOCK_SIZE - (l), 0, (l))

#define set_ctr(x,v)                                \
    {   uint8_t *_p = (uint8_t*)(x) + BLOCK_SIZE;   \
        length_t _t = (v), _l = ctr_len(x);         \
        do                                          \
        {    *--_p = (uint8_t)_t; _t >>= 8; }       \
        while(--_l);                                \
    }

#define inc_ctr(x)                                  \
    {   uint8_t *_p = (uint8_t*)(x) + BLOCK_SIZE;   \
        length_t _l = ctr_len(x);                   \
        while(_l-- && ++(*--_p) == 0) ;             \
    }

#ifdef LONG_MESSAGES

#  define mlen_len(x)                   \
    (((x) & 0xff00000000000000) ? 7 :   \
     ((x) & 0xffff000000000000) ? 6 :   \
     ((x) & 0xffffff0000000000) ? 5 :   \
     ((x) & 0xffffffff00000000) ? 4 :   \
     ((x) & 0xffffffffff000000) ? 3 :   \
     ((x) & 0xffffffffffff0000) ? 2 : 1)

#else
#  define mlen_len(x) (((x) & 0xff000000) ? 3 : ((x) & 0xffff0000) ? 2 : 1)
#endif

ret_type ccm_init_and_key(                  /* initialise mode and set key  */
            const unsigned char key[],      /* the key value                */
            unsigned long key_len,          /* and its length in bytes      */
            ccm_ctx ctx[1])                 /* the mode context             */
{
    ctx->state = RETURN_GOOD;
    if(key_len != 16 && key_len != 24 && key_len != 32)
        ctx->state = CCM_BAD_KEY;
    aes_encrypt_key(key, (int)key_len, ctx->aes);
    return RETURN_GOOD;
}

ret_type ccm_init_message(                  /* initialise for a new message */
            const unsigned char iv[],       /* the initialisation vector    */
            unsigned long iv_len,           /* the nonce length             */
            length_t hdr_len,               /* the associated data length   */
            length_t msg_len,               /* message data length          */
            unsigned long tag_len,          /* authentication field length  */
            ccm_ctx ctx[1])                 /* the mode context             */
{   uint32_t cnt;

    ctx->state = RETURN_GOOD;
    if(
#ifdef ALLOW_CCM_STAR
        tag_len &&  
#endif
                   tag_len < 4 || tag_len > 16 || (tag_len & 1))
        ctx->state = CCM_BAD_AUTH_FIELD_LENGTH;
    else if(iv_len && iv_len < 7 || iv_len > 13)
        ctx->state = CCM_BAD_NONCE_LENGTH;
    if(ctx->state)
        return ctx->state;

    ctx->iv_len = iv_len;
    ctx->hdr_len = hdr_len;
    ctx->msg_len = msg_len;
    ctx->tag_len = tag_len;
    ctx->cnt = 0;
    ctx->txt_acnt = 0;
    ctx->hdr_lim = hdr_len;

    UI8_PTR(ctx->ctr_val)[0] =
        (uint8_t)(iv_len ? BLOCK_SIZE - 2 - iv_len : mlen_len(ctx->msg_len));

    /* move the iv into the block    */
    for(cnt = 1; cnt < (uint32_t)BLOCK_SIZE - UI8_PTR(ctx->ctr_val)[0] - 1; ++cnt)
        UI8_PTR(ctx->ctr_val)[cnt] = iv[cnt - 1];

    clr_ctr(ctx->ctr_val, UI8_PTR(ctx->ctr_val)[0] + 1);/* clear the counter value  */
    memcpy(ctx->cbc_buf, ctx->ctr_val, BLOCK_SIZE);     /* copy block to CBC buffer */
    UI8_PTR(ctx->ctr_val)[BLOCK_SIZE - 1] = 0;          /* set initial counter      */
    set_ctr(ctx->cbc_buf, ctx->msg_len);                /* store the message length */

    UI8_PTR(ctx->cbc_buf)[0] |= (ctx->hdr_lim ? 0x40 : 0) 
                              + (ctx->tag_len ? (ctx->tag_len - 2) << 2 : 0);
                                                            /* encrypt the cbc block    */
    aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);

    if(ctx->hdr_len)
    {   /* encode the length field if there is some associated data */

        if(ctx->hdr_len < 65536 - 256)
        {
            UI8_PTR(ctx->cbc_buf)[0] ^= (uint8_t)(ctx->hdr_lim >> 8);
            UI8_PTR(ctx->cbc_buf)[1] ^= (uint8_t) ctx->hdr_lim;
            ctx->cnt = 2;
        }
#ifndef LONG_MESSAGES
        else
        {
            ctx->state = CCM_AUTH_LENGTH_ERROR;
            return ctx->state;
        }
#else
        else if(ctx->hdr_len < 0x0000000100000000)
        {
            UI8_PTR(ctx->cbc_buf)[0] ^= 0xff;
            UI8_PTR(ctx->cbc_buf)[1] ^= 0xfe;
            UI8_PTR(ctx->cbc_buf)[2] ^= (uint8_t)(ctx->hdr_lim >> 24);
            UI8_PTR(ctx->cbc_buf)[3] ^= (uint8_t)(ctx->hdr_lim >> 16);
            UI8_PTR(ctx->cbc_buf)[4] ^= (uint8_t)(ctx->hdr_lim >>  8);
            UI8_PTR(ctx->cbc_buf)[5] ^= (uint8_t) ctx->hdr_lim;
            ctx->cnt = 6;
        }
        else
        {
            UI8_PTR(ctx->cbc_buf)[0] ^= 0xff;
            UI8_PTR(ctx->cbc_buf)[1] ^= 0xff;
            UI8_PTR(ctx->cbc_buf)[2] ^= (uint8_t)(ctx->hdr_lim >> 56);
            UI8_PTR(ctx->cbc_buf)[3] ^= (uint8_t)(ctx->hdr_lim >> 48);
            UI8_PTR(ctx->cbc_buf)[4] ^= (uint8_t)(ctx->hdr_lim >> 40);
            UI8_PTR(ctx->cbc_buf)[5] ^= (uint8_t)(ctx->hdr_lim >> 32);
            UI8_PTR(ctx->cbc_buf)[6] ^= (uint8_t)(ctx->hdr_lim >> 24);
            UI8_PTR(ctx->cbc_buf)[7] ^= (uint8_t)(ctx->hdr_lim >> 16);
            UI8_PTR(ctx->cbc_buf)[8] ^= (uint8_t)(ctx->hdr_lim >>  8);
            UI8_PTR(ctx->cbc_buf)[9] ^= (uint8_t) ctx->hdr_lim;
            ctx->cnt = 10;
        }
#endif
        ctx->hdr_lim += ctx->cnt;
        ctx->state = AUTH_STATE;
    }
    else    /* there is no associated data  */
    {
        ctx->cnt = 0;
        ctx->state = MSG_STATE;
    }

    ctx->hdr_lim = ctx->hdr_len + ctx->cnt;
    ctx->txt_acnt = ctx->cnt;

    return RETURN_GOOD;
}

ret_type ccm_auth_header(                   /* authenticate the header      */
            const unsigned char hdr[],      /* the header buffer            */
            unsigned long hdr_len,          /* and its length in bytes      */
            ccm_ctx ctx[1])                 /* the mode context             */
{   uint32_t cnt = 0, b_pos = (uint32_t)ctx->cnt & BLK_ADR_MASK;

    if(!hdr_len)
        return RETURN_GOOD;

    if(ctx->state < 0)
        return ctx->state;

    if(ctx->state != AUTH_STATE)
        return (ctx->state = CCM_BAD_AUTH_CALL);

    if(ctx->cnt + hdr_len > ctx->hdr_lim)
        return (ctx->state = CCM_AUTH_LENGTH_ERROR);

    if(!((hdr - (UI8_PTR(ctx->cbc_buf) + b_pos)) & BUF_ADRMASK))
    {
	    while(cnt < hdr_len && (b_pos & BUF_ADRMASK))
		    UI8_PTR(ctx->cbc_buf)[b_pos++] ^= hdr[cnt++];

        while(cnt + BUF_INC <= hdr_len && b_pos <= BLOCK_SIZE - BUF_INC)
        {
            *UNIT_PTR(UI8_PTR(ctx->cbc_buf) + b_pos) ^= *UNIT_PTR(hdr + cnt);
            cnt += BUF_INC; b_pos += BUF_INC;
        }

        while(cnt + BLOCK_SIZE <= hdr_len)
        {
            aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
            xor_block_aligned(ctx->cbc_buf, ctx->cbc_buf, hdr + cnt);
            cnt += BLOCK_SIZE;
        }
    }
    else
    {
        while(cnt < hdr_len && b_pos < BLOCK_SIZE)
            UI8_PTR(ctx->cbc_buf)[b_pos++] ^= hdr[cnt++];

        while(cnt + BLOCK_SIZE <= hdr_len)
        {
            aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
            xor_block(ctx->cbc_buf, ctx->cbc_buf, hdr + cnt);
            cnt += BLOCK_SIZE;
        }
    }

    while(cnt < hdr_len)
    {
        if(b_pos == BLOCK_SIZE)
        {
            aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
            b_pos = 0;
        }
        UI8_PTR(ctx->cbc_buf)[b_pos++] ^= hdr[cnt++];

    }

    ctx->cnt += cnt;
    if(b_pos == BLOCK_SIZE || ctx->cnt == ctx->hdr_lim)
        aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);

    if(ctx->cnt == ctx->hdr_lim)
    {
        ctx->state = MSG_STATE;
        ctx->cnt = 0;
        ctx->txt_acnt = 0;
    }

    return RETURN_GOOD;
}

ret_type ccm_auth_data(                     /* authenticate the plaintext   */
            const unsigned char data[],     /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            ccm_ctx ctx[1])                 /* the mode context             */
{   uint32_t cnt = 0, b_pos = (uint32_t)ctx->txt_acnt & BLK_ADR_MASK;

    if(ctx->state < 0)
        return ctx->state;

    if(ctx->state == AUTH_STATE)
        return (ctx->state = CCM_AUTH_LENGTH_ERROR);

    if(ctx->txt_acnt + data_len > ctx->msg_len)
        return (ctx->state = CCM_MSG_LENGTH_ERROR);

    if(!((data - (UI8_PTR(ctx->cbc_buf) + b_pos)) & BUF_ADRMASK))
    {
	    while(cnt < data_len && (b_pos & BUF_ADRMASK))
		    UI8_PTR(ctx->cbc_buf)[b_pos++] ^= data[cnt++];

        while(cnt + BUF_INC <= data_len && b_pos <= BLOCK_SIZE - BUF_INC)
        {
            *UNIT_PTR(UI8_PTR(ctx->cbc_buf) + b_pos) ^= *UNIT_PTR(data + cnt);
            cnt += BUF_INC; b_pos += BUF_INC;
        }

        while(cnt + BLOCK_SIZE <= data_len)
        {
            aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
            xor_block_aligned(ctx->cbc_buf, ctx->cbc_buf, data + cnt);
            cnt += BLOCK_SIZE;
        }
    }
    else
    {
        while(cnt < data_len && b_pos < BLOCK_SIZE)
            UI8_PTR(ctx->cbc_buf)[b_pos++] ^= data[cnt++];

        while(cnt + BLOCK_SIZE <= data_len)
        {
            aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
            xor_block(ctx->cbc_buf, ctx->cbc_buf, data + cnt);
            cnt += BLOCK_SIZE;
        }
    }

    while(cnt < data_len)
    {
        if(b_pos == BLOCK_SIZE)
            aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes), b_pos = 0;
        UI8_PTR(ctx->cbc_buf)[b_pos++] ^= data[cnt++];
    }

    if(b_pos == BLOCK_SIZE)
        aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);

    ctx->txt_acnt += cnt;
    return RETURN_GOOD;
}

ret_type ccm_crypt_data(                    /* encrypt or decrypt data      */
            unsigned char data[],           /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            ccm_ctx ctx[1])                 /* the mode context             */
{   uint32_t cnt = 0, b_pos = (uint32_t)ctx->cnt & BLK_ADR_MASK;

    if(ctx->state < 0)
        return ctx->state;

    if(ctx->state == AUTH_STATE)
        return (ctx->state = CCM_AUTH_LENGTH_ERROR);

    if(ctx->cnt + data_len > ctx->msg_len)
        return (ctx->state = CCM_MSG_LENGTH_ERROR);

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
            inc_ctr(ctx->ctr_val);
            aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
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
            inc_ctr(ctx->ctr_val);
            aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
            xor_block(data + cnt, data + cnt, ctx->enc_ctr);
            cnt += BLOCK_SIZE;
        }
    }

    while(cnt < data_len)
    {
        if(b_pos == BLOCK_SIZE || !b_pos)
        {
            inc_ctr(ctx->ctr_val);
            aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
            b_pos = 0;
        }
        data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
    }

    ctx->cnt += cnt;
    return RETURN_GOOD;
}

ret_type ccm_compute_tag(                   /* compute authentication tag   */
            unsigned char tag[],            /* the buffer for the tag       */
            unsigned long tag_len,          /* and its length in bytes      */
            ccm_ctx ctx[1])                 /* the mode context             */
{   uint32_t cnt;

    if(ctx->state < 0)
        return ctx->state;

    if(   tag_len != ctx->tag_len
       || ctx->state == INIT_STATE && ctx->hdr_lim
       || ctx->state == AUTH_STATE)
        return (ctx->state = CCM_AUTH_LENGTH_ERROR);

    if(ctx->txt_acnt != ctx->cnt && ctx->cnt > 0)
        return RETURN_ERROR;

    if(   ctx->cnt < ctx->msg_len
       || ctx->cnt + tag_len > ctx->msg_len + ctx->tag_len)
        return (ctx->state = CCM_MSG_LENGTH_ERROR);

    /* if at the start of the authentication field  */
    if(tag_len > 0 && ctx->cnt == ctx->msg_len)
    {
        if(ctx->cnt & BLK_ADR_MASK)
            aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
        memcpy(ctx->enc_ctr, ctx->ctr_val, BLOCK_SIZE);
        set_ctr(ctx->enc_ctr, 0);
        aes_encrypt(UI8_PTR(ctx->enc_ctr), UI8_PTR(ctx->enc_ctr), ctx->aes);
    }

    cnt = 0;
    while(cnt < tag_len)
    {
        tag[cnt] = UI8_PTR(ctx->cbc_buf)[cnt] ^ UI8_PTR(ctx->enc_ctr)[cnt];
        ++cnt;
    }

    return (ctx->cnt == ctx->txt_acnt ? RETURN_GOOD : RETURN_WARN);
}

ret_type ccm_end(                           /* clean up and end operation   */
            ccm_ctx ctx[1])                 /* the mode context             */
{
    memset(ctx, 0, sizeof(ccm_ctx));
    return RETURN_GOOD;
}

ret_type ccm_encrypt(                       /* authenticate & encrypt data  */
            unsigned char data[],           /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            ccm_ctx ctx[1])                 /* the mode context             */
{

    ccm_auth_data(data, data_len, ctx);
    ccm_crypt_data(data, data_len, ctx);
    return RETURN_GOOD;
}

ret_type ccm_decrypt(                       /* decrypt & authenticate data  */
            unsigned char data[],           /* the data buffer              */
            unsigned long data_len,         /* and its length in bytes      */
            ccm_ctx ctx[1])                 /* the mode context             */
{
    ccm_crypt_data(data, data_len, ctx);
    ccm_auth_data(data, data_len, ctx);
    return RETURN_GOOD;
}

ret_type ccm_encrypt_message(               /* encrypt an entire message    */
            const unsigned char iv[],       /* the initialisation vector    */
            unsigned long iv_len,           /* and its length in bytes      */
            const unsigned char hdr[],      /* the header buffer            */
            unsigned long hdr_len,          /* and its length in bytes      */
            unsigned char msg[],            /* the message buffer           */
            unsigned long msg_len,          /* and its length in bytes      */
            unsigned char tag[],            /* the buffer for the tag       */
            unsigned long tag_len,          /* and its length in bytes      */
            ccm_ctx ctx[1])                 /* the mode context             */
{
    ccm_init_message(iv, iv_len, hdr_len, msg_len, tag_len, ctx);
    ccm_auth_header(hdr, hdr_len, ctx);
    ccm_encrypt(msg, msg_len, ctx);
    return ccm_compute_tag(tag, tag_len, ctx) ? RETURN_ERROR : RETURN_GOOD;
}

ret_type ccm_decrypt_message(               /* decrypt an entire message    */
            const unsigned char iv[],       /* the initialisation vector    */
            unsigned long iv_len,           /* and its length in bytes      */
            const unsigned char hdr[],      /* the header buffer            */
            unsigned long hdr_len,          /* and its length in bytes      */
            unsigned char msg[],            /* the message buffer           */
            unsigned long msg_len,          /* and its length in bytes      */
            const unsigned char tag[],      /* the buffer for the tag       */
            unsigned long tag_len,          /* and its length in bytes      */
            ccm_ctx ctx[1])                 /* the mode context             */
{   uint8_t local_tag[BLOCK_SIZE];
    ret_type rr;

    ccm_init_message(iv, iv_len, hdr_len, msg_len, tag_len, ctx);
    ccm_auth_header(hdr, hdr_len, ctx);
    ccm_decrypt(msg, msg_len, ctx);
    rr = ccm_compute_tag(local_tag, tag_len, ctx);
    return (rr != RETURN_GOOD || memcmp(tag, local_tag, tag_len)) ? RETURN_ERROR : RETURN_GOOD;
}

#if defined(__cplusplus)
}
#endif
