// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CallOnce.hpp"
#include "Mutex.hpp"

namespace MCF {

namespace {
	Mutex g_mtxOnceGuard;

	static_assert(std::is_trivially_destructible<Mutex>::value, "Please fix this.");
}

namespace Impl_CallOnce {
	void LockOnceMutex() noexcept {
		g_mtxOnceGuard.Lock();
	}
	void UnlockOnceMutex() noexcept {
		g_mtxOnceGuard.Unlock();
	}
}

}
