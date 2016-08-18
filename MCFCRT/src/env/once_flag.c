// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#define __MCFCRT_ONCE_FLAG_INLINE_OR_EXTERN     extern inline
#include "once_flag.h"
#include "_nt_timeout.h"
#include "../ext/assert.h"
#include "../ext/expect.h"
#include <winternl.h>
#include <ntstatus.h>

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

// 第一个字节保留给 Itanium ABI 用于标记是否已初始化。
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#   define BSUSR(v_)            ((uintptr_t)((uintptr_t)(v_) << CHAR_BIT))
#   define BSFB(v_)             ((uintptr_t)((uintptr_t)(v_)            ))
#else
#   define BSUSR(v_)            ((uintptr_t)((uintptr_t)(v_)                                        ))
#   define BSFB(v_)             ((uintptr_t)((uintptr_t)(v_) << ((sizeof(uintptr_t) - 1) * CHAR_BIT)))
#endif

#define MASK_LOCKED             ((uintptr_t)( BSUSR(0x01)              ))
#define MASK_FINISHED           ((uintptr_t)(                BSFB(0x01)))
#define MASK_THREADS_TRAPPED    ((uintptr_t)(~BSUSR(0x01) & ~BSFB(0xFF)))

#define THREAD_TRAPPED_ONE      ((uintptr_t)(MASK_THREADS_TRAPPED & -MASK_THREADS_TRAPPED))
#define THREAD_TRAPPED_MAX      ((uintptr_t)(MASK_THREADS_TRAPPED / THREAD_TRAPPED_ONE))

__attribute__((__always_inline__))
static inline _MCFCRT_OnceResult ReallyTryOnceFlag(volatile uintptr_t *puControl){
	{
		uintptr_t uOld, uNew;
		uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
		if(_MCFCRT_EXPECT(uOld & MASK_FINISHED)){
			return _MCFCRT_kOnceResultFinished;
		}
		if(_MCFCRT_EXPECT(!(uOld & MASK_LOCKED))){
			uNew = uOld | MASK_LOCKED;
			if(_MCFCRT_EXPECT(__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))){
				return _MCFCRT_kOnceResultInitial;
			}
		}
	}
	return _MCFCRT_kOnceResultTimedOut;
}
__attribute__((__always_inline__))
static inline _MCFCRT_OnceResult RealWaitForOnceFlag(volatile uintptr_t *puControl, bool bMayTimeOut, uint64_t u64UntilFastMonoClock){
	for(;;){
		bool bFinished, bTaken;
		{
			uintptr_t uOld, uNew;
			uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
			do {
				bFinished = !!(uOld & MASK_FINISHED);
				if(bFinished){
					break;
				}
				bTaken = !(uOld & MASK_LOCKED);
				if(!bTaken){
					uNew = uOld + THREAD_TRAPPED_ONE;
				} else {
					uNew = uOld + MASK_LOCKED; // uOld | MASK_LOCKED;
				}
			} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)));
		}
		if(_MCFCRT_EXPECT(bFinished)){
			return _MCFCRT_kOnceResultFinished;
		}
		if(_MCFCRT_EXPECT(bTaken)){
			return _MCFCRT_kOnceResultInitial;
		}
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
					} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)));
				}
				if(bDecremented){
					return _MCFCRT_kOnceResultTimedOut;
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
	}
}
__attribute__((__always_inline__))
static inline void RealSetAndSignalOnceFlag(volatile uintptr_t *puControl, bool bFinished, size_t uMaxCountToSignal){
	uintptr_t uCountToSignal;
	{
		uintptr_t uOld, uNew;
		uOld = __atomic_load_n(puControl, __ATOMIC_RELAXED);
		do {
			_MCFCRT_ASSERT_MSG(uOld & MASK_LOCKED,      L"一次性初始化标志没有被任何线程锁定。");
			_MCFCRT_ASSERT_MSG(!(uOld & MASK_FINISHED), L"一次性初始化标志已被使用。");

			uNew = uOld & ~MASK_LOCKED;
			if(bFinished){
				uNew |= MASK_FINISHED;
			}
			const size_t uThreadsTrapped = (uOld & MASK_THREADS_TRAPPED) / THREAD_TRAPPED_ONE;
			uCountToSignal = (uThreadsTrapped <= uMaxCountToSignal) ? uThreadsTrapped : uMaxCountToSignal;
			uNew -= uCountToSignal * THREAD_TRAPPED_ONE;
		} while(_MCFCRT_EXPECT_NOT(!__atomic_compare_exchange_n(puControl, &uOld, uNew, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)));
	}
	for(size_t i = 0; i < uCountToSignal; ++i){
		NTSTATUS lStatus = NtReleaseKeyedEvent(nullptr, (void *)puControl, false, nullptr);
		_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseKeyedEvent() 失败。");
		_MCFCRT_ASSERT(lStatus != STATUS_TIMEOUT);
	}
}

_MCFCRT_OnceResult _MCFCRT_WaitForOnceFlag(_MCFCRT_OnceFlag *pOnceFlag, uint64_t u64UntilFastMonoClock){
	_MCFCRT_OnceResult eResult = ReallyTryOnceFlag(&(pOnceFlag->__u));
	if(_MCFCRT_EXPECT_NOT(eResult == _MCFCRT_kOnceResultTimedOut) && _MCFCRT_EXPECT_NOT(u64UntilFastMonoClock != 0)){
		eResult = RealWaitForOnceFlag(&(pOnceFlag->__u), true, u64UntilFastMonoClock);
	}
	return eResult;
}
_MCFCRT_OnceResult _MCFCRT_WaitForOnceFlagForever(_MCFCRT_OnceFlag *pOnceFlag){
	_MCFCRT_OnceResult eResult = ReallyTryOnceFlag(&(pOnceFlag->__u));
	if(_MCFCRT_EXPECT_NOT(eResult == _MCFCRT_kOnceResultTimedOut)){
		eResult = RealWaitForOnceFlag(&(pOnceFlag->__u), false, UINT64_MAX);
	}
	_MCFCRT_ASSERT(eResult != _MCFCRT_kOnceResultTimedOut);
	return eResult;
}
void _MCFCRT_SignalOnceFlagAsFinished(_MCFCRT_OnceFlag *pOnceFlag){
	RealSetAndSignalOnceFlag(&(pOnceFlag->__u), true, SIZE_MAX);
}
void _MCFCRT_SignalOnceFlagAsAborted(_MCFCRT_OnceFlag *pOnceFlag){
	RealSetAndSignalOnceFlag(&(pOnceFlag->__u), false, 1);
}
