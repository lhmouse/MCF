// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/System.hpp"
using namespace MCF;

namespace MCF {

template<>
bool Mutex::UniqueLock::xDoTry() const noexcept {
	return xm_pOwner->Try();
}
template<>
void Mutex::UniqueLock::xDoLock() const noexcept {
	xm_pOwner->Lock();
}
template<>
void Mutex::UniqueLock::xDoUnlock() const noexcept {
	xm_pOwner->Unlock();
}

}

// 构造函数和析构函数。
Mutex::Mutex(std::size_t uSpinCount)
	: xm_vSemaphore(0), xm_uSpinCount(uSpinCount)
	, xm_splQueueSize(0), xm_uLockingThreadId(0)
{
	__atomic_thread_fence(__ATOMIC_RELEASE);
}

// 其他非静态成员函数。
bool Mutex::xTryWithHint(unsigned long ulThreadId) noexcept {
	ASSERT(!IsLockedByCurrentThread());

	std::size_t i = 1;
	if(EXPECT(GetProcessorCount() != 0)){
		i += GetSpinCount();
	}
	for(;;){
		std::size_t uExpected = 0;
		if(EXPECT_NOT(__atomic_compare_exchange_n(
			&xm_uLockingThreadId, &uExpected, ulThreadId, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)))
		{
			return true;
		}
		if(EXPECT_NOT(--i == 0)){
			return false;
		}
		__builtin_ia32_pause();
	}
}

bool Mutex::IsLockedByCurrentThread() const noexcept {
	return __atomic_load_n(&xm_uLockingThreadId, __ATOMIC_ACQUIRE) == ::GetCurrentThreadId();
}

bool Mutex::Try() noexcept {
	ASSERT(!IsLockedByCurrentThread());

	return xTryWithHint(::GetCurrentThreadId());
}
void Mutex::Lock() noexcept {
	ASSERT(!IsLockedByCurrentThread());

	const auto dwThreadId = ::GetCurrentThreadId();

	auto uQueueSize = xm_splQueueSize.Lock();
	if(EXPECT(uQueueSize == 0)){
		xm_splQueueSize.Unlock(uQueueSize);
		if(EXPECT(xTryWithHint(dwThreadId))){
			return;
		}
		uQueueSize = xm_splQueueSize.Lock();
	}
	for(;;){
		++uQueueSize;
		xm_splQueueSize.Unlock(uQueueSize);
		xm_vSemaphore.Wait();

		if(EXPECT_NOT(xTryWithHint(dwThreadId))){
			return;
		}
		uQueueSize = xm_splQueueSize.Lock();
	}
}
void Mutex::Unlock() noexcept {
	ASSERT(IsLockedByCurrentThread());

	__atomic_store_n(&xm_uLockingThreadId, 0, __ATOMIC_RELEASE);

	auto uQueueSize = xm_splQueueSize.Lock();
	if(EXPECT(uQueueSize != 0)){
		--uQueueSize;
		xm_vSemaphore.Post();
	}
	xm_splQueueSize.Unlock(uQueueSize);
}

Mutex::UniqueLock Mutex::TryLock() noexcept {
	UniqueLock vLock(*this, false);
	vLock.Try();
	return std::move(vLock);
}
Mutex::UniqueLock Mutex::GetLock() noexcept {
	return UniqueLock(*this);
}
