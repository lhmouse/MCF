// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Time.hpp"
using namespace MCF;

namespace MCF {

std::uint64_t GetNtTime() noexcept {
	union {
		FILETIME ft;
		ULARGE_INTEGER uli;
	} un;
	::GetSystemTimeAsFileTime(&un.ft);
	return un.uli.QuadPart;
}
std::uint64_t GetUnixTime() noexcept {
	return UnixTimeFromNtTime(GetNtTime());
}
std::uint64_t NtTimeFromUnixTime(std::uint64_t u64UnixTime) noexcept {
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return u64UnixTime * 10000000u + 0x019DB1DED53E8000ull;
}
std::uint64_t UnixTimeFromNtTime(std::uint64_t u64NtTime) noexcept {
	return (u64NtTime - 0x019DB1DED53E8000ull) / 10000000u;
}

double GetHiResCounter() noexcept {
	static bool s_bInited = false;
	static long double s_llfFreqRecip;

	LARGE_INTEGER liTemp;
	if(!__atomic_load_n(&s_bInited, __ATOMIC_ACQUIRE)){
		::QueryPerformanceFrequency(&liTemp);
		s_llfFreqRecip = 1.0l / liTemp.QuadPart;
		__atomic_store_n(&s_bInited, true, __ATOMIC_RELEASE);
	}
	::QueryPerformanceCounter(&liTemp);
	return (double)(liTemp.QuadPart * s_llfFreqRecip);
}

}
