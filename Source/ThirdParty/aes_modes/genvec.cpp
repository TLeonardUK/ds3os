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
Issue Date: 22/11/2013
*/

#include <iomanip>
#include "genvec.h"
#include "aes.h"

/* AES based random number generator */

static unsigned char key[16] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 2, 3, 8, 4 };
static unsigned char rnd[16] = { 2, 7, 1, 8, 2, 8, 1, 8, 2, 8, 4, 5, 9, 0, 4, 5 };
static aes_encrypt_ctx ctx[1];
static int     r_cnt = -1;  /* initialised negative to trigger AES keying */

void rand_update(void)
{
	if(r_cnt < 0)
		aes_encrypt_key(key, 16, ctx);
	/* OFB stream cipher for randomness */
	aes_encrypt(rnd, rnd, ctx);
	/* number of valid random bytes in buffer */
	r_cnt = 16;
	return;
}

/* return one pseudo random byte */

unsigned char rand8(void)
{
	if(r_cnt <= 0)
		rand_update();
	return rnd[--r_cnt];
}

/* fill a byte array with pseudo random values */

void block_rndfill(unsigned char l[], size_t len)
{
	unsigned long  i;

	for(i = 0; i < len; ++i)
		l[i] = rand8();
}

/* input a decimal number from a C string */

char *decimal_in(const char *s, int64_t *n)
{
	int g = -1;
	*n = 0;
	if(*s == '-')
		++s;
	else
		g = 1;

	while(isdigit(*s))
		*n = 10 * *n + g * (*s++ - '0');

	return (char*)s;
}

int to_hex(char ch)
{
	return (isdigit(ch) ? 0 : 9) + (ch & 0x0f);
}

/* input a hexadecimal number from a C string */

char *hexadecimal_in(const char *s, int64_t *n)
{
	*n = 0;
	while(isxdigit(*s))
		*n = (*n << 4) + to_hex(*s++);
	return (char*)s;
}

/* input an 8-bit byte from two charcters in a C string */

char *hexbyte_in(const char *s, unsigned char *n)
{
	if(isxdigit(*s) && isxdigit(*(s + 1)))
	{
		*n = (to_hex(*s) << 4) | to_hex(*(s + 1));
		return (char*)(s + 2);
	}
	else
		return 0;
}

uint64_t le_bytes_to_nbr(const unsigned char x[], size_t len)
{
	uint64_t t = 0;

	x += len;
	while(len--)
		t = 256 * t + *--x;
	return t;
}

uint64_t be_bytes_to_nbr(const unsigned char x[], size_t len)
{
	uint64_t t = 0;

	while(len--)
		t = 256 * t + *x++;
	return t;
}

/* 
input strings are comma separated values which are either hexadecimal
sequences or 'bracket terms' consisting of up to five comma separated
numerical values within brackets.  This routine parses the latter
bracket terms and puts the number of values obtained in cnt[0] and
the values in cnt[1] .. cnt[5]. Numbers are hexadecimal unless they
are preceeded by a '#', in which case they are decimal.
*/

int get_loop_vars(char **cp, int64_t cnt[], int hex_flag)
{
	char *p = *cp;
	int i, retv = 0;

	/* remove any leading space */
	while(*p && (*p == ' ' || *p == '\t'))
		++p;
	/* parse up to closing bracket or a maximum of 5 inputs */
	for(i = 1 ; *p != ')' && i < 6 ; ++i)
	{
		if(*p == '#')
			p = decimal_in(p + 1, cnt + i);
		else
			p = hexadecimal_in(p, cnt + i);
		while(*p && (*p == ' ' || *p == '\t' || *p == ','))
			++p;
	}

	if(*p == ')' && i < 7)    /* good input */
		cnt[0] = retv = i - 1;
	else                      /* bad input - remove to next closing bracket */
	{
		while(*p && *p != ')')
			++p;
		if(*p)
			++p;
	}
	*cp = p;
	return retv;
}

