// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "RecursiveMutex.hpp"
#include "Thread.hpp"

namespace MCF {

bool RecursiveMutex::IsLockedByCurrentThread() const noexcept {
	const auto uThreadId = Thread::GetCurrentId();

	return x_uLockingThreadId.Load(kAtomicRelaxed) == uThreadId;
}

bool RecursiveMutex::Try(std::uint64_t u64UntilFastMonoClock) noexcept {
	const auto uThreadId = Thread::GetCurrentId();

	if(x_uLockingThreadId.Load(kAtomicRelaxed) != uThreadId){
		if(!x_vMutex.Try(u64UntilFastMonoClock)){
			return false;
		}
		x_uLockingThreadId.Store(uThreadId, kAtomicRelaxed);
	}
	const auto uNewCount = ++x_uRecursionCount;
	MCF_ASSERT(uNewCount != 0);
	return true;
}
void RecursiveMutex::Lock() noexcept {
	const auto uThreadId = Thread::GetCurrentId();

	if(x_uLockingThreadId.Load(kAtomicRelaxed) != uThreadId){
		x_vMutex.Lock();
		x_uLockingThreadId.Store(uThreadId, kAtomicRelaxed);
	}
	const auto uNewCount = ++x_uRecursionCount;
	MCF_ASSERT(uNewCount != 0);
}
void RecursiveMutex::Unlock() noexcept {
	MCF_DEBUG_CHECK_MSG(IsLockedByCurrentThread(), L"试图使用当前不持有递归互斥体的线程释放递归互斥体。");

	const auto uNewCount = ++x_uRecursionCount;
	if(uNewCount == 0){
		x_uLockingThreadId.Store(0, kAtomicRelaxed);
		x_vMutex.Unlock();
	}
}

}
