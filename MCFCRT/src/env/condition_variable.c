// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#define __MCFCRT_CONDITION_VARIABLE_INLINE_OR_EXTERN     extern inline
#include "condition_variable.h"
#include "_nt_timeout.h"
#include "../ext/assert.h"
#include "../ext/expect.h"
#include <winternl.h>
#include <ntstatus.h>

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

#define MASK_THREADS_TRAPPED    ((uintptr_t)~0x0000)

#define THREAD_TRAPPED_ONE      ((uintptr_t)(MASK_THREADS_TRAPPED & -MASK_THREADS_TRAPPED))
#define THREAD_TRAPPED_MAX      ((uintptr_t)(MASK_THREADS_TRAPPED / THREAD_TRAPPED_ONE))

__attribute__((__always_inline__))
static inline bool ReallyWaitForConditionVariable(volatile uintptr_t *puControl,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext,
	bool bMayTimeOut, uint64_t u64UntilFastMonoClock, bool bRelockIfTimeOut)
{
	{
//		uintptr_t uOld, uNew;
//		uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
//		do {
//			uNew = uOld + THREAD_TRAPPED_ONE;
//		} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
		__atomic_fetch_add(puControl, THREAD_TRAPPED_ONE, __ATOMIC_RELAXED);
	}
	const intptr_t nUnlocked = (*pfnUnlockCallback)(nContext);
	if(bMayTimeOut){
		LARGE_INTEGER liTimeout;
		__MCFCRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
		NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)puControl, false, &liTimeout);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
		while(_MCFCRT_EXPECT(lStatus == STATUS_TIMEOUT)){
			bool bDecremented;
			{
				uintptr_t uOld, uNew;
				uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
				do {
					const size_t uThreadsTrapped = (uOld & MASK_THREADS_TRAPPED) / THREAD_TRAPPED_ONE;
					bDecremented = (uThreadsTrapped > 0);
					if(!bDecremented){
						break;
					}
					uNew = uOld - THREAD_TRAPPED_ONE;
				} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
			}
			if(bDecremented){
				if(bRelockIfTimeOut){
					(*pfnRelockCallback)(nContext, nUnlocked);
				}
				return false;
			}
			liTimeout.QuadPart = 0;
			lStatus = NtWaitForKeyedEvent(nullptr, (void *)puControl, false, &liTimeout);
			_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
		}
	} else {
		NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)puControl, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
		_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
	}
	(*pfnRelockCallback)(nContext, nUnlocked);
	return true;
}
__attribute__((__always_inline__))
static inline size_t ReallySignalConditionVariable(volatile uintptr_t *puControl, size_t uMaxCountToSignal){
	uintptr_t uCountToSignal;
	{
		uintptr_t uOld, uNew;
		uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
		do {
			const size_t uThreadsTrapped = (uOld & MASK_THREADS_TRAPPED) / THREAD_TRAPPED_ONE;
			uCountToSignal = (uThreadsTrapped <= uMaxCountToSignal) ? uThreadsTrapped : uMaxCountToSignal;
			if(uCountToSignal == 0){
				break;
			}
			uNew = uOld - uCountToSignal * THREAD_TRAPPED_ONE;
		} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
	}
	for(size_t i = 0; i < uCountToSignal; ++i){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)puControl, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
		_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
	}
	return uCountToSignal;
}

bool _MCFCRT_WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext, uint64_t u64UntilFastMonoClock)
{
	const bool bSignaled = ReallyWaitForConditionVariable(&(pConditionVariable->__u), pfnUnlockCallback, pfnRelockCallback, nContext, true, u64UntilFastMonoClock, true);
	return bSignaled;
}
bool _MCFCRT_WaitForConditionVariableOrAbandon(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext, uint64_t u64UntilFastMonoClock)
{
	const bool bSignaled = ReallyWaitForConditionVariable(&(pConditionVariable->__u), pfnUnlockCallback, pfnRelockCallback, nContext, true, u64UntilFastMonoClock, false);
	return bSignaled;
}
void _MCFCRT_WaitForConditionVariableForever(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext)
{
	const bool bSignaled = ReallyWaitForConditionVariable(&(pConditionVariable->__u), pfnUnlockCallback, pfnRelockCallback, nContext, false, UINT64_MAX, true);
	_MCFCRT_ASSERT(bSignaled);
}
size_t _MCFCRT_SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToSignal){
	return ReallySignalConditionVariable(&(pConditionVariable->__u), uMaxCountToSignal);
}
size_t _MCFCRT_BroadcastConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable){
	return ReallySignalConditionVariable(&(pConditionVariable->__u), SIZE_MAX);
}
