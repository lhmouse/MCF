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

bool _MCFCRT_WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext, uint64_t u64UntilFastMonoClock)
{
	const intptr_t nLocked = (*pfnUnlockCallback)(nContext);
	if(_MCFCRT_EXPECT(u64UntilFastMonoClock == 0)){
		(*pfnRelockCallback)(nContext, nLocked);
		return false;
	}
	__atomic_fetch_add(pConditionVariable, 1, __ATOMIC_RELAXED);
	LARGE_INTEGER liTimeout;
	__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, &liTimeout);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
	if(lStatus == STATUS_TIMEOUT){
		size_t uCountDecreased;
		{
			uintptr_t uOld, uNew;
			uOld = __atomic_load_n(pConditionVariable, __ATOMIC_RELAXED);
			do {
				if(uOld == 0){
					uCountDecreased = 0;
				} else {
					uCountDecreased = 1;
				}
				if(uCountDecreased == 0){
					break;
				}
				uNew = uOld - uCountDecreased;
			} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(pConditionVariable, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
		}
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
	__atomic_fetch_add(pConditionVariable, 1, __ATOMIC_RELAXED);
	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
	(*pfnRelockCallback)(nContext, nLocked);
}
size_t _MCFCRT_SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToSignal){
	size_t uCountSignaled;
	{
		uintptr_t uOld, uNew;
		uOld = __atomic_load_n(pConditionVariable, __ATOMIC_RELAXED);
		do {
			if(uOld < uMaxCountToSignal){
				uCountSignaled = uOld;
			} else {
				uCountSignaled = uMaxCountToSignal;
			}
			if(uCountSignaled == 0){
				break;
			}
			uNew = uOld - uCountSignaled;
		} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(pConditionVariable, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
	}
	for(size_t i = 0; _MCFCRT_EXPECT(i < uCountSignaled); ++i){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
	}
	return uCountSignaled;
}
size_t _MCFCRT_BroadcastConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable){
	size_t uCountSignaled;
	{
		uCountSignaled = __atomic_exchange_n(pConditionVariable, 0, __ATOMIC_RELAXED);
	}
	for(size_t i = 0; _MCFCRT_EXPECT(i < uCountSignaled); ++i){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pConditionVariable, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
	}
	return uCountSignaled;
}
