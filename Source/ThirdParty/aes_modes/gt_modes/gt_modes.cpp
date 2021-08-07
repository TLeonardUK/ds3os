
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
Issue Date: 21/07/2013
*/

#include <time.h>
#include <iomanip>
#include "genvec.h"

const char *t_name[]
{
	"KEY", "PTX", "CTX", "NCE", "IV ", "HDR", "TGL", "TAG", "VEC", "GEN", "END"
};

const size_t t_length = sizeof(t_name) / sizeof(t_name[0]);

#define cstr_cmp _strnicmp

#if 0
# define TEST_CCM
#endif
#if 0
# define TEST_GCM
#endif
#if 0
# define TEST_EAX
#endif
#if 1
# define GEN_TEST
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#include "aes.h"
#include "ccm.h"
#include "ltc_ccm.h"
#include "gcm.h"
#include "ltc_gcm.h"
#include "eax.h"
#include "ltc_eax.h"

#if defined( _MSC_VER ) && ( _MSC_VER < 1300 ) 
  typedef __int64 int64_t;
#else
  typedef long long int64_t;
#endif

#define LTC_ENCRYPT 0
#define LTC_DECRYPT 1

	  typedef ret_type(*brg_i)(
		  const unsigned char key[],
		  unsigned long key_len,
		  void *ctx);

	  typedef ret_type(*brg_f)(
		  const unsigned char iv[], unsigned long iv_len,
		  const unsigned char hdr[], unsigned long hdr_len,
		  unsigned char msg[], unsigned long msg_len,
		  unsigned char tag[], unsigned long tag_len,
		  void *ctx);

	  typedef int(*ltc_f)(
		  const unsigned char key[], unsigned long key_len,
		  const unsigned char nonce[], unsigned long nonce_len,
		  const unsigned char hdr[], unsigned long hdr_len,
		  unsigned char pt[], unsigned long pt_len,
		  unsigned char ct[],
		  unsigned char tag[], unsigned long tag_len,
		  int dir, int *stat);

	  struct                        {
    brg_i   brg_ifun[3];
    brg_f   brg_efun[3];
    brg_f   brg_dfun[3];
    ltc_f   ltc_fun[3];
} ea_funs =
{   { (brg_i)ccm_init_and_key, (brg_i)gcm_init_and_key, (brg_i)eax_init_and_key },
    { (brg_f)ccm_encrypt_message, (brg_f)gcm_encrypt_message, (brg_f)eax_encrypt_message },
    { (brg_f)ccm_decrypt_message, (brg_f)gcm_decrypt_message, (brg_f)eax_decrypt_message },
    { (ltc_f)ccm_memory, (ltc_f)gcm_memory, (ltc_f)eax_memory }
} ;

typedef enum mode   { m_ccm = 0, m_gcm = 1, m_eax = 2 };
char *mode_name[] = { "CCM", "GCM", "EAX" };

typedef union
{
    ccm_ctx ccm[1];
    gcm_ctx gcm[1];
    eax_ctx eax[1];
} ctx_union;

#if defined(__cplusplus)
}
#endif

char *rem = 
    "\nREM All inputs and outputs are arrays of 8-bit bytes (octets) with bytes being"
    "\nREM represented by consecutive pairs of hexadecimal digits (the pair 'ab', for"
    "\nREM example, yields the byte value 0xab in C). The index positions of bytes in"
    "\nREM arrays increase by 1 from left to right for each pair of digits and arrays"
    "\nREM on consecutive lines with the same initial designators are concatenated so"
    "\nREM that bytes on later lines have higher array indexes.  Numeric significance"
    "\nREM is undefined except between the two digits that form each individual byte."
    "\n"
    "\nREM KEY = The AES key"
    "\nREM NCE = The nonce"
    "\nREM HDR = The header data"
    "\nREM PTX = The plaintext data"
    "\nREM CTX = The ciphertext data" 
    "\nREM TAG = The tag value\n"; 

#define MAX_BLOCK_SIZE 16 * 256     /* 4096 byte limit on blocks */

