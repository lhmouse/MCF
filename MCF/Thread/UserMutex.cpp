// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UserMutex.hpp"
#include "../Core/System.hpp"
using namespace MCF;

namespace {

inline void SpinPause() throw() { // FIXME: g++ 4.9.2 ICE
	__builtin_ia32_pause();
}

inline unsigned long LockSpin(volatile unsigned long &ulCount) noexcept {
	unsigned long ulOld;
	for(;;){
		ulOld = __atomic_exchange_n(&ulCount, (unsigned long)-1, __ATOMIC_SEQ_CST);
		if(EXPECT_NOT(ulOld != (unsigned long)-1)){
			break;
		}
		SpinPause();
	}
	return ulOld;
}
inline void UnlockSpin(volatile unsigned long &ulCount, unsigned long ulOld) noexcept {
	__atomic_store_n(&ulCount, ulOld, __ATOMIC_SEQ_CST);
}

}

namespace MCF {

template<>
bool UserMutex::UniqueLock::xDoTry() const noexcept {
	return xm_pOwner->Try();
}
template<>
void UserMutex::UniqueLock::xDoLock() const noexcept {
	xm_pOwner->Lock();
}
template<>
void UserMutex::UniqueLock::xDoUnlock() const noexcept {
	xm_pOwner->Unlock();
}

}

// 构造函数和析构函数。
UserMutex::UserMutex(unsigned long ulSpinCount)
	: xm_vSemaphore(0), xm_ulSpinCount(ulSpinCount)
	, xm_ulQueueSize(0), xm_ulLockingThreadId(0)
{
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

// 其他非静态成员函数。
bool UserMutex::xTryWithHint(unsigned long ulThreadId) noexcept {
	ASSERT(!IsLockedByCurrentThread());

	std::size_t i = 1;
	if(EXPECT(GetProcessorCount() != 0)){
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
		SpinPause();
	}
}

bool UserMutex::IsLockedByCurrentThread() const noexcept {
	return __atomic_load_n(&xm_ulLockingThreadId, __ATOMIC_ACQUIRE) == ::GetCurrentThreadId();
}

bool UserMutex::Try() noexcept {
	ASSERT(!IsLockedByCurrentThread());

	return xTryWithHint(::GetCurrentThreadId());
}
void UserMutex::Lock() noexcept {
	ASSERT(!IsLockedByCurrentThread());

	const auto dwThreadId = ::GetCurrentThreadId();

	auto ulQueueSize = LockSpin(xm_ulQueueSize);
	if(EXPECT(ulQueueSize == 0)){
		UnlockSpin(xm_ulQueueSize, ulQueueSize);
		if(xTryWithHint(dwThreadId)){
			return;
		}
		ulQueueSize = LockSpin(xm_ulQueueSize);
	}
	for(;;){
		++ulQueueSize;
		UnlockSpin(xm_ulQueueSize, ulQueueSize);
		xm_vSemaphore.Wait();

		if(EXPECT_NOT(xTryWithHint(dwThreadId))){
			break;
		}
		ulQueueSize = LockSpin(xm_ulQueueSize);
	}
}
void UserMutex::Unlock() noexcept {
	ASSERT(IsLockedByCurrentThread());

	__atomic_store_n(&xm_ulLockingThreadId, 0, __ATOMIC_RELEASE);

	auto ulQueueSize = LockSpin(xm_ulQueueSize);
	if(EXPECT(ulQueueSize != 0)){
		--ulQueueSize;
		xm_vSemaphore.Post();
	}
	UnlockSpin(xm_ulQueueSize, ulQueueSize);
}

UserMutex::UniqueLock UserMutex::TryLock() noexcept {
	UniqueLock vLock(*this, false);
	vLock.Try();
	return std::move(vLock);
}
UserMutex::UniqueLock UserMutex::GetLock() noexcept {
	return UniqueLock(*this);
}