int init_sequence(char **cp, byte_array &arr, int64_t cnt[5])
{
	int retv, i;

	(*cp)++;    /* step past opening bracket */

	/* input the values in the bracket term  */
	retv = get_loop_vars(cp, cnt, 1);
	switch(retv)
	{
	case 1:
		/* (nn) - nn random bytes */
		i = (int)cnt[1];
		while(i--)
			arr.push_back(rand8());
		/* step past closing bracket */
		(*cp)++;
		break;
	case 2:
		/* (nn,hh) - nn bytes each of value hh  */
		cnt[3] = 0;
	case 3:
		/* (nn,hh,hi) - nn bytes of value hh, hh + hi, hh + 2 * hi, ...  */
		for(i = 0; i < cnt[1]; ++i)
		{
			arr.push_back((unsigned char)(cnt[2]));
			cnt[2] += cnt[3];
		}
		/* step past closing bracket */
		(*cp)++;
		break;
	case 4:
		/* (nn,v1,v2,v3=1..4) - signal with negative cnt[5] value */
		cnt[5] = -cnt[4];
	case 5:
		/* (nn,v1,v2,v3,v4) */
		/* signal a sequence of values */
		return 2;
	default:
		/* signal no value input */
		return 0;
	}
	return 1;
}

void v_rec::add_to_record(const char *c)
{
	const char *cp = c + strlen(c) - 1;

	/* remove extraneous characters at end of input */
	while(*cp <= 0x20 || *cp >= 0x7f)
		--cp;
	if(cp++ < c)
		return;

	/* if a string has not been allocated or it has but is not long enough */
	if(!rp || len < strlen(rp) + strlen(c) + 1)
	{
		/* set a new maximum length and allocate a new string */
		len = (int)((rp ? (unsigned int)strlen(rp) : 0) + cp - c + 80);
		char *p = new char[len];
		*p = '\0';
		/* if necessary copy old content into it */
		if(rp)
		{
			strcpy(p, rp);
			delete[]rp;
		}
		rp = p;
	}
	/* if there is content in the string and the new value is not a */
	/* continuation of a hexadecimal sequence, insert a comma       */
	if(*(rp) && *c != ',' && rp[strlen(rp) - 1] != ',' &&
		(!isxdigit(rp[strlen(rp) - 1]) || !isxdigit(*c)))
		strcat(rp, ",");
	/* add the new content */
	strncat(rp, c, cp - c);
}

