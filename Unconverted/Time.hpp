// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TIME_HPP__
#define __MCF_TIME_HPP__

#include "StdMCF.hpp"

namespace MCF {
	extern DWORD GetUNIXTime();
	extern DWORD GenRandSeed();
	extern unsigned long long GetHiResTimer();
}

#endif
