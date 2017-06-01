// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#define __MCFCRT_CONDITION_VARIABLE_INLINE_OR_EXTERN     extern inline
#include "condition_variable.h"
#include "_nt_timeout.h"
#include "xassert.h"
#include "expect.h"
#include <winternl.h>
#include <ntstatus.h>

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

__attribute__((__dllimport__, __stdcall__, __const__))
extern BOOLEAN RtlDllShutdownInProgress(void);

#ifndef __BYTE_ORDER__
#  error Byte order is unknown.
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define BSUSR(v_)             ((uintptr_t)(v_) << CHAR_BIT)
#  define BSFB(v_)              ((uintptr_t)(v_)            )
#else
#  define BSUSR(v_)             ((uintptr_t)(v_)                                        )
#  define BSFB(v_)              ((uintptr_t)(v_) << ((sizeof(uintptr_t) - 1) * CHAR_BIT))
#endif

#define MASK_THREADS_RELEASED   ((uintptr_t)( BSFB(0x03)))
#define MASK_THREADS_SPINNING   ((uintptr_t)( BSFB(0x0C)))
#define MASK_SPIN_FAILURE_COUNT ((uintptr_t)( BSFB(0xF0)))
#define MASK_THREADS_TRAPPED    ((uintptr_t)(~BSFB(0xFF)))

#define THREADS_RELEASED_ONE    ((uintptr_t)(MASK_THREADS_RELEASED & -MASK_THREADS_RELEASED))
#define THREADS_RELEASED_MAX    ((uintptr_t)(MASK_THREADS_RELEASED / THREADS_RELEASED_ONE))

#define THREADS_SPINNING_ONE    ((uintptr_t)(MASK_THREADS_SPINNING & -MASK_THREADS_SPINNING))
#define THREADS_SPINNING_MAX    ((uintptr_t)(MASK_THREADS_SPINNING / THREADS_SPINNING_ONE))

#define SPIN_FAILURE_COUNT_ONE  ((uintptr_t)(MASK_SPIN_FAILURE_COUNT & -MASK_SPIN_FAILURE_COUNT))
#define SPIN_FAILURE_COUNT_MAX  ((uintptr_t)(MASK_SPIN_FAILURE_COUNT / SPIN_FAILURE_COUNT_ONE))

#define THREADS_TRAPPED_ONE     ((uintptr_t)(MASK_THREADS_TRAPPED & -MASK_THREADS_TRAPPED))
#define THREADS_TRAPPED_MAX     ((uintptr_t)(MASK_THREADS_TRAPPED / THREADS_TRAPPED_ONE))

static_assert(__builtin_popcountll(MASK_THREADS_RELEASED) == __builtin_popcountll(MASK_THREADS_SPINNING), "MASK_THREADS_RELEASED must have the same number of bits set as MASK_THREADS_SPINNING.");

#define MIN_SPIN_COUNT          ((uintptr_t)64)
#define MAX_SPIN_MULTIPLIER     ((uintptr_t)32)

