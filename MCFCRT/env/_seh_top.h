// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_SEH_TOP_H_
#define __MCFCRT_ENV_SEH_TOP_H_

#include "_crtdef.h"
#include "mcfwin.h"

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_C_STDCALL
extern long __MCFCRT_TopSehHandler(EXCEPTION_POINTERS *__pContext) _MCFCRT_NOEXCEPT
	__asm__("@__MCFCRT_TopSehHandler");

_MCFCRT_EXTERN_C_END

#define __MCFCRT_HAS_SEH_TOP         __attribute__((__section__(".text$__MCFCRT")))

#ifdef _WIN64
#	define __MCFCRT_SEH_TOP_BEGIN    __asm__ volatile (	\
	                                    "53933: \n"	\
	                                    "	.seh_handler __C_specific_handler, @except \n"	\
	                                    "	.seh_handlerdata \n"	\
	                                    "	.long 1 \n"	\
	                                    "	.rva 53933b, 53933f, @__MCFCRT_TopSehHandler, 53933f \n"	\
	                                    "	.section .text$__MCFCRT \n"	\
                                    );
#	define __MCFCRT_SEH_TOP_END      __asm__ volatile (	\
	                                    "	nop \n"	\
	                                    "	.balign 16 \n"	\
	                                    "53933: \n"	\
	                                    "	.balign 1 \n"	\
                                    );
#else
	

#endif

#endif
