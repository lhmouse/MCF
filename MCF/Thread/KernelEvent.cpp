// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelEvent.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
#include <winternl.h>
#include <ntdef.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtOpenEvent(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtCreateEvent(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes, EVENT_TYPE eEventType, BOOLEAN bInitialState) noexcept;

struct EventBasicInformation {
	EVENT_TYPE eEventType;
	LONG lState;
};

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtQueryEvent(HANDLE hEvent, int nInfoClass, void *pInfo, DWORD dwInfoSize, DWORD *pdwInfoSizeRet) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtSetEvent(HANDLE hEvent, LONG *plPrevState) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtResetEvent(HANDLE hEvent, LONG *plPrevState) noexcept;

namespace MCF {

Impl_UniqueNtHandle::UniqueNtHandle KernelEvent::X_CreateEventHandle(bool bInitSet, const WideStringView &wsvName, std::uint32_t u32Flags){
	const auto uSize = wsvName.GetSize() * sizeof(wchar_t);
	if(uSize > UINT16_MAX){
		DEBUG_THROW(SystemError, ERROR_INVALID_PARAMETER, "The name for a kernel event is too long"_rcs);
	}
	::UNICODE_STRING ustrObjectName;
	ustrObjectName.Length        = uSize;
	ustrObjectName.MaximumLength = uSize;
	ustrObjectName.Buffer        = (PWSTR)wsvName.GetBegin();

	ULONG ulAttributes;
	if(u32Flags & kDontCreate){
		ulAttributes = OBJ_OPENIF;
	} else {
		if(u32Flags & kFailIfExists){
			ulAttributes = 0;
		} else {
			ulAttributes = OBJ_OPENIF;
		}
	}

	const auto hRootDirectory = X_OpenBaseNamedObjectDirectory(u32Flags);

	::OBJECT_ATTRIBUTES vObjectAttributes;
	InitializeObjectAttributes(&vObjectAttributes, &ustrObjectName, ulAttributes, hRootDirectory.Get(), nullptr);

	HANDLE hEvent;
	if(u32Flags & kDontCreate){
		const auto lStatus = ::NtOpenEvent(&hEvent, EVENT_ALL_ACCESS, &vObjectAttributes);
		if(!NT_SUCCESS(lStatus)){
			DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtOpenEvent"_rcs);
		}
	} else {
		const auto lStatus = ::NtCreateEvent(&hEvent, EVENT_ALL_ACCESS, &vObjectAttributes, NotificationEvent, bInitSet);
		if(!NT_SUCCESS(lStatus)){
			DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtCreateEvent"_rcs);
		}
	}
	try {
		if(!(u32Flags & kFailIfExists)){
			EventBasicInformation vBasicInfo;
			const auto lStatus = ::NtQueryEvent(hEvent, 0 /* EventBasicInformation */, &vBasicInfo, sizeof(vBasicInfo), nullptr);
			if(!NT_SUCCESS(lStatus)){
				ASSERT_MSG(false, L"NtQueryEvent() 失败。");
			}
			if(vBasicInfo.eEventType != NotificationEvent){
				DEBUG_THROW(SystemError, ERROR_ALREADY_EXISTS, "CreateEventHandle"_rcs);
			}
		}
	} catch(...){
		Impl_UniqueNtHandle::NtHandleCloser()(hEvent);
		throw;
	}
	return Impl_UniqueNtHandle::UniqueNtHandle(hEvent);
}

// 其他非静态成员函数。
bool KernelEvent::Wait(std::uint64_t u64UntilFastMonoClock) const noexcept {
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
void KernelEvent::Wait() const noexcept {
	const auto lStatus = ::NtWaitForSingleObject(x_hEvent.Get(), false, nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
}
bool KernelEvent::IsSet() const noexcept {
	EventBasicInformation vBasicInfo;
	const auto lStatus = ::NtQueryEvent(x_hEvent.Get(), 0 /* EventBasicInformation */, &vBasicInfo, sizeof(vBasicInfo), nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtQueryEvent() 失败。");
	}
	return vBasicInfo.lState;
}
bool KernelEvent::Set() noexcept {
	LONG lPrevState;
	const auto lStatus = ::NtSetEvent(x_hEvent.Get(), &lPrevState);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtSetEvent() 失败。");
	}
	return lPrevState;
}
bool KernelEvent::Reset() noexcept {
	LONG lPrevState;
	const auto lStatus = ::NtResetEvent(x_hEvent.Get(), &lPrevState);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtResetEvent() 失败。");
	}
	return lPrevState;
}

}
