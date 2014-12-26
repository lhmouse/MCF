// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RecursiveMutex.hpp"
using namespace MCF;

namespace MCF {

template<>
bool RecursiveMutex::UniqueLock::xDoTry() const noexcept {
	return xm_pOwner->Try() != RecursiveMutex::Result::R_TRY_FAILED;
}
template<>
void RecursiveMutex::UniqueLock::xDoLock() const noexcept {
	xm_pOwner->Lock();
}
template<>
void RecursiveMutex::UniqueLock::xDoUnlock() const noexcept {
	xm_pOwner->Unlock();
}

}

// 构造函数和析构函数。
RecursiveMutex::RecursiveMutex(std::size_t uSpinCount)
	: xm_vMutex(uSpinCount), xm_uRecursionCount(0)
{
}

// 其他非静态成员函数。
RecursiveMutex::Result RecursiveMutex::Try() noexcept {
	if(xm_vMutex.IsLockedByCurrentThread()){
		++xm_uRecursionCount;
		return R_RECURSIVE;
	}
	if(xm_vMutex.Try()){
		++xm_uRecursionCount;
		return R_STATE_CHANGED;
	}
	return R_TRY_FAILED;
}
RecursiveMutex::Result RecursiveMutex::Lock() noexcept {
	if(xm_vMutex.IsLockedByCurrentThread()){
		++xm_uRecursionCount;
		return R_RECURSIVE;
	}
	xm_vMutex.Lock();
	++xm_uRecursionCount;
	return R_STATE_CHANGED;
}
RecursiveMutex::Result RecursiveMutex::Unlock() noexcept {
	ASSERT(IsLockedByCurrentThread());

	if(--xm_uRecursionCount != 0){
		return R_RECURSIVE;
	}
	xm_vMutex.Unlock();
	return R_STATE_CHANGED;
}

std::size_t RecursiveMutex::UncheckedGetRecursionCount() const noexcept {
	ASSERT(IsLockedByCurrentThread());
	return xm_uRecursionCount;
}
std::size_t RecursiveMutex::GetRecursionCount() const noexcept {
	if(!IsLockedByCurrentThread()){
		return 0;
	}
	return UncheckedGetRecursionCount();
}

RecursiveMutex::UniqueLock RecursiveMutex::TryLock() noexcept {
	UniqueLock vLock(*this, false);
	vLock.Try();
	return vLock;
}
RecursiveMutex::UniqueLock RecursiveMutex::GetLock() noexcept {
	return UniqueLock(*this);
}
