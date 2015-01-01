// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "LastError.hpp"
#include "Exception.hpp"
#include "../Utilities/Defer.hpp"
using namespace MCF;

namespace MCF {

WideString GetWin32ErrorDescription(unsigned long ulErrorCode){
	void *pBuffer;
	const auto uLen = ::FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
		nullptr, ulErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPWSTR)&pBuffer, 0, nullptr);
	if(uLen == 0){
		DEBUG_THROW(SystemError, "FormatMessageW");
	}
	DEFER([&]{ ::LocalFree(pBuffer); });
	return WideString((const wchar_t *)pBuffer, uLen);
}

}
