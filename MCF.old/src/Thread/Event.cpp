// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "Event.hpp"

namespace MCF {

bool Event::Wait(std::uint64_t u64UntilFastMonoClock) const noexcept {
	auto vLock = x_mtxGuard.GetLock();
	while(!x_bSet){
		if(!x_cvWaiter.WaitOrAbandon(vLock, u64UntilFastMonoClock)){
			return false;
		}
	}
	return true;
}
void Event::Wait() const noexcept {
	auto vLock = x_mtxGuard.GetLock();
	while(!x_bSet){
		x_cvWaiter.Wait(vLock);
	}
}
bool Event::IsSet() const noexcept {
	auto vLock = x_mtxGuard.GetLock();
	return x_bSet;
}
bool Event::Set() noexcept {
	auto vLock = x_mtxGuard.GetLock();
	const auto bOld = x_bSet;
	x_bSet = true;
	x_cvWaiter.Broadcast();
	return bOld;
}
bool Event::Reset() noexcept {
	auto vLock = x_mtxGuard.GetLock();
	const auto bOld = x_bSet;
	x_bSet = false;
	return bOld;
}

}
