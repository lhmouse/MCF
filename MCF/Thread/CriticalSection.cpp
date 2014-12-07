// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CriticalSection.hpp"
#include "../Core/System.hpp"
using namespace MCF;

namespace {

unsigned long LockSpin(volatile unsigned long &ulCount) throw() { // FIXME: g++ 4.9.2 ICE
	unsigned long ulOld;
	for(;;){
		ulOld = __atomic_exchange_n(&ulCount, (unsigned long)-1, __ATOMIC_SEQ_CST);
		if(EXPECT_NOT(ulOld != (unsigned long)-1)){
			break;
		}
		__builtin_ia32_pause();
	}
	return ulOld;
}
void UnlockSpin(volatile unsigned long &ulCount, unsigned long ulOld) noexcept {
	__atomic_store_n(&ulCount, ulOld, __ATOMIC_SEQ_CST);
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
	, xm_ulQueueSize(0), xm_ulLockingThreadId(0), xm_ulRecursionCount(0)
{
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

// 其他非静态成员函数。
bool CriticalSection::xNonRecursiveTry(unsigned long ulThreadId) throw() { // FIXME: g++ 4.9.2 ICE
	std::size_t i = 1;
	if(GetProcessorCount() != 0){
		i += GetSpinCount();
	}
	for(;;){
		unsigned long ulExpected = 0;
		if(EXPECT_NOT(__atomic_compare_exchange_n(
			&xm_ulLockingThreadId, &ulExpected, ulThreadId, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)))
		{
			return true;
		}
		if(EXPECT_NOT(--i == 0)){
			return false;
		}
		__builtin_ia32_pause();
	}
}
void CriticalSection::xNonRecursiveAcquire(unsigned long ulThreadId) noexcept {
	while(!xNonRecursiveTry(ulThreadId)){
		auto ulQueueSize = LockSpin(xm_ulQueueSize);
		++ulQueueSize;
		UnlockSpin(xm_ulQueueSize, ulQueueSize);
		xm_vSemaphore.Wait();
	}
}
void CriticalSection::xNonRecursiveRelease() noexcept {
	__atomic_store_n(&xm_ulLockingThreadId, 0, __ATOMIC_RELEASE);

	auto ulQueueSize = LockSpin(xm_ulQueueSize);
	if(ulQueueSize != 0){
		--ulQueueSize;
		xm_vSemaphore.Post();
	}
	UnlockSpin(xm_ulQueueSize, ulQueueSize);
}

CriticalSection::Result CriticalSection::Try() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if(__atomic_load_n(&xm_ulLockingThreadId, __ATOMIC_ACQUIRE) == dwThreadId){
		ASSERT(xm_ulRecursionCount != 0);
		++xm_ulRecursionCount;
		return R_RECURSIVE;
	}
	if(!xNonRecursiveTry(dwThreadId)){
		return R_TRY_FAILED;
	}
	++xm_ulRecursionCount;
	return R_STATE_CHANGED;
}
CriticalSection::Result CriticalSection::Acquire() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if(__atomic_load_n(&xm_ulLockingThreadId, __ATOMIC_ACQUIRE) == dwThreadId){
		ASSERT(xm_ulRecursionCount != 0);
		++xm_ulRecursionCount;
		return R_RECURSIVE;
	}
	xNonRecursiveAcquire(dwThreadId);
	++xm_ulRecursionCount;
	return R_STATE_CHANGED;
}
CriticalSection::Result CriticalSection::Release() noexcept {
	ASSERT(IsLockedByCurrentThread());

	if(--xm_ulRecursionCount != 0){
		return R_RECURSIVE;
	}
	xNonRecursiveRelease();
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
