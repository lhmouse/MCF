// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RecursiveMutex.hpp"
#include "Atomic.hpp"

namespace MCF {

template<>
bool RecursiveMutex::UniqueLock::xDoTry() const noexcept {
	return x_pOwner->Try() != RecursiveMutex::Result::kResTryFailed;
}
template<>
void RecursiveMutex::UniqueLock::xDoLock() const noexcept {
	x_pOwner->Lock();
}
template<>
void RecursiveMutex::UniqueLock::xDoUnlock() const noexcept {
	x_pOwner->Unlock();
}

// 构造函数和析构函数。
RecursiveMutex::RecursiveMutex(std::size_t uSpinCount)
	: x_vMutex(uSpinCount)
	, x_uLockingThreadId(0), x_uRecursionCount(0)
{
}

// 其他非静态成员函数。
bool RecursiveMutex::IsLockedByCurrentThread() const noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	return AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == dwThreadId;
}

RecursiveMutex::Result RecursiveMutex::Try() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if(AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == dwThreadId){
		++x_uRecursionCount;
		return kResRecursive;
	}
	if(!x_vMutex.Try()){
		return kResTryFailed;
	}
	AtomicStore(x_uLockingThreadId, dwThreadId, MemoryModel::kRelease);
	++x_uRecursionCount;
	return kResStateChanged;
}
RecursiveMutex::Result RecursiveMutex::Lock() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if(AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == dwThreadId){
		++x_uRecursionCount;
		return kResRecursive;
	}
	x_vMutex.Lock();
	AtomicStore(x_uLockingThreadId, dwThreadId, MemoryModel::kRelease);
	++x_uRecursionCount;
	return kResStateChanged;
}
RecursiveMutex::Result RecursiveMutex::Unlock() noexcept {
	ASSERT(IsLockedByCurrentThread());

	if(--x_uRecursionCount != 0){
		return kResRecursive;
	}
	AtomicStore(x_uLockingThreadId, 0, MemoryModel::kRelaxed);
	x_vMutex.Unlock();
	return kResStateChanged;
}

}
