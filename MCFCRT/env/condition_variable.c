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

static inline void AtomicAddRelaxed(volatile uintptr_t *pValue, uintptr_t uAddend){
	__atomic_fetch_add(pValue, uAddend, __ATOMIC_RELAXED);
}
static inline uintptr_t AtomicSaturatedSubRelaxed(volatile uintptr_t *pValue, uintptr_t uSubtrahend){
	uintptr_t uOld, uNew;
	{
		uOld = __atomic_load_n(pValue, __ATOMIC_RELAXED);
		do {
			if(uOld <= uSubtrahend){
				uNew = 0;
			} else {
				uNew = uOld - uSubtrahend;
			}
			if(uNew == uOld){
				break;
			}
		} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(pValue, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
	}
	return uOld - uNew;
}

static inline bool RealWaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, bool bMayTimeOut, uint64_t u64UntilFastMonoClock){
	if(bMayTimeOut && _MCFCRT_EXPECT(u64UntilFastMonoClock == 0)){
		return false;
	}
	AtomicAddRelaxed(pConditionVariable, 1);
	LARGE_INTEGER liTimeout;
	__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, &liTimeout);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
	if(bMayTimeOut && _MCFCRT_EXPECT(lStatus == STATUS_TIMEOUT)){
		const size_t uCountDecreased = AtomicSaturatedSubRelaxed(pConditionVariable, 1);
		if(uCountDecreased != 0){
			return false;
		}
		lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
	}
	_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
	return true;
}
static inline size_t RealSignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToSignal){
	const size_t uCountSignaled = AtomicSaturatedSubRelaxed(pConditionVariable, uMaxCountToSignal);
	for(size_t i = 0; i < uCountSignaled; ++i){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
		_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
	}
	return uCountSignaled;
}

bool _MCFCRT_WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext, uint64_t u64UntilFastMonoClock)
{
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);
	const bool bSignaled = RealWaitForConditionVariable(pConditionVariable, true, u64UntilFastMonoClock);
	(*pfnRelockCallback)(nContext, nLocked);
	return bSignaled;
}
void _MCFCRT_WaitForConditionVariableForever(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext)
{
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);
	const bool bSignaled = RealWaitForConditionVariable(pConditionVariable, false, UINT64_MAX);
	(*pfnRelockCallback)(nContext, nLocked);
	_MCFCRT_ASSERT(bSignaled);
}
size_t _MCFCRT_SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToSignal){
	const size_t uCountSignaled = RealSignalConditionVariable(pConditionVariable, uMaxCountToSignal);
	return uCountSignaled;
}
size_t _MCFCRT_BroadcastConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable){
	const size_t uCountSignaled = RealSignalConditionVariable(pConditionVariable, SIZE_MAX);
	return uCountSignaled;
}
