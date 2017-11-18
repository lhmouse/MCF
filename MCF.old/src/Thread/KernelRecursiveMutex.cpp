// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "KernelRecursiveMutex.hpp"
#include "../Core/Exception.hpp"
#include <MCFCRT/env/_nt_timeout.h>
#include <ntdef.h>

extern "C" {

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtOpenMutant(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtCreateMutant(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes, BOOLEAN bInitialOwner) noexcept;

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtWaitForSingleObject(HANDLE hObject, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtReleaseMutant(HANDLE hMutant, LONG *plPrevCount) noexcept;

__attribute__((__dllimport__, __stdcall__))
extern ULONG WINAPI RtlNtStatusToDosError(NTSTATUS lStatus) noexcept;

}

namespace MCF {

KernelRecursiveMutex::KernelRecursiveMutex(const WideStringView &wsvName, std::uint32_t u32Flags){
	Impl_UniqueNtHandle::UniqueNtHandle hRootDirectory;
	::OBJECT_ATTRIBUTES vObjectAttributes;

	const auto uNameSize = wsvName.GetSize() * sizeof(wchar_t);
	if(uNameSize == 0){
		InitializeObjectAttributes(&vObjectAttributes, nullptr, 0, nullptr, nullptr);
	} else {
		if(uNameSize > USHRT_MAX){
			MCF_THROW(Exception, ERROR_BUFFER_OVERFLOW, Rcntws::View(L"KernelRecursiveMutex: 内核对象的路径太长。"));
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
	if(u32Flags & kDontCreate){
		const auto lStatus = ::NtOpenMutant(&hTemp, MUTANT_ALL_ACCESS, &vObjectAttributes);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelRecursiveMutex: NtOpenMutant() 失败。"));
		}
	} else {
		const auto lStatus = ::NtCreateMutant(&hTemp, MUTANT_ALL_ACCESS, &vObjectAttributes, false);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelRecursiveMutex: NtCreateMutant() 失败。"));
		}
	}
	x_hMutex.Reset(hTemp);
}

bool KernelRecursiveMutex::Try(std::uint64_t u64UntilFastMonoClock) noexcept {
	::LARGE_INTEGER liTimeout;
	::__MCFCRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	const auto lStatus = ::NtWaitForSingleObject(x_hMutex.Get(), false, &liTimeout);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
	return lStatus != (NTSTATUS)STATUS_TIMEOUT;
}
void KernelRecursiveMutex::Lock() noexcept {
	const auto lStatus = ::NtWaitForSingleObject(x_hMutex.Get(), false, nullptr);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
}
void KernelRecursiveMutex::Unlock() noexcept {
	LONG lPrevCount;
	const auto lStatus = ::NtReleaseMutant(x_hMutex.Get(), &lPrevCount);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseMutant() 失败。");
	MCF_ASSERT_MSG(lPrevCount <= 0, L"互斥锁没有被任何线程锁定。");
}

}
