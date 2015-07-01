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
	, x_uQueueSize(0), x_uLockingThreadId(0)
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

	const auto uSpinCount = GetSpinCount();
	const auto dwThreadId = ::GetCurrentThreadId();

	const auto uQueueSize = AtomicIncrement(x_uQueueSize, MemoryModel::kRelaxed);
	if(uQueueSize == 1){
		if(TryMutexWithHint(x_uLockingThreadId, uSpinCount, dwThreadId)){
			return true;
		}
	}
	AtomicDecrement(x_uQueueSize, MemoryModel::kRelaxed);
	return false;
}
void Mutex::Lock() noexcept {
	ASSERT(!IsLockedByCurrentThread());

	const auto uSpinCount = GetSpinCount();
	const auto dwThreadId = ::GetCurrentThreadId();

	const auto uQueueSize = AtomicIncrement(x_uQueueSize, MemoryModel::kRelaxed);
	if(uQueueSize == 1){
		if(TryMutexWithHint(x_uLockingThreadId, uSpinCount, dwThreadId)){
			return;
		}
	}
	for(;;){
		x_vSemaphore.Wait();
		if(TryMutexWithHint(x_uLockingThreadId, uSpinCount, dwThreadId)){
			return;
		}
		x_vSemaphore.Post();
	}
}
void Mutex::Unlock() noexcept {
	ASSERT(IsLockedByCurrentThread());

	AtomicStore(x_uLockingThreadId, 0, MemoryModel::kRelaxed);
	const auto uQueueSize = AtomicDecrement(x_uQueueSize, MemoryModel::kRelaxed);
	if(uQueueSize != 0){
		x_vSemaphore.Post();
	}
}

}
