// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelSemaphore.hpp"
#include "../Core/Exception.hpp"
#include "../../MCFCRT/env/_nt_timeout.h"
#include <winternl.h>
#include <ntdef.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtOpenSemaphore(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtCreateSemaphore(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes, LONG lInitialCount, LONG lMaximumCount) noexcept;

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LONG *plPrevCount) noexcept;

namespace MCF {

Impl_UniqueNtHandle::UniqueNtHandle KernelSemaphore::X_CreateSemaphoreHandle(std::size_t uInitCount, const WideStringView &wsvName, std::uint32_t u32Flags){
	if(uInitCount >= static_cast<std::size_t>(LONG_MAX)){
		MCF_THROW(Exception, ERROR_INVALID_PARAMETER, Rcntws::View(L"KernelSemaphore: 信号量初始计数太大。"));
	}

	Impl_UniqueNtHandle::UniqueNtHandle hRootDirectory;
	::OBJECT_ATTRIBUTES vObjectAttributes;

	const auto uNameSize = wsvName.GetSize() * sizeof(wchar_t);
	if(uNameSize == 0){
		InitializeObjectAttributes(&vObjectAttributes, nullptr, 0, nullptr, nullptr);
	} else {
		if(uNameSize > USHRT_MAX){
			MCF_THROW(Exception, ERROR_BUFFER_OVERFLOW, Rcntws::View(L"KernelSemaphore: 内核对象的路径太长。"));
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
		const auto lStatus = ::NtOpenSemaphore(&hTemp, SEMAPHORE_ALL_ACCESS, &vObjectAttributes);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelSemaphore: NtOpenSemaphore() 失败。"));
		}
	} else {
		const auto lStatus = ::NtCreateSemaphore(&hTemp, SEMAPHORE_ALL_ACCESS, &vObjectAttributes, static_cast<LONG>(uInitCount), LONG_MAX);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelSemaphore: NtCreateSemaphore() 失败。"));
		}
	}
	Impl_UniqueNtHandle::UniqueNtHandle hSemaphore(hTemp);

	return hSemaphore;
}

bool KernelSemaphore::Wait(std::uint64_t u64UntilFastMonoClock) noexcept {
	::LARGE_INTEGER liTimeout;
	::__MCFCRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	const auto lStatus = ::NtWaitForSingleObject(x_hSemaphore.Get(), false, &liTimeout);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
	return lStatus != STATUS_TIMEOUT;
}
void KernelSemaphore::Wait() noexcept {
	const auto lStatus = ::NtWaitForSingleObject(x_hSemaphore.Get(), false, nullptr);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
}
std::size_t KernelSemaphore::Post(std::size_t uPostCount) noexcept {
	MCF_ASSERT_MSG(uPostCount < static_cast<std::size_t>(LONG_MAX), L"信号量自增数量超过上限。");

	LONG lPrevCount;
	const auto lStatus = ::NtReleaseSemaphore(x_hSemaphore.Get(), static_cast<long>(uPostCount), &lPrevCount);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtReleaseSemaphore() 失败。");
	return static_cast<std::size_t>(lPrevCount);
}

}
