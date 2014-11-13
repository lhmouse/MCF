// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "LastError.hpp"
#include "Exception.hpp"
using namespace MCF;

namespace MCF {

WideString GetWin32ErrorDesc(unsigned long ulErrorCode){
	WideString wsRet;
	wsRet.Resize(1);
	for(;;){
		const auto uMaxLen = wsRet.GetCapacity();
		wsRet.Resize(uMaxLen);
		const auto uLen = ::FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			nullptr, ulErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			wsRet.GetData(), uMaxLen, nullptr);
		if(uLen != 0){
			wsRet.Resize(uLen);
			break;
		}
		const auto dwError = ::GetLastError();
		if(dwError != ERROR_INSUFFICIENT_BUFFER){
			DEBUG_THROW(SystemError, "FormatMessageW", dwError);
		}
		wsRet.Clear();
		wsRet.Reserve(uMaxLen * 2);
	}
	return std::move(wsRet);
}

}
