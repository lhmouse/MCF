// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CRTDEF_H_
#define __MCFCRT_ENV_CRTDEF_H_

#if !defined(__amd64__) && !defined(__i386__)
#	error This processor architecture is not supported.
#endif

#if defined(__cplusplus) && (__cplusplus < 201402l)
#	error This compiler does not support C++14.
#endif

#ifdef __cplusplus
#	define restrict                     __restrict__
#	include <cstddef>
#	include <cstdint>
#	include <climits>
#	include <cstring>
#	include <cassert>
#	define _MCFCRT_EXTERN_C_BEGIN       extern "C" {
#	define _MCFCRT_EXTERN_C_END         }
#	define _MCFCRT_STD                  ::std::
#	define _MCFCRT_NOEXCEPT             noexcept
#	define _MCFCRT_CONSTEXPR            constexpr
#else
#	define nullptr                      ((void *)0)
#	include <stdbool.h>                 // bool, true, false
#	include <stdalign.h>                // alignof, alignas
#	include <uchar.h>                   // char16_t, char32_t
#	include <stddef.h>
#	include <stdint.h>
#	include <limits.h>
#	include <string.h>
#	include <assert.h>
#	define _MCFCRT_EXTERN_C_BEGIN       //
#	define _MCFCRT_EXTERN_C_END         //
#	define _MCFCRT_STD                  //
#	define _MCFCRT_NOEXCEPT             //
#	define _MCFCRT_CONSTEXPR            static inline
#endif

#define __MCFCRT_C_CALLBACK_DECL        __attribute__((__nothrow__, __force_align_arg_pointer__, __aligned__(16)))

#define __MCFCRT_C_CDECL                __attribute__((__cdecl__))    __MCFCRT_C_CALLBACK_DECL
#define __MCFCRT_C_STDCALL              __attribute__((__stdcall__))  __MCFCRT_C_CALLBACK_DECL
#define __MCFCRT_C_FASTCALL             __attribute__((__fastcall__)) __MCFCRT_C_CALLBACK_DECL

#endif
