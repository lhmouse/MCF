// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CallOnce.hpp"
#include "Mutex.hpp"

namespace MCF {

namespace {
	Mutex g_vOnceMutex;
}

namespace Impl_CallOnce {
	void OnceMutexLock() noexcept {
		g_vOnceMutex.Lock();
	}
	void OnceMutexUnlock() noexcept {
		g_vOnceMutex.Unlock();
	}
}

}
