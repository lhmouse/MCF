// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RecursiveMutex.hpp"
#include "Thread.hpp"

namespace MCF {

// 其他非静态成员函数。
bool RecursiveMutex::IsLockedByCurrentThread() const noexcept {
	const auto uThreadId = Thread::GetCurrentId();

	return x_uLockingThreadId.Load(kAtomicConsume) == uThreadId;
}

bool RecursiveMutex::Try() noexcept {
	const auto uThreadId = Thread::GetCurrentId();

	if(x_uLockingThreadId.Load(kAtomicConsume) == uThreadId){
		++x_uRecursionCount;
		return true;
	}
	if(!x_vMutex.Try()){
		return false;
	}
	x_uLockingThreadId.Store(uThreadId, kAtomicRelease);
	++x_uRecursionCount;
	return true;
}
void RecursiveMutex::Lock() noexcept {
	const auto uThreadId = Thread::GetCurrentId();

	if(x_uLockingThreadId.Load(kAtomicConsume) == uThreadId){
		++x_uRecursionCount;
		return;
	}
	x_vMutex.Lock();
	x_uLockingThreadId.Store(uThreadId, kAtomicRelease);
	++x_uRecursionCount;
}
void RecursiveMutex::Unlock() noexcept {
	ASSERT_MSG(IsLockedByCurrentThread(), L"试图使用当前不持有递归互斥体的线程释放递归互斥体。");

	--x_uRecursionCount;
	if(x_uRecursionCount != 0){
		return;
	}
	x_uLockingThreadId.Store(0, kAtomicRelaxed);
	x_vMutex.Unlock();
}

}
