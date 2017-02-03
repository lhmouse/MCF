// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "KernelEvent.hpp"
#include "../Core/Exception.hpp"
#include <MCFCRT/env/_nt_timeout.h>
#include <winternl.h>
#include <ntdef.h>
#include <ntstatus.h>

extern "C" {

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtOpenEvent(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtCreateEvent(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes, EVENT_TYPE eEventType, BOOLEAN bInitialState) noexcept;

typedef enum tagEventInformationClass {
	EventBasicInformation,
} EVENT_INFORMATION_CLASS;

typedef struct tagEVENT_BASIC_INFORMATION {
	EVENT_TYPE eEventType;
	LONG lState;
} EVENT_BASIC_INFORMATION;

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtQueryEvent(HANDLE hEvent, EVENT_INFORMATION_CLASS eInfoClass, void *pInfo, DWORD dwInfoSize, DWORD *pdwInfoSizeRet) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtSetEvent(HANDLE hEvent, LONG *plPrevState) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtResetEvent(HANDLE hEvent, LONG *plPrevState) noexcept;

}

namespace MCF {

KernelEvent::KernelEvent(bool bInitSet, const WideStringView &wsvName, std::uint32_t u32Flags){
	Impl_UniqueNtHandle::UniqueNtHandle hRootDirectory;
	::OBJECT_ATTRIBUTES vObjectAttributes;
	const auto uNameSize = wsvName.GetSize() * sizeof(wchar_t);
	if(uNameSize == 0){
		InitializeObjectAttributes(&vObjectAttributes, nullptr, 0, nullptr, nullptr);
	} else {
		if(uNameSize > USHRT_MAX){
			MCF_THROW(Exception, ERROR_BUFFER_OVERFLOW, Rcntws::View(L"KernelEvent: 内核对象的路径太长。"));
		}
		::UNICODE_STRING ustrObjectName;
		ustrObjectName.Length        = (USHORT)uNameSize;
		ustrObjectName.MaximumLength = (USHORT)uNameSize;
		ustrObjectName.Buffer        = (PWSTR)wsvName.GetBegin();

		ULONG ulAttributes;
		if(u32Flags & kFailIfExists){
			ulAttributes = 0;
		} else {
			ulAttributes = OBJ_OPENIF;
		}

		hRootDirectory = Y_OpenBaseNamedObjectDirectory(u32Flags);

		InitializeObjectAttributes(&vObjectAttributes, &ustrObjectName, ulAttributes, hRootDirectory.Get(), nullptr);
	}

	HANDLE hTemp;
	bool bNameExists;
	if(u32Flags & kDontCreate){
		const auto lStatus = ::NtOpenEvent(&hTemp, EVENT_ALL_ACCESS, &vObjectAttributes);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelEvent: NtOpenEvent() 失败。"));
		}
		bNameExists = false;
	} else {
		const auto lStatus = ::NtCreateEvent(&hTemp, EVENT_ALL_ACCESS, &vObjectAttributes, NotificationEvent, bInitSet);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelEvent: NtCreateEvent() 失败。"));
		}
		bNameExists = (lStatus == STATUS_OBJECT_NAME_EXISTS);
	}
	x_hEvent.Reset(hTemp);

	if(bNameExists){
		EVENT_BASIC_INFORMATION vBasicInfo;
		const auto lStatus = ::NtQueryEvent(x_hEvent.Get(), EventBasicInformation, &vBasicInfo, sizeof(vBasicInfo), nullptr);
		MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtQueryEvent() 失败。");
		if(vBasicInfo.eEventType != NotificationEvent){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(STATUS_OBJECT_TYPE_MISMATCH), Rcntws::View(L"KernelEvent: 内核事件类型不匹配。"));
		}
	}
}

bool KernelEvent::Wait(std::uint64_t u64UntilFastMonoClock) const noexcept {
	::LARGE_INTEGER liTimeout;
	::__MCFCRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	const auto lStatus = ::NtWaitForSingleObject(x_hEvent.Get(), false, &liTimeout);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
	return lStatus != STATUS_TIMEOUT;
}
void KernelEvent::Wait() const noexcept {
	const auto lStatus = ::NtWaitForSingleObject(x_hEvent.Get(), false, nullptr);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
}
bool KernelEvent::IsSet() const noexcept {
	EVENT_BASIC_INFORMATION vBasicInfo;
	const auto lStatus = ::NtQueryEvent(x_hEvent.Get(), EventBasicInformation, &vBasicInfo, sizeof(vBasicInfo), nullptr);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtQueryEvent() 失败。");
	return vBasicInfo.lState;
}
bool KernelEvent::Set() noexcept {
	LONG lPrevState;
	const auto lStatus = ::NtSetEvent(x_hEvent.Get(), &lPrevState);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtSetEvent() 失败。");
	return lPrevState;
}
bool KernelEvent::Reset() noexcept {
	LONG lPrevState;
	const auto lStatus = ::NtResetEvent(x_hEvent.Get(), &lPrevState);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtResetEvent() 失败。");
	return lPrevState;
}

}
