// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STRING_ASM_H_
#define __MCFCRT_STRING_ASM_H_

#include "../../env/_crtdef.h"

#ifdef _WIN64

#	define __MCFCRT_RAX     "rax"
#	define __MCFCRT_RCX     "rcx"
#	define __MCFCRT_RDX     "rdx"
#	define __MCFCRT_RBX     "rbx"
#	define __MCFCRT_RBP     "rbp"
#	define __MCFCRT_RSI     "rsi"
#	define __MCFCRT_RDI     "rdi"

#else

#	define __MCFCRT_RAX     "eax"
#	define __MCFCRT_RCX     "ecx"
#	define __MCFCRT_RDX     "edx"
#	define __MCFCRT_RBX     "ebx"
#	define __MCFCRT_RBP     "ebp"
#	define __MCFCRT_RSI     "esi"
#	define __MCFCRT_RDI     "edi"

#endif

#endif
