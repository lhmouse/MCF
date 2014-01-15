// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "Time.hpp"
using namespace MCF;

namespace MCF {
	extern DWORD GetUNIXTime(){
		union {
			FILETIME ft;
			ULARGE_INTEGER uli;
		} u;
		::GetSystemTimeAsFileTime(&u.ft);
		// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间的纳秒数。
		return (DWORD)((u.uli.QuadPart - (unsigned long long)0x019DB1DED53E8000) / 10000000);
	}
	extern DWORD GenRandSeed(){
		FILETIME ft;
		::GetSystemTimeAsFileTime(&ft);
		return ft.dwLowDateTime;
	}

	namespace {
		struct HIRESTIMERFREQUENCY {
			LARGE_INTEGER m_liFrequency;
			unsigned long long m_ullFrequencyReciprocal;

			HIRESTIMERFREQUENCY(){
				VERIFY(::QueryPerformanceFrequency(&m_liFrequency));
				m_ullFrequencyReciprocal = (unsigned long long)0xFFFFFFFFFFFFFFFF / (unsigned long long)m_liFrequency.QuadPart + 1;
			}
		} HiResTimerFrequency;
	}
	extern unsigned long long GetHiResTimer(){
		LARGE_INTEGER liCounter;
		VERIFY(::QueryPerformanceCounter(&liCounter));

		const unsigned long long ullSeconds = ((unsigned long long)(liCounter.QuadPart / HiResTimerFrequency.m_liFrequency.QuadPart)) << 32;
		const unsigned long long ullRemainder = ((unsigned long long)((liCounter.QuadPart % HiResTimerFrequency.m_liFrequency.QuadPart * HiResTimerFrequency.m_ullFrequencyReciprocal))) >> 32;
		return ullSeconds + ullRemainder;
	}
}
