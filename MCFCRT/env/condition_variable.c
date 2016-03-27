// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "condition_variable.h"
#include "clocks.h"
#include "../ext/assert.h"
#include <winternl.h>
#include <ntstatus.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

static inline bool WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, const LARGE_INTEGER *pliTimeout){
	__atomic_add_fetch(pConditionVariable, 1, __ATOMIC_RELAXED);

	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, pliTimeout);
	if(!NT_SUCCESS(lStatus)){
		_MCFCRT_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		uintptr_t uOld, uNew;
		uOld = __atomic_load_n(pConditionVariable, __ATOMIC_RELAXED);
		do {
			if(uOld == 0){
				goto jDontMissIt;
			}
			uNew = uOld - 1;
		} while(!__atomic_compare_exchange_n(pConditionVariable, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
		return false;

	jDontMissIt:
		lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			_MCFCRT_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
		}
	}
	return true;
}
static inline void SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uCount){
	for(size_t i = 0; i < uCount; ++i){
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

	if(u64UntilFastMonoClock != 0){
		LARGE_INTEGER liTimeout;
		const uint64_t u64Now = _MCFCRT_GetFastMonoClock();
		if(u64Now >= u64UntilFastMonoClock){
			liTimeout.QuadPart = 0;
		} else {
			const uint64_t u64DeltaMillisec = u64UntilFastMonoClock - u64Now;
			const int64_t n64Delta100Nanosec = (int64_t)(u64DeltaMillisec * 10000);
			if((uint64_t)(n64Delta100Nanosec / 10000) != u64DeltaMillisec){
				liTimeout.QuadPart = INT64_MIN;
			} else {
				liTimeout.QuadPart = -n64Delta100Nanosec;
			}
		}
		if(WaitForConditionVariable(pConditionVariable, &liTimeout)){
			bSignaled = true;
		}
	}

	(*pfnRelockCallback)(nContext, nLocked);
	return bSignaled;
}
void _MCFCRT_WaitForConditionVariableForever(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext)
{
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);

	if(!WaitForConditionVariable(pConditionVariable, nullptr)){
		_MCFCRT_ASSERT(false);
	}

	(*pfnRelockCallback)(nContext, nLocked);
}
size_t _MCFCRT_SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToWakeUp){
	size_t uCount;

	uintptr_t uOld, uNew;
	uOld = __atomic_load_n(pConditionVariable, __ATOMIC_RELAXED);
	do {
		uCount = uOld;
		if(uCount > uMaxCountToWakeUp){
			uCount = uMaxCountToWakeUp;
		}
		uNew = uOld - uCount;
	} while(!__atomic_compare_exchange_n(pConditionVariable, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

	SignalConditionVariable(pConditionVariable, uCount);
	return uCount;
}
size_t _MCFCRT_BroadcastConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable){
	size_t uCount;

	uCount = __atomic_exchange_n(pConditionVariable, 0, __ATOMIC_RELAXED);

	SignalConditionVariable(pConditionVariable, uCount);
	return uCount;
}
