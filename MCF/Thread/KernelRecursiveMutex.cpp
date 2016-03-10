// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelRecursiveMutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Clocks.hpp"
#include <winternl.h>
#include <ntdef.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtOpenMutant(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtCreateMutant(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes, BOOLEAN bInitialOwner) noexcept;

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseMutant(HANDLE hMutant, LONG *plPrevCount) noexcept;

namespace MCF {

Impl_UniqueNtHandle::UniqueNtHandle KernelRecursiveMutex::X_CreateMutexHandle(const WideStringView &wsvName, std::uint32_t u32Flags){
	Impl_UniqueNtHandle::UniqueNtHandle hRootDirectory;
	::OBJECT_ATTRIBUTES vObjectAttributes;

	const auto uNameSize = wsvName.GetSize() * sizeof(wchar_t);
	if(uNameSize == 0){
		InitializeObjectAttributes(&vObjectAttributes, nullptr, 0, nullptr, nullptr);
	} else {
		if(uNameSize > USHRT_MAX){
			DEBUG_THROW(Exception, ERROR_BUFFER_OVERFLOW, Rcntws::View(L"KernelRecursiveMutex: 内核对象的路径太长。"));
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
			DEBUG_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelRecursiveMutex: NtOpenMutant() 失败。"));
		}
	} else {
		const auto lStatus = ::NtCreateMutant(&hTemp, MUTANT_ALL_ACCESS, &vObjectAttributes, false);
		if(!NT_SUCCESS(lStatus)){
			DEBUG_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelRecursiveMutex: NtCreateMutant() 失败。"));
		}
	}
	Impl_UniqueNtHandle::UniqueNtHandle hMutex(hTemp);

	return hMutex;
}

bool KernelRecursiveMutex::Try(std::uint64_t u64UntilFastMonoClock) noexcept {
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
	const auto lStatus = ::NtWaitForSingleObject(x_hMutex.Get(), false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
	return lStatus != STATUS_TIMEOUT;
}
void KernelRecursiveMutex::Lock() noexcept {
	const auto lStatus = ::NtWaitForSingleObject(x_hMutex.Get(), false, nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
}
void KernelRecursiveMutex::Unlock() noexcept {
	LONG lPrevCount;
	const auto lStatus = ::NtReleaseMutant(x_hMutex.Get(), &lPrevCount);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtReleaseMutant() 失败。");
	}
	ASSERT_MSG(lPrevCount <= 0, L"互斥锁没有被任何线程锁定。");
}

}
