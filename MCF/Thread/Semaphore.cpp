// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Semaphore.hpp"
#include "../Core/Time.hpp"

namespace MCF {

// 构造函数和析构函数。
Semaphore::Semaphore(std::size_t uInitCount) noexcept
	: x_mtxGuard(), x_cvWaiter(), x_uCount(uInitCount)
{
}

// 其他非静态成员函数。
bool Semaphore::Wait(std::uint64_t u64MilliSeconds) noexcept {
	auto u64Now = GetFastMonoClock();
	const auto u64Until = u64Now + u64MilliSeconds;

	Mutex::UniqueLock vLock(x_mtxGuard);
	while(x_uCount == 0){
		if(u64Until <= u64Now){
			return false;
		}
		if(!x_cvWaiter.Wait(vLock, u64Until - u64Now)){
			return false;
		}
		u64Now = GetFastMonoClock();
	}
	--x_uCount;
	return true;
}
void Semaphore::Wait() noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	while(x_uCount == 0){
		x_cvWaiter.Wait(vLock);
	}
	--x_uCount;
}
std::size_t Semaphore::Post(std::size_t uPostCount) noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	const auto uOldCount = x_uCount;
	const auto uNewCount = uOldCount + uPostCount;
	if(uNewCount < uOldCount){
		ASSERT_MSG(false, L"算术运算结果超出可表示范围。");
	}
	switch(uNewCount - uOldCount){
	case 0:
		break;
	case 1:
		x_cvWaiter.Signal();
		x_uCount = uNewCount;
		break;
	default:
		x_cvWaiter.Broadcast();
		x_uCount = uNewCount;
		break;
	}
	return uOldCount;
}

}
