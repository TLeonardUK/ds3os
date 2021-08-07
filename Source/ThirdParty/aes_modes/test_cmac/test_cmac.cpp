/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 11/10/2008
*/

#include <iomanip>
#include <new>

#include "testvec.h"
#include "cmac.h"

static unsigned int rand32(void)
{
	static unsigned int   r4, r_cnt = -1, w = 521288629, z = 362436069;

	z = 36969 * (z & 65535) + (z >> 16);
	w = 18000 * (w & 65535) + (w >> 16);

	r_cnt = 0; r4 = (z << 16) + w; return r4;
}

static unsigned char rand8(void)
{
	static unsigned int   r4, r_cnt = 4;

	if(r_cnt == 4)
	{
		r4 = rand32(); r_cnt = 0;
	}

	return (char)(r4 >> (8 * r_cnt++));
}

// fill a block with random charactrers

void block_rfill(unsigned char l[], unsigned int len)
{
	unsigned int  i;

	for(i = 0; i < len; ++i)

		l[i] = rand8();
}

const char *t_name[] = {
	"KEY", "MSG", "TAG", "VEC", "GEN", "END"
};
const size_t t_length = sizeof(t_name) / sizeof(t_name[0]);

#define MAX_BLOCK_SIZE  5120
#define BLOCK_SIZE      AES_BLOCK_SIZE

void do_test(const char *in_dir, const char *out_dir, const char *name, int gen_flag)
{
	std::string     line, f_name("CMAC");
	int64_t			i, err = -1, v_err = -1, file_no = 0, vec_no, vec_count;
	test_vec		v;
	std::fstream    inf, outf;
	char			ext[3] = { '.', '0', '\0' };
	cmac_ctx		contx[1];

	for( ; ; )
	{
		++file_no;
		if(!open_files(inf, outf, std::string(in_dir), std::string(out_dir),
			f_name, file_no, gen_flag))
			break;

		vec_count = v_err = 0;
		for( ; ; )
		{
			if(!input_vector(inf, v, &vec_no, outf, gen_flag))
				break;
			++vec_count;
			err = 0;
			
			size_t key_len, msg_len, tag_len;
			int64_t msg_rpt;

			const uint8_t * const key = v.get_value("KEY", &key_len);
			const uint8_t * const msg = v.get_value("MSG", &msg_len, &msg_rpt);
			const uint8_t * const tag = v.get_value("TAG", &tag_len);

			uint8_t * buf = new uint8_t[tag_len];
			cmac_init(key, key_len, contx);
			cmac_data((unsigned char*)msg, msg_len, contx);
			cmac_end(buf, contx);
            if(memcmp(buf, tag, tag_len))
            {
				std::cout << std::endl << "\tauthentication error on test number " << vec_no;
				err++;
            }
			delete[]buf;
			if(gen_flag && !err)
				v.vector_out(outf, vec_no);
			if(err)
			  ++v_err;
		}
		inf.close();
		if (gen_flag && !v_err)
		{
			outf << std::endl << "END " << std::endl;
			outf.close();
		}
		if(v_err == 0)
			std::cout << ": all " << vec_count << " vectors matched";
		else if(v_err > 0)
			std::cout << std::endl << v_err << " errors in " << vec_no << " vectors";
		else
			std::cout << ": test vector file(s) not found";
    }
    return;
}

int main(int argc, char *argv[])
{
	if(argc == 4)
		do_test(argv[1], argv[2], argv[3], 1);
	else
		std::cout << std::endl << "usage: input_directory output_directory mode_name";
	std::cout << std::endl;

	// test incremental use
	cmac_ctx ctx[1];
	uint8_t key[16], data[1024], tag0[16];
	block_rfill(key, 16);
	block_rfill(data, 1024);

	cmac_init(key, 16, ctx);
	cmac_data(data, 1024, ctx);
	cmac_end(tag0, ctx);

	int err = 0;
	for(int i = 0; i < 100; ++i)
	{
		uint8_t  tag[16];
		int rpos[32];
		for(int i = 0; i < 32; ++i)
			rpos[i] = rand32() % 32;

		cmac_init(key, 16, ctx);
		uint8_t *dp = data;
		for(int i = 0; i < 32; ++i)
		{
			cmac_data(dp, rpos[i], ctx);
			dp += rpos[i];
		}
		cmac_data(dp, data + 1024 - dp, ctx);
		cmac_end(tag, ctx);
		if(memcmp(tag0, tag, 16))
		{
			err += 1;
			for(unsigned int i = 0; i < 16; ++i)
				std::cout << std::setw(2) << std::setfill('0') << std::hex << (unsigned int)tag0[i];
			std::cout << std::endl;
			for(unsigned int i = 0; i < 16; ++i)
				std::cout << std::setw(2) << std::setfill('0') << std::hex << (unsigned int)tag[i];
			std::cout << std::endl;
		}
	}
	if(err == 0)
		std::cout << "Incremental message authntication passed" << std::endl << std::endl;

	return 0;
}
