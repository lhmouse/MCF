// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
using namespace MCF;

namespace MCF {

Utf16String GetWin32ErrorDesc(unsigned long ulErrorCode){
	struct LocalFreer {
		constexpr HLOCAL operator()() const {
			return NULL;
		}
		void operator()(HLOCAL hLocal) const {
			::LocalFree(hLocal);
		}
	};

	Utf16String wcsRet;
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
	const UniqueHandle<LocalFreer> hLocal((HLOCAL)pDescBuffer); // RAII
	wcsRet.Assign((LPCWSTR)pDescBuffer, uLen);
	return std::move(wcsRet);
}

}
