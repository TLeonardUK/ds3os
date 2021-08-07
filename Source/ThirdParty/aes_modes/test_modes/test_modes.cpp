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

#include <iomanip>
#include "testvec.h"

const std::string in_dir("../testvals/");
const std::string out_dir("../outvals/");

#if defined(__cplusplus)
extern "C"
{
#endif

const char *t_name[] = {
	"KEY", "HDR", "PTX", "CTX", "TAG", "NCE", "IV ", "VEC", "RPT", "END", "REM", "MDE"
};
const size_t t_length = sizeof(t_name) / sizeof(t_name[0]);

#define RET_TYPE_DEFINED
typedef int  ret_type;
typedef ret_type t_init_and_key(const unsigned char[], unsigned long, void*);
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

#if defined(__cplusplus)
}
#endif

typedef struct
{   char                *name;
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
	f->init_and_key =    (t_init_and_key*)mode(init_and_key); \
	f->init_message =    (t_init_message*)mode(init_message); \
	f->auth_header =     (t_auth_header*)mode(auth_header); \
	f->auth_data =       (t_auth_data*)mode(auth_data); \
	f->crypt_data =      (t_crypt_data*)mode(crypt_data); \
	f->compute_tag =     (t_compute_tag*)mode(compute_tag); \
	f->encrypt =         (t_encrypt*)mode(encrypt); \
	f->decrypt =         (t_decrypt*)mode(decrypt); \
	f->encrypt_message = (t_encrypt_message*)mode(encrypt_message); \
	f->decrypt_message = (t_decrypt_message*)mode(decrypt_message); \
	f->end =             (t_end*)mode(end);

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

void do_test(mode_fns f[1], void* contx, bool gen_flag)
{
	std::string     line, f_name(f->name);
	int64_t			err = -1, v_err = -1, file_no = 0, vec_no, vec_count;
	test_vec		v;
	std::fstream    inf, outf;
	char			ext[3] = { '.', '0', '\0' };

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

			size_t key_len = 0, hdr_len = 0, ptx_len = 0,
				ctx_len = 0, tag_len = 0, nce_len = 0;
			int64_t hdr_rpt, ptx_rpt, ctx_rpt, rpt;

			const uint8_t * const key = v.get_value("KEY", &key_len);
			const uint8_t * const hdr = v.get_value("HDR", &hdr_len, &hdr_rpt);
			const uint8_t * const ptx = v.get_value("PTX", &ptx_len, &ptx_rpt);
			const uint8_t * const ctx = v.get_value("CTX", &ctx_len);
			const uint8_t * const tag = v.get_value("TAG", &tag_len);
			uint8_t * nce = nullptr;
			if(v.is_key("NCE"))
				nce = v.get_value("NCE", &nce_len);
			else if(v.is_key("IV"))
				nce = v.get_value("IV", &nce_len);
			else
				nce_len = 0;
			f->init_and_key(key, key_len, contx);

			if(strcmp(f->name, "CCM") == 0)
			{
				int ret = ((s_init_message*)f->init_message)(
					nce, nce_len, hdr_len * hdr_rpt, ptx_len * ptx_rpt, tag_len, contx);
				if(ret)
				{
					if(err == CCM_AUTH_LENGTH_ERROR && hdr_len >= 65536 - 256)
						continue;
				}
			}
			else
				f->init_message(nce, nce_len, contx);

			rpt = hdr_rpt;
			while(rpt--)
				f->auth_header(hdr, hdr_len, contx);

			uint8_t *enc_buf = new uint8_t[ptx_len];
			rpt = ptx_rpt;
			while(rpt--)
			{
				memcpy(enc_buf, ptx, ptx_len);
				f->encrypt(enc_buf, ptx_len, contx);
			}
			if(ptx_rpt == 1 && memcmp(enc_buf, ctx, ptx_len))
			{
				std::cout << std::endl << "\tencrypt ciphertext error on test number " << vec_no;
				err++;
			}
			delete[] enc_buf;

			uint8_t *enc_tag = new uint8_t[tag_len];
			f->compute_tag(enc_tag, tag_len, contx);
			f->end(contx);

			if(gen_flag)
				v.vector_out(outf, vec_no);
			else if(memcmp(enc_tag, tag, tag_len))
			{
				std::cout << std::endl << "\tencrypt tag error on test number " << vec_no;
				err++;
			}

			f->init_and_key(key, key_len, contx);

			if(strcmp(f->name, "CCM") == 0)
			{
				int err = ((s_init_message*)f->init_message)(
					nce, nce_len, hdr_len * hdr_rpt, ptx_len * ptx_rpt, tag_len, contx);
			}
			else
				f->init_message(nce, nce_len, contx);

			rpt = hdr_rpt;
			while(rpt--)
				f->auth_header(hdr, hdr_len, contx);

			uint8_t *dec_buf = new uint8_t[ptx_len];
			rpt = ptx_rpt;
			while(rpt--)
			{
				memcpy(dec_buf, ctx, ptx_len);
				f->decrypt(dec_buf, ptx_len, contx);
			}
			if(ptx_rpt == 1 && memcmp(dec_buf, ptx, ptx_len))
			{
				std::cout << std::endl << "\tdecrypt plaintext error on test number " << vec_no;
				err++;
			}
			delete[] dec_buf;

			uint8_t *dec_tag = new uint8_t[tag_len];
			f->compute_tag(dec_tag, tag_len, contx);
			if(memcmp(dec_tag, tag, tag_len))
			{
				std::cout << std::endl << "\tdecrypt tag error on test number " << vec_no;
				err++;
			}
			delete[] enc_tag;
			delete[] dec_tag;
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

void ccm_tests(unsigned long key_len, unsigned long iv_len, bool flag)
{
	ccm_ctx ctx[1];
	mode_fns f[1];

	ccm_functions(f);
	do_test(f, ctx, flag);
}

void cwc_tests(unsigned long key_len, unsigned long iv_len, bool flag)
{
	cwc_ctx ctx[1];
	mode_fns f[1];

	cwc_functions(f);
	do_test(f, ctx, flag);
}

void eax_tests(unsigned long key_len, unsigned long iv_len, bool flag)
{
	eax_ctx ctx[1];
	mode_fns f[1];

	eax_functions(f);
	do_test(f, ctx, flag);
}

void gcm_tests(unsigned long key_len, unsigned long iv_len, bool flag)
{
	gcm_ctx ctx[1];
	mode_fns f[1];

	gcm_functions(f);
	do_test(f, ctx, flag);
}

int main(void)
{
	int     flag = false;

	ccm_tests(16, 12, flag);
	cwc_tests(16, 12, flag);
	eax_tests(16, 12, flag);
	gcm_tests(16, 12, flag);
	std::cout << std::endl;
}