__attribute__((__always_inline__))
static inline bool ReallyWaitForConditionVariable(volatile uintptr_t *puControl,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext,
	size_t uMaxSpinCountInitial, bool bMayTimeOut, uint64_t u64UntilFastMonoClock, bool bRelockIfTimeOut)
{
	size_t uMaxSpinCount, uSpinMultiplier;
	bool bSignaled, bSpinnable;
	{
		uintptr_t uOld, uNew;
		uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
		do {
			const size_t uSpinFailureCount = (uOld & MASK_SPIN_FAILURE_COUNT) / SPIN_FAILURE_COUNT_ONE;
			if(uMaxSpinCountInitial > MIN_SPIN_COUNT){
				uMaxSpinCount = (uMaxSpinCountInitial >> uSpinFailureCount) | MIN_SPIN_COUNT;
				uSpinMultiplier = MAX_SPIN_MULTIPLIER >> uSpinFailureCount;
			} else {
				uMaxSpinCount = uMaxSpinCountInitial;
				uSpinMultiplier = 0;
			}
			bSignaled = (uOld & MASK_THREADS_RELEASED) != 0;
			bSpinnable = false;
			if(!bSignaled){
				if(uMaxSpinCount != 0){
					const size_t uThreadsSpinning = (uOld & MASK_THREADS_SPINNING) / THREADS_SPINNING_ONE;
					bSpinnable = uThreadsSpinning < THREADS_SPINNING_MAX;
				}
				if(!bSpinnable){
					break;
				}
				uNew = uOld + THREADS_SPINNING_ONE;
			} else {
				const bool bSpinFailureCountDecremented = uSpinFailureCount != 0;
				uNew = uOld - THREADS_RELEASED_ONE - bSpinFailureCountDecremented * SPIN_FAILURE_COUNT_ONE;
			}
		} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
	}
	if(_MCFCRT_EXPECT(bSignaled)){
		// This is effectively a spurious wakeup.
		return true;
	}
	intptr_t nUnlocked;
	if(_MCFCRT_EXPECT(bSpinnable)){
		nUnlocked = (*pfnUnlockCallback)(nContext);
		for(size_t i = 0; _MCFCRT_EXPECT(i < uMaxSpinCount); ++i){
			register size_t j = uSpinMultiplier + 1;
			__atomic_thread_fence(__ATOMIC_SEQ_CST);
			do {
				__builtin_ia32_pause();
			} while(--j != 0);
			__atomic_thread_fence(__ATOMIC_SEQ_CST);
			{
				uintptr_t uOld, uNew;
				uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
				do {
					bSignaled = (uOld & MASK_THREADS_RELEASED) != 0;
					if(!bSignaled){
						break;
					}
					const size_t uSpinFailureCount = (uOld & MASK_SPIN_FAILURE_COUNT) / SPIN_FAILURE_COUNT_ONE;
					const bool bSpinFailureCountDecremented = uSpinFailureCount != 0;
					uNew = uOld - THREADS_SPINNING_ONE - THREADS_RELEASED_ONE - bSpinFailureCountDecremented * SPIN_FAILURE_COUNT_ONE;
				} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
			}
			if(_MCFCRT_EXPECT_NOT(bSignaled)){
				(*pfnRelockCallback)(nContext, nUnlocked);
				return true;
			}
		}
		{
			uintptr_t uOld, uNew;
			uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
			do {
				const size_t uSpinFailureCount = (uOld & MASK_SPIN_FAILURE_COUNT) / SPIN_FAILURE_COUNT_ONE;
				bSignaled = (uOld & MASK_THREADS_RELEASED) != 0;
				if(!bSignaled){
					const bool bSpinFailureCountIncremented = uSpinFailureCount < SPIN_FAILURE_COUNT_MAX;
					uNew = uOld - THREADS_SPINNING_ONE + THREADS_TRAPPED_ONE + bSpinFailureCountIncremented * SPIN_FAILURE_COUNT_ONE;
				} else {
					const bool bSpinFailureCountDecremented = uSpinFailureCount != 0;
					uNew = uOld - THREADS_SPINNING_ONE - THREADS_RELEASED_ONE - bSpinFailureCountDecremented * SPIN_FAILURE_COUNT_ONE;
				}
			} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
		}
		if(_MCFCRT_EXPECT(bSignaled)){
			(*pfnRelockCallback)(nContext, nUnlocked);
			return true;
		}
	} else {
		{
			uintptr_t uOld, uNew;
			uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
			do {
				bSignaled = (uOld & MASK_THREADS_RELEASED) != 0;
				if(!bSignaled){
					uNew = uOld + THREADS_TRAPPED_ONE;
				} else {
					uNew = uOld - THREADS_RELEASED_ONE;
				}
			} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
		}
		if(_MCFCRT_EXPECT(bSignaled)){
			return true;
		}
		nUnlocked = (*pfnUnlockCallback)(nContext);
	}
	if(bMayTimeOut){
		LARGE_INTEGER liTimeout;
		__MCFCRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
		NTSTATUS lStatus = NtWaitForKeyedEvent(_MCFCRT_NULLPTR, (void *)puControl, false, &liTimeout);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
		while(_MCFCRT_EXPECT(lStatus == STATUS_TIMEOUT)){
			bool bDecremented;
			{
				uintptr_t uOld, uNew;
				uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
				do {
					const size_t uThreadsTrapped = (uOld & MASK_THREADS_TRAPPED) / THREADS_TRAPPED_ONE;
					bDecremented = uThreadsTrapped != 0;
					if(!bDecremented){
						break;
					}
					uNew = uOld - THREADS_TRAPPED_ONE;
				} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
			}
			if(bDecremented){
				if(bRelockIfTimeOut){
					(*pfnRelockCallback)(nContext, nUnlocked);
				}
				return false;
			}
			liTimeout.QuadPart = 0;
			lStatus = NtWaitForKeyedEvent(_MCFCRT_NULLPTR, (void *)puControl, false, &liTimeout);
			_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
		}
	} else {
		NTSTATUS lStatus = NtWaitForKeyedEvent(_MCFCRT_NULLPTR, (void *)puControl, false, _MCFCRT_NULLPTR);
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
			const size_t uThreadsSpinning = (uOld & MASK_THREADS_SPINNING) / THREADS_SPINNING_ONE;
			const size_t uThreadsTrapped = (uOld & MASK_THREADS_TRAPPED) / THREADS_TRAPPED_ONE;
			uCountToSignal = (uThreadsTrapped <= uMaxCountToSignal) ? uThreadsTrapped : uMaxCountToSignal;
			uNew = (uOld & ~MASK_THREADS_RELEASED) + uThreadsSpinning * THREADS_RELEASED_ONE - uCountToSignal * THREADS_TRAPPED_ONE;
			if(uNew == uOld){
				break;
			}
		} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
	}
	// If `RtlDllShutdownInProgress()` is `true`, other threads will have been terminated.
	// Calling `NtReleaseKeyedEvent()` when no thread is waiting results in deadlocks. Don't do that.
	if(_MCFCRT_EXPECT_NOT((uCountToSignal > 0) && !RtlDllShutdownInProgress())){
		for(size_t i = 0; i < uCountToSignal; ++i){
			NTSTATUS lStatus = NtReleaseKeyedEvent(_MCFCRT_NULLPTR, (void *)puControl, false, _MCFCRT_NULLPTR);
			_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
			_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
		}
	}
	return uCountToSignal;
}

