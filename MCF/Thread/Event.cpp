// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Event.hpp"
#include "../Core/Time.hpp"

namespace MCF {

// 构造函数和析构函数。
Event::Event(bool bInitSet) noexcept
	: x_mtxGuard(), x_cvWaiter(), x_bSet(bInitSet)
{
}

// 其他非静态成员函数。
bool Event::Wait(std::uint64_t u64MilliSeconds) const noexcept {
	auto u64Now = GetFastMonoClock();
	const auto u64Until = u64Now + u64MilliSeconds;

	Mutex::UniqueLock vLock(x_mtxGuard);
	if(!x_bSet){
		for(;;){
			if(u64Until <= u64Now){
				return false;
			}
			if(!x_cvWaiter.Wait(vLock, u64Until - u64Now)){
				return false;
			}
			if(x_bSet){
				break;
			}
			u64Now = GetFastMonoClock();
		}
	}
	return true;
}
void Event::Wait() const noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	if(!x_bSet){
		for(;;){
			x_cvWaiter.Wait(vLock);
			if(x_bSet){
				break;
			}
		}
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
