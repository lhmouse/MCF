// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UserRecursiveMutex.hpp"
using namespace MCF;

namespace MCF {

template<>
bool UserRecursiveMutex::UniqueLock::xDoTry() const noexcept {
	return xm_pOwner->Try() != UserRecursiveMutex::Result::R_TRY_FAILED;
}
template<>
void UserRecursiveMutex::UniqueLock::xDoLock() const noexcept {
	xm_pOwner->Lock();
}
template<>
void UserRecursiveMutex::UniqueLock::xDoUnlock() const noexcept {
	xm_pOwner->Unlock();
}

}

// 构造函数和析构函数。
UserRecursiveMutex::UserRecursiveMutex(unsigned long ulSpinCount)
	: xm_vMutex(ulSpinCount), xm_ulRecursionCount(0)
{
}

// 其他非静态成员函数。
UserRecursiveMutex::Result UserRecursiveMutex::Try() noexcept {
	if(xm_vMutex.IsLockedByCurrentThread()){
		++xm_ulRecursionCount;
		return R_RECURSIVE;
	}
	if(xm_vMutex.Try()){
		++xm_ulRecursionCount;
		return R_STATE_CHANGED;
	}
	return R_TRY_FAILED;
}
UserRecursiveMutex::Result UserRecursiveMutex::Lock() noexcept {
	if(xm_vMutex.IsLockedByCurrentThread()){
		++xm_ulRecursionCount;
		return R_RECURSIVE;
	}
	xm_vMutex.Lock();
	++xm_ulRecursionCount;
	return R_STATE_CHANGED;
}
UserRecursiveMutex::Result UserRecursiveMutex::Unlock() noexcept {
	ASSERT(IsLockedByCurrentThread());

	if(--xm_ulRecursionCount != 0){
		return R_RECURSIVE;
	}
	xm_vMutex.Unlock();
	return R_STATE_CHANGED;
}

unsigned long UserRecursiveMutex::UncheckedGetRecursionCount() const noexcept {
	ASSERT(IsLockedByCurrentThread());
	return xm_ulRecursionCount;
}
unsigned long UserRecursiveMutex::GetRecursionCount() const noexcept {
	if(!IsLockedByCurrentThread()){
		return 0;
	}
	return UncheckedGetRecursionCount();
}

UserRecursiveMutex::UniqueLock UserRecursiveMutex::TryLock() noexcept {
	UniqueLock vLock(*this, false);
	vLock.Try();
	return std::move(vLock);
}
UserRecursiveMutex::UniqueLock UserRecursiveMutex::GetLock() noexcept {
	return UniqueLock(*this);
}
