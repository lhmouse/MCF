// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

// http://research.microsoft.com/pubs/64242/ImplementingCVs.pdf
// 因为 Semaphore 现在维护一个大体上 FIFO 的顺序，我们就没必要操心了。

// 构造函数和析构函数。
ConditionVariable::ConditionVariable() noexcept
	: x_mtxGuard()
{
	static_assert(sizeof(x_aImpl) == sizeof(::CONDITION_VARIABLE), "!");

	::InitializeConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl));
}

// 其他非静态成员函数。
bool ConditionVariable::X_SleepOnMutex(Mutex &vMutex, std::uint64_t u64MilliSeconds) noexcept {
	auto u64Now = GetFastMonoClock();
	const auto u64Until = u64Now + u64MilliSeconds;
	for(;;){
		const bool bTakenOver = ::SleepConditionVariableSRW(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl), reinterpret_cast<::SRWLOCK *>(vMutex.x_aImpl), Min(u64MilliSeconds, 0x7FFFFFFFu), 0);
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
bool ConditionVariable::X_SleepOnMutex(Mutex &vMutex) noexcept {
	const bool bTakenOver = ::SleepConditionVariableSRW(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl), reinterpret_cast<::SRWLOCK *>(vMutex.x_aImpl), INFINITE, 0);
	return bTakenOver;
}

bool ConditionVariable::Wait(Mutex::UniqueLock &vLock, std::uint64_t u64MilliSeconds) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	const bool bTakenOver = X_SleepOnMutex(vLock.GetOwner(), u64MilliSeconds);
	return bTakenOver;
}
void ConditionVariable::Wait(Mutex::UniqueLock &vLock) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	X_SleepOnMutex(vLock.GetOwner());
}
bool ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64MilliSeconds) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	x_mtxGuard.Lock();
	vLock.Unlock();
	const bool bTakenOver = X_SleepOnMutex(x_mtxGuard, u64MilliSeconds);
	x_mtxGuard.Unlock();
	vLock.Lock();
	return bTakenOver;
}
void ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	x_mtxGuard.Lock();
	vLock.Unlock();
	X_SleepOnMutex(x_mtxGuard);
	x_mtxGuard.Unlock();
	vLock.Lock();
}
void ConditionVariable::Signal() noexcept {
	::WakeConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl));
}
void ConditionVariable::Broadcast() noexcept {
	::WakeAllConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(x_aImpl));
}

}
