// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Utilities.hpp"
#include "UniqueHandle.hpp"
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

unsigned int GetUNIXTime() noexcept {
	union {
		FILETIME ft;
		ULARGE_INTEGER uli;
	} u;
	::GetSystemTimeAsFileTime(&u.ft);
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return (unsigned int)((u.uli.QuadPart - 0x019DB1DED53E8000ull) / 10000000ull);
}
std::uint32_t GenRandSeed() noexcept {
	FILETIME ft;
	::GetSystemTimeAsFileTime(&ft);
	return (std::uint32_t)ft.dwLowDateTime;
}
std::uint64_t GetHiResTimer() noexcept {
	static bool s_bInited = false;
	static std::uint32_t s_u32Freq;
	static std::uint64_t s_u64FreqRecip;

	LARGE_INTEGER liTemp;

	if(!__atomic_load_n(&s_bInited, __ATOMIC_ACQUIRE)){
		::QueryPerformanceFrequency(&liTemp);
		s_u32Freq = (std::uint32_t)liTemp.QuadPart;
		s_u64FreqRecip = 0xFFFFFFFFFFFFFFFFull / (std::uint64_t)liTemp.QuadPart + 1;

		__atomic_store_n(&s_bInited, true, __ATOMIC_RELEASE);
	}

	::QueryPerformanceCounter(&liTemp);
	const auto u64Counter = (std::uint64_t)liTemp.QuadPart;
	const auto u32Seconds = (std::uint32_t)((u64Counter * (s_u64FreqRecip >> 32)) >> 32);
	const auto u32Remainder = (std::uint32_t)(((u64Counter - ((std::uint64_t)u32Seconds) * s_u32Freq) * s_u64FreqRecip) >> 32);
	return ((std::uint64_t)u32Seconds << 32) | u32Remainder;
}

}
