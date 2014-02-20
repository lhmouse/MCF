// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Utilities.hpp"
#include "UniqueHandle.hpp"
#include "../../MCFCRT/env/bail.h"
using namespace MCF;

namespace MCF {

__MCF_CPP_NORETURN_IF_NDEBUG void Bail(const wchar_t *pwszDescription){
	::__MCF_CRT_Bail(pwszDescription);
}

UTF16String GetWin32ErrorDesc(unsigned long ulErrorCode){
	struct LocalFreer {
		constexpr HLOCAL operator()() const {
			return NULL;
		}
		void operator()(HLOCAL hLocal) const {
			::LocalFree(hLocal);
		}
	};

	UTF16String u16sRet;
	PVOID pDescBuffer;
	const auto uLen = ::FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		ulErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&pDescBuffer,
		0,
		nullptr
	);
	const UniqueHandle<HLOCAL, LocalFreer> hLocal((HLOCAL)pDescBuffer); // RAII
	u16sRet.Assign((LPCWSTR)pDescBuffer, uLen);
	return std::move(u16sRet);
}

}
