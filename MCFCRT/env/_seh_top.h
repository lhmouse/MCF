// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_SEH_TOP_H_
#define __MCFCRT_ENV_SEH_TOP_H_

#include "_crtdef.h"
#include "mcfwin.h"

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_C_CDECL
extern EXCEPTION_DISPOSITION __MCFCRT_SehTopDispatcher(EXCEPTION_RECORD *__pRecord, void *__pEstablisherFrame, CONTEXT *__pContext, void *__pDispatcherContext) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#ifdef _WIN64

#	define __MCFCRT_SEH_TOP_BEGIN       { __asm__ volatile (	\
	                                        ".seh_handler __MCFCRT_SehTopDispatcher, @except \n");	\
	                                      {
#	define __MCFCRT_SEH_TOP_END           }	\
	                                    }

#else

#	define __MCFCRT_SEH_TOP_BEGIN       { void *__seh_unused;	\
	                                      void *__seh_node[2];	\
	                                      __asm__ volatile (	\
	                                        "mov %0, dword ptr fs:[0] \n"	\
	                                        "mov dword ptr[%1], %0 \n"	\
	                                        "mov dword ptr[%1 + 4], offset ___MCFCRT_SehTopDispatcher \n"	\
	                                        "mov dword ptr fs:[0], %1 \n"	\
	                                        : "=&r"(__seh_unused) : "r"(__seh_node));	\
	                                      {
#	define __MCFCRT_SEH_TOP_END           }	\
	                                      __asm__ volatile (	\
	                                        "mov %0, dword ptr[%1] \n"	\
	                                        "mov dword ptr fs:[0], %0 \n"	\
	                                        : "=&r"(__seh_unused) : "r"(__seh_node));	\
	                                    }

#endif

#endif
