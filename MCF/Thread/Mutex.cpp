// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/System.hpp"

namespace MCF {

namespace {
	bool TryMutexWithHint(volatile std::size_t &uLockingThreadId, std::size_t uSpinCount, unsigned long ulThreadId) noexcept {
		std::size_t i = uSpinCount;
		for(;;){
			std::size_t uExpected = 0;
			if(EXPECT_NOT(AtomicCompareExchange(uLockingThreadId, uExpected, ulThreadId, MemoryModel::kSeqCst, MemoryModel::kSeqCst))){
				return true;
			}
			if(EXPECT_NOT(i == 0)){
				return false;
			}
			--i;
			AtomicPause();
		}
	}
}

template<>
bool Mutex::UniqueLock::xDoTry() const noexcept {
	return x_pOwner->Try();
}
template<>
void Mutex::UniqueLock::xDoLock() const noexcept {
	x_pOwner->Lock();
}
template<>
void Mutex::UniqueLock::xDoUnlock() const noexcept {
	x_pOwner->Unlock();
}

// 构造函数和析构函数。
Mutex::Mutex(std::size_t uSpinCount)
	: x_vSemaphore(0), x_uSpinCount(0)
	, x_splQueueSize(0), x_uLockingThreadId(0)
{
	SetSpinCount(uSpinCount);

	AtomicFence(MemoryModel::kRelease);
}

// 其他非静态成员函数。
void Mutex::SetSpinCount(std::size_t uSpinCount) noexcept {
	if(GetProcessorCount() == 0){
		return;
	}
	AtomicStore(x_uSpinCount, uSpinCount, MemoryModel::kRelaxed);
}

bool Mutex::IsLockedByCurrentThread() const noexcept {
	return AtomicLoad(x_uLockingThreadId, MemoryModel::kRelaxed) == ::GetCurrentThreadId();
}

bool Mutex::Try() noexcept {
	ASSERT(!IsLockedByCurrentThread());

	const auto dwThreadId = ::GetCurrentThreadId();

	return TryMutexWithHint(x_uLockingThreadId, x_uSpinCount, dwThreadId);
}
void Mutex::Lock() noexcept {
	ASSERT(!IsLockedByCurrentThread());

	const auto dwThreadId = ::GetCurrentThreadId();

	auto uQueueSize = x_splQueueSize.Lock();
	if(EXPECT(uQueueSize == 0)){
		x_splQueueSize.Unlock(uQueueSize);
		if(EXPECT(TryMutexWithHint(x_uLockingThreadId, x_uSpinCount, dwThreadId))){
			return;
		}
		uQueueSize = x_splQueueSize.Lock();
	}
	for(;;){
		++uQueueSize;
		x_splQueueSize.Unlock(uQueueSize);
		x_vSemaphore.Wait();

		if(EXPECT_NOT(TryMutexWithHint(x_uLockingThreadId, x_uSpinCount, dwThreadId))){
			return;
		}
		uQueueSize = x_splQueueSize.Lock();
	}
}
void Mutex::Unlock() noexcept {
	ASSERT(IsLockedByCurrentThread());

	AtomicStore(x_uLockingThreadId, 0, MemoryModel::kRelease);

	auto uQueueSize = x_splQueueSize.Lock();
	if(EXPECT(uQueueSize != 0)){
		--uQueueSize;
		x_vSemaphore.Post();
	}
	x_splQueueSize.Unlock(uQueueSize);
}

}
