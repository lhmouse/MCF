// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "DynamicLinkLibrary.hpp"
#include "Exception.hpp"

namespace MCF {

void *DynamicLinkLibrary::X_LibraryFreer::operator()() noexcept {
	return nullptr;
}
void DynamicLinkLibrary::X_LibraryFreer::operator()(void *hDll) noexcept {
	::FreeLibrary(static_cast<HINSTANCE>(hDll));
}

// 构造函数和析构函数。
DynamicLinkLibrary::DynamicLinkLibrary(const wchar_t *pwszPath){
	if(!x_hDll.Reset(static_cast<void *>(::LoadLibraryW(pwszPath)))){
		DEBUG_THROW(SystemError, "LoadLibraryW"_rcs);
	}
}

// 其他非静态成员函数。
const void *DynamicLinkLibrary::GetBaseAddress() const noexcept {
	return x_hDll.Get();
}
DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawGetProcAddress(const char *pszName){
	if(!x_hDll){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No shared library opened"_rcs);
	}

	return ::GetProcAddress(static_cast<HINSTANCE>(x_hDll.Get()), pszName);
}
DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawRequireProcAddress(const char *pszName){
	if(!x_hDll){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No shared library opened"_rcs);
	}

	const auto pfnRet = ::GetProcAddress(static_cast<HINSTANCE>(x_hDll.Get()), pszName);
	if(!pfnRet){
		DEBUG_THROW(SystemError, "GetProcAddress"_rcs);
	}
	return pfnRet;
}

bool DynamicLinkLibrary::IsOpen() const noexcept {
	return !!x_hDll;
}
void DynamicLinkLibrary::Open(const wchar_t *pwszPath){
	DynamicLinkLibrary(pwszPath).Swap(*this);
}
bool DynamicLinkLibrary::OpenNoThrow(const wchar_t *pwszPath){
	try {
		Open(pwszPath);
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
