// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Semaphore.hpp"

namespace MCF {

bool Semaphore::Wait(std::uint64_t u64UntilFastMonoClock) noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	while(x_uCount == 0){
		if(!x_cvWaiter.Wait(vLock, u64UntilFastMonoClock)){
			return false;
		}
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
		MCF_ASSERT_MSG(false, L"算术运算结果超出可表示范围。");
	}
	x_uCount = uNewCount;
	x_cvWaiter.Signal(uPostCount);
	return uOldCount;
}

}
