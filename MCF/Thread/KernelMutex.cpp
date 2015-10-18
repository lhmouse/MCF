// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelMutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
#include <winternl.h>
#include <ntdef.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtCreateEvent(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes, EVENT_TYPE eEventType, BOOLEAN bInitialState) noexcept;

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtSetEvent(HANDLE hEvent, LONG *plPrevState) noexcept;

namespace MCF {

namespace {
	Impl_UniqueNtHandle::UniqueNtHandle CreateEventHandle(const WideStringView &wsvName, bool bFailIfExists){
		const auto uSize = wsvName.GetSize() * sizeof(wchar_t);
		if(uSize > UINT16_MAX){
			DEBUG_THROW(SystemError, ERROR_INVALID_PARAMETER, "The name for a kernel event is too long"_rcs);
		}
		::UNICODE_STRING ustrObjectName;
		ustrObjectName.Length        = uSize;
		ustrObjectName.MaximumLength = uSize;
		ustrObjectName.Buffer        = (PWSTR)wsvName.GetBegin();
		::OBJECT_ATTRIBUTES vObjectAttributes;
		InitializeObjectAttributes(&vObjectAttributes, &ustrObjectName, bFailIfExists ? 0 : OBJ_OPENIF, nullptr, nullptr);

		HANDLE hEvent;
		const auto lStatus = ::NtCreateEvent(&hEvent, EVENT_ALL_ACCESS, &vObjectAttributes, SynchronizationEvent, true);
		if(!NT_SUCCESS(lStatus)){
			DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtCreateEvent"_rcs);
		}
		return Impl_UniqueNtHandle::UniqueNtHandle(hEvent);
	}
}

// 构造函数和析构函数。
KernelMutex::KernelMutex()
	: x_hEvent(CreateEventHandle(nullptr, false))
{
}
KernelMutex::KernelMutex(const WideStringView &wsvName, bool bFailIfExists)
	: x_hEvent(CreateEventHandle(wsvName, bFailIfExists))
{
}

// 其他非静态成员函数。
bool KernelMutex::Try(std::uint64_t u64UntilFastMonoClock) noexcept {
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
	const auto lStatus = ::NtWaitForSingleObject(x_hEvent.Get(), false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
	return lStatus != STATUS_TIMEOUT;
}
void KernelMutex::Lock() noexcept {
	const auto lStatus = ::NtWaitForSingleObject(x_hEvent.Get(), false, nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
}
void KernelMutex::Unlock() noexcept {
	LONG lPrevState;
	const auto lStatus = ::NtSetEvent(x_hEvent.Get(), &lPrevState);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtSetEvent() 失败。");
	}
	ASSERT_MSG(lPrevState, L"互斥锁没有被任何线程锁定。");
}

}
