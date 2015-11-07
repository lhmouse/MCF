// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Event.hpp"

namespace MCF {

// 其他非静态成员函数。
bool Event::Wait(std::uint64_t u64UntilFastMonoClock) const noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	while(!x_bSet){
		if(!x_cvWaiter.Wait(vLock, u64UntilFastMonoClock)){
			return false;
		}
	}
	return true;
}
void Event::Wait() const noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	while(!x_bSet){
		x_cvWaiter.Wait(vLock);
	}
}
bool Event::IsSet() const noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	return x_bSet;
}
bool Event::Set() noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	const auto bOld = x_bSet;
	x_bSet = true;
	x_cvWaiter.Broadcast();
	return bOld;
}
bool Event::Reset() noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	const auto bOld = x_bSet;
	x_bSet = false;
	return bOld;
}

}
