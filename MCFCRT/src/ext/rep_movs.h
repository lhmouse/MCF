// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_REP_MOVS_H_
#define __MCFCRT_EXT_REP_MOVS_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_REP_MOVS_INLINE_OR_EXTERN
#  define __MCFCRT_REP_MOVS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

#define __MCFCRT_REP_MOVS_DEFINE(__name_, __mnemonic_, __type_)	\
	__attribute__((__artificial__)) __MCFCRT_REP_MOVS_INLINE_OR_EXTERN __type_ * __name_(__type_ **_MCFCRT_RESTRICT __es2, __type_ *__s1, const __type_ *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {	\
		__type_ *__p1 = __s1;	\
		const __type_ *__p2 = __s2;	\
		_MCFCRT_STD size_t __dumb;	\
		__asm__ (	\
			__mnemonic_ " \n"	\
			: "=o"(*(char (*)[])__p1), "+D"(__p1), "+S"(__p2), "=c"(__dumb)	\
			: "o"(*(const char (*)[])__p2), "c"(__n)	\
		);	\
		if(__es2){	\
			*__es2 = (__type_ *)__p2;	\
		}	\
		return __p1;	\
	}

__MCFCRT_REP_MOVS_DEFINE(_MCFCRT_rep_movsb, "rep movsb", _MCFCRT_STD  uint8_t)
__MCFCRT_REP_MOVS_DEFINE(_MCFCRT_rep_movsw, "rep movsw", _MCFCRT_STD uint16_t)
__MCFCRT_REP_MOVS_DEFINE(_MCFCRT_rep_movsd, "rep movsd", _MCFCRT_STD uint32_t)
#ifdef _WIN64
__MCFCRT_REP_MOVS_DEFINE(_MCFCRT_rep_movsq, "rep movsq", _MCFCRT_STD uint64_t)
#endif

_MCFCRT_EXTERN_C_END

#endif
