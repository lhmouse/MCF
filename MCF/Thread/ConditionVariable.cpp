// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "Atomic.hpp"
#include "Mutex.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

// http://research.microsoft.com/pubs/64242/ImplementingCVs.pdf
// 因为 Semaphore 现在维护一个大体上 FIFO 的顺序，我们就没必要操心了。

// 构造函数和析构函数。
ConditionVariable::ConditionVariable(Mutex &vMutex)
	: x_vMutex(vMutex), x_uWaiting(0), x_vSemaphore(0, nullptr)
{
	AtomicFence(MemoryModel::RELEASE);
}

// 其他非静态成员函数。
bool ConditionVariable::Wait(unsigned long long ullMilliSeconds) noexcept {
	ASSERT(x_vMutex.IsLockedByCurrentThread());

	++x_uWaiting;
	x_vMutex.Unlock();
//	\\ 退出临界区。
	const bool bResult = x_vSemaphore.Wait(ullMilliSeconds);
//	// 进入临界区。
	x_vMutex.Lock();
	if(!bResult){
		--x_uWaiting;
	}
	return bResult;
}
void ConditionVariable::Wait() noexcept {
	ASSERT(x_vMutex.IsLockedByCurrentThread());

	++x_uWaiting;
	x_vMutex.Unlock();
//	\\ 退出临界区。
	x_vSemaphore.Wait();
//	// 进入临界区。
	x_vMutex.Lock();
}
void ConditionVariable::Signal(std::size_t uMaxCount) noexcept {
	const bool bIsLocking = x_vMutex.IsLockedByCurrentThread();
	if(!bIsLocking){
		x_vMutex.Lock();
	}
//	// 进入临界区。
	const auto uToPost = Min(x_uWaiting, uMaxCount);
	if(uToPost != 0){
		x_vSemaphore.Post(uToPost);
		x_uWaiting -= uToPost;
	}
//	\\ 退出临界区。
	if(!bIsLocking){
		x_vMutex.Unlock();
	}
}
void ConditionVariable::Broadcast() noexcept {
	const bool bIsLocking = x_vMutex.IsLockedByCurrentThread();
	if(!bIsLocking){
		x_vMutex.Lock();
	}
//	// 进入临界区。
	if(x_uWaiting != 0){
		x_vSemaphore.Post(x_uWaiting);
		x_uWaiting = 0;
	}
//	\\ 退出临界区。
	if(!bIsLocking){
		x_vMutex.Unlock();
	}
}

}
