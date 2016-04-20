#include <MCF/StdMCF.hpp>
#include <cwchar>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(){
	static constexpr wchar_t s1[] = L"01234567";
	static constexpr wchar_t s2[] = L"0123765_";

	volatile auto p = &::wmemcmp;
	std::printf("result = %d\n", (*p)(s1, s2, _countof(s1)));

	return 0;
}
