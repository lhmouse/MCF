// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_STRINGASM_H__
#define __MCF_CRT_STRINGASM_H__

#include "../../env/_crtdef.h"

#if defined(__amd64__)

#	define	__AX__		"rax"
#	define	__CX__		"rcx"
#	define	__DX__		"rdx"
#	define	__BX__		"rbx"
#	define	__BP__		"rbp"
#	define	__SI__		"rsi"
#	define	__DI__		"rdi"

#elif defined(__i386__)

#	define	__AX__		"eax"
#	define	__CX__		"ecx"
#	define	__DX__		"edx"
#	define	__BX__		"ebx"
#	define	__BP__		"ebp"
#	define	__SI__		"esi"
#	define	__DI__		"edi"

#else

#	error Oops, this platform is not supported.

#endif

#endif
