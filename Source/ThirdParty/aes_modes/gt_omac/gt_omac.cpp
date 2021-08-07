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
 Issue Date: 25/11/2013
*/

#include <time.h>
#include <iomanip>
#include "genvec.h"
#include "omac.h"

const char *t_name[] = {
	"KEY", "MSG", "TAG", "VEC", "GEN", "END"
};
const size_t t_length = sizeof(t_name) / sizeof(t_name[0]);

char *rem =
	"\nREM All inputs and outputs are arrays of 8-bit bytes (octets) with bytes being"
	"\nREM represented by consecutive pairs of hexadecimal digits (the pair 'ab', for"
	"\nREM example, yields the byte value 0xab in C). The index positions of bytes in"
	"\nREM arrays increase by 1 from left to right for each pair of digits and arrays"
	"\nREM on consecutive lines with the same initial designators are concatenated so"
	"\nREM that bytes on later lines have higher array indexes.  Numeric significance"
	"\nREM is undefined except between the two digits that form each individual byte."
	"\n"
	"\nREM KEY = The OMAC key"
	"\nREM MSG = The message"
	"\nREM TAG = The authentication tag\n";

#define MAX_BLOCK_SIZE 5120

int gen_vectors(char *in_file_name, char *out_file_name)
{
	uint64_t		vec_no;
	int				err = 0;
	std::fstream    in_file, out_file;
	std::string		line;
	omac_ctx        ctx[1];
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

		if (strncmp(line.c_str() + 5, "OMAC", 3))
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
			{
				size_t key_len, msg_len;
				unsigned char * const key = v["KEY"].get_value(&key_len);
				unsigned char * const msg = v["MSG"].get_value(&msg_len);
				unsigned char tag[AES_BLOCK_SIZE];

				omac_init( key, key_len, ctx);
				omac_data( msg, msg_len, ctx);
				omac_end(tag, ctx);

				v["TAG"].set_binary(tag, AES_BLOCK_SIZE);
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
    if(argc == 3)
        gen_vectors( argv[1], argv[2] );
    else
        std::cout << std::endl << "usage: gentest vector_rule_file vector_output_file";
	std::cout << std::endl << std::endl;
    return 0;
}

