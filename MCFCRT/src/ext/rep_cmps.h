// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_REP_CMPS_H_
#define __MCFCRT_EXT_REP_CMPS_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

#define __MCFCRT_REP_CMPS_DEFINE(__name_, __mnemonic_, __type_)	\
	int __name_(__type_ **_MCFCRT_RESTRICT __es1, __type_ **_MCFCRT_RESTRICT __es2, const __type_ *__s1, const __type_ *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {	\
		bool __z, __b;	\
		const __type_ *__p1 = __s1;	\
		const __type_ *__p2 = __s2;	\
		__asm__ (	\
			__mnemonic_ " \n"	\
			: "+S"(__p1), "+D"(__p2), "=@ccz"(__z), "=@ccb"(__b)	\
			: "o"(*(const char (*)[])__p1), "o"(*(const char (*)[])__p2), "c"(__n)	\
		);	\
		if(__es1){	\
			*__es1 = (__type_ *)(__p1 + __z - 1);	\
		}	\
		if(__es2){	\
			*__es2 = (__type_ *)(__p2 + __z - 1);	\
		}	\
		return __z ? 0 : (__b ? -1 : 1);	\
	}

static inline __MCFCRT_REP_CMPS_DEFINE( _MCFCRT_repz_cmpsb,  "repz cmpsb", _MCFCRT_STD  uint8_t)
static inline __MCFCRT_REP_CMPS_DEFINE( _MCFCRT_repz_cmpsw,  "repz cmpsw", _MCFCRT_STD uint16_t)
static inline __MCFCRT_REP_CMPS_DEFINE( _MCFCRT_repz_cmpsd,  "repz cmpsd", _MCFCRT_STD uint32_t)
#ifdef _WIN64
static inline __MCFCRT_REP_CMPS_DEFINE( _MCFCRT_repz_cmpsq,  "repz cmpsq", _MCFCRT_STD uint64_t)
#endif

static inline __MCFCRT_REP_CMPS_DEFINE(_MCFCRT_repnz_cmpsb, "repnz cmpsb", _MCFCRT_STD  uint8_t)
static inline __MCFCRT_REP_CMPS_DEFINE(_MCFCRT_repnz_cmpsw, "repnz cmpsw", _MCFCRT_STD uint16_t)
static inline __MCFCRT_REP_CMPS_DEFINE(_MCFCRT_repnz_cmpsd, "repnz cmpsd", _MCFCRT_STD uint32_t)
#ifdef _WIN64
static inline __MCFCRT_REP_CMPS_DEFINE(_MCFCRT_repnz_cmpsq, "repnz cmpsq", _MCFCRT_STD uint64_t)
#endif

_MCFCRT_EXTERN_C_END

#endif
