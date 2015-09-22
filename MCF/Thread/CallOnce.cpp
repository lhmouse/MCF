// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CallOnce.hpp"

namespace MCF {

namespace {
	::SRWLOCK g_srwLock = SRWLOCK_INIT;
}

namespace Impl_CallOnce {
	void LockOnceMutex() noexcept {
		::AcquireSRWLockExclusive(&g_srwLock);
	}
	void UnlockOnceMutex() noexcept {
		::ReleaseSRWLockExclusive(&g_srwLock);
	}
}

}
