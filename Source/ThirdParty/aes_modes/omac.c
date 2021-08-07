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
*/

#include "omac.h"

#define BLK_ADR_MASK    (BLOCK_SIZE - 1)

void omac_init(const unsigned char key[], unsigned long key_len, omac_ctx ctx[1])
{
    memset(ctx, 0, sizeof(omac_ctx));
    aes_encrypt_key(key, key_len, ctx->aes);
}

void omac_data(unsigned char buf[], unsigned long len, omac_ctx ctx[1])
{   uint32_t cnt = 0, b_pos = ctx->txt_cnt & BLK_ADR_MASK;

    if(!len)
        return;

	if (ctx->txt_cnt && !b_pos)
		b_pos = BLOCK_SIZE;

    if(!((buf - (UI8_PTR(ctx->txt_cbc) + b_pos)) & BUF_ADRMASK))
    {
        while(cnt < len && (b_pos & BUF_ADRMASK))
            UI8_PTR(ctx->txt_cbc)[b_pos++] ^= buf[cnt++];

		if(cnt + BLOCK_SIZE <= len)
		{
			while(cnt + BUF_INC <= len && b_pos <= BLOCK_SIZE - BUF_INC)
			{
				*UNIT_PTR(UI8_PTR(ctx->txt_cbc) + b_pos) ^= *UNIT_PTR(buf + cnt);
				cnt += BUF_INC; b_pos += BUF_INC;
			}
		}

        while(cnt + BLOCK_SIZE <= len)
        {
            aes_encrypt(UI8_PTR(ctx->txt_cbc), UI8_PTR(ctx->txt_cbc), ctx->aes);
            xor_block_aligned(ctx->txt_cbc, ctx->txt_cbc, buf + cnt);
            cnt += BLOCK_SIZE;
        }
    }
    else
    {
        while(cnt < len && (b_pos & BLK_ADR_MASK))
            UI8_PTR(ctx->txt_cbc)[b_pos++] ^= buf[cnt++];
        
        while(cnt + BLOCK_SIZE <= len)
        {
            aes_encrypt(UI8_PTR(ctx->txt_cbc), UI8_PTR(ctx->txt_cbc), ctx->aes);
            xor_block(ctx->txt_cbc, ctx->txt_cbc, buf + cnt);
            cnt += BLOCK_SIZE;
        }
    }

    while(cnt < len)
    {
        if(b_pos == BLOCK_SIZE)
        {
            aes_encrypt(UI8_PTR(ctx->txt_cbc), UI8_PTR(ctx->txt_cbc), ctx->aes);
            b_pos = 0;
        }
        UI8_PTR(ctx->txt_cbc)[b_pos++] ^= buf[cnt++];
    }

    ctx->txt_cnt += cnt;
}

static const unsigned char c_xor[4] = { 0x00, 0x87, 0x0e, 0x89 };

static void gf_mulx(uint8_t pad[BLOCK_SIZE])
{   int i, t = pad[0] >> 7;

    for(i = 0; i < BLOCK_SIZE - 1; ++i)
        pad[i] = (pad[i] << 1) | (pad[i + 1] >> 7);
    pad[BLOCK_SIZE - 1] = (pad[BLOCK_SIZE - 1] << 1) ^ c_xor[t];
}

#if OMAC_VERSION == 1

void gf_mulx2(uint8_t pad[BLOCK_SIZE])
{   int i, t = pad[0] >> 6;

    for(i = 0; i < BLOCK_SIZE - 1; ++i)
        pad[i] = (pad[i] << 2) | (pad[i + 1] >> 6);
    pad[BLOCK_SIZE - 2] ^= (t >> 1);
    pad[BLOCK_SIZE - 1] = (pad[BLOCK_SIZE - 1] << 2) ^ c_xor[t];
}

#else

void gf_divx(uint8_t pad[BLOCK_SIZE])
{   int i, t = pad[BLOCK_SIZE - 1];

    for(i = BLOCK_SIZE - 1; i > 0; --i)
        pad[i] = (pad[i] >> 1) | (pad[i - 1] << 7);
    pad[0] = (pad[0] >> 1) ^ ((t & 1) ? 0x80 : 0);
    pad[BLOCK_SIZE - 1] ^= ((t & 1) ? 0x43 : 0);
}

#endif

#if 0   /* little endian counters   */

void gf_mulx(unsigned char pad[BLOCK_SIZE])
{   int i, t;

    for(i = BLOCK_SIZE - 1, t = pad[BLOCK_SIZE - 1]; i > 0; --i)
        pad[i] = (pad[i] << 1) | (pad[i - 1] >> 7);
    pad[0] = (pad[0] << 1) ^ ((t & 0x80) ? 0x87 : 0);
}

void gf_divx(unsigned char pad[BLOCK_SIZE])
{   int i, t;

    for(i = 0, t = pad[0]; i < BLOCK_SIZE - 1; ++i)
        pad[i] = (pad[i] >> 1) | (pad[i + 1] << 7);
    pad[BLOCK_SIZE - 1] = (pad[BLOCK_SIZE - 1] >> 1) ^ ((t & 1) ? 0x80 : 0);
    pad[0] ^= ((t & 1) ? 0x43 : 0);
}

#endif

void omac_end(unsigned char auth_tag[], omac_ctx ctx[1])
{   buf_type pad;
    int i;

    memset(pad, 0, sizeof(pad));
    aes_encrypt(UI8_PTR(pad), UI8_PTR(pad), ctx->aes);
    i = ctx->txt_cnt & BLK_ADR_MASK;
    if(ctx->txt_cnt == 0 || i)
    {
        UI8_PTR(ctx->txt_cbc)[i] ^= 0x80;
#if OMAC_VERSION == 1
        gf_mulx2(UI8_PTR(pad));
#else
        gf_divx(UI8_PTR(pad));
#endif
    }
    else
        gf_mulx(UI8_PTR(pad));

    xor_block_aligned(pad, pad, ctx->txt_cbc);
    aes_encrypt(UI8_PTR(pad), UI8_PTR(pad), ctx->aes);

    for(i = 0; i < BLOCK_SIZE; ++i)
        auth_tag[i] = UI8_PTR(pad)[i];
}
