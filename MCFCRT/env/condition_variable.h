// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CONDITION_VARIABLE_H_
#define __MCFCRT_ENV_CONDITION_VARIABLE_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

// 初始化为 { 0 } 即可。
typedef struct __MCFCRT_tagConditionVariable {
	volatile _MCFCRT_STD uintptr_t __u;
} volatile _MCFCRT_ConditionVariable;

typedef _MCFCRT_STD intptr_t (*_MCFCRT_ConditionVariableUnlockCallback)(_MCFCRT_STD intptr_t __nContext);
typedef void (*_MCFCRT_ConditionVariableRelockCallback)(_MCFCRT_STD intptr_t __nContext, _MCFCRT_STD intptr_t __nUnlocked);

static inline void _MCFCRT_InitializeConditionVariable(_MCFCRT_ConditionVariable *__pConditionVariable) _MCFCRT_NOEXCEPT {
	__atomic_store_n(&(__pConditionVariable->__u), 0, __ATOMIC_RELEASE);
}

extern bool _MCFCRT_WaitForConditionVariable(_MCFCRT_ConditionVariable *__pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback __pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback __pfnRelockCallback, _MCFCRT_STD intptr_t __nContext, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_WaitForConditionVariableForever(_MCFCRT_ConditionVariable *__pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback __pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback __pfnRelockCallback, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_SignalConditionVariable(_MCFCRT_ConditionVariable *__pConditionVariable, _MCFCRT_STD size_t __uMaxCountToSignal) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_BroadcastConditionVariable(_MCFCRT_ConditionVariable *__pConditionVariable) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
