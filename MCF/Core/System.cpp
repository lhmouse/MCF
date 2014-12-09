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

volatile bool g_bOsVersionInfoInited = false;
::OSVERSIONINFOW g_vOsVersionInfo;

void RequireOsVersionInfo() noexcept {
	if(__atomic_load_n(&g_bOsVersionInfoInited, __ATOMIC_ACQUIRE) == false){
		g_vOsVersionInfo.dwOSVersionInfoSize = sizeof(g_vOsVersionInfo);
		::GetVersionExW(&g_vOsVersionInfo);
		__atomic_store_n(&g_bOsVersionInfoInited, true, __ATOMIC_RELEASE);
	}
}

}

namespace MCF {

std::size_t GetProcessorCount() noexcept {
	RequireSystemInfo();
	return g_vSystemInfo.dwNumberOfProcessors;
}
std::size_t GetPageSize() noexcept {
	RequireSystemInfo();
	return g_vSystemInfo.dwPageSize;
}

WindowsVersion GetWindowsVersion() noexcept {
	RequireOsVersionInfo();
	WindowsVersion vRet;
	vRet.uMajor			= g_vOsVersionInfo.dwMajorVersion;
	vRet.uMinor			= g_vOsVersionInfo.dwMinorVersion;
	vRet.uBuild			= g_vOsVersionInfo.dwBuildNumber;
	vRet.pwszServPack	= g_vOsVersionInfo.szCSDVersion;
	return vRet;
}

}
