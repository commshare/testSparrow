#ifndef __SC_EDIAN_H_
#define __SC_EDIAN_H_


#define SC_LIL_ENDIAN  1234
#define SC_BIG_ENDIAN  4321

#ifndef SC_BYTEORDER           /* Not defined in SDL_config.h? */
#ifdef __linux__
#include <endian.h>
#define SC_BYTEORDER  __BYTE_ORDER
#else /* __linux __ */
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MISPEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__sparc__)
#define SC_BYTEORDER   SDL_BIG_ENDIAN
#else
#define SC_BYTEORDER   SDL_LIL_ENDIAN
#endif
#endif /* __linux __ */
#endif /* !SDL_BYTEORDER */
#endif
