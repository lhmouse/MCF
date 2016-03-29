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

static inline void AtomicAddRelaxed(volatile uintptr_t *puValue, uintptr_t uDelta){
	__atomic_add_fetch(puValue, uDelta, __ATOMIC_RELAXED);
}
static inline uintptr_t AtomicSaturatedSubRelaxed(volatile uintptr_t *puValue, uintptr_t uMaxDelta){
	uintptr_t uOld, uDelta, uNew;
	uOld = __atomic_load_n(puValue, __ATOMIC_RELAXED);
	do {
		if(uOld < uMaxDelta){
			uDelta = uOld;
		} else {
			uDelta = uMaxDelta;
		}
		uNew = uOld - uDelta;
	} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puValue, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
	return uDelta;
}

static inline bool WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, const LARGE_INTEGER *pliTimeout){
	AtomicAddRelaxed(pConditionVariable, 1);
	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, pliTimeout);
	if(!NT_SUCCESS(lStatus)){
		_MCFCRT_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		const uintptr_t uDecrement = AtomicSaturatedSubRelaxed(pConditionVariable, 1);
		if(uDecrement != 0){
			return false;
		}
		lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			_MCFCRT_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
		}
	}
	return true;
}
static inline size_t SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToSignal){
	const size_t uCountSignaled = AtomicSaturatedSubRelaxed(pConditionVariable, uMaxCountToSignal);
	for(size_t i = 0; _MCFCRT_EXPECT_NOT(i < uCountSignaled); ++i){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			_MCFCRT_ASSERT_MSG(false, L"NtReleaseKeyedEvent() 失败。");
		}
	}
	return uCountSignaled;
}

bool _MCFCRT_WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext, uint64_t u64UntilFastMonoClock)
{
	bool bSignaled;
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);
	{
		if(_MCFCRT_EXPECT(u64UntilFastMonoClock == 0)){
			bSignaled = false;
		} else {
			LARGE_INTEGER liTimeout;
			__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
			bSignaled = WaitForConditionVariable(pConditionVariable, &liTimeout);
		}
	}
	(*pfnRelockCallback)(nContext, nLocked);
	return bSignaled;
}
void _MCFCRT_WaitForConditionVariableForever(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext)
{
	bool bSignaled;
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);
	{
		bSignaled = WaitForConditionVariable(pConditionVariable, nullptr);
		_MCFCRT_ASSERT(bSignaled);
	}
	(*pfnRelockCallback)(nContext, nLocked);
}
size_t _MCFCRT_SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToWakeUp){
	return SignalConditionVariable(pConditionVariable, uMaxCountToWakeUp);
}
size_t _MCFCRT_BroadcastConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable){
	return SignalConditionVariable(pConditionVariable, SIZE_MAX);
}
