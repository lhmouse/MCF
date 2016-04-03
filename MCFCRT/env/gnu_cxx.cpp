// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "gnu_cxx.h"

namespace __gnu_cxx {

extern __attribute__((__weak__))
void __freeres() noexcept;

}

extern "C" {

bool __MCFCRT_GnuCxxInit() noexcept {
	return true;
}
void __MCFCRT_GnuCxxUninit() noexcept {
	::__gnu_cxx::__freeres();
}

}
