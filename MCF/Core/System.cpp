// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "System.hpp"

namespace MCF {

namespace {
	struct SystemInfo : public ::SYSTEM_INFO {
		SystemInfo() noexcept {
			::GetNativeSystemInfo(this);
		}
	} g_vSystemInfo __attribute__((__init_priority__(101)));
}

std::size_t GetLogicalProcessorCount() noexcept {
	return g_vSystemInfo.dwNumberOfProcessors;
}
std::size_t GetPageSize() noexcept {
	return g_vSystemInfo.dwPageSize;
}

namespace {
	struct OsVersionInfo : public ::OSVERSIONINFOW {
		OsVersionInfo() noexcept {
			dwOSVersionInfoSize = sizeof(::OSVERSIONINFOW);
			::GetVersionExW(this);
		}
	} g_vOsVersionInfo __attribute__((__init_priority__(101)));
}

WindowsVersion GetWindowsVersion() noexcept {
	WindowsVersion vRet;
	vRet.uMajor			= g_vOsVersionInfo.dwMajorVersion;
	vRet.uMinor			= g_vOsVersionInfo.dwMinorVersion;
	vRet.uBuild			= g_vOsVersionInfo.dwBuildNumber;
	vRet.pwszServPack	= g_vOsVersionInfo.szCSDVersion;
	return vRet;
}

}
