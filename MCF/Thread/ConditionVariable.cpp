// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

// 构造函数和析构函数。
ConditionVariable::ConditionVariable() noexcept
	: x_mtxGuard()
{
	static_assert(sizeof(x_aImpl) == sizeof(::CONDITION_VARIABLE), "!");

	::InitializeConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl));
}

// 其他非静态成员函数。
bool ConditionVariable::Wait(Mutex::UniqueLock &vLock, std::uint64_t u64MilliSeconds) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	auto u64Now = GetFastMonoClock();
	const auto u64Until = u64Now + u64MilliSeconds;
	for(;;){
		const bool bTakenOver = ::SleepConditionVariableSRW(
			reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl), reinterpret_cast<::SRWLOCK *>(vLock.GetOwner().x_aImpl), Min(u64MilliSeconds, 0x7FFFFFFFu), 0);
		if(bTakenOver){
			return true;
		}
		if(GetLastError() != ERROR_TIMEOUT){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
		u64Now = GetFastMonoClock();
		if(u64Until <= u64Now){
			return false;
		}
	}
}
void ConditionVariable::Wait(Mutex::UniqueLock &vLock) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	::SleepConditionVariableSRW(
		reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl), reinterpret_cast<::SRWLOCK *>(vLock.GetOwner().x_aImpl), INFINITE, 0);
}

bool ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64MilliSeconds) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	bool bTakenOver;
	{
		Mutex::UniqueLock vGuardLock(x_mtxGuard);
		vLock.Unlock();
		bTakenOver = Wait(vGuardLock, u64MilliSeconds);
	}
	vLock.Lock();
	return bTakenOver;
}
void ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	{
		Mutex::UniqueLock vGuardLock(x_mtxGuard);
		vLock.Unlock();
		Wait(vGuardLock);
	}
	vLock.Lock();
}

void ConditionVariable::Signal() noexcept {
	::WakeConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl));
}
void ConditionVariable::Broadcast() noexcept {
	::WakeAllConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl));
}

}
