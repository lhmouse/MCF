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

static void AtomicAdd(volatile uintptr_t *puValue, uintptr_t uDelta){
	__atomic_add_fetch(puValue, uDelta, __ATOMIC_RELAXED);
}
static uintptr_t AtomicSaturatedSub(volatile uintptr_t *puValue, uintptr_t uMaxDelta){
	uintptr_t uOld, uNew, uDelta;
	uOld = __atomic_load_n(puValue, __ATOMIC_RELAXED);
	do {
		if(uOld < uMaxDelta){
			uNew = 0;
			uDelta = uOld;
		} else {
			uNew = uOld - uMaxDelta;
			uDelta = uMaxDelta;
		}
	} while(_MCFCRT_EXPECT(!__atomic_compare_exchange_n(puValue, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
	return uDelta;
}
static uintptr_t AtomicExchange(volatile uintptr_t *puValue, uintptr_t uNewValue){
	return __atomic_exchange_n(puValue, uNewValue, __ATOMIC_RELAXED);
}

static inline bool WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, const LARGE_INTEGER *pliTimeout){
	AtomicAdd(pConditionVariable, 1);

	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, pliTimeout);
	if(!NT_SUCCESS(lStatus)){
		_MCFCRT_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		const uintptr_t uDecrement = AtomicSaturatedSub(pConditionVariable, 1);
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
static inline void SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uCount){
	for(size_t i = 0; _MCFCRT_EXPECT(i < uCount); ++i){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			_MCFCRT_ASSERT_MSG(false, L"NtReleaseKeyedEvent() 失败。");
		}
	}
}

bool _MCFCRT_WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext, uint64_t u64UntilFastMonoClock)
{
	bool bSignaled = false;
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);
	{
		if(_MCFCRT_EXPECT_NOT(u64UntilFastMonoClock != 0)){
			LARGE_INTEGER liTimeout;
			__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
			if(WaitForConditionVariable(pConditionVariable, &liTimeout)){
				bSignaled = true;
			}
		}
	}
	(*pfnRelockCallback)(nContext, nLocked);
	return bSignaled;
}
void _MCFCRT_WaitForConditionVariableForever(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext)
{
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);
	{
		if(!WaitForConditionVariable(pConditionVariable, nullptr)){
			_MCFCRT_ASSERT(false);
		}
	}
	(*pfnRelockCallback)(nContext, nLocked);
}
size_t _MCFCRT_SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToWakeUp){
	const uintptr_t uCount = AtomicSaturatedSub(pConditionVariable, uMaxCountToWakeUp);
	SignalConditionVariable(pConditionVariable, uCount);
	return uCount;
}
size_t _MCFCRT_BroadcastConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable){
	const uintptr_t uCount = AtomicExchange(pConditionVariable, 0);
	SignalConditionVariable(pConditionVariable, uCount);
	return uCount;
}
