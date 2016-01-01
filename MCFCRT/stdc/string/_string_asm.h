// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_STRING_ASM_H_
#define __MCF_CRT_STRING_ASM_H_

#include "../../env/_crtdef.h"

#if defined(__x86_64__)

#	define __RAX    "rax"
#	define __RCX    "rcx"
#	define __RDX    "rdx"
#	define __RBX    "rbx"
#	define __RBP    "rbp"
#	define __RSI    "rsi"
#	define __RDI    "rdi"

#elif defined(__i386__)

#	define __RAX    "eax"
#	define __RCX    "ecx"
#	define __RDX    "edx"
#	define __RBX    "ebx"
#	define __RBP    "ebp"
#	define __RSI    "esi"
#	define __RDI    "edi"

#else

#	error Oops, this platform is not supported.

#endif

#endif
