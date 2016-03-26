// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "condition_variable.h"
#include "clocks.h"
#include "../ext/assert.h"
#include <winternl.h>
#include <ntstatus.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

bool _MCFCRT_WaitForConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext, uint64_t u64UntilFastMonoClock)
{
	//
}
void _MCFCRT_WaitForConditionVariableForever(_MCFCRT_ConditionVariable *pConditionVariable,
	_MCFCRT_ConditionVariableUnlockCallback pfnUnlockCallback, _MCFCRT_ConditionVariableRelockCallback pfnRelockCallback, intptr_t nContext)
{
	//
}
size_t _MCFCRT_SignalConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable, size_t uMaxCountToWakeUp){
	//
}
size_t _MCFCRT_BroadcastConditionVariable(_MCFCRT_ConditionVariable *pConditionVariable){
	//
}


/*
// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "../Core/Clocks.hpp"
#include "../Utilities/MinMax.hpp"
#include <winternl.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;

namespace MCF {

bool ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64UntilFastMonoClock) noexcept {
	x_uControl.Increment(kAtomicRelaxed);
	const auto uCount = vLock.Y_UnlockAll();
	MCF_ASSERT_MSG(uCount != 0, L"你会用条件变量吗？");

	::LARGE_INTEGER liTimeout;
	liTimeout.QuadPart = 0;
	if(u64UntilFastMonoClock != 0){
		const auto u64Now = GetFastMonoClock();
		if(u64Now < u64UntilFastMonoClock){
			const auto u64DeltaMillisec = u64UntilFastMonoClock - u64Now;
			const auto n64Delta100Nanosec = static_cast<std::int64_t>(u64DeltaMillisec * 10000);
			if(static_cast<std::uint64_t>(n64Delta100Nanosec / 10000) != u64DeltaMillisec){
				liTimeout.QuadPart = INT64_MIN;
			} else {
				liTimeout.QuadPart = -n64Delta100Nanosec;
			}
		}
	}
	const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		MCF_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		auto uOld = x_uControl.Load(kAtomicRelaxed);
	jCasFailureTimedOut:
		auto uNew = uOld;
		if(uNew == 0){
			const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, nullptr);
			if(!NT_SUCCESS(lStatus)){
				MCF_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
			}
		} else {
			--uNew;
			if(!x_uControl.CompareExchange(uOld, uNew, kAtomicRelaxed, kAtomicRelaxed)){
				goto jCasFailureTimedOut;
			}
		}

		vLock.Y_RelockAll(uCount);
		return false;
	}

	vLock.Y_RelockAll(uCount);
	return true;
}
void ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept {
	x_uControl.Increment(kAtomicRelaxed);

	const auto uCount = vLock.Y_UnlockAll();
	MCF_ASSERT_MSG(uCount != 0, L"你会用条件变量吗？");

	const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, nullptr);
	if(!NT_SUCCESS(lStatus)){
		MCF_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
	}

	vLock.Y_RelockAll(uCount);
}

std::size_t ConditionVariable::Signal(std::size_t uMaxCountToWakeUp) noexcept {
	auto uOld = x_uControl.Load(kAtomicRelaxed);
jCasFailure:
	auto uNew = uOld;
	const auto uThreadsToWakeUp = Min(uNew, uMaxCountToWakeUp);
	if(uThreadsToWakeUp == 0){
		return 0;
	}
	uNew -= uThreadsToWakeUp;
	if(!x_uControl.CompareExchange(uOld, uNew, kAtomicRelaxed, kAtomicRelaxed)){
		goto jCasFailure;
	}

	for(std::size_t i = 0; i < uThreadsToWakeUp; ++i){
		const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			MCF_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
		}
	}
	return uThreadsToWakeUp;
}
std::size_t ConditionVariable::Broadcast() noexcept {
	const auto uThreadsToWakeUp = x_uControl.Exchange(0, kAtomicRelaxed);
	if(uThreadsToWakeUp == 0){
		return 0;
	}

	for(std::size_t i = 0; i < uThreadsToWakeUp; ++i){
		const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			MCF_ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
		}
	}
	return uThreadsToWakeUp;
}

}
*/
