// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelRecursiveMutex.hpp"
#include "../Core/Exception.hpp"
#include "../Utilities/MinMax.hpp"
#include <winternl.h>
#include <ntdef.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtCreateMutant(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, OBJECT_ATTRIBUTES *pObjectAttributes, BOOL bInitialOwner) noexcept;

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseMutant(HANDLE hMutant, LONG *plPrevCount) noexcept;

namespace MCF {

namespace {
	Impl_UniqueNtHandle::UniqueNtHandle CheckedCreateMutex(const WideStringObserver &wsoName){
		const auto uLength = Min(wsoName.GetSize(), 0xFFFFu);
		UNICODE_STRING ustrObjectName;
		ustrObjectName.Length        = uLength;
		ustrObjectName.MaximumLength = uLength;
		ustrObjectName.Buffer        = (PWSTR)wsoName.GetBegin();
		OBJECT_ATTRIBUTES vObjectAttributes;
		InitializeObjectAttributes(&vObjectAttributes, &ustrObjectName, OBJ_OPENIF, nullptr, nullptr);

		HANDLE hMutex;
		const auto lStatus = ::NtCreateMutant(&hMutex, MUTANT_ALL_ACCESS, &vObjectAttributes, false);
		if(!NT_SUCCESS(lStatus)){
			DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtCreateMutant"_rcs);
		}
		return Impl_UniqueNtHandle::UniqueNtHandle(hMutex);
	}
}

namespace Impl_UniqueLockTemplate {
	template<>
	bool KernelRecursiveMutex::UniqueLock::X_DoTry() const noexcept {
		return x_pOwner->Try();
	}
	template<>
	void KernelRecursiveMutex::UniqueLock::X_DoLock() const noexcept {
		x_pOwner->Lock();
	}
	template<>
	void KernelRecursiveMutex::UniqueLock::X_DoUnlock() const noexcept {
		x_pOwner->Unlock();
	}
}

// 构造函数和析构函数。
KernelRecursiveMutex::KernelRecursiveMutex(const WideStringObserver &wsoName)
	: x_hMutex(CheckedCreateMutex(wsoName))
{
}

// 其他非静态成员函数。
bool KernelRecursiveMutex::Try(std::uint64_t u64MilliSeconds) noexcept {
	if(u64MilliSeconds > static_cast<std::uint64_t>(INT64_MIN) / 10000){
		Lock();
		return true;
	}

	::LARGE_INTEGER liTimeout;
	liTimeout.QuadPart = -static_cast<std::int64_t>(u64MilliSeconds * 10000);
	const auto lStatus = ::NtWaitForSingleObject(x_hMutex.Get(), false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		const DWORD dwErrorCode = ::RtlNtStatusToDosError(lStatus);
		if(dwErrorCode == STATUS_TIMEOUT){
			return false;
		}
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
	return true;
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
}

}
