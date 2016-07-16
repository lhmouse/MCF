// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_THREAD_ENV_H_
#define __MCFCRT_ENV_THREAD_ENV_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_ThreadEnvInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_ThreadEnvUninit(void) _MCFCRT_NOEXCEPT;

// mopthread = the mother of pthread
// 线程的参数将指向由 __pParams 和 __uSizeOfParams 定义的内存块的副本。
extern _MCFCRT_STD uintptr_t __MCFCRT_MopthreadCreate(void (*__pfnProc)(void *), const void *__pParams, _MCFCRT_STD size_t __uSizeOfParams) _MCFCRT_NOEXCEPT;
__attribute__((__noreturn__))
extern void __MCFCRT_MopthreadExit(void (*__pfnModifier)(void *, _MCFCRT_STD intptr_t), _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;
extern bool __MCFCRT_MopthreadJoin(_MCFCRT_STD uintptr_t __uTid, void *__pParams) _MCFCRT_NOEXCEPT;
extern bool __MCFCRT_MopthreadDetach(_MCFCRT_STD uintptr_t __uTid) _MCFCRT_NOEXCEPT;

extern void __MCFCRT_TlsCleanup(void) _MCFCRT_NOEXCEPT;

// 成功应当返回 0，否则参见 _MCFCRT_TlsRequire() 上面的注释。
typedef unsigned long (*_MCFCRT_TlsConstructor)(_MCFCRT_STD intptr_t __nContext, void *__pStorage);
typedef void (*_MCFCRT_TlsDestructor)(_MCFCRT_STD intptr_t __nContext, void *__pStorage);

typedef struct __MCFCRT_tagTlsKeyHandle {
	int __n;
} *_MCFCRT_TlsKeyHandle;

// 失败返回 nullptr。
extern _MCFCRT_TlsKeyHandle _MCFCRT_TlsAllocKey(_MCFCRT_STD size_t __uSize, _MCFCRT_TlsConstructor __pfnConstructor, _MCFCRT_TlsDestructor __pfnDestructor, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_TlsFreeKey(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD size_t _MCFCRT_TlsGetSize(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;
extern _MCFCRT_TlsConstructor _MCFCRT_TlsGetConstructor(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;
extern _MCFCRT_TlsDestructor _MCFCRT_TlsGetDestructor(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD intptr_t _MCFCRT_TlsGetContext(_MCFCRT_TlsKeyHandle __hTlsKey) _MCFCRT_NOEXCEPT;

// 如果 __hTlsKey 有效，该函数永远不失败。
// 如果未设定过任何值，*__ppStorage 返回一个空指针。
extern bool _MCFCRT_TlsGet(_MCFCRT_TlsKeyHandle __hTlsKey, void **restrict __ppStorage) _MCFCRT_NOEXCEPT;
// 如果该线程局部存储设定过值，*__ppStorage 返回指向它的指针。
// 否则，该线程局部存储的内存被分配并清零，然后 _MCFCRT_TlsAllocKey() 中指定的构造函数被调用用于初始化该存储。
// 若构造函数返回 0，*__ppStorage 返回指向它的指针；否则，已分配的内存被立即释放，然后该函数返回 false，GetLastError() 返回构造函数返回的值。
extern bool _MCFCRT_TlsRequire(_MCFCRT_TlsKeyHandle __hTlsKey, void **restrict __ppStorage) _MCFCRT_NOEXCEPT;

typedef void (*_MCFCRT_AtThreadExitCallback)(_MCFCRT_STD intptr_t __nContext);

extern bool _MCFCRT_AtThreadExit(_MCFCRT_AtThreadExitCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
