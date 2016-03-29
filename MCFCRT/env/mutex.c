// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "mutex.h"
#include "_nt_timeout.h"
#include "../ext/assert.h"
#include "../ext/expect.h"
#include <winternl.h>
#include <ntstatus.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

static inline uintptr_t AtomicCompareExchangeSeqCst(volatile uintptr_t *puValue, uintptr_t uExpected, uintptr_t uExchangeWith){
	__atomic_compare_exchange_n(puValue, &uExpected, uExchangeWith, false, __ATOMIC_SEQ_CST, __ATOMIC_CONSUME);
	return uExpected;
}

typedef struct tagQueuedThread {
	struct tagQueuedThread *pNext;
	volatile bool bReleased;
} QueuedThread;

#define QUEUED_NULL     ((uintptr_t)(QueuedThread *)nullptr)

#define FLAG_BIT_COUNT  ((size_t)2)
#define FLAG_LOCKED     ((uintptr_t)1)
#define FLAG_RESERVED   ((uintptr_t)2)

static inline bool TryMutexOneShot(_MCFCRT_Mutex *pMutex){
	return AtomicCompareExchangeSeqCst(pMutex, QUEUED_NULL, QUEUED_NULL | FLAG_LOCKED) == QUEUED_NULL;
}
static inline bool WaitForMutex(_MCFCRT_Mutex *pMutex, const LARGE_INTEGER *pliTimeout){
	return true;
}
static inline void SignalMutex(_MCFCRT_Mutex *pMutex){
}


/*
#define FLAG_LOCKED     ((uintptr_t)1)
#define FLAG_RESERVED   ((uintptr_t)2)
#define FLAG_BIT_COUNT  2

#define MUTEX_LOCKED    ((size_t)-1)

// 若锁定互斥体成功，返回 MUTEX_LOCKED；否则返回当前等待互斥体的线程数。
static inline size_t TryMutexAndGetWaitingThreadCount(_MCFCRT_Mutex *pMutex){
	uintptr_t uOld, uNew;
	uOld = __atomic_load_n(pMutex, __ATOMIC_RELAXED);
	do {
		uNew = uOld | FLAG_LOCKED;
		if(uNew == uOld){
			goto jDontMissIt;
		}
	} while(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED));
	return MUTEX_LOCKED;

jDontMissIt:
	// 已被其他线程锁定。
	return uOld >> FLAG_BIT_COUNT;
}
static inline bool WaitForMutex(_MCFCRT_Mutex *pMutex, const LARGE_INTEGER *pliTimeout){
	__atomic_fetch_add(pMutex, (1 << FLAG_BIT_COUNT), __ATOMIC_RELAXED);

	NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pMutex, false, pliTimeout);
	if(!NT_SUCCESS(lStatus)){
		_MCFCRT_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		uintptr_t uOld, uNew;
		uOld = __atomic_load_n(pMutex, __ATOMIC_RELAXED);
		do {
			if(uOld < (1 << FLAG_BIT_COUNT)){
				goto jDontMissIt;
			}
			uNew = uOld - (1 << FLAG_BIT_COUNT);
		} while(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
		return false;

	jDontMissIt:
		lStatus = NtWaitForKeyedEvent(nullptr, (void *)pMutex, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			_MCFCRT_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
		}
	}
	return true;
}
static inline void UnlockMutex(_MCFCRT_Mutex *pMutex){
	bool bSignalOne;

	uintptr_t uOld, uNew;
	uOld = __atomic_load_n(pMutex, __ATOMIC_RELAXED);
	do {
		_MCFCRT_ASSERT_MSG(uOld & FLAG_LOCKED, L"互斥锁没有被任何线程锁定。");
		if(uOld < (1 << FLAG_BIT_COUNT)){
			uNew = uOld & ~FLAG_LOCKED;
			bSignalOne = false;
		} else {
			uNew = (uOld & ~FLAG_LOCKED) - (1 << FLAG_BIT_COUNT);
			bSignalOne = true;
		}
	} while(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED));

	if(bSignalOne){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pMutex, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			_MCFCRT_ASSERT_MSG(false, L"NtReleaseKeyedEvent() 失败。");
		}
	}
}


	// 尝试是否可以立即锁定互斥体。
	size_t uWaitingThreads = TryMutexAndGetWaitingThreadCount(pMutex);
	if(_MCFCRT_EXPECT(uWaitingThreads == MUTEX_LOCKED)){
		return true;
	}

	if(_MCFCRT_EXPECT_NOT(u64UntilFastMonoClock != 0)){
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
		__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
		if(!WaitForMutex(pMutex, &liTimeout)){
			return false;
		}
	}
*/
bool _MCFCRT_WaitForMutex(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount, uint64_t u64UntilFastMonoClock){
	if(_MCFCRT_EXPECT(TryMutexOneShot(pMutex))){
		return true;
	}

	bool bLocked;
	bLocked = false;
	if(_MCFCRT_EXPECT_NOT(u64UntilFastMonoClock != 0)){
		LARGE_INTEGER liTimeout;
		__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
		bLocked = WaitForMutex(pMutex, &liTimeout);
	}
	return bLocked;
}
void _MCFCRT_WaitForMutexForever(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount){
	if(_MCFCRT_EXPECT(TryMutexOneShot(pMutex))){
		return;
	}

	bool bLocked;
	bLocked = WaitForMutex(pMutex, nullptr);
	_MCFCRT_ASSERT(bLocked);
}
void _MCFCRT_SignalMutex(_MCFCRT_Mutex *pMutex){
	SignalMutex(pMutex);
}
