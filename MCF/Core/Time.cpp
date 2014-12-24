// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Time.hpp"
#include "../Utilities/Bail.hpp"
using namespace MCF;

namespace MCF {

std::uint64_t GetUtcTime() noexcept {
	::FILETIME ftUtc;
	::GetSystemTimeAsFileTime(&ftUtc);
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return (reinterpret_cast<const std::uint64_t &>(ftUtc) - 0x019DB1DED53E8000ull) / 10000u;
}
std::uint64_t GetLocalTime() noexcept {
	::FILETIME ftUtc, ftLocal;
	::GetSystemTimeAsFileTime(&ftUtc);
	::FileTimeToLocalFileTime(&ftUtc, &ftLocal);
	return (reinterpret_cast<const std::uint64_t &>(ftLocal) - 0x019DB1DED53E8000ull) / 10000u;
}

std::uint64_t GetFastMonoClock() noexcept {
	return ::GetTickCount64();
}
double GetHiResMonoClock() noexcept {
	static volatile bool s_bInited = false;
	static double s_lfFreqRecip;

	::LARGE_INTEGER liTemp;
	if(!__atomic_load_n(&s_bInited, __ATOMIC_ACQUIRE)){
		if(!::QueryPerformanceFrequency(&liTemp)){
			Bail(L"::QueryPerformanceFrequency() 失败。");
		}
		s_lfFreqRecip = 1000.0 / liTemp.QuadPart;
		__atomic_store_n(&s_bInited, true, __ATOMIC_RELEASE);
	}
	if(!::QueryPerformanceCounter(&liTemp)){
		Bail(L"::QueryPerformanceCounter() 失败。");
	}
	return liTemp.QuadPart * s_lfFreqRecip;
}

}
