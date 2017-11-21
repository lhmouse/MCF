// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_ENV_ENDIAN_H_
#define MCFBUILD_ENV_ENDIAN_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

#define MCFBUILD_ENDIAN_DEFINE_(name_ld_, name_st_, type_, op_)	\
	static inline type_ name_ld_(const type_ *p) MCFBUILD_NOEXCEPT {	\
		return op_(*p);	\
	}	\
	static inline void name_st_(type_ *p, type_ n) MCFBUILD_NOEXCEPT {	\
		*p = op_(n);	\
	}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint16, MCFBUILD_store_be_uint16, MCFBUILD_STD uint16_t, __builtin_bswap16)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint32, MCFBUILD_store_be_uint32, MCFBUILD_STD uint32_t, __builtin_bswap32)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint64, MCFBUILD_store_be_uint64, MCFBUILD_STD uint64_t, __builtin_bswap64)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint16, MCFBUILD_store_le_uint16, MCFBUILD_STD uint16_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint32, MCFBUILD_store_le_uint32, MCFBUILD_STD uint32_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint64, MCFBUILD_store_le_uint64, MCFBUILD_STD uint64_t,                  )
#else
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint16, MCFBUILD_store_be_uint16, MCFBUILD_STD uint16_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint32, MCFBUILD_store_be_uint32, MCFBUILD_STD uint32_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint64, MCFBUILD_store_be_uint64, MCFBUILD_STD uint64_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint16, MCFBUILD_store_le_uint16, MCFBUILD_STD uint16_t, __builtin_bswap16)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint32, MCFBUILD_store_le_uint32, MCFBUILD_STD uint32_t, __builtin_bswap32)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint64, MCFBUILD_store_le_uint64, MCFBUILD_STD uint64_t, __builtin_bswap64)
#endif

MCFBUILD_EXTERN_C_END

#endif
