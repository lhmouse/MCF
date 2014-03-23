// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Time.hpp"
using namespace MCF;

namespace MCF {

std::uint64_t GetUnixTime() noexcept {
	union {
		FILETIME ft;
		ULARGE_INTEGER uli;
	} u;
	::GetSystemTimeAsFileTime(&u.ft);
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return (u.uli.QuadPart - 0x019DB1DED53E8000ull) / 10000000ull;
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
