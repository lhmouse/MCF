// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "DynamicLinkLibrary.hpp"
#include "Exception.hpp"

namespace MCF {

DynamicLinkLibrary::X_LibraryFreer::Handle DynamicLinkLibrary::X_LibraryFreer::operator()() noexcept {
	return NULL;
}
void DynamicLinkLibrary::X_LibraryFreer::operator()(DynamicLinkLibrary::X_LibraryFreer::Handle hDll) noexcept {
	::FreeLibrary(reinterpret_cast<HINSTANCE>(hDll));
}

// 其他非静态成员函数。
const void *DynamicLinkLibrary::GetBaseAddress() const noexcept {
	return x_hDll.Get();
}
DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawGetProcAddress(const char *pszName){
	if(!x_hDll){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No shared library open");
	}

	return ::GetProcAddress(reinterpret_cast<HINSTANCE>(x_hDll.Get()), pszName);
}
DynamicLinkLibrary::RawProc DynamicLinkLibrary::RawRequireProcAddress(const char *pszName){
	if(!x_hDll){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No shared library open");
	}

	const auto pfnRet = ::GetProcAddress(reinterpret_cast<HINSTANCE>(x_hDll.Get()), pszName);
	if(!pfnRet){
		DEBUG_THROW(SystemError, "GetProcAddress");
	}
	return pfnRet;
}

bool DynamicLinkLibrary::IsOpen() const noexcept {
	return !!x_hDll;
}
void DynamicLinkLibrary::Open(const wchar_t *pwszPath){
	UniqueHandle<X_LibraryFreer> hDll;
	if(!hDll.Reset(reinterpret_cast<X_LibraryFreer::Handle>(::LoadLibraryW(pwszPath)))){
		DEBUG_THROW(SystemError, "LoadLibraryW");
	}

	x_hDll = std::move(hDll);
}
void DynamicLinkLibrary::Open(const WideString &wsPath){
	Open(wsPath.GetStr());
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
bool DynamicLinkLibrary::OpenNoThrow(const WideString &wsPath){
	try {
		Open(wsPath);
		return true;
	} catch(SystemError &e){
		::SetLastError(e.GetCode());
		return false;
	}
}
void DynamicLinkLibrary::Close() noexcept {
	x_hDll.Reset();
}

}
