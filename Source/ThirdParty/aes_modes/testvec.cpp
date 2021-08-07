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

#include <sstream>
#include <iomanip>
#include "testvec.h"

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

char *hexbyte_in(const char *s, uint8_t *n)
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

void test_vec::load_vector(std::string tag, const char *p)
{
	uint8_t byte[1];
	vec[tag].second = 1;
	while(isxdigit(*p) && isxdigit(*(p + 1)))
	{
		hexbyte_in(p, byte);
		vec[tag].first.push_back(byte[0]);
		p += 2;
	}
}

bool input_vector(std::fstream &inf, test_vec &v, int64_t *vec_no, std::fstream& outf, bool gen_flag)
{
	std::string line;
	v.reset();
	/* process lines in template file */
	std::string ty;
	bool in_vec = false;
	int line_count = 0;
	while(!inf.eof())
	{
		std::getline(inf, line);
		if(line.size() == 0 && in_vec)
			return true;
		if(line.find("END") == 0)
			return in_vec && !v.is_empty();

		if(line.find("VEC") == 0)
		{
			if(in_vec)
				v.reset();
			decimal_in(line.c_str() + 4, vec_no);
			in_vec = true;
		}
		else if(in_vec)
			ty = v.set_tag(line.c_str());
		else if(gen_flag)
		{
			line_count++;
			if(line_count > 2)
				outf << std::endl << line;
		}
	}
	return false;
}

/* output a byte array preceeded by a descriptor */

void hex_out(std::fstream& outf, const char * desc, const unsigned char buf[], size_t len)
{
	if(len == 0)
		outf << std::endl << desc << ' ';
	else
		for(unsigned int i = 0; i < len; ++i)
		{
			if(i % 32 == 0)
				outf << std::endl << desc << ' ';
			outf << std::setw(2) << std::setfill('0') << std::hex << (unsigned int)buf[i];
		}
}

void output_time(std::ostream& out_file, const char* msg)
{
	/* output date and time */
	time_t secs;
	struct tm now;
	char buf[32];
	time(&secs);
	localtime_s(&now, &secs);
	asctime_s(buf, 32, &now);
	out_file << std::endl << msg << buf;
}

void test_vec::vector_out(std::fstream& outf, uint64_t vec_no)
{
	outf << std::endl << std::dec << "VEC " << (int)vec_no;
	for( auto p = tag_order.cbegin() ; p != tag_order.cend() ; ++p )
	{
		size_t len;
		int64_t rep;
		const uint8_t * const value = get_value(p->c_str(), &len, &rep);
		hex_out(outf, p->c_str(), value, len);
		if(rep > 1)
			outf << std::endl << "RPT " << std::dec << rep;
	}
	outf << std::endl;
}

std::string base_name(const std::string& dir, const std::string& file)
{
	std::string s(dir);
	if(s.find('\\') != std::string::npos)
	{
		if(s.rfind('\\') != s.size() - 1)
			s += '\\';
	}
	else if(s.find('/') != std::string::npos)
	{
		if(s.rfind('/') != s.size() - 1)
			s += '\\';
	}
	else
		s += '\\';
	s += file;
	for(auto p = s.begin() ; p < s.end() ; ++p)
		*p = tolower(*p);
	return s;
}

bool open_files(std::fstream &inf, std::fstream &outf, const std::string& in,
	const std::string& out, const std::string& f_name, int64_t file_no, bool gen_flag)
{
	std::string fn(f_name), line;
	std::ostringstream buf;
	buf << file_no;
	fn += "." + buf.str();
	std::string in_fn(base_name(in, fn)), out_fn(base_name(out, fn));
	inf.open(in_fn, std::ios_base::in);
	if(!inf)
		return false;

	std::cout << std::endl << "Using " << f_name << " test vectors in \"" << in_fn << "\"";
	do
	{
		std::getline(inf, line);
		if(line.find("MDE") == 0)
			break;
		if(line.find("END") == 0)
			break;
	}
	while
		(!inf.eof());

	if(inf.eof() || line.find("END") == 0)
		return false;

	if(line.find(f_name) != 4)
	{
		std::cout << std::endl << "this file does not match " << f_name;
		return false;
	}

	if(gen_flag)
	{
		outf.open(out_fn, std::ios_base::out | std::ios_base::trunc);
		if(!outf)
		{
			inf.close();
			return false;
		}
		// output date and time
		output_time(outf, "REM Produced by MODETEST on ");
		outf << "MDE " << f_name << std::endl;
	}
	return true;
}

