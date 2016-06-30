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

#if defined(__cplusplus) && __cplusplus >= 201402l
#	define _MCFCRT_CXX14                1
#endif

#if defined(__cplusplus) && __cplusplus >= 201103l
#	define _MCFCRT_CXX11                1
#endif

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <stdalign.h>
#include <assert.h>

#ifdef __cplusplus
#	define _MCFCRT_EXTERN_C_BEGIN       extern "C" {
#	define _MCFCRT_EXTERN_C_END         }
#	define _MCFCRT_STD                  ::
#else
#	define _MCFCRT_EXTERN_C_BEGIN
#	define _MCFCRT_EXTERN_C_END
#	define _MCFCRT_STD                  //
#endif

#define restrict                        __restrict__

#ifndef _MCFCRT_CXX11
#	define nullptr                      0
#endif

#define __MCFCRT_C_CALLBACK_DECL        __attribute__((__nothrow__, __force_align_arg_pointer__, __aligned__(16)))
#define __MCFCRT_C_CDECL                __attribute__((__cdecl__))    __MCFCRT_C_CALLBACK_DECL
#define __MCFCRT_C_STDCALL              __attribute__((__stdcall__))  __MCFCRT_C_CALLBACK_DECL
#define __MCFCRT_C_FASTCALL             __attribute__((__fastcall__)) __MCFCRT_C_CALLBACK_DECL

#if defined(_MCFCRT_CXX11)
#	define _MCFCRT_NOEXCEPT             noexcept
#	define _MCFCRT_CONSTEXPR            constexpr
#elif defined(__cplusplus)
#	define _MCFCRT_NOEXCEPT             throw()
#	define _MCFCRT_CONSTEXPR            static inline
#else
#	define _MCFCRT_NOEXCEPT             //
#	define _MCFCRT_CONSTEXPR            static inline
#endif

#endif