int v_rec::get_value(byte_array &arr)
{
	char *p = rp + pos, chr[1];
	int retv = 0, i;
	arr.clear();

	if(rp == 0)
		return 0;

	/* remove white space and a ',' delimiter if present */
	while(*p && (*p == ' ' || *p == '\t'))
		++p;
	if(*p == ',')
		++p;

	if(*p)
	{
		if(*p == '=')
		{   /* use previous ciphertext */
			++p;
			retv = 1 | ctx_copy;
		}
		else if(isxdigit(*p) && isxdigit(*(p + 1)))
		{   /* input a hexadecimal byte sequence */
			while(hexbyte_in(p, (unsigned char*)chr))
			{
				arr.push_back(chr[0]);
				p += 2;
			}
			retv = 1;
		}
		else if(*p == '#')
		{   /* input a decimal value and assemble into  */
			/* little endian byte sequence              */
			int64_t num;
			int f = 0;
			if(*++p == '>' || *p == '<')
			if(*p++ == '>')
				f = 1;
			p = decimal_in(p, &num);
			while(num)
			{
				arr.push_back((unsigned char)(num & 0xff));
				num >>= 8;
			}
			if(f)
				arr = byte_array(arr.rbegin(), arr.rend());
			retv = 1;
		}
		else if(*p == '(')    /* initialise for a bracket term */
			retv = init_sequence(&p, arr, cnt);

		/* after the initialisation of a bracket term, the */
		/* parse point is positioned on the final bracket  */
		if(retv == 2 || *p == ')')
		{
			if(cnt[1] == 0)     /* if the sequence has finished */
			{                   /* go to next input value       */
				/* leave value in len untouched unless there is */
				/* actual input because we may need the prior   */
				/* length in case we have to copy the previous  */
				/* ciphertext into the next plaintext           */
				pos = (unsigned int)(++p - rp);
				retv = get_value(arr);
			}
			else
			{
				(cnt[1])--;  /* reduce remaining item count  */
				i = (int)(cnt[2]);  /* normal length count   */
				if(cnt[5] == -1 || cnt[5] == -2)
				{
					/* assemble sequential values v1 + n * v2   */
					/* (nn,v1,v2,1) - little endian byte array  */
					/* (nn,v1,v2,2) - big endian byte array     */
					int64_t x = cnt[2];
					cnt[2] += cnt[3];
					while(x)
					{
						arr.push_back((unsigned char)x);
						x >>= 8;
					}
					if(cnt[5] == -2)
						arr = byte_array(arr.rbegin(), arr.rend());
				}
				else if(cnt[5] == -3)
				{
					/* (nn,v1,v2,3) - assemble random array of  */
					/* bytes of increasing length v1 + n * v2   */
					while(i--)
						arr.push_back(rand8());
					cnt[2] += cnt[3];
				}
				else if(cnt[5] == -4)
				{
					/* (nn,v1,v2,4) - assemble random array of  */
					/* bytes of random length v1 <= len <= v2   */
					uint64_t t = 0;
					block_rndfill((unsigned char*)&t, 4);
					i += (int)(((cnt[3] + 1 - cnt[2]) * t) >> 32);
					while(i--)
						arr.push_back(rand8());
				}
				else
				{
					/* (nn,v1,v2,h1,h2) - assemble random array */
					/* bytes of increasing length v1 + n * v2   */
					/* in which bytes also increment in value   */
					/* h1 + n * h2 withing each array generated */
					int j = (int)(cnt[4]);
					while(i--)
					{
						arr.push_back((unsigned char)j);
						j += (int)(cnt[5]);
					}
					cnt[2] += cnt[3];
				}
				retv = 1;
			}
		}
	}
	/* if a value is returned signal its length */
	next_pos = (unsigned int)(p - rp);
	return retv;
}

bool input_test_def(std::fstream &inf, test_def &v, uint64_t *vec_no)
{
	std::string line, ty;
	v.reset();
	/* process lines in template file */
	bool in_vec = false;
	while(!inf.eof())
	{
		std::getline(inf, line);
		unsigned char *t;
		if(!line.empty())
		{
			ty = v.set_tag(line.c_str());
			if(ty == "VEC")
			{
				size_t l;
				t = v["VEC"].get_value(&l);
				*vec_no = be_bytes_to_nbr(t, l);
			}
			else if(ty == "END" || ty == "GEN")
			{
				return in_vec;
			}
			else if(ty == "TAG" && line.find("?") != std::string::npos)
			{
				if(!v.is_key("TGL") || v.is_empty("TGL"))
				{
					v.register_tag("TGL");
					std::string t = v.set_tag("TGL #16");
				}
				v.register_tag(ty.c_str());
			}
			else
			{
				v.register_tag(ty.c_str());
				in_vec = true;
			}
		}
		else if(in_vec)
			return true;
	}
	return false;
}

/* output a byte array preceeded by a descriptor */

void hex_out(std::fstream& outf, const char * desc, const unsigned char buf[], size_t len)
{
	if(len == 0)
		outf << std::endl << desc;
	else
		for(unsigned int i = 0; i < len; ++i)
		{
			if(i % 32 == 0)
				outf << std::endl << desc << ' ';
			outf << std::setw(2) << std::setfill('0') << std::hex << (unsigned int)buf[i];
		}
}

void test_def::vector_out(std::fstream& outf, uint64_t vec_no)
{
	outf << std::endl << std::dec << "VEC " << (int)vec_no;
	for(auto p = tag_order.cbegin() ; p != tag_order.cend() ; ++p )
	{
		if (*p != "TGL")
		{
			size_t len;
			const unsigned char *con(tags[*p].get_value(&len));
			hex_out(outf, p->c_str(), con, len);
		}
	}
	outf << std::endl;
}
