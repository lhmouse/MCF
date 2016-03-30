// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "condition_variable.h"
#include "_nt_timeout.h"
#include "../ext/assert.h"
#include "../ext/expect.h"
#include <winternl.h>
#include <ntstatus.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

static inline void atomic_increment_relaxed(volatile uintptr_t *p){
	__atomic_fetch_add(p, 1, __ATOMIC_RELAXED);
}
static inline uintptr_t atomic_saturated_sub_relaxed(volatile uintptr_t *p, uintptr_t max){
	uintptr_t delta, old, new;
	old = __atomic_load_n(p, __ATOMIC_RELAXED);
	do {
		if(old < max){
			delta = old;
		} else {
			delta = max;
		}
		if(_MCFCRT_EXPECT_NOT(delta == 0)){
			break;
		}
		new = old - delta;
	} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(p, &old, new, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
	return delta;
}
static inline uintptr_t atomic_exchange_relaxed(volatile uintptr_t *p, uintptr_t u){
	return __atomic_exchange_n(p, u, __ATOMIC_RELAXED);
}

bool _MCFCRT_WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext, uint64_t u64UntilFastMonoClock)
{
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);
	if(_MCFCRT_EXPECT(u64UntilFastMonoClock == 0)){
		(*pfnRelockCallback)(nContext, nLocked);
		return false;
	}
	atomic_increment_relaxed(pConditionVariable);
	LARGE_INTEGER liTimeout;
	__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, &liTimeout);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
	if(_MCFCRT_EXPECT(lStatus == STATUS_TIMEOUT)){
		const size_t uCountDecreased = atomic_saturated_sub_relaxed(pConditionVariable, 1);
		if(uCountDecreased != 0){
			(*pfnRelockCallback)(nContext, nLocked);
			return false;
		}
		lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
	}
	(*pfnRelockCallback)(nContext, nLocked);
	return true;
}
void _MCFCRT_WaitForConditionVariableForever(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext)
{
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);
	atomic_increment_relaxed(pConditionVariable);
	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
	(*pfnRelockCallback)(nContext, nLocked);
}
size_t _MCFCRT_SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToSignal){
	const size_t uCountSignaled = atomic_saturated_sub_relaxed(pConditionVariable, uMaxCountToSignal);
	for(size_t i = 0; i < uCountSignaled; ++i){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
	}
	return uCountSignaled;
}
size_t _MCFCRT_BroadcastConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable){
	const size_t uCountSignaled = atomic_exchange_relaxed(pConditionVariable, 0);
	for(size_t i = 0; i < uCountSignaled; ++i){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
	}
	return uCountSignaled;
}
