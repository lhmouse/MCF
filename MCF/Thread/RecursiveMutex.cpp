// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RecursiveMutex.hpp"

namespace MCF {

template<>
bool RecursiveMutex::UniqueLock::XDoTry() const noexcept {
	return x_pOwner->Try() != RecursiveMutex::Result::kResTryFailed;
}
template<>
void RecursiveMutex::UniqueLock::XDoLock() const noexcept {
	x_pOwner->Lock();
}
template<>
void RecursiveMutex::UniqueLock::XDoUnlock() const noexcept {
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
	return x_uLockingThreadId.Load(kAtomicConsume) == dwThreadId;
}

RecursiveMutex::Result RecursiveMutex::Try() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if(x_uLockingThreadId.Load(kAtomicConsume) == dwThreadId){
		++x_uRecursionCount;
		return kResRecursive;
	}
	if(!x_vMutex.Try()){
		return kResTryFailed;
	}
	x_uLockingThreadId.Store(dwThreadId, kAtomicRelease);
	++x_uRecursionCount;
	return kResStateChanged;
}
RecursiveMutex::Result RecursiveMutex::Lock() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if(x_uLockingThreadId.Load(kAtomicConsume) == dwThreadId){
		++x_uRecursionCount;
		return kResRecursive;
	}
	x_vMutex.Lock();
	x_uLockingThreadId.Store(dwThreadId, kAtomicRelease);
	++x_uRecursionCount;
	return kResStateChanged;
}
RecursiveMutex::Result RecursiveMutex::Unlock() noexcept {
	ASSERT_MSG(IsLockedByCurrentThread(), L"试图使用当前不持有递归互斥体的线程释放递归互斥体。");

	if(--x_uRecursionCount != 0){
		return kResRecursive;
	}
	x_uLockingThreadId.Store(0, kAtomicRelaxed);
	x_vMutex.Unlock();
	return kResStateChanged;
}

}
