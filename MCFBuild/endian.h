// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_ENV_ENDIAN_H_
#define MCFBUILD_ENV_ENDIAN_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

#define MCFBUILD_ENDIAN_DEFINE_(name_ld_, name_st_, name_mv_, type_, op_)	\
	__attribute__((__always_inline__, __artificial__))	\
	static inline type_ name_ld_(const type_ *r) MCFBUILD_NOEXCEPT {	\
		return op_(*r);	\
	}	\
	__attribute__((__always_inline__, __artificial__))	\
	static inline void name_st_(type_ *w, type_ n) MCFBUILD_NOEXCEPT {	\
		*w = op_(n);	\
	}	\
	__attribute__((__always_inline__, __artificial__))	\
	static inline void name_mv_(type_ *w, const type_ *r) MCFBUILD_NOEXCEPT {	\
		*w = op_(*r);	\
	}

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint16, MCFBUILD_store_be_uint16, MCFBUILD_move_be_uint16, MCFBUILD_STD uint16_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint32, MCFBUILD_store_be_uint32, MCFBUILD_move_be_uint32, MCFBUILD_STD uint32_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint64, MCFBUILD_store_be_uint64, MCFBUILD_move_be_uint64, MCFBUILD_STD uint64_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint16, MCFBUILD_store_le_uint16, MCFBUILD_move_le_uint16, MCFBUILD_STD uint16_t, __builtin_bswap16)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint32, MCFBUILD_store_le_uint32, MCFBUILD_move_le_uint32, MCFBUILD_STD uint32_t, __builtin_bswap32)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint64, MCFBUILD_store_le_uint64, MCFBUILD_move_le_uint64, MCFBUILD_STD uint64_t, __builtin_bswap64)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint16, MCFBUILD_store_be_uint16, MCFBUILD_move_be_uint16, MCFBUILD_STD uint16_t, __builtin_bswap16)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint32, MCFBUILD_store_be_uint32, MCFBUILD_move_be_uint32, MCFBUILD_STD uint32_t, __builtin_bswap32)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_be_uint64, MCFBUILD_store_be_uint64, MCFBUILD_move_be_uint64, MCFBUILD_STD uint64_t, __builtin_bswap64)
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint16, MCFBUILD_store_le_uint16, MCFBUILD_move_le_uint16, MCFBUILD_STD uint16_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint32, MCFBUILD_store_le_uint32, MCFBUILD_move_le_uint32, MCFBUILD_STD uint32_t,                  )
MCFBUILD_ENDIAN_DEFINE_(MCFBUILD_load_le_uint64, MCFBUILD_store_le_uint64, MCFBUILD_move_le_uint64, MCFBUILD_STD uint64_t,                  )
#else
#  error This architecture is not supported.
#endif

MCFBUILD_EXTERN_C_END

#endif
