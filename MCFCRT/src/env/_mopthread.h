// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_MOPTHREAD_H_
#define __MCFCRT_ENV_MOPTHREAD_H_

#include "_crtdef.h"
#include "thread.h"

_MCFCRT_EXTERN_C_BEGIN

// mopthread = the mother of pthread

// The parameter of the thread procedure will point to a copy of the memory block that __pParams and __uSize define.
extern _MCFCRT_STD uintptr_t __MCFCRT_MopthreadCreate(void (*__pfnProc)(void *), const void *__pParams, _MCFCRT_STD size_t __uSizeOfParams) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD uintptr_t __MCFCRT_MopthreadCreateDetached(void (*__pfnProc)(void *), const void *__pParams, _MCFCRT_STD size_t __uSizeOfParams) _MCFCRT_NOEXCEPT;
__attribute__((__noreturn__))
extern void __MCFCRT_MopthreadExit(void (*__pfnModifier)(void *, _MCFCRT_STD size_t, _MCFCRT_STD intptr_t), _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;
extern bool __MCFCRT_MopthreadJoin(_MCFCRT_STD uintptr_t __uTid, void *_MCFCRT_RESTRICT __pParams, _MCFCRT_STD size_t *_MCFCRT_RESTRICT __puSizeOfParams) _MCFCRT_NOEXCEPT;
extern bool __MCFCRT_MopthreadDetach(_MCFCRT_STD uintptr_t __uTid) _MCFCRT_NOEXCEPT;

// Returns a pointer to a HANDLE, which is a pseudo handle if __uTid refers the calling thread, or NULL on failure.
// The returned pointer must be unlocked.
extern const _MCFCRT_ThreadHandle *__MCFCRT_MopthreadLockHandle(_MCFCRT_STD uintptr_t __uTid) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_MopthreadUnlockHandle(const _MCFCRT_ThreadHandle *__phThread) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
