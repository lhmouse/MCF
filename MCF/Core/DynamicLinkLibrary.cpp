// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "DynamicLinkLibrary.hpp"
#include "Exception.hpp"
#include <winternl.h>
#include <ntdef.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS LdrLoadDll(PCWSTR pwszPathToSearch, DWORD dwFlags, const ::UNICODE_STRING *pFileName, HANDLE *pHandle) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS LdrUnloadDll(HANDLE hDll) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS LdrGetProcedureAddress(HANDLE hDll, const ANSI_STRING *pProcName, WORD wOridinal, FARPROC *ppfnProcAddress) noexcept;

namespace MCF {

void DynamicLinkLibrary::X_DllUnloader::operator()(void *hDll) noexcept {
	const auto lStatus = ::LdrUnloadDll(hDll);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"::LdrUnloadDll() 失败。");
	}
}

// 构造函数和析构函数。
DynamicLinkLibrary::DynamicLinkLibrary(const WideStringView &wsvPath)
	: DynamicLinkLibrary()
{
	const auto uSize = wsvPath.GetSize() * sizeof(wchar_t);
	if(uSize > UINT16_MAX){
		DEBUG_THROW(SystemError, ERROR_INVALID_PARAMETER, "The path for a library is too long"_rcs);
	}
	::UNICODE_STRING ustrFileName;
	ustrFileName.Length             = uSize;
	ustrFileName.MaximumLength      = uSize;
	ustrFileName.Buffer             = (PWSTR)wsvPath.GetBegin();

	HANDLE hDll;
	const auto lStatus = ::LdrLoadDll(nullptr, 0, &ustrFileName, &hDll);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "LdrLoadDll"_rcs);
	}
	x_hDll.Reset(hDll);
}

// 其他非静态成员函数。
const void *DynamicLinkLibrary::GetBaseAddress() const noexcept {
	return x_hDll.Get();
}

DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawGetProcAddress(const NarrowStringView &nsvName){
	if(!x_hDll){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No shared library opened"_rcs);
	}

	const auto uSize = nsvName.GetSize();
	if(uSize > UINT16_MAX){
		DEBUG_THROW(SystemError, ERROR_INVALID_PARAMETER, "The path for a library function is too long"_rcs);
	}
	::ANSI_STRING strProcName;
	strProcName.Length          = uSize;
	strProcName.MaximumLength   = uSize;
	strProcName.Buffer          = (PSTR)nsvName.GetBegin();

	::FARPROC pfnProcAddress;
	const auto lStatus = ::LdrGetProcedureAddress(x_hDll.Get(), &strProcName, 0xFFFF, &pfnProcAddress);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "LdrGetProcedureAddress"_rcs);
	}
	return pfnProcAddress;
}
DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawRequireProcAddress(const NarrowStringView &nsvName){
	const auto pfnRet = RawGetProcAddress(nsvName);
	if(!pfnRet){
		DEBUG_THROW(SystemError, "RawGetProcAddress"_rcs);
	}
	return pfnRet;
}


DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawGetProcAddress(unsigned uOridinal){
	if(!x_hDll){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No shared library opened"_rcs);
	}

	if(uOridinal > UINT16_MAX){
		DEBUG_THROW(SystemError, ERROR_INVALID_PARAMETER, "The oridinal for a library function is too large"_rcs);
	}

	::FARPROC pfnProcAddress;
	const auto lStatus = ::LdrGetProcedureAddress(x_hDll.Get(), nullptr, uOridinal, &pfnProcAddress);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "LdrGetProcedureAddress"_rcs);
	}
	return pfnProcAddress;
}
DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawRequireProcAddress(unsigned uOridinal){
	const auto pfnRet = RawGetProcAddress(uOridinal);
	if(!pfnRet){
		DEBUG_THROW(SystemError, "RawGetProcAddress"_rcs);
	}
	return pfnRet;
}

bool DynamicLinkLibrary::IsOpen() const noexcept {
	return !!x_hDll;
}
void DynamicLinkLibrary::Open(const WideStringView &wsvPath){
	DynamicLinkLibrary(wsvPath).Swap(*this);
}
bool DynamicLinkLibrary::OpenNoThrow(const WideStringView &wsvPath){
	try {
		Open(wsvPath);
		return true;
	} catch(SystemError &e){
		::SetLastError(e.GetCode());
		return false;
	}
}
void DynamicLinkLibrary::Close() noexcept {
	if(!x_hDll){
		return;
	}

	DynamicLinkLibrary().Swap(*this);
}

}
