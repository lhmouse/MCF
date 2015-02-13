// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/System.hpp"

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

// 构造函数和析构函数。
Mutex::Mutex(std::size_t uSpinCount)
	: xm_vSemaphore(0), xm_uSpinCount(0)
	, xm_splQueueSize(0), xm_uLockingThreadId(0)
{
	SetSpinCount(uSpinCount);

	AtomicFence(MemoryModel::RELEASE);
}

// 其他非静态成员函数。
bool Mutex::xTryWithHint(unsigned long ulThreadId) noexcept {
	ASSERT(!IsLockedByCurrentThread());

	std::size_t i = GetSpinCount();
	for(;;){
		std::size_t uExpected = 0;
		if(EXPECT_NOT(AtomicCompareExchange(xm_uLockingThreadId, uExpected, ulThreadId, MemoryModel::ACQ_REL))){
			return true;
		}
		if(EXPECT_NOT(i == 0)){
			return false;
		}
		--i;
		AtomicPause();
	}
}

void Mutex::SetSpinCount(std::size_t uSpinCount) noexcept {
	if(GetProcessorCount() == 0){
		return;
	}
	AtomicStore(xm_uSpinCount, uSpinCount, MemoryModel::RELAXED);
}

bool Mutex::IsLockedByCurrentThread() const noexcept {
	return AtomicLoad(xm_uLockingThreadId, MemoryModel::ACQUIRE) == ::GetCurrentThreadId();
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

	AtomicStore(xm_uLockingThreadId, 0, MemoryModel::RELEASE);

	auto uQueueSize = xm_splQueueSize.Lock();
	if(EXPECT(uQueueSize != 0)){
		--uQueueSize;
		xm_vSemaphore.Post();
	}
	xm_splQueueSize.Unlock(uQueueSize);
}

}
