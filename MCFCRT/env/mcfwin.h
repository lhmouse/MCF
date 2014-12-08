// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_MCFWIN_H_
#define MCF_CRT_MCFWIN_H_

#define __MCF_C_CALLBACK_DECL	__attribute__((__nothrow__, __force_align_arg_pointer__))

#define __MCF_C_CDECL			__attribute__((__cdecl__)) __MCF_C_CALLBACK_DECL
#define __MCF_C_STDCALL			__attribute__((__stdcall__)) __MCF_C_CALLBACK_DECL
#define __MCF_C_FASTCALL		__attribute__((__fastcall__)) __MCF_C_CALLBACK_DECL

#undef WINVER
#undef _WIN32_WINNT
#undef WIN32_LEAN_AND_MEAN

#define WINVER					0x0601
#define _WIN32_WINNT			0x0601
#define WIN32_LEAN_AND_MEAN		1

#undef CDECL
#undef CALLBACK
#undef WINAPI
#undef WINAPIV
#undef APIENTRY

#define CDECL					__MCF_C_CDECL
#define CALLBACK				__MCF_C_STDCALL
#define WINAPI					__MCF_C_STDCALL
#define WINAPIV					__MCF_C_CDECL
#define APIENTRY				__MCF_C_STDCALL

#include <windows.h>

#endif