bool __MCFCRT_ReallyWaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext,
	size_t uMaxSpinCount, uint64_t u64UntilFastMonoClock)
{
	const bool bSignaled = ReallyWaitForConditionVariable(&(pConditionVariable->__u), pfnUnlockCallback, pfnRelockCallback, nContext,
		uMaxSpinCount, true, u64UntilFastMonoClock, true);
	return bSignaled;
}
bool __MCFCRT_ReallyWaitForConditionVariableOrAbandon(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext,
	size_t uMaxSpinCount, uint64_t u64UntilFastMonoClock)
{
	const bool bSignaled = ReallyWaitForConditionVariable(&(pConditionVariable->__u), pfnUnlockCallback, pfnRelockCallback, nContext,
		uMaxSpinCount, true, u64UntilFastMonoClock, false);
	return bSignaled;
}
void __MCFCRT_ReallyWaitForConditionVariableForever(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext,
	size_t uMaxSpinCount)
{
	const bool bSignaled = ReallyWaitForConditionVariable(&(pConditionVariable->__u), pfnUnlockCallback, pfnRelockCallback, nContext,
		uMaxSpinCount, false, UINT64_MAX, true);
	_MCFCRT_ASSERT(bSignaled);
}
size_t __MCFCRT_ReallySignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToSignal){
	return ReallySignalConditionVariable(&(pConditionVariable->__u), uMaxCountToSignal);
}
size_t __MCFCRT_ReallyBroadcastConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable){
	return ReallySignalConditionVariable(&(pConditionVariable->__u), SIZE_MAX);
}
