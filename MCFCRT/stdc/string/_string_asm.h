// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_STRING_ASM_H_
#define MCF_CRT_STRING_ASM_H_

#include "../../env/_crtdef.h"

#if defined(__x86_64__)

#	define RAX		"rax"
#	define RCX		"rcx"
#	define RDX		"rdx"
#	define RBX		"rbx"
#	define RBP		"rbp"
#	define RSI		"rsi"
#	define RDI		"rdi"

#elif defined(__i386__)

#	define RAX		"eax"
#	define RCX		"ecx"
#	define RDX		"edx"
#	define RBX		"ebx"
#	define RBP		"ebp"
#	define RSI		"esi"
#	define RDI		"edi"

#else
#	error Oops, this platform is not supported.
#endif

#endif
