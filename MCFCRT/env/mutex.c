// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "mutex.h"
#include "clocks.h"
#include "../ext/assert.h"
#include "../ext/expect.h"
#include <limits.h>
#include <winternl.h>
#include <ntstatus.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

static inline bool TrySubtract(uintptr_t *puRet, uintptr_t uOp1, uintptr_t uOp2){
	if(uOp1 < uOp2){
		*puRet = uOp1;
		return false;
	}
	*puRet = uOp1 - uOp2;
	return true;
}

#define FLAG_LOCKED     ((uintptr_t)1)
#define FLAG_RESERVED   ((uintptr_t)2)
#define FLAG_BIT_COUNT  2

#define MUTEX_LOCKED    ((size_t)-1)

// 若锁定互斥体成功，返回 MUTEX_LOCKED；否则返回当前等待互斥体的线程数。
static inline size_t TryMutexAndGetWaitingThreadCount(_MCFCRT_Mutex *pMutex){
	uintptr_t uOld, uNew;
	uOld = __atomic_load_n(pMutex, __ATOMIC_RELAXED);
	do {
		uNew = uOld;
		uNew |= FLAG_LOCKED;
		if(uNew == uOld){
			// 已被其他线程锁定。
			return uOld >> FLAG_BIT_COUNT;
		}
	} while(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED));

	return MUTEX_LOCKED;
}
static inline bool WaitForMutex(_MCFCRT_Mutex *pMutex, const LARGE_INTEGER *pliTimeout){
	uintptr_t uOld, uNew;
	bool bSignalOne;
//	uOld = __atomic_load_n(pMutex, __ATOMIC_RELAXED);
//	do {
//		uNew = uOld;
//		uNew += (1 << FLAG_BIT_COUNT);
//	} while(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
	__atomic_fetch_add(pMutex, (1 << FLAG_BIT_COUNT), __ATOMIC_RELAXED);

	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pMutex, false, pliTimeout);
	if(!NT_SUCCESS(lStatus)){
		_MCFCRT_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		uOld = __atomic_load_n(pMutex, __ATOMIC_RELAXED);
		do {
			bSignalOne = TrySubtract(&uNew, uOld, (1 << FLAG_BIT_COUNT));
			if(!bSignalOne){
				lStatus = NtWaitForKeyedEvent(nullptr, (void *)pMutex, false, nullptr);
				if(!NT_SUCCESS(lStatus)){
					_MCFCRT_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
				}
				return false;
			}
		} while(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

		return false;
	}
	return true;
}
static inline void UnlockMutex(_MCFCRT_Mutex *pMutex){
	uintptr_t uOld, uNew;
	uOld = __atomic_load_n(pMutex, __ATOMIC_RELAXED);
	bool bSignalOne;
	do {
		_MCFCRT_ASSERT_MSG(uOld & FLAG_LOCKED, L"互斥锁没有被任何线程锁定。");
		bSignalOne = TrySubtract(&uNew, uOld, (1 << FLAG_BIT_COUNT));
		uNew &= ~FLAG_LOCKED;
	} while(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED));

	if(bSignalOne){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pMutex, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			_MCFCRT_ASSERT_MSG(false, L"NtReleaseKeyedEvent() 失败。");
		}
	}
}

bool _MCFCRT_TryMutex(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount, uint64_t u64UntilFastMonoClock){
	// 尝试是否可以立即锁定互斥体。
	size_t uWaitingThreads = TryMutexAndGetWaitingThreadCount(pMutex);
	if(_MCFCRT_EXPECT(uWaitingThreads == MUTEX_LOCKED)){
		return true;
	}
	if(u64UntilFastMonoClock == 0){
		return false;
	}

	for(;;){
		uWaitingThreads = TryMutexAndGetWaitingThreadCount(pMutex);
		if(_MCFCRT_EXPECT_NOT(uWaitingThreads == MUTEX_LOCKED)){
			return true;
		}

		// 如果没有正在内核态等待互斥体的线程，尝试一定次数的自旋。
		if(uWaitingThreads == 0){
			for(size_t i = 0; _MCFCRT_EXPECT(i < uMaxSpinCount); ++i){
				__builtin_ia32_pause();

				uWaitingThreads = TryMutexAndGetWaitingThreadCount(pMutex);
				if(_MCFCRT_EXPECT_NOT(uWaitingThreads == MUTEX_LOCKED)){
					return true;
				}
			}
		}

		// 陷入内核同步。
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
		if(!WaitForMutex(pMutex, &liTimeout)){
			return false;
		}
	}
}
void _MCFCRT_LockMutex(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount){
	// 尝试是否可以立即锁定互斥体。
	size_t uWaitingThreads = TryMutexAndGetWaitingThreadCount(pMutex);
	if(_MCFCRT_EXPECT(uWaitingThreads == MUTEX_LOCKED)){
		return;
	}

	for(;;){
		uWaitingThreads = TryMutexAndGetWaitingThreadCount(pMutex);
		if(_MCFCRT_EXPECT_NOT(uWaitingThreads == MUTEX_LOCKED)){
			return;
		}

		// 如果没有正在内核态等待互斥体的线程，尝试一定次数的自旋。
		if(uWaitingThreads == 0){
			for(size_t i = 0; _MCFCRT_EXPECT(i < uMaxSpinCount); ++i){
				__builtin_ia32_pause();

				uWaitingThreads = TryMutexAndGetWaitingThreadCount(pMutex);
				if(_MCFCRT_EXPECT_NOT(uWaitingThreads == MUTEX_LOCKED)){
					return;
				}
			}
		}

		// 陷入内核同步。
		if(!WaitForMutex(pMutex, nullptr)){
			_MCFCRT_ASSERT(false);
		}
	}
}
void _MCFCRT_UnlockMutex(_MCFCRT_Mutex *pMutex){
	UnlockMutex(pMutex);
}
