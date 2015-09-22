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
	while(!x_bSet){
		if(u64Until <= u64Now){
			return false;
		}
		if(!x_cvWaiter.Wait(vLock, u64Until - u64Now)){
			return false;
		}
		u64Now = GetFastMonoClock();
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
	return Wait(0);
}
void Event::Set() noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	x_bSet = true;
	x_cvWaiter.Broadcast();
}
void Event::Reset() noexcept {
	Mutex::UniqueLock vLock(x_mtxGuard);
	x_bSet = false;
}

}