void do_test( unsigned int ntests, enum mode mm )
{
    unsigned char key[2 * AES_BLOCK_SIZE];    /* the AES key              */
    unsigned char iv[MAX_BLOCK_SIZE];         /* the nonce                */
    unsigned char hdr[MAX_BLOCK_SIZE];        /* the header               */
    unsigned char ptx[MAX_BLOCK_SIZE];        /* the plaintext            */
    unsigned char ctx[2][MAX_BLOCK_SIZE];     /* BRG and LTC ciphertexts  */
    unsigned char res[2][MAX_BLOCK_SIZE];     /* BRG and LTC decrypts     */
    unsigned char tag[2][AES_BLOCK_SIZE];     /* BRG and LTC tags         */
    unsigned int  i, key_len = AES_BLOCK_SIZE, iv_len, hdr_len, msg_len, 
                  err = 0, err1, stat;
    ctx_union     context[1];

    err1 = RETURN_GOOD;
    for( i = 0; i < ntests; ++i )
    {
        /* the mode key value   */
        block_rndfill( key, 16 );

        /* the iv/nonce value   */
        block_rndfill( (unsigned char*)&iv_len, 4 );
        iv_len &= 0x1fff;

        /* adjust for nonce format variations */
        switch(mm)
        {
        case m_ccm:
            iv_len = 6 + (iv_len & 7);
            iv_len = ( iv_len < 7 ? 7 : iv_len );
            break;
        case m_gcm:
            iv_len &= 0x1fff;
            iv_len = (iv_len & 0x1000 ? 12 : iv_len);
            break;
        case m_eax:
            iv_len &= 0x1fff;
            iv_len = (iv_len & 0x1000 ? iv_len & 0x001f : iv_len);
        }
        block_rndfill( iv, iv_len );

        /* the authenticated header */
        block_rndfill( (unsigned char*)&hdr_len, 4 );
        hdr_len &= 0x1fff;
        /* half messages have no headers */
        hdr_len = ( hdr_len & 0x1000 ? 0 : hdr_len );
        block_rndfill( hdr, hdr_len );

        /* the plaintext message */
        block_rndfill( (unsigned char*)&msg_len, 4 );
        msg_len &= 0x0fff;
        block_rndfill( ptx, msg_len );

        /* initialise BRG version */
        ea_funs.brg_ifun[mm](key, key_len, context);
        memcpy( ctx[0], ptx, msg_len );
        /* encrypt and authenticate for BRG version*/
        ea_funs.brg_efun[mm](iv, iv_len, hdr, hdr_len, ctx[0], 
                        msg_len, tag[0], AES_BLOCK_SIZE, context );

        /* initialise BRG version */
        ea_funs.brg_ifun[mm]( key, key_len, context );
        memcpy( res[0], ctx[0], msg_len );
        /* decrypt and verify for BRG version*/
        err = ea_funs.brg_dfun[mm]( iv, iv_len, hdr, hdr_len, res[0], 
                        msg_len, tag[0], AES_BLOCK_SIZE, context );

        /* encrypt and authenticate for LTC version*/
        ea_funs.ltc_fun[mm]( key, key_len, iv, iv_len, hdr, hdr_len, 
                ptx, msg_len, ctx[1], tag[1], AES_BLOCK_SIZE, LTC_ENCRYPT, 0);
        /* decrypt and verify for LTC version*/
        ea_funs.ltc_fun[mm]( key, key_len, iv, iv_len, hdr, hdr_len, 
                res[1], msg_len, ctx[1], tag[1], AES_BLOCK_SIZE, LTC_DECRYPT, (int*)&stat);

        if(memcmp(ptx, res[0], msg_len))
        {
			std::cout << std::endl << mode_name[mm] << ": (BRG) encryption or decryption error";
            err1 = RETURN_ERROR;
        }
        if(err != RETURN_GOOD)
        {
			std::cout << std::endl << mode_name[mm] << ": (BRG) tag error";
            err1 = RETURN_ERROR;
        }
        if(memcmp(ptx, res[1], msg_len))
        {
			std::cout << std::endl << mode_name[mm] << ": (LTC) encryption or decryption error";
            err1 = RETURN_ERROR;
        }
        if(stat != RETURN_GOOD)
        {
			std::cout << std::endl << mode_name[mm] << ": (LTC)tag error";
            err1 = RETURN_ERROR;
        }
        if(memcmp(ctx[0], ctx[1], msg_len))
        {
			std::cout << std::endl << mode_name[mm] << ": ciphertext mismatch error";
            err1 = RETURN_ERROR;
        }
        if(memcmp(tag[0], tag[1], AES_BLOCK_SIZE))
        {
			std::cout << std::endl << mode_name[mm] << ": tag mismatch error";
            err1 = RETURN_ERROR;
        }
    }
    if(err1 == RETURN_GOOD)
    {
		std::cout << std::endl << mode_name[mm] << ": (BRG) and (LTC) outputs match";
    }
}

