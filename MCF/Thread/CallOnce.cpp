// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CallOnce.hpp"
using namespace MCF;

namespace {

Mutex g_vCallOnceMutex __attribute__((__init_priority__(101)));

}

Mutex &Impl::OnceFlag::GetMutex() noexcept {
	return g_vCallOnceMutex;
}
