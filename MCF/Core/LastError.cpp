// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "LastError.hpp"
#include "Exception.hpp"
#include "../Utilities/Defer.hpp"

namespace MCF {

WideString GetWin32ErrorDescription(unsigned long ulErrorCode){
	constexpr auto kFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK;

	WideString wcsRet;
	void *pBuffer;
	const auto uLen = ::FormatMessageW(kFlags, nullptr, ulErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), reinterpret_cast<LPWSTR>(&pBuffer), 0, nullptr);
	if(uLen == 0){
		DEBUG_THROW(SystemError, "FormatMessageW"_rcs);
	}
	DEFER([&]{ ::LocalFree(pBuffer); });
	wcsRet.Assign(static_cast<const wchar_t *>(pBuffer), uLen);
	return wcsRet;
}

}
