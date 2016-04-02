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

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtDelayExecution(BOOLEAN bAlertable, const LARGE_INTEGER *pInterval);

#define FLAG_LOCKED             ((uintptr_t)0x0001)
#define FLAG_URGENT             ((uintptr_t)0x0002)
#define FLAGS_RESERVED          ((size_t)4)

#define GET_THREAD_COUNT(v_)    ((size_t)(uintptr_t)(v_) >> FLAGS_RESERVED)
#define MAKE_THREAD_COUNT(v_)   ((uintptr_t)(size_t)(v_) << FLAGS_RESERVED)

static inline bool RealWaitForMutex(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount, bool bMayTimeOut, uint64_t u64UntilFastMonoClock){
	{
		uintptr_t uOld, uNew;
		uOld = __atomic_load_n(pMutex, __ATOMIC_CONSUME);
		if(_MCFCRT_EXPECT(!(uOld & FLAG_LOCKED))){
			uNew = (uOld & ~FLAG_URGENT) + FLAG_LOCKED;
			if(_MCFCRT_EXPECT(__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED))){
				return true;
			}
		}
	}
	if(bMayTimeOut && _MCFCRT_EXPECT(u64UntilFastMonoClock == 0)){
		return false;
	}

	for(;;){
		__atomic_fetch_add(pMutex, MAKE_THREAD_COUNT(1), __ATOMIC_RELAXED);
		{
			size_t uSpinnedCount = 0;
			do {
				enum {
					kKeepSpinning,
					kTrap,
					kReturnLocked,
				} eResult;
				{
					uintptr_t uOld, uNew;
					uOld = __atomic_load_n(pMutex, __ATOMIC_CONSUME);
					do {
						if(GET_THREAD_COUNT(uOld) == 0){
							eResult = kTrap;
							break;
						} else if(!(uOld & FLAG_LOCKED)){
							eResult = kReturnLocked;
							uNew = (uOld & ~FLAG_URGENT) + FLAG_LOCKED - MAKE_THREAD_COUNT(1);
						} else if(uOld & FLAG_URGENT){
							eResult = kTrap;
							uNew = (uOld & ~FLAG_URGENT);
						} else {
							eResult = kKeepSpinning;
							break;
						}
					} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)));
				}
				if(eResult == kReturnLocked){
					return true;
				}
				if(eResult == kTrap){
					break;
				}
				__builtin_ia32_pause();
			} while(_MCFCRT_EXPECT(uSpinnedCount++ < uMaxSpinCount));
		}
		if(bMayTimeOut){
			LARGE_INTEGER liTimeout;
			__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
			NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pMutex, false, &liTimeout);
			_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
			if(_MCFCRT_EXPECT(lStatus == STATUS_TIMEOUT)){
				bool bDecremented;
				{
					uintptr_t uOld, uNew;
					uOld = __atomic_load_n(pMutex, __ATOMIC_RELAXED);
					do {
						bDecremented = (GET_THREAD_COUNT(uOld) != 0);
						if(!bDecremented){
							break;
						}
						uNew = uOld - MAKE_THREAD_COUNT(1);
					} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
				}
				if(_MCFCRT_EXPECT(!bDecremented)){
					lStatus = NtWaitForKeyedEvent(nullptr, (void *)pMutex, false, nullptr);
					_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
					_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
				}
				return false;
			}
		} else {
			NTSTATUS lStatus = NtWaitForKeyedEvent(nullptr, (void *)pMutex, false, nullptr);
			_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForKeyedEvent() 失败。");
			_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
		}
		{
			LARGE_INTEGER liTimeout;
			liTimeout.QuadPart= -1;
			NTSTATUS lStatus = NtDelayExecution(false, &liTimeout);
			_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtDelayExecution() 失败。");
		}
	}
}

static inline void RealSignalMutex(_MCFCRT_Mutex *pMutex){
	bool bSignalOne;
	{
		uintptr_t uOld, uNew;
		uOld = __atomic_load_n(pMutex, __ATOMIC_CONSUME);
		do {
			_MCFCRT_ASSERT_MSG(uOld & FLAG_LOCKED, L"互斥体没有被任何线程锁定。");
			_MCFCRT_ASSERT(!(uOld & FLAG_URGENT));

			bSignalOne = (GET_THREAD_COUNT(uOld) != 0);
			uNew = uOld - FLAG_LOCKED + (bSignalOne ? (FLAG_URGENT - MAKE_THREAD_COUNT(1)) : 0);
		} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(pMutex, &uOld, uNew, false, __ATOMIC_ACQ_REL, __ATOMIC_CONSUME)));
	}
	if(_MCFCRT_EXPECT(bSignalOne)){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)pMutex, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
		_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
	}
}

bool _MCFCRT_WaitForMutex(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount, uint64_t u64UntilFastMonoClock){
	const bool bLocked = RealWaitForMutex(pMutex, uMaxSpinCount, true, u64UntilFastMonoClock);
	return bLocked;
}
void _MCFCRT_WaitForMutexForever(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount){
	const bool bLocked = RealWaitForMutex(pMutex, uMaxSpinCount, false, UINT64_MAX);
	_MCFCRT_ASSERT(bLocked);
}
void _MCFCRT_SignalMutex(_MCFCRT_Mutex *pMutex){
	RealSignalMutex(pMutex);
}
