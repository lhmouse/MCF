// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CriticalSection.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

namespace {

unsigned long LockSpin(volatile unsigned long &ulCount){
	unsigned long ulOld;
	for(;;){
		ulOld = __atomic_exchange_n(&ulCount, (unsigned long)-1, __ATOMIC_ACQ_REL);
		if(EXPECT_NOT(ulOld != (unsigned long)-1)){
			break;
		}
		__builtin_ia32_pause();
	}
	return ulOld;
}
void UnlockSpin(volatile unsigned long &ulCount, unsigned long ulOld){
	__atomic_store_n(&ulCount, ulOld, __ATOMIC_RELEASE);
}

}

namespace MCF {

template<>
bool CriticalSection::Lock::xDoTry() const noexcept {
	return xm_pOwner->Try() != CriticalSection::R_TRY_FAILED;
}
template<>
void CriticalSection::Lock::xDoLock() const noexcept {
	xm_pOwner->Acquire();
}
template<>
void CriticalSection::Lock::xDoUnlock() const noexcept {
	xm_pOwner->Release();
}

}

// 构造函数和析构函数。
CriticalSection::CriticalSection(unsigned long ulSpinCount)
	: xm_vSemaphore(0), xm_ulSpinCount(ulSpinCount)
	, xm_ulWantingEvent(0), xm_ulLockingThreadId(0), xm_ulRecursionCount(0)
{
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

// 其他非静态成员函数。
CriticalSection::Result CriticalSection::Try() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if(__atomic_load_n(&xm_ulLockingThreadId, __ATOMIC_ACQUIRE) == dwThreadId){
		ASSERT(xm_ulRecursionCount != 0);
		++xm_ulRecursionCount;
		return R_RECURSIVE;
	}

	const auto ulWaiting = LockSpin(xm_ulWantingEvent);
	DWORD dwOldLocking = 0;
	if(__atomic_compare_exchange_n(&xm_ulLockingThreadId, &dwOldLocking, dwThreadId,
		false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
	{
		// 无竞态。
		UnlockSpin(xm_ulWantingEvent, ulWaiting + 1);
		ASSERT(xm_ulRecursionCount == 0);
		++xm_ulRecursionCount;
		return R_STATE_CHANGED;
	}
	// 有竞态。
	UnlockSpin(xm_ulWantingEvent, ulWaiting);
	return R_TRY_FAILED;
}
CriticalSection::Result CriticalSection::Acquire() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if(__atomic_load_n(&xm_ulLockingThreadId, __ATOMIC_ACQUIRE) == dwThreadId){
		ASSERT(xm_ulRecursionCount != 0);
		++xm_ulRecursionCount;
		return R_RECURSIVE;
	}

	const auto ulWaiting = LockSpin(xm_ulWantingEvent);
	UnlockSpin(xm_ulWantingEvent, ulWaiting + 1);
	if(ulWaiting == 0){
		auto i = GetSpinCount() + 1;
		do {
			DWORD dwOldLocking = 0;
			if(__atomic_compare_exchange_n(&xm_ulLockingThreadId, &dwOldLocking, dwThreadId,
				false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
			{
				// 无竞态。
				ASSERT(xm_ulRecursionCount == 0);
				++xm_ulRecursionCount;
				return R_STATE_CHANGED;
			}
			__builtin_ia32_pause();
		} while(EXPECT(--i != 0));
	}
	// 有竞态。
	xm_vSemaphore.Wait();
	ASSERT(__atomic_load_n(&xm_ulLockingThreadId, __ATOMIC_ACQUIRE) == 0);
	__atomic_store_n(&xm_ulLockingThreadId, dwThreadId, __ATOMIC_RELEASE);
	++xm_ulRecursionCount;
	return R_STATE_CHANGED;
}
CriticalSection::Result CriticalSection::Release() noexcept {
	ASSERT(IsLockedByCurrentThread());

	if(--xm_ulRecursionCount != 0){
		return R_RECURSIVE;
	}

	__atomic_store_n(&xm_ulLockingThreadId, 0, __ATOMIC_RELEASE);

	const auto ulWaiting = LockSpin(xm_ulWantingEvent);
	if(ulWaiting != 1){
		xm_vSemaphore.Post();
	}
	UnlockSpin(xm_ulWantingEvent, ulWaiting - 1);

	return R_STATE_CHANGED;
}

bool CriticalSection::IsLockedByCurrentThread() const noexcept {
	return __atomic_load_n(&xm_ulLockingThreadId, __ATOMIC_ACQUIRE) == ::GetCurrentThreadId();
}
unsigned long CriticalSection::UncheckedGetRecursionCount() const noexcept {
	ASSERT(IsLockedByCurrentThread());
	return xm_ulRecursionCount;
}
unsigned long CriticalSection::GetRecursionCount() const noexcept {
	if(!IsLockedByCurrentThread()){
		return 0;
	}
	return UncheckedGetRecursionCount();
}

CriticalSection::Lock CriticalSection::TryLock() noexcept {
	Lock vLock(*this, false);
	vLock.Try();
	return std::move(vLock);
}
CriticalSection::Lock CriticalSection::GetLock() noexcept {
	return Lock(*this);
}
