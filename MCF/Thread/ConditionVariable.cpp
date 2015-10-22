// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "../Core/Time.hpp"
#include <winternl.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;

namespace MCF {

// 其他非静态成员函数。
bool ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64UntilFastMonoClock) noexcept {
	const auto uCount = vLock.X_UnlockAll();
	ASSERT_MSG(uCount != 0, L"你会用条件变量吗？");

	::LARGE_INTEGER liTimeout;
	const auto u64Now = GetFastMonoClock();
	if(u64Now >= u64UntilFastMonoClock){
		liTimeout.QuadPart = 0;
	} else {
		const auto u64DeltaMillisec = u64UntilFastMonoClock - u64Now;
		const auto n64Delta100Nanosec = static_cast<std::int64_t>(u64DeltaMillisec * 10000);
		if(static_cast<std::uint64_t>(n64Delta100Nanosec / 10000) != u64DeltaMillisec){
			liTimeout.QuadPart = INT64_MIN;
		} else {
			liTimeout.QuadPart = -n64Delta100Nanosec;
		}
	}
	x_uWaitingThreads.Increment(kAtomicRelaxed);
	const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
	}
	x_uWaitingThreads.Decrement(kAtomicRelaxed);

	vLock.X_RelockAll(uCount);
	return lStatus != STATUS_TIMEOUT;
}
void ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept {
	const auto uCount = vLock.X_UnlockAll();
	ASSERT_MSG(uCount != 0, L"你会用条件变量吗？");

	x_uWaitingThreads.Increment(kAtomicRelaxed);
	const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
	}
	x_uWaitingThreads.Decrement(kAtomicRelaxed);

	vLock.X_RelockAll(uCount);
}

namespace {
	constexpr ::LARGE_INTEGER kZeroTimeout = { };
}

std::size_t ConditionVariable::Signal(std::size_t uMaxToWakeUp) noexcept {
	const auto uToWakeUp = std::min(x_uWaitingThreads.Load(kAtomicRelaxed), uMaxToWakeUp);
	std::size_t uWokenUp = 0;
	while(uWokenUp < uToWakeUp){
		const auto lStatus = ::NtReleaseKeyedEvent(nullptr, this, false, &kZeroTimeout);
		if(!NT_SUCCESS(lStatus)){
			ASSERT_MSG(false, L"NtReleaseKeyedEvent() 失败。");
		}
		if(lStatus == STATUS_TIMEOUT){
			break;
		}
		++uWokenUp;
	}
	return uWokenUp;
}
std::size_t ConditionVariable::Broadcast() noexcept {
	return Signal(static_cast<std::size_t>(-1));
}

}
