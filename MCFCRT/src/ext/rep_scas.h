// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_REP_SCAS_H_
#define __MCFCRT_EXT_REP_SCAS_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_REP_SCAS_INLINE_OR_EXTERN
#  define __MCFCRT_REP_SCAS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

#define __MCFCRT_REP_SCAS_DEFINE(__name_, __mnemonic_, __type_)	\
	__attribute__((__artificial__))	\
	__MCFCRT_REP_SCAS_INLINE_OR_EXTERN int __name_(__type_ **_MCFCRT_RESTRICT __es, const __type_ *__s, __type_ __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {	\
		bool __z, __b;	\
		const __type_ *__p = __s;	\
		__asm__ (	\
			__mnemonic_ " \n"	\
			: "+D"(__p), "=@ccz"(__z), "=@ccb"(__b)	\
			: "o"(*(const char (*)[])__p), "a"(__c), "c"(__n)	\
		);	\
		if(__es){	\
			*__es = (__type_ *)(__p + __z - 1);	\
		}	\
		return __z ? 0 : (__b ? -1 : 1);	\
	}

__MCFCRT_REP_SCAS_DEFINE( _MCFCRT_repz_scasb,  "repz scasb", _MCFCRT_STD  uint8_t)
__MCFCRT_REP_SCAS_DEFINE( _MCFCRT_repz_scasw,  "repz scasw", _MCFCRT_STD uint16_t)
__MCFCRT_REP_SCAS_DEFINE( _MCFCRT_repz_scasd,  "repz scasd", _MCFCRT_STD uint32_t)
#ifdef _WIN64
__MCFCRT_REP_SCAS_DEFINE( _MCFCRT_repz_scasq,  "repz scasq", _MCFCRT_STD uint64_t)
#endif

__MCFCRT_REP_SCAS_DEFINE(_MCFCRT_repnz_scasb, "repnz scasb", _MCFCRT_STD  uint8_t)
__MCFCRT_REP_SCAS_DEFINE(_MCFCRT_repnz_scasw, "repnz scasw", _MCFCRT_STD uint16_t)
__MCFCRT_REP_SCAS_DEFINE(_MCFCRT_repnz_scasd, "repnz scasd", _MCFCRT_STD uint32_t)
#ifdef _WIN64
__MCFCRT_REP_SCAS_DEFINE(_MCFCRT_repnz_scasq, "repnz scasq", _MCFCRT_STD uint64_t)
#endif

_MCFCRT_EXTERN_C_END

#endif
