// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RecursiveMutex.hpp"

namespace MCF {

namespace Impl_UniqueLockTemplate {
	template<>
	bool RecursiveMutex::UniqueLock::X_DoTry() const noexcept {
		return x_pOwner->Try() != RecursiveMutex::Result::kTryFailed;
	}
	template<>
	void RecursiveMutex::UniqueLock::X_DoLock() const noexcept {
		x_pOwner->Lock();
	}
	template<>
	void RecursiveMutex::UniqueLock::X_DoUnlock() const noexcept {
		x_pOwner->Unlock();
	}
}

// 构造函数和析构函数。
RecursiveMutex::RecursiveMutex(std::size_t uSpinCount)
	: x_vMutex(uSpinCount)
	, x_uLockingThreadId(0), x_uRecursionCount(0)
{
}

// 其他非静态成员函数。
bool RecursiveMutex::IsLockedByCurrentThread() const noexcept {
	const std::size_t uThreadId = ::GetCurrentThreadId();
	return x_uLockingThreadId.Load(kAtomicConsume) == uThreadId;
}

RecursiveMutex::Result RecursiveMutex::Try() noexcept {
	const std::size_t uThreadId = ::GetCurrentThreadId();
	if(x_uLockingThreadId.Load(kAtomicConsume) == uThreadId){
		++x_uRecursionCount;
		return kRecursive;
	}
	if(!x_vMutex.Try()){
		return kTryFailed;
	}
	x_uLockingThreadId.Store(uThreadId, kAtomicRelease);
	++x_uRecursionCount;
	return kStateChanged;
}
RecursiveMutex::Result RecursiveMutex::Lock() noexcept {
	const std::size_t uThreadId = ::GetCurrentThreadId();
	if(x_uLockingThreadId.Load(kAtomicConsume) == uThreadId){
		++x_uRecursionCount;
		return kRecursive;
	}
	x_vMutex.Lock();
	x_uLockingThreadId.Store(uThreadId, kAtomicRelease);
	++x_uRecursionCount;
	return kStateChanged;
}
RecursiveMutex::Result RecursiveMutex::Unlock() noexcept {
	ASSERT_MSG(IsLockedByCurrentThread(), L"试图使用当前不持有递归互斥体的线程释放递归互斥体。");

	if(--x_uRecursionCount != 0){
		return kRecursive;
	}
	x_uLockingThreadId.Store(0, kAtomicRelaxed);
	x_vMutex.Unlock();
	return kStateChanged;
}

}
