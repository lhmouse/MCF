// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CRT_PROCESS_H_
#define __MCFCRT_ENV_CRT_PROCESS_H_

#include "_crtdef.h"
#include "mutex.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_ProcessInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_ProcessUninit(void) _MCFCRT_NOEXCEPT;

typedef struct __MCFCRT_tagProcessEnvironmentBlock {
	volatile _MCFCRT_STD size_t __uRefCount;
} __MCFCRT_ProcessEnvironmentBlock;

__attribute__((__returns_nonnull__))
extern __MCFCRT_ProcessEnvironmentBlock *__MCFCRT_LockProcessEnvironmentBlock(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_UnlockProcessEnvironmentBlock(__MCFCRT_ProcessEnvironmentBlock *__pCrtPeb) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
