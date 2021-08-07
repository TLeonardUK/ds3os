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

#ifndef _GENVEC_H
#define _GENVEC_H

#include <cstdint>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>
#include <stdexcept>

#if defined(__cplusplus)
extern "C"
{
#endif

extern const char *t_name[];
extern const size_t t_length;

typedef std::vector<unsigned char> byte_array;
class test_def;

void block_rndfill(unsigned char l[], size_t len);
uint64_t le_bytes_to_nbr(const unsigned char x[], size_t len);
uint64_t be_bytes_to_nbr(const unsigned char x[], size_t len);
bool input_test_def(std::fstream &inf, test_def &v, uint64_t *vec_no);
void hex_out(std::fstream &outf, const char *desc, const unsigned char buf[], size_t len);

#if defined(__cplusplus)
}
#endif

/* 
This structure accumulates the character strings for each specific
line designator in a test vector template. The values are accumulated
in the dynamically allocated C string pointed to by rp.  It also
records the position in this string while parsing it (pos) and
numerical values obtained as a result (cnt[6]). The number of values
obtained, up to five, is set in cnt[0]
*/

class v_rec {
	char *rp;
	unsigned int  len;
	unsigned int  pos;
	unsigned int  next_pos;
	unsigned int  retv;
	bool          init;
	int64_t       cnt[6];
	typedef       std::vector<unsigned char> byte_array;
	byte_array    value;
	static const int ctx_copy = 0xffff0000;
public:
	
	v_rec() : rp(nullptr), len(0), pos(0), next_pos(0), retv(0), init(false)
	{
	}
	
	void reset()
	{
		if(rp != nullptr)
			delete[] rp;
		rp = nullptr;
		len = 0;
		pos = 0;
		retv = 0;
		next_pos = 0;
		init = false;
	}

	~v_rec()
	{
		reset();
	}

	bool is_empty()
	{
		return rp == nullptr || *rp == '\0';
	}

	int get_value(byte_array &arr);

	void add_to_record(const char *c);

	bool next()
	{
		pos = next_pos;
		retv = get_value(value);
		if(!retv)
		{
			pos = next_pos = 0;
			init = false;
		}
		return (retv & ~ctx_copy) != 0;
	}

	unsigned char * const get_value(size_t *len, unsigned char const w[] = nullptr)
	{	
		if(!init)
		{
			retv = get_value(value);
			init = true;
		}

		if(retv & ctx_copy && w != nullptr)
		{
			size_t i = *len;
			value.clear();
			while(i--)
				value.push_back(*w++);
		}
		*len = value.size();
		return *len ? &(value[0]) : nullptr;
	}

	void set_binary(const unsigned char *v, size_t len)
	{
		value.clear();
		while(len--)
			value.push_back(*v++);
		init = true;
	}
};

class test_def
{
private:
	typedef std::unordered_map<std::string, v_rec> tag_str;
	tag_str tags;
	std::vector<std::string> tag_order;
public:

	class key_error : public std::exception {
	public:
		key_error()
		{
			std::cout << std::endl << "An undefined tag has been used";
		}
	};

	test_def()
	{
		for( int i = 0 ; i < t_length ; ++i )
		{
			add_tag(t_name[i]);
		}
	}

	void reset()
	{
		tag_order.clear();
		for(auto p = tags.begin(); p != tags.end() ; ++p )
		{
			p->second.reset();
		}
	}

	bool is_key(const char *key) const
	{
		return tags.find(std::string(key)) != tags.end();
	}

	bool add_tag(const char *key)
	{
		if(!is_key(key))
		{
			tags.insert(tag_str::value_type(key, v_rec()));
			return true;
		}
		else
			throw key_error();
	}

	bool is_empty(const char *key)
	{
		if(is_key(key))
		{
			return tags[key].is_empty();
		}
		else
			throw key_error();
	}

	std::string set_tag(const char *p)
	{
		std::string key_name;
		while(*p >= 'A' &&  *p <= 'Z' || *p >= 'a' && *p <= 'z' )
		{
			key_name.push_back(*p++);
		}

		while(*p == ' ' || *p == '\t')
			++p;

		for( int i = 0 ; i < t_length ; ++i )
		{
			if(!strncmp(key_name.c_str(), t_name[i], 3))
			{
				if(*p != '?')
					tags[key_name].add_to_record(p);
				return key_name;
			}
		}
		return std::string();
	}

	void register_tag(const char *key)
	{
		for( auto i = tag_order.cbegin() ; i != tag_order.cend() ; ++i )
		{
			if(*i == key)
				return;
		}
		tag_order.push_back(key);
	}

	v_rec& operator[](const char *key)
	{
		if(is_key(key))
		{
			return tags[key];
		}
		else
			throw key_error();
	}

	bool next()
	{
		auto i = tag_order.rbegin();
		do
		{
			if(tags[*i++].next())
				break;
		}
		while
			(i != tag_order.rend());
		return (i != tag_order.rend());
	}

	void vector_out(std::fstream& outf, uint64_t vec_no);
};

#endif
