// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UserMutex.hpp"
#include "../Core/System.hpp"
using namespace MCF;

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
	, xm_ulLockingThreadId(0)
{
	__atomic_thread_fence(__ATOMIC_RELEASE);
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
		__builtin_ia32_pause();
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

	auto ulQueueSize = xm_splQueueSize.Lock();
	if(EXPECT(ulQueueSize == 0)){
		xm_splQueueSize.Unlock(ulQueueSize);
		if(xTryWithHint(dwThreadId)){
			return;
		}
		ulQueueSize = xm_splQueueSize.Lock();
	}
	for(;;){
		++ulQueueSize;
		xm_splQueueSize.Unlock(ulQueueSize);
		xm_vSemaphore.Wait();

		if(EXPECT_NOT(xTryWithHint(dwThreadId))){
			break;
		}
		ulQueueSize = xm_splQueueSize.Lock();
	}
}
void UserMutex::Unlock() noexcept {
	ASSERT(IsLockedByCurrentThread());

	__atomic_store_n(&xm_ulLockingThreadId, 0, __ATOMIC_RELEASE);

	auto ulQueueSize = xm_splQueueSize.Lock();
	if(EXPECT(ulQueueSize != 0)){
		--ulQueueSize;
		xm_vSemaphore.Post();
	}
	xm_splQueueSize.Unlock(ulQueueSize);
}

UserMutex::UniqueLock UserMutex::TryLock() noexcept {
	UniqueLock vLock(*this, false);
	vLock.Try();
	return std::move(vLock);
}
UserMutex::UniqueLock UserMutex::GetLock() noexcept {
	return UniqueLock(*this);
}
