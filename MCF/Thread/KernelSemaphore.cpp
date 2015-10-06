// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelSemaphore.hpp"
#include "../Core/Exception.hpp"
#include <winternl.h>
#include <ntdef.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtCreateSemaphore(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, OBJECT_ATTRIBUTES *pObjectAttributes, LONG lInitialCount, LONG lMaximumCount) noexcept;

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LONG *plPrevCount) noexcept;

namespace MCF {

// 构造函数和析构函数。
KernelSemaphore::KernelSemaphore(std::size_t uInitCount, const WideStringView &wsvName){
	if(uInitCount >= static_cast<std::size_t>(LONG_MAX)){
		DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, "Initial count for a kernel semaphore is too large"_rcs);
	}

	const auto uSize = wsvName.GetSize() * sizeof(wchar_t);
	if(uSize > UINT16_MAX){
		DEBUG_THROW(SystemError, ERROR_INVALID_PARAMETER, "The name for a kernel semaphore is too long"_rcs);
	}
	::UNICODE_STRING ustrObjectName;
	ustrObjectName.Length        = uSize;
	ustrObjectName.MaximumLength = uSize;
	ustrObjectName.Buffer        = (PWSTR)wsvName.GetBegin();
	::OBJECT_ATTRIBUTES vObjectAttributes;
	InitializeObjectAttributes(&vObjectAttributes, &ustrObjectName, 0, nullptr, nullptr);

	HANDLE hSemaphore;
	const auto lStatus = ::NtCreateSemaphore(&hSemaphore, SEMAPHORE_ALL_ACCESS, &vObjectAttributes, static_cast<LONG>(uInitCount), LONG_MAX);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtCreateSemaphore"_rcs);
	}
	x_hSemaphore.Reset(hSemaphore);
}

// 其他非静态成员函数。
bool KernelSemaphore::Wait(std::uint64_t u64MilliSeconds) noexcept {
	if(u64MilliSeconds > static_cast<std::uint64_t>(INT64_MIN) / 10000){
		Wait();
		return true;
	}

	::LARGE_INTEGER liTimeout;
	liTimeout.QuadPart = -static_cast<std::int64_t>(u64MilliSeconds * 10000);
	const auto lStatus = ::NtWaitForSingleObject(x_hSemaphore.Get(), false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		return false;
	}
	return true;
}
void KernelSemaphore::Wait() noexcept {
	const auto lStatus = ::NtWaitForSingleObject(x_hSemaphore.Get(), false, nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
}
std::size_t KernelSemaphore::Post(std::size_t uPostCount) noexcept {
	if(uPostCount >= static_cast<std::size_t>(LONG_MAX)){
		ASSERT_MSG(false, L"信号量自增数量超过上限。");
	}

	LONG lPrevCount;
	const auto lStatus = ::NtReleaseSemaphore(x_hSemaphore.Get(), static_cast<long>(uPostCount), &lPrevCount);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtReleaseSemaphore() 失败。");
	}
	return static_cast<std::size_t>(lPrevCount);
}

}
