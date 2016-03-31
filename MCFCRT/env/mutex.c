// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "mutex.h"
#include "_nt_timeout.h"
#include "../ext/assert.h"
#include "../ext/expect.h"
#include <limits.h>
#include <winternl.h>
#include <ntstatus.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

static inline uintptr_t AtomicLoadConsume(volatile uintptr_t *p){
	return __atomic_load_n(p, __ATOMIC_CONSUME);
}
static inline bool AtomicCompareExchangeAcqRel(volatile uintptr_t *p, uintptr_t *cmp, uintptr_t xchg){
	return __atomic_compare_exchange_n(p, cmp, xchg, false, __ATOMIC_ACQ_REL, __ATOMIC_CONSUME);
}

#define FLAG_LOCKED             ((uintptr_t)0x0001)
#define FLAG_URGENT             ((uintptr_t)0x0002)
#define FLAGS_RESERVED          ((size_t)4)

#define GET_THREAD_COUNT(v_)    ((size_t)(uintptr_t)(v_) >> FLAG_COUNT)
#define MAKE_THREAD_COUNT(v_)   ((uintptr_t)(size_t)(v_) << FLAG_COUNT)

static inline bool RealWaitForMutex(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount, bool bMayTimeOut, uint64_t u64UntilFastMonoClock){
	//
}
static inline void RealSignalMutex(_MCFCRT_Mutex *pMutex){
	bool bSignalOne;
	{
		uintptr_t uOld, uNew;
		uOld = AtomicLoadConsume(pMutex);
		do {
			_MCFCRT_ASSERT_MSG(uOld & FLAG_LOCKED, L"互斥体没有被任何线程锁定。");
			_MCFCRT_ASSERT(!(uOld & FLAG_URGENT));

			if(GET_THREAD_COUNT(uOld) == 0){
				bSignalOne = false;
				uNew = uOld - FLAG_LOCKED + FLAG_URGENT - MAKE_THREAD_COUNT(1);
			} else {
				bSignalOne = true;
				uNew = uOld - FLAG_LOCKED;
			}
		} while(_MCFCRT_EXPECT_NOT(!AtomicCompareExchangeAcqRel(pValue, &uOld, uNew)));
	}
	if(bSignalOne){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pMutex, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
		_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
	}
}

bool _MCFCRT_WaitForMutex(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount, uint64_t u64UntilFastMonoClock){
	const bool bLocked = RealWaitForMutex(pMutex, uMaxSpinCount, false, UINT64_MAX);
	return bLocked;
}
void _MCFCRT_WaitForMutexForever(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount){
	const bool bLocked = RealWaitForMutex(pMutex, uMaxSpinCount, false, UINT64_MAX);
	_MCFCRT_ASSERT(bLocked);
}
void _MCFCRT_SignalMutex(_MCFCRT_Mutex *pMutex){
	RealSignalMutex(pMutex);
}
