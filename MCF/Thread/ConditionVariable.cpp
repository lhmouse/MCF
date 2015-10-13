// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "../Utilities/Defer.hpp"
#include "../Core/Time.hpp"
#include <winternl.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;

namespace MCF {

// 其他非静态成员函数。
bool ConditionVariable::Wait(Mutex::UniqueLock &vLock, std::uint64_t u64MilliSeconds) noexcept {
	auto &vMutex = vLock.GetOwner();
	auto vTemp = std::move(vLock);
	DEFER([&]{ vLock = std::move(vTemp); });

	auto u64Now = GetFastMonoClock();
	const auto u64Until = u64Now + u64MilliSeconds;
	for(;;){
		const bool bTakenOver = ::SleepConditionVariableSRW(reinterpret_cast<::CONDITION_VARIABLE *>(&x_uImpl), reinterpret_cast<::SRWLOCK *>(&vMutex.x_uImpl),
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
	auto &vMutex = vLock.GetOwner();
	auto vTemp = std::move(vLock);
	DEFER([&]{ vLock = std::move(vTemp); });

	::SleepConditionVariableSRW(reinterpret_cast<::CONDITION_VARIABLE *>(&x_uImpl), reinterpret_cast<::SRWLOCK *>(&vMutex.x_uImpl), INFINITE, 0);
}

void ConditionVariable::Signal() noexcept {
	::WakeConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(&x_uImpl));
}
void ConditionVariable::Broadcast() noexcept {
	::WakeAllConditionVariable(reinterpret_cast<::CONDITION_VARIABLE *>(&x_uImpl));
}

}
