// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "DynamicLinkLibrary.hpp"
#include "Exception.hpp"
#include <winternl.h>
#include <ntdef.h>
#include <ntstatus.h>

extern "C" {

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS LdrLoadDll(PCWSTR pwszPathToSearch, DWORD dwFlags, const ::UNICODE_STRING *pFileName, HANDLE *pHandle) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS LdrUnloadDll(HANDLE hDll) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS LdrGetProcedureAddress(HANDLE hDll, const ANSI_STRING *pProcName, WORD wOrdinal, FARPROC *ppfnProcAddress) noexcept;

}

namespace MCF {

void DynamicLinkLibrary::X_DllUnloader::operator()(void *hDll) noexcept {
	const auto lStatus = ::LdrUnloadDll(hDll);
	MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"LdrUnloadDll() 失败。");
}

DynamicLinkLibrary::DynamicLinkLibrary(const WideStringView &wsvPath)
	: DynamicLinkLibrary()
{
	const auto uSize = wsvPath.GetSize() * sizeof(wchar_t);
	if(uSize > USHRT_MAX){
		MCF_THROW(Exception, ERROR_BUFFER_OVERFLOW, Rcntws::View(L"DynamicLinkLibrary: 路径太长。"));
	}
	::UNICODE_STRING ustrFileName;
	ustrFileName.Length             = (USHORT)uSize;
	ustrFileName.MaximumLength      = (USHORT)uSize;
	ustrFileName.Buffer             = (PWSTR)wsvPath.GetBegin();

	HANDLE hDll;
	const auto lStatus = ::LdrLoadDll(nullptr, 0, &ustrFileName, &hDll);
	if(!NT_SUCCESS(lStatus)){
		MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"DynamicLinkLibrary: LdrLoadDll() 失败。"));
	}
	x_hDll.Reset(hDll);
}

const void *DynamicLinkLibrary::GetBaseAddress() const noexcept {
	return x_hDll.Get();
}

DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawGetProcAddress(const NarrowStringView &nsvName){
	if(!x_hDll){
		MCF_THROW(Exception, ERROR_INVALID_HANDLE, Rcntws::View(L"DynamicLinkLibrary: 尚未加载任何动态库。"));
	}

	const auto uSize = nsvName.GetSize();
	if(uSize > USHRT_MAX){
		MCF_THROW(Exception, ERROR_INVALID_PARAMETER, Rcntws::View(L"DynamicLinkLibrary: 导出函数名太长。"));
	}
	::ANSI_STRING strProcName;
	strProcName.Length          = (USHORT)uSize;
	strProcName.MaximumLength   = (USHORT)uSize;
	strProcName.Buffer          = (PSTR)nsvName.GetBegin();

	::FARPROC pfnProcAddress;
	const auto lStatus = ::LdrGetProcedureAddress(x_hDll.Get(), &strProcName, 0xFFFF, &pfnProcAddress);
	if(!NT_SUCCESS(lStatus)){
		MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"DynamicLinkLibrary: LdrGetProcedureAddress() 失败。"));
	}
	return pfnProcAddress;
}
DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawRequireProcAddress(const NarrowStringView &nsvName){
	const auto pfnRet = RawGetProcAddress(nsvName);
	if(!pfnRet){
		MCF_THROW(Exception, ERROR_PROC_NOT_FOUND, Rcntws::View(L"DynamicLinkLibrary: 指定的导出函数未找到。"));
	}
	return pfnRet;
}


DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawGetProcAddress(unsigned uOrdinal){
	if(!x_hDll){
		MCF_THROW(Exception, ERROR_INVALID_HANDLE, Rcntws::View(L"DynamicLinkLibrary: 尚未加载任何动态库。"));
	}

	if(uOrdinal > UINT16_MAX){
		MCF_THROW(Exception, ERROR_INVALID_PARAMETER, Rcntws::View(L"DynamicLinkLibrary: 导出函数序数无效。"));
	}

	::FARPROC pfnProcAddress;
	const auto lStatus = ::LdrGetProcedureAddress(x_hDll.Get(), nullptr, static_cast<WORD>(uOrdinal), &pfnProcAddress);
	if(!NT_SUCCESS(lStatus)){
		MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"DynamicLinkLibrary: LdrGetProcedureAddress() 失败。"));
	}
	return pfnProcAddress;
}
DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawRequireProcAddress(unsigned uOrdinal){
	const auto pfnRet = RawGetProcAddress(uOrdinal);
	if(!pfnRet){
		MCF_THROW(Exception, ERROR_PROC_NOT_FOUND, Rcntws::View(L"DynamicLinkLibrary: 指定的导出函数未找到。"));
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
	} catch(Exception &e){
		::SetLastError(e.GetErrorCode());
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
