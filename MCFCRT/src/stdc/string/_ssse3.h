// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_STRING_SSSE3_H_
#define __MCFCRT_STDC_STRING_SSSE3_H_

#include "../../env/_crtdef.h"
#include <tmmintrin.h>

_MCFCRT_EXTERN_C_BEGIN

__attribute__((__always_inline__))
static inline void __MCFCRT_xmmalign_26(__m128i *_MCFCRT_RESTRICT __words, const __m128i *_MCFCRT_RESTRICT __view, unsigned __shift) _MCFCRT_NOEXCEPT {
	for(unsigned __i = 0; __i < 2; ++__i){
		switch(__shift % 16){
#define __MCFCRT_CASE(__k_)	\
		case (__k_):	\
			__words[__i] = _mm_alignr_epi8(__view[__shift / 16 + __i + 1], __view[__shift / 16 + __i], (__k_));	\
			break;
//=============================================================================
		__MCFCRT_CASE(0x00)
		__MCFCRT_CASE(0x01)
		__MCFCRT_CASE(0x02)
		__MCFCRT_CASE(0x03)
		__MCFCRT_CASE(0x04)
		__MCFCRT_CASE(0x05)
		__MCFCRT_CASE(0x06)
		__MCFCRT_CASE(0x07)
		__MCFCRT_CASE(0x08)
		__MCFCRT_CASE(0x09)
		__MCFCRT_CASE(0x0A)
		__MCFCRT_CASE(0x0B)
		__MCFCRT_CASE(0x0C)
		__MCFCRT_CASE(0x0D)
		__MCFCRT_CASE(0x0E)
		__MCFCRT_CASE(0x0F)
//=============================================================================
#undef __MCFCRT_CASE
		}
	}
}

_MCFCRT_EXTERN_C_END

#endif
