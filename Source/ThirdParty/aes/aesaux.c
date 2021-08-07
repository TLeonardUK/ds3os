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

#ifndef __GNUC__
#  include <windows.h>
#  include <direct.h>
#endif
#include "aes.h"
#include "aesaux.h"
#include "aestst.h"

#if defined(__cplusplus)
extern "C"
{
#endif

char *st[8] = { "vk", "vt", "nk", "nt", "me", "md", "me", "md" };

char *file_name(char* buf, size_t len, const unsigned long type, const unsigned long blen, const unsigned long klen)
{   char *sp;

    if(len < 12)
        return 0;
    sp = copy_str(buf, (type < 6 ? "ecb" : "cbc"));
    sp = copy_str(sp, st[type]);
    *sp++ = (char)('0' + (char)blen / 4);
    *sp++ = (char)('0' + (char)klen / 4);
    sp = copy_str(sp, ".txt");
    *sp = '\0';
    return buf;
}

#if !defined( _MSC_VER )
int fopen_s(FILE** pFile, const char *filename, const char *mode)
{	char ul_name[64], *d = ul_name;
	const char *s = filename;
	FILE * fp;

	do
	{
		*d++ = (char)(*s == '\\' ? '/' : *s);
	}
	while(*s++);

	*pFile = fp = fopen(ul_name, mode);
	return fp == NULL;
}
#endif

const char *pos(const char *s)
{
    while(*s++ != '=')
        ;
    while(*s++ == ' ')
        ;
    return s - 1;
}

// Strings to locate lines in test vector files

const char *fstr[NO_LTYPES] =
        { BADL_STR, BLEN_STR, KLEN_STR, TEST_STR, IV_STR, KEY_STR, PT_STR, CT_STR };

char    *hxx = "0123456789abcdef";

// Hexadecimal conversion from hex character to a number in range 0 <= no <= 15

int to_hex(int ch)
{
    return (ch & 15) + (ch >= '0' && ch <= '9' ? 0 : 9);
}

int get_line(FILE *inf, char s[], int max_len)
{
    if(feof(inf))
        return EXIT_FAILURE;

    return fgets(s, max_len, inf) != s ? EXIT_FAILURE : EXIT_SUCCESS;
}

char *copy_str(char *d, const char *s)
{
    while(*s)
        *d++ = *s++;

    *d = '\0';
    return d;
}

const char *df_string(const char *f)
{   const char *p = f, *p1, *p2;
 
    p1 = p2 = 0;
    while(*p)
        if(*p++ == '\\')
        {
            p1 = p2; p2  = p;
        }
    return (p1 ? p1 : p2 ? p2 : f);
}

// Read from a string of hexadecimal digits into an array of 8-bit bytes
// Each pair of hexadecimal digits is used to compile one 8-bit unsigned char and
// the first (last) digit forms the most (least) significant four bits
// of each unsigned char. The hexadecimal digits are read from left (low) to right
// (high) and are placed in increasing index positions in the unsigned char array

int block_in(unsigned char l[], const char *p)
{   int i = 0;

    p = pos(p);

    while(*p && *(p + 1) && isxdigit(*p) && isxdigit(*(p + 1)))
    {
        l[i++] = (to_hex(*p) << 4) + to_hex(*(p + 1)); p += 2;
    }

    return i;
}

// clears an array of 8-bit bytes

void block_clear(unsigned char l[], const unsigned long len)
{   unsigned char *lp = l;

    while(lp < l + len)

        *lp++ = 0;
}

// reverses the order of an array of 8-bit bytes

void block_reverse(unsigned char l[], const unsigned long len)
{    unsigned char  *blo = l, *bhi = l + len - 1, by;

    while(blo < bhi)

        by = *blo, *blo++ = *bhi, *bhi-- = by;
}

// copies an array of 8-bit bytes (non overlapping)

void block_copy(unsigned char l[], const unsigned char r[], const unsigned long len)
{   unsigned char          *lp = l;
    const unsigned char    *rp = r;

    while(lp < l + len)

        *lp++ = *rp++;
}

// xors one array of 8-bit bytes with another

void block_xor(unsigned char l[], const unsigned char r[], const unsigned long len)
{   unsigned char          *lp = l;
    const unsigned char    *rp = r;

    while(lp < l + len)

        *lp++ ^= *rp++;
}

// compares two arrays of 8-bit bytes and return true if the same or
// false otherwise

int block_cmp(const unsigned char l[], const unsigned char r[], const unsigned long len)
{   const unsigned char    *lp = l, *rp = r;

    while(lp < l + len)
        if(*lp++ != *rp++)
            return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

uint32_t rand32(void)
{   static uint32_t r4, w = 521288629, z = 362436069;

    z = 36969 * (z & 65535) + (z >> 16);
    w = 18000 * (w & 65535) + (w >> 16);

    r4 = (z << 16) + w; return r4;
}

unsigned char rand8(void)
{   static unsigned long   r4,r_cnt = 4;

    if(r_cnt == 4)
    {
        r4 = rand32(); r_cnt = 0;
    }

    return (unsigned char)(r4 >> (8 * r_cnt++));
}

// fill a block with random characters

void block_rndfill(unsigned char l[], unsigned long len)
{   unsigned long  i;

    for(i = 0; i < len; ++i)

        l[i] = rand8();
}

// Convert decimal number to a character string

void put_dec(char *s, unsigned long val)
{   char    *p = s, *q = s, ch;

    do
    {
        *p++ = (char)('0' + (val % 10)); val /= 10;
    }
    while
        (val);

    *p-- = '\0';

    while(p > q)
    {
        ch = *p, *p-- = *q, *q++ = ch;
    }
}

// Extract decimal number from a character string

unsigned long get_dec(const char *s)
{   const char  *p = pos(s);
    unsigned long nbr = 0;

    while(*p && *p >= '0' && *p <= '9')
    {
        nbr = 10 * nbr + (*p - '0'); ++p;
    }

    return nbr;
}

// Compare two strings ignoring case

int cmp_nocase(const char *s1, const char *s2)
{   const char  *p1 = pos(s1), *p2 = pos(s2);

    while(*p1 && *p2)
    {
        if(toupper(*p1) != toupper(*p2))
        {
            return (toupper(*p1) < toupper(*p2) ? -1 : 1);
        }

        ++p1; ++p2;
    }

    while(*p1) p1++; while(*p2) p2++;

    return (int)(p2 - s2) - (int)(p1 - s1);
}

// Input command line arguments

int test_args(int argc, char *argv[], char des_chr, char tst_chr)
{   int i, j;

    for(i = 1; i < argc; ++i)
    {
        if(argv[i][0] != '-' && argv[i][0] != '/')

            continue;

        if(argv[i][1] != tolower(des_chr) && argv[i][1] != toupper(des_chr))

            continue;

        else if(!tst_chr)

            return TRUE;

        if(argv[i][2] != ':')

            continue;

        for(j = 3; argv[i][j]; ++j)

            if(argv[i][j] == tolower(tst_chr) || argv[i][j] == toupper(tst_chr))

                return TRUE;
    }

    return FALSE;
}

// Find a given string s2 in s1

int find_string(const char *s1, const char s2[])
{   const char  *p1 = s1, *q1, *q2;

    while(*p1)
    {
        q1 = p1; q2 = s2;

        while(*q1 && *q2 && *q1 == *q2)
        {
            q1++; q2++;
        }

        if(!*q2)

            return (int)(p1 - s1);

        p1++;
    }

    return -1;
}

// Find a line with a given header and return line type
// or -1 if end of file

enum line_type find_line(FILE *inf, char str[])
{   int i;

    while(get_line(inf, str, 128) == EXIT_SUCCESS)   // input a line
    {
        for(i = 1; i < NO_LTYPES; ++i)  // compare line header
        {
            if(find_string(str, fstr[i]) >= 0)
            {
                return (enum line_type)i;
            }
        }
    }

    return bad_line;            // end of file
}

// Output an array of bytes to a string of hexadecimal digits using
// the conventions described for input above.  But if type (ty) = 1
// it outputs a test number in decimal notation

void block_out(const enum line_type ty, const unsigned char b[], FILE *outf, const unsigned long len)
{   unsigned int i;

    fprintf(outf, "\n%s", fstr[ty]);

    if(ty <= test_no)
        fprintf(outf, "%lu", len);
    else
        for(i = 0; i < len; ++i)
            fprintf(outf, "%c%c", hxx[(b[i] >> 4) & 15], hxx[b[i] & 15]);
}

#if defined( DLL_IMPORT ) && defined( DLL_DYNAMIC_LOAD )

HINSTANCE init_dll(fn_ptrs *fn)
{   HINSTANCE   h_dll;
    int ok = 1;

#ifdef FIND_DLL
   char* buf;

   if((buf = _getcwd( NULL, 0 )))
   {
      printf("\n%s", buf);
      printf("\n%s", dll_path);
      free(buf);
   }
#endif

   if(!(h_dll = LoadLibrary(dll_path)))
    {
        DWORD dw = GetLastError();
        printf("\nLoadLibrary failed with error %d\n",  dw);
        ExitProcess(dw);
    }

    fn->fn_enc_key128 = (g_enc_key*)GetProcAddress(h_dll, ek_name128);
    fn->fn_enc_key192 = (g_enc_key*)GetProcAddress(h_dll, ek_name192);
    fn->fn_enc_key256 = (g_enc_key*)GetProcAddress(h_dll, ek_name256);
    fn->fn_enc_key = (g_enc_keyv*)GetProcAddress(h_dll, ek_name);
    fn->fn_enc_blk = (g_enc_blk*)GetProcAddress(h_dll, eb_name);
    fn->fn_dec_key128 = (g_dec_key*)GetProcAddress(h_dll, dk_name128);
    fn->fn_dec_key192 = (g_dec_key*)GetProcAddress(h_dll, dk_name192);
    fn->fn_dec_key256 = (g_dec_key*)GetProcAddress(h_dll, dk_name256);
    fn->fn_dec_key = (g_dec_keyv*)GetProcAddress(h_dll, dk_name);
    fn->fn_dec_blk = (g_dec_blk*)GetProcAddress(h_dll, db_name);

#if defined( AES_128 )
    if(!fn->fn_enc_key128 || !fn->fn_dec_key128)
        ok = 0;
#endif

#if defined( AES_192 )
    if(!fn->fn_enc_key192 || !fn->fn_dec_key192)
        ok = 0;
#endif

#if defined( AES_256 )
    if(!fn->fn_enc_key128 || !fn->fn_dec_key128)
        ok = 0;
#endif

#if defined( AES_VAR )
    if(!fn->fn_enc_key || !fn->fn_dec_key)
        ok = 0;
#endif

    if(!fn->fn_enc_blk || !fn->fn_dec_blk)
        ok = 0;

#ifdef  AES_MODES

    fn->fn_test_align = (g_talign*)GetProcAddress(h_dll, etad_name);
    fn->fn_mode_reset = (g_reset*)GetProcAddress(h_dll, eres_name);
    fn->fn_ecb_enc = (g_enc1*)GetProcAddress(h_dll, ecbe_name);
    fn->fn_ecb_dec = (g_dec1*)GetProcAddress(h_dll, ecbd_name);
    fn->fn_cbc_enc = (g_enc2*)GetProcAddress(h_dll, cbce_name);
    fn->fn_cbc_dec = (g_dec2*)GetProcAddress(h_dll, cbcd_name);
    fn->fn_cfb_enc = (g_enc3*)GetProcAddress(h_dll, cfbe_name);
    fn->fn_cfb_dec = (g_enc3*)GetProcAddress(h_dll, cfbd_name);
    fn->fn_ofb_cry = (g_enc3*)GetProcAddress(h_dll, ofb_name);
    fn->fn_ctr_cry = (g_enc4*)GetProcAddress(h_dll, ctr_name);

    if(   !fn->fn_mode_reset || !fn->fn_test_align
       || !fn->fn_ecb_enc || !fn->fn_ecb_dec
       || !fn->fn_cbc_enc || !fn->fn_cbc_dec
       || !fn->fn_cfb_enc || !fn->fn_cfb_dec
       || !fn->fn_ofb_cry || !fn->fn_ctr_cry )
       ok = 0;
#endif

    if(GetProcAddress(h_dll, gt_name))
        GetProcAddress(h_dll, gt_name)();
    else
        ok = 0;

    if(!ok)
    {
        printf("\n\nRequired DLL Entry Point(s) not found\n\n");
        FreeLibrary(h_dll);
        return 0;
    }

    return h_dll;
}

#endif

#if defined(__cplusplus)
}
#endif
