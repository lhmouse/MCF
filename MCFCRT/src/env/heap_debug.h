// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_HEAP_DEBUG_H_
#define __MCFCRT_ENV_HEAP_DEBUG_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_HeapDebugInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_HeapDebugUninit(void) _MCFCRT_NOEXCEPT;

// This function returns the number of bytes that should be passed to underlying heap allocation functions.
// This function returns `SIZE_MAX` if the size would overflow.
extern _MCFCRT_STD size_t __MCFCRT_HeapDebugCalculateSizeToAlloc(_MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
// After the underlying allocation succeeds, this function creates a record for that memory block which is used for validation should the memory block be freed.
// `*__ppBlock` is set to a pointer to the payload, which is at least `__uSize` bytes large.
// The `__uSize` parameter shall be equal to (or less than, if you like) the one passed to the corresponding `__MCFCRT_HeapDebugCalculateSizeToAlloc()`.
// This function will not fail. If `__pStorage` is a null pointer, the behavior is undefined.
extern void __MCFCRT_HeapDebugRegister(void **_MCFCRT_RESTRICT __ppBlock, _MCFCRT_STD size_t __uSize, void *__pStorage, const void *__pRetAddrOuter, const void *__pRetAddrInner) _MCFCRT_NOEXCEPT;
// This function checks and removes the record for a memory block.
// `*__puSize` and `*__ppStorage` are set to `__uSize` and `__pStorage` that were passed to the corresponding `__MCFCRT_HeapDebugRegister()`, respectively.
// The bytes in the underlying storage that follow the payload are zeroed before the function returns successfully.
// This function returns `false` if the memory block is corrupted, in which case the record is not removed. The memory block MUST NOT be freed thereafter.
// If `__pBlock` is a null pointer, the behavior is undefined.
extern bool __MCFCRT_HeapDebugValidateAndUnregister(_MCFCRT_STD size_t *_MCFCRT_RESTRICT __puSize, void **_MCFCRT_RESTRICT __ppStorage, void *__pBlock) _MCFCRT_NOEXCEPT;
// This function reverts the effects of the previous `__MCFCRT_HeapDebugValidateAndUnregister()`.
extern void __MCFCRT_HeapDebugUndoUnregister(void *__pStorage) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
