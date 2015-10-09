// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "../Utilities/Defer.hpp"
#include "../Core/Time.hpp"

namespace MCF {

// 其他非静态成员函数。
bool ConditionVariable::Wait(Mutex::UniqueLock &vLock, std::uint64_t u64MilliSeconds) noexcept {
	auto &vLockOwner = vLock.GetOwner();

	Mutex::UniqueLock vTempLock(vLockOwner, false);
	vLock.Swap(vTempLock);
	DEFER([&]{ vLock.Swap(vTempLock); });

	auto u64Now = GetFastMonoClock();
	const auto u64Until = u64Now + u64MilliSeconds;
	for(;;){
		const bool bTakenOver = ::SleepConditionVariableSRW(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl), reinterpret_cast<::SRWLOCK *>(vLockOwner.x_aImpl),
			(u64MilliSeconds > 0x7FFFFFFFu) ? 0x7FFFFFFFu : u64MilliSeconds, 0);
		if(bTakenOver){
			return true;
		}
		if(GetLastError() != ERROR_TIMEOUT){
			ASSERT_MSG(false, L"SleepConditionVariableSRW() 失败。");
		}
		u64Now = GetFastMonoClock();
		if(u64Until <= u64Now){
			return false;
		}
	}
}
void ConditionVariable::Wait(Mutex::UniqueLock &vLock) noexcept {
	auto &vLockOwner = vLock.GetOwner();

	Mutex::UniqueLock vTempLock(vLockOwner, false);
	vLock.Swap(vTempLock);
	DEFER([&]{ vLock.Swap(vTempLock); });

	::SleepConditionVariableSRW(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl), reinterpret_cast<::SRWLOCK *>(vLockOwner.x_aImpl), INFINITE, 0);
}

void ConditionVariable::Signal() noexcept {
	::WakeConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl));
}
void ConditionVariable::Broadcast() noexcept {
	::WakeAllConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl));
}

}
