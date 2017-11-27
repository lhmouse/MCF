// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_ENDIAN_H_
#define __MCFCRT_ENV_ENDIAN_H_

#include "_crtdef.h"

#ifndef __MCFCRT_ENDIAN_INLINE_OR_EXTERN
#  define __MCFCRT_ENDIAN_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

#define __MCFCRT_ENDIAN_DEFINE(__name_ld_, __name_st_, __name_mv_, __type_, __op_)	\
	__attribute__((__artificial__))	\
	__MCFCRT_ENDIAN_INLINE_OR_EXTERN __type_ __name_ld_(const __type_ *__r) _MCFCRT_NOEXCEPT {	\
		return __op_(*__r);	\
	}	\
	__attribute__((__artificial__))	\
	__MCFCRT_ENDIAN_INLINE_OR_EXTERN void __name_st_(__type_ *__w, __type_ __n) _MCFCRT_NOEXCEPT {	\
		*__w = __op_(__n);	\
	}	\
	__attribute__((__artificial__))	\
	__MCFCRT_ENDIAN_INLINE_OR_EXTERN void __name_mv_(__type_ *__w, const __type_ *__r) _MCFCRT_NOEXCEPT {	\
		*__w = __op_(*__r);	\
	}

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_be_uint16, _MCFCRT_store_be_uint16, _MCFCRT_move_be_uint16, _MCFCRT_STD uint16_t,                  )
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_be_uint32, _MCFCRT_store_be_uint32, _MCFCRT_move_be_uint32, _MCFCRT_STD uint32_t,                  )
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_be_uint64, _MCFCRT_store_be_uint64, _MCFCRT_move_be_uint64, _MCFCRT_STD uint64_t,                  )
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_le_uint16, _MCFCRT_store_le_uint16, _MCFCRT_move_le_uint16, _MCFCRT_STD uint16_t, __builtin_bswap16)
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_le_uint32, _MCFCRT_store_le_uint32, _MCFCRT_move_le_uint32, _MCFCRT_STD uint32_t, __builtin_bswap32)
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_le_uint64, _MCFCRT_store_le_uint64, _MCFCRT_move_le_uint64, _MCFCRT_STD uint64_t, __builtin_bswap64)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_be_uint16, _MCFCRT_store_be_uint16, _MCFCRT_move_be_uint16, _MCFCRT_STD uint16_t, __builtin_bswap16)
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_be_uint32, _MCFCRT_store_be_uint32, _MCFCRT_move_be_uint32, _MCFCRT_STD uint32_t, __builtin_bswap32)
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_be_uint64, _MCFCRT_store_be_uint64, _MCFCRT_move_be_uint64, _MCFCRT_STD uint64_t, __builtin_bswap64)
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_le_uint16, _MCFCRT_store_le_uint16, _MCFCRT_move_le_uint16, _MCFCRT_STD uint16_t,                  )
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_le_uint32, _MCFCRT_store_le_uint32, _MCFCRT_move_le_uint32, _MCFCRT_STD uint32_t,                  )
__MCFCRT_ENDIAN_DEFINE(_MCFCRT_load_le_uint64, _MCFCRT_store_le_uint64, _MCFCRT_move_le_uint64, _MCFCRT_STD uint64_t,                  )
#else
#  error This architecture is not supported.
#endif

_MCFCRT_EXTERN_C_END

#endif