int gen_vectors(char *in_file_name, char *out_file_name)
{
    unsigned char   ctx[2][MAX_BLOCK_SIZE];     /* BRG and LTC ciphertexts                      */
    unsigned char   tag[2][AES_BLOCK_SIZE];     /* BRG and LTC tags                             */
    int             err = RETURN_ERROR;
    uint64_t		vec_no;
    std::fstream    in_file, out_file;
	std::string     line;
	enum mode       mm;
    ctx_union       context[1];
	test_def		v;

	/* open template input file */
	in_file.open(in_file_name, std::ios_base::in);
	if(!in_file)
		goto exit3;

	/* open test vector output file */
	out_file.open(out_file_name, std::ios_base::out | std::ios_base::trunc);
	if(!out_file)
		goto exit2;
    
    while(!in_file.eof())
    {
        do  /* look for start line 'MODE XXX' */
        {
			std::getline(in_file, line);
        }
        while
			(!in_file.eof() && strncmp((char*)line.c_str(), "MODE", 4) != 0);

		if (in_file.eof())
			break;

		if (!strncmp(line.c_str() + 5, mode_name[0], 3))
            mm = m_ccm; /* CCM */
        else if( !strncmp( line.c_str() + 5, mode_name[1], 3 ) )
            mm = m_gcm; /* GCM */
        else if( !strncmp( line.c_str() + 5, mode_name[2], 3 ) )
            mm = m_eax; /* EAX */
        else
            continue;

        /* output mode header 'MDE XXX' */
        out_file << std::endl << "MDE " << line.c_str() + 5;

		/* output date and time */
		time_t secs;
		struct tm now;
		char buf[32];
		time(&secs);
		localtime_s(&now, &secs);
		asctime_s(buf, 32, &now);
		out_file << std::endl << "REM Produced by GENTEST on " << buf;

        /* output line designator summary */
        out_file << rem;

        while(!in_file.eof())
		{
			if(!input_test_def(in_file, v, &vec_no))
				break;

			do
			{	size_t l_key, l_nce, l_hdr, l_ptx, l_tgl;
				unsigned char * const v_key = v["KEY"].get_value(&l_key);			/* key       */
				unsigned char * const v_nce = v["NCE"].get_value(&l_nce);			/* nonce     */
				unsigned char * const v_hdr = v["HDR"].get_value(&l_hdr);			/* header    */
				unsigned char * const v_ptx = v["PTX"].get_value(&l_ptx, ctx[0]);	/* plaintext */
				unsigned char * const v_tgl = v["TGL"].get_value(&l_tgl);			/* tag       */
                                
				/* run mode funtions here             */
                /* key:        v_key of length l_key  */
                /* nonce:      v_key of length l_nce  */
                /* header:     v_hdr of length l_hdr  */
                /* plaintext:  v_ptx of length l_ptx  */
                /* tag length: v_tgl of length l_tgl  */

                /* my mode functions */
				if(ea_funs.brg_ifun[mm](v_key, (unsigned long) l_key, context) != RETURN_GOOD)
                {
					std::cout << std::endl << "Initialisation failure in BRG's " << mode_name[mm] << " mode code";
                    goto exit1;
                }

				memcpy(ctx[0], v_ptx, l_ptx);
                if(ea_funs.brg_efun[mm](
					v_nce, (unsigned long)l_nce,
					v_hdr, (unsigned long)l_hdr,
					ctx[0], (unsigned long)l_ptx,
					tag[0], (unsigned long)*(v_tgl),
                    context ) != RETURN_GOOD)
                {
					std::cout << std::endl << "Encryption failure in BRG's " << mode_name[mm] << " mode code";
                    goto exit1;
                }

                /* mode functions derived from Tom St Denis' LTC */
                if(ea_funs.ltc_fun[mm](
					v_key, (unsigned long)l_key,
					v_nce, (unsigned long)l_nce,
					v_hdr, (unsigned long)l_hdr,
					v_ptx, (unsigned long)l_ptx,
                    ctx[1],
					tag[1], (unsigned long)*(v_tgl),
                    LTC_ENCRYPT, 0) != EXIT_SUCCESS)
                {
					std::cout << std::endl << "Failure in LTC's " << mode_name[mm] << "mode code";
                    goto exit1;
                }
                
				/* check for ciphertext equality */
                if( memcmp( ctx[0], ctx[1], l_ptx ) )
                {
					std::cout << std::endl << "Ciphertext mismatch for " << mode_name[mm] << "mode on vector" << (int)vec_no;
                    goto exit1;
                }
                
				/* check for tag equality */
				if(memcmp(tag[0], tag[1], *(v_tgl)))
                {
					std::cout << std::endl << "Tag mismatch for " << mode_name[mm] << " mode on vector " << (int)vec_no;
                    goto exit1;
                }

				v["CTX"].set_binary(ctx[0], l_ptx);
				v["TAG"].set_binary(tag[0], *(v_tgl));

                /* output the generated test vector */
				v.vector_out(out_file, vec_no);
				++vec_no;
			}
			while(v.next());
		}
    }
    out_file << std::endl << "END" << std::endl;
exit1:
    out_file.close();
exit2:
    in_file.close();
exit3:
    return err;
}

int main(int argc, char *argv[])
{
#ifdef TEST_CCM
    do_test( 10000, m_ccm );
#endif
#ifdef TEST_GCM
    do_test( 10000, m_gcm );
#endif
#ifdef TEST_EAX
    do_test( 10000, m_eax );
#endif
#ifdef GEN_TEST
    if(argc == 3)
        gen_vectors( argv[1], argv[2] );
    else
        std::cout << std::endl << "usage: gentest vector_rule_file vector_output_file" << std::endl;
#endif
	std::cout << std::endl << std::endl;
    return 0;
}
