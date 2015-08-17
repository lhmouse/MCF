// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "Mutex.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

// http://research.microsoft.com/pubs/64242/ImplementingCVs.pdf
// 因为 Semaphore 现在维护一个大体上 FIFO 的顺序，我们就没必要操心了。

// 构造函数和析构函数。
ConditionVariable::ConditionVariable(Mutex &vMutex)
	: x_vMutex(vMutex), x_uWaiting(0), x_vSemaphore(0, nullptr)
{
}

// 其他非静态成员函数。
bool ConditionVariable::Wait(std::uint64_t u64MilliSeconds) noexcept {
	x_uWaiting.Increment(MemoryModel::kRelaxed);
	x_vMutex.Unlock();

	const bool bResult = x_vSemaphore.Wait(u64MilliSeconds);

	x_vMutex.Lock();
	if(!bResult){
		x_uWaiting.Decrement(MemoryModel::kRelaxed);
	}
	return bResult;
}
void ConditionVariable::Wait() noexcept {
	x_uWaiting.Increment(MemoryModel::kRelaxed);
	x_vMutex.Unlock();

	x_vSemaphore.Wait();

	x_vMutex.Lock();
}
void ConditionVariable::Signal(std::size_t uMaxCount) noexcept {
	std::size_t uToPost;
	std::size_t uOld = x_uWaiting.Load(MemoryModel::kRelaxed), uNew;
	do {
		if(uOld >= uMaxCount){
			uToPost = uMaxCount;
			uNew = uOld - uMaxCount;
		} else {
			uToPost = uOld;
			uNew = 0;
		}
	} while(!x_uWaiting.CompareExchange(uOld, uNew, MemoryModel::kRelaxed));

	if(uToPost != 0){
		x_vSemaphore.Post(uToPost);
	}
}
void ConditionVariable::Broadcast() noexcept {
	const auto uToPost = x_uWaiting.Exchange(0, MemoryModel::kRelaxed);

	if(uToPost != 0){
		x_vSemaphore.Post(uToPost);
	}
}

}
