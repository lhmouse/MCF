#include <MCF/StdMCF.hpp>
#include <MCFCRT/ext/stpcpy.h>
#include <MCFCRT/ext/wcpcpy.h>

extern "C" unsigned MCFMain(){
	{
		static const char src[] = "abcdefghijklmn";
		char dst[32] = { };
		for(unsigned i = 0; i < 20; ++i){
			std::memset(dst, '_', sizeof(dst) - 1);
			::MCF_stppcpy(dst, dst + i + 1, src);
			std::printf("dst = %s\n", dst);
		}
	}
	{
		static const wchar_t src[] = L"abcdefghijklmn";
		wchar_t dst[32] = { };
		for(unsigned i = 0; i < 20; ++i){
			std::wmemset(dst, L'_', sizeof(dst) / sizeof(wchar_t) - 1);
			::MCF_wcppcpy(dst, dst + i + 1, src);
			std::printf("dst = %ls\n", dst);
		}
	}
	return 0;
}
