#include "mode_hdr.h"

void gf_mulx(void *x)
{
#if UNIT_BITS == 8

    uint8_t i = 16, t = ((uint8_t*)x)[15];
    while(--i)
        ((uint8_t*)x)[i] = (((uint8_t*)x)[i] << 1) | (((uint8_t*)x)[i - 1] & 0x80 ? 1 : 0);
    ((uint8_t*)x)[0] = (((uint8_t*)x)[0] << 1) ^ (t & 0x80 ? 0x87 : 0x00);

#elif PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN

#  if UNIT_BITS == 64

#   define GF_MASK  li_64(8000000000000000) 
#   define GF_XOR   li_64(0000000000000087) 
    uint64_t _tt = ((UPTR_CAST(x,64)[1] & GF_MASK) ? GF_XOR : 0);
    UPTR_CAST(x,64)[1] = (UPTR_CAST(x,64)[1] << 1) | (UPTR_CAST(x,64)[0] & GF_MASK ? 1 : 0);
    UPTR_CAST(x,64)[0] = (UPTR_CAST(x,64)[0] << 1) ^ _tt;

#  else /* UNIT_BITS == 32 */

#   define GF_MASK  li_32(80000000) 
#   define GF_XOR   li_32(00000087) 
    uint32_t _tt = ((UPTR_CAST(x,32)[3] & GF_MASK) ? GF_XOR : 0);;
    UPTR_CAST(x,32)[3] = (UPTR_CAST(x,32)[3] << 1) | (UPTR_CAST(x,32)[2] & GF_MASK ? 1 : 0);
    UPTR_CAST(x,32)[2] = (UPTR_CAST(x,32)[2] << 1) | (UPTR_CAST(x,32)[1] & GF_MASK ? 1 : 0);
    UPTR_CAST(x,32)[1] = (UPTR_CAST(x,32)[1] << 1) | (UPTR_CAST(x,32)[0] & GF_MASK ? 1 : 0);
    UPTR_CAST(x,32)[0] = (UPTR_CAST(x,32)[0] << 1) ^ _tt;

#  endif

#else /* PLATFORM_BYTE_ORDER == IS_BIG_ENDIAN */

#  if UNIT_BITS == 64

#   define MASK_01  li_64(0101010101010101)
#   define GF_MASK  li_64(0000000000000080) 
#   define GF_XOR   li_64(8700000000000000) 
    uint64_t _tt = ((UPTR_CAST(x,64)[1] & GF_MASK) ? GF_XOR : 0);
    UPTR_CAST(x,64)[1] =  ((UPTR_CAST(x,64)[1] << 1) & ~MASK_01) 
        | (((UPTR_CAST(x,64)[1] >> 15) | (UPTR_CAST(x,64)[0] << 49)) & MASK_01);
    UPTR_CAST(x,64)[0] = (((UPTR_CAST(x,64)[0] << 1) & ~MASK_01) 
        |  ((UPTR_CAST(x,64)[0] >> 15) & MASK_01)) ^ _tt;

#  else /* UNIT_BITS == 32 */

#   define MASK_01  li_32(01010101)
#   define GF_MASK  li_32(00000080) 
#   define GF_XOR   li_32(87000000) 
    uint32_t _tt = ((UPTR_CAST(x,32)[3] & GF_MASK) ? GF_XOR : 0);
    UPTR_CAST(x,32)[3] =  ((UPTR_CAST(x,32)[3] << 1) & ~MASK_01) 
        | (((UPTR_CAST(x,32)[3] >> 15) | (UPTR_CAST(x,32)[2] << 17)) & MASK_01);
    UPTR_CAST(x,32)[2] =  ((UPTR_CAST(x,32)[2] << 1) & ~MASK_01) 
        | (((UPTR_CAST(x,32)[2] >> 15) | (UPTR_CAST(x,32)[1] << 17)) & MASK_01);
    UPTR_CAST(x,32)[1] =  ((UPTR_CAST(x,32)[1] << 1) & ~MASK_01) 
        | (((UPTR_CAST(x,32)[1] >> 15) |   (UPTR_CAST(x,32)[0] << 17)) & MASK_01);
    UPTR_CAST(x,32)[0] = (((UPTR_CAST(x,32)[0] << 1) & ~MASK_01) 
        |  ((UPTR_CAST(x,32)[0] >> 15) & MASK_01)) ^ _tt;

#  endif

#endif
}
