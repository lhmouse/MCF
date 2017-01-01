// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "ConditionVariable.hpp"

namespace MCF {

std::intptr_t ConditionVariable::X_UnlockCallback(std::intptr_t nContext) noexcept {
	const auto pLock = reinterpret_cast<UniqueLockBase *>(nContext);

	const auto uLockCount = pLock->Y_UnlockAll();
	MCF_ASSERT(uLockCount != 0);
	return static_cast<std::intptr_t>(uLockCount);
}
void ConditionVariable::X_RelockCallback(std::intptr_t nContext, std::intptr_t nUnlocked) noexcept {
	const auto pLock = reinterpret_cast<UniqueLockBase *>(nContext);

	pLock->Y_RelockAll(static_cast<std::uintptr_t>(nUnlocked));
}

}
