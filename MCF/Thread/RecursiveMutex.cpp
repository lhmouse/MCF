// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RecursiveMutex.hpp"
#include "Thread.hpp"

namespace MCF {

// 其他非静态成员函数。
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
	++x_uRecursionCount;
	return true;
}
void RecursiveMutex::Lock() noexcept {
	const auto uThreadId = Thread::GetCurrentId();

	if(x_uLockingThreadId.Load(kAtomicRelaxed) != uThreadId){
		x_vMutex.Lock();
		x_uLockingThreadId.Store(uThreadId, kAtomicRelaxed);
	}
	++x_uRecursionCount;
}
void RecursiveMutex::Unlock() noexcept {
	ASSERT_MSG(IsLockedByCurrentThread(), L"试图使用当前不持有递归互斥体的线程释放递归互斥体。");

	--x_uRecursionCount;
	if(x_uRecursionCount == 0){
		x_uLockingThreadId.Store(0, kAtomicRelaxed);
		x_vMutex.Unlock();
	}
}

}
