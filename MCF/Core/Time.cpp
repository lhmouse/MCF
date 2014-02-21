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

std::uint64_t GetHiResCounter() noexcept {
	static bool s_bInited = false;
	static long double s_llfFreq;
	static long double s_llfFreqRecip;
	static long double s_llfRemainderCoef;

	LARGE_INTEGER liTemp;

	if(!__atomic_load_n(&s_bInited, __ATOMIC_ACQUIRE)){
		::QueryPerformanceFrequency(&liTemp);
		const auto llFreq = (long double)liTemp.QuadPart;
		s_llfFreq = llFreq;
		s_llfFreqRecip = 1.0l / llFreq;
		s_llfRemainderCoef = (1ull << (64 - HI_RES_COUNTER_SECOND_BITS)) / llFreq;

		__atomic_store_n(&s_bInited, true, __ATOMIC_RELEASE);
	}

	::QueryPerformanceCounter(&liTemp);
	const auto llfCounter = (long double)liTemp.QuadPart;
	const auto u64Seconds = (std::uint64_t)(llfCounter * s_llfFreqRecip);
	const auto u32Remainder = (std::uint32_t)((llfCounter - u64Seconds * s_llfFreq) * s_llfRemainderCoef);
	return (u64Seconds << (64 - HI_RES_COUNTER_SECOND_BITS)) | u32Remainder;
}

}
