// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_THREAD_H_
#define __MCFCRT_ENV_THREAD_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_ThreadEnvInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_ThreadEnvUninit(void) _MCFCRT_NOEXCEPT;

typedef unsigned long (__attribute__((__stdcall__)) *_MCFCRT_NativeThreadProc)(void *__pParam);

typedef struct __MCFCRT_tagThreadHandle {
	int __n;
} *_MCFCRT_ThreadHandle;

extern _MCFCRT_ThreadHandle _MCFCRT_CreateNativeThread(_MCFCRT_NativeThreadProc __pfnThreadProc, void *__pParam, bool __bSuspended, _MCFCRT_STD uintptr_t *_MCFCRT_RESTRICT __puThreadId) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_CloseThread(_MCFCRT_ThreadHandle __hThread) _MCFCRT_NOEXCEPT;

extern void _MCFCRT_Sleep(_MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
// _MCFCRT_AlertableSleep() returns true if the current thread has been woken up by APC and false if the current thread has timed out.
extern bool _MCFCRT_AlertableSleep(_MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_AlertableSleepForever(void) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_YieldThread(void) _MCFCRT_NOEXCEPT;

extern long _MCFCRT_SuspendThread(_MCFCRT_ThreadHandle __hThread) _MCFCRT_NOEXCEPT;
extern long _MCFCRT_ResumeThread(_MCFCRT_ThreadHandle __hThread) _MCFCRT_NOEXCEPT;

// _MCFCRT_WaitForThread() returns true if the other thread has terminated and false if the current thread has timed out.
extern bool _MCFCRT_WaitForThread(_MCFCRT_ThreadHandle __hThread, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_WaitForThreadForever(_MCFCRT_ThreadHandle __hThread) _MCFCRT_NOEXCEPT;

__attribute__((__const__))
extern _MCFCRT_STD uintptr_t _MCFCRT_GetCurrentThreadId(void) _MCFCRT_NOEXCEPT;

// mopthread = the mother of pthread
typedef enum __MCFCRT_tagMopthreadErrorCode {
	__MCFCRT_kMopthreadSuccess      = 0,
	__MCFCRT_kMopthreadSeeLastError = 1,
	__MCFCRT_kMopthreadNotFound     = 2,
} __MCFCRT_MopthreadErrorCode;

// The parameter of the thread procedure will point to a copy of the memory block that __pParams and __uSize define.
extern _MCFCRT_STD uintptr_t __MCFCRT_MopthreadCreate(void (*__pfnProc)(void *), const void *__pParams, _MCFCRT_STD size_t __uSizeOfParams) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD uintptr_t __MCFCRT_MopthreadCreateDetached(void (*__pfnProc)(void *), const void *__pParams, _MCFCRT_STD size_t __uSizeOfParams) _MCFCRT_NOEXCEPT;
__attribute__((__noreturn__))
extern void __MCFCRT_MopthreadExit(void (*__pfnModifier)(void *, _MCFCRT_STD intptr_t), _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;
extern __MCFCRT_MopthreadErrorCode __MCFCRT_MopthreadJoin(_MCFCRT_STD uintptr_t __uTid, void *__pParams) _MCFCRT_NOEXCEPT;
extern __MCFCRT_MopthreadErrorCode __MCFCRT_MopthreadDetach(_MCFCRT_STD uintptr_t __uTid) _MCFCRT_NOEXCEPT;

#define __MCFCRT_MOPTHREAD_PRIORITY_MIN  -2
#define __MCFCRT_MOPTHREAD_PRIORITY_MAX   2

extern __MCFCRT_MopthreadErrorCode __MCFCRT_MopthreadGetPriority(int *__pnPriority, _MCFCRT_STD uintptr_t __uTid) _MCFCRT_NOEXCEPT;
extern __MCFCRT_MopthreadErrorCode __MCFCRT_MopthreadSetPriority(_MCFCRT_STD uintptr_t __uTid, int __nPriority) _MCFCRT_NOEXCEPT;

extern void __MCFCRT_TlsCleanup(void) _MCFCRT_NOEXCEPT;

// The constructor shall return 0 upon success. Otherwise, see comments above _MCFCRT_TlsRequire().
typedef unsigned long (*_MCFCRT_TlsConstructor)(_MCFCRT_STD intptr_t __nContext, void *__pStorage);
typedef void (*_MCFCRT_TlsDestructor)(_MCFCRT_STD intptr_t __nContext, void *__pStorage);

typedef struct __MCFCRT_tagTlsKeyHandle {
	int __n;
} *_MCFCRT_TlsKeyHandle;

// _MCFCRT_TlsAllocKey() returns `_MCFCRT_NULLPTR` upon failure.
extern _MCFCRT_TlsKeyHandle _MCFCRT_TlsAllocKey(_MCFCRT_STD size_t __uSize, _MCFCRT_TlsConstructor __pfnConstructor, _MCFCRT_TlsDestructor __pfnDestructor, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_TlsFreeKey(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD size_t _MCFCRT_TlsGetSize(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;
extern _MCFCRT_TlsConstructor _MCFCRT_TlsGetConstructor(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;
extern _MCFCRT_TlsDestructor _MCFCRT_TlsGetDestructor(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD intptr_t _MCFCRT_TlsGetContext(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;

// _MCFCRT_TlsGet() never fails, provided __hTlsKey is a valid key.
// If the storage for the current thread has not been initialized, a null pointer is returned into *__ppStorage.
extern bool _MCFCRT_TlsGet(_MCFCRT_TlsKeyHandle __hTlsKey, void **_MCFCRT_RESTRICT __ppStorage) _MCFCRT_NOEXCEPT;
// _MCFCRT_TlsRequire() is identical to _MCFCRT_TlsGet() if the storage for the current thread has already been initialized.
// If it is not, the storage is allocated and zeroed, then the constructor is called, and if the constructor returns 0, a pointer to the initialized storage is returned into *__ppStorage.
// If memory allocation fails, `false` is returned when GetLastError() returns ERROR_NOT_ENOUGH_MEMORY.
// If the constructor returns a non-zero value, the storage is deallocated immediately and `false` is returned when GetLastError() returns that non-zero value.
extern bool _MCFCRT_TlsRequire(_MCFCRT_TlsKeyHandle __hTlsKey, void **_MCFCRT_RESTRICT __ppStorage) _MCFCRT_NOEXCEPT;

typedef void (*_MCFCRT_AtThreadExitCallback)(_MCFCRT_STD intptr_t __nContext);

extern bool _MCFCRT_AtThreadExit(_MCFCRT_AtThreadExitCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
