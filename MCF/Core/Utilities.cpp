// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Utilities.hpp"
#include "UniqueHandle.hpp"
#include <cmath>
using namespace MCF;

namespace MCF {

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

unsigned int GetUnixTime() noexcept {
	union {
		FILETIME ft;
		ULARGE_INTEGER uli;
	} u;
	::GetSystemTimeAsFileTime(&u.ft);
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return (unsigned int)((u.uli.QuadPart - 0x019DB1DED53E8000ull) / 10000000ull);
}
std::uint32_t GenRandSeed() noexcept {
	LARGE_INTEGER liTemp;
	::QueryPerformanceCounter(&liTemp);
	return (std::uint32_t)liTemp.QuadPart;
}

HI_RES_COUNTER GetHiResCounter() noexcept {
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
		s_llfRemainderCoef = (1ull << (64 - HI_RES_COUNTER::SECOND_BITS)) / llFreq;

		__atomic_store_n(&s_bInited, true, __ATOMIC_RELEASE);
	}

	::QueryPerformanceCounter(&liTemp);
	const auto llfCounter = (long double)liTemp.QuadPart;
	const auto u64Seconds = (std::uint64_t)(llfCounter * s_llfFreqRecip);
	const auto u32Remainder = (std::uint32_t)((llfCounter - u64Seconds * s_llfFreq) * s_llfRemainderCoef);
	return HI_RES_COUNTER{u64Seconds, u32Remainder};
}

}
