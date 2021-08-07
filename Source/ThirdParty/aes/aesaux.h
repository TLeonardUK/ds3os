/*
---------------------------------------------------------------------------
Copyright (c) 1998-2013, Brian Gladman, Worcester, UK. All rights reserved.

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
Issue Date: 25/09/2018
*/

#ifndef AESAUX_H
#define AESAUX_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "aestst.h"
#include "brg_types.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define FALSE   0
#define TRUE    1

enum line_type { bad_line = 0, block_len, key_len, test_no, iv_val, key_val, pt_val, ct_val };
#define NO_LTYPES   8
#define BADL_STR    "BADLINE="
#define BLEN_STR    "BLOCKSIZE="
#define KLEN_STR    "KEYSIZE=  "
#define TEST_STR    "TEST= "
#define IV_STR      "IV=   "
#define KEY_STR     "KEY=  "
#define PT_STR      "PT=   "
#define CT_STR      "CT=   "

char *file_name(char* buf, size_t len, const unsigned long type, const unsigned long blen, const unsigned long klen);
const char *pos(const char *s);
int to_hex(int ch);
int get_line(FILE *inf, char s[], int max_len);
char *copy_str(char *s, const char *fstr);
const char *df_string(const char *p);
int block_in(unsigned char l[], const char *p);
void block_clear(unsigned char l[], const unsigned long len);
void block_reverse(unsigned char l[], const unsigned long len);
void block_copy(unsigned char l[], const unsigned char r[], const unsigned long len);
void block_xor(unsigned char l[], const unsigned char r[], const unsigned long len);
int block_cmp(const unsigned char l[], const unsigned char r[], const unsigned long len);
uint32_t rand32(void);
unsigned char rand8(void);
void block_rndfill(unsigned char l[], unsigned long len);
void put_dec(char *s, unsigned long val);
unsigned long get_dec(const char *s);
int cmp_nocase(const char *s1, const char *s2);
int test_args(int argc, char *argv[], char des_chr, char tst_chr);
int find_string(const char *s1, const char s2[]);
enum line_type find_line(FILE *inf, char str[]);
void block_out(const enum line_type ty, const unsigned char b[], FILE *outf, const unsigned long len);
#if defined( DLL_IMPORT ) && defined(  DLL_DYNAMIC_LOAD  )
  HINSTANCE init_dll(fn_ptrs *fn);
#endif

#if !defined( _MSC_VER )
  int fopen_s(FILE** pFile, const char *filename, const char *mode);
#endif

#if defined(__cplusplus)
}
#endif
#endif
