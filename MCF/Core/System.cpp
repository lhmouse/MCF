// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "System.hpp"
using namespace MCF;

namespace {

volatile bool g_bSystemInfoInited = false;
::SYSTEM_INFO g_vSystemInfo;

void RequireSystemInfo() noexcept {
	if(__atomic_load_n(&g_bSystemInfoInited, __ATOMIC_ACQUIRE) == false){
		::GetNativeSystemInfo(&g_vSystemInfo);
		__atomic_store_n(&g_bSystemInfoInited, true, __ATOMIC_RELEASE);
	}
}

}

namespace MCF {

unsigned GetProcessorCount() noexcept {
	RequireSystemInfo();
	return g_vSystemInfo.dwNumberOfProcessors;
}
std::size_t GetPageSize() noexcept {
	RequireSystemInfo();
	return g_vSystemInfo.dwPageSize;
}

}
