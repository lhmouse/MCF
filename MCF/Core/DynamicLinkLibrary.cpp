// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "DynamicLinkLibrary.hpp"
#include "Exception.hpp"

namespace MCF {

DynamicLinkLibrary::xLibraryFreer::Handle DynamicLinkLibrary::xLibraryFreer::operator()() noexcept {
	return NULL;
}
void DynamicLinkLibrary::xLibraryFreer::operator()(DynamicLinkLibrary::xLibraryFreer::Handle hDll) noexcept {
	::FreeLibrary(reinterpret_cast<HINSTANCE>(hDll));
}

// 其他非静态成员函数。
const void *DynamicLinkLibrary::GetBaseAddress() const noexcept {
	return x_hDll.Get();
}
auto DynamicLinkLibrary::RawGetProcAddress(const char *pszName) -> std::intptr_t (__stdcall *)() {
	if(!x_hDll){
		DEBUG_THROW(Exception, "No shared library open", ERROR_INVALID_HANDLE);
	}

	return ::GetProcAddress(reinterpret_cast<HINSTANCE>(x_hDll.Get()), pszName);
}

bool DynamicLinkLibrary::IsOpen() const noexcept {
	return !!x_hDll;
}
void DynamicLinkLibrary::Open(const wchar_t *pwszPath){
	UniqueHandle<xLibraryFreer> hDll;
	if(!hDll.Reset(reinterpret_cast<xLibraryFreer::Handle>(::LoadLibraryW(pwszPath)))){
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
