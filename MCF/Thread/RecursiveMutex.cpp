// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RecursiveMutex.hpp"

namespace MCF {

template<>
bool RecursiveMutex::UniqueLock::xDoTry() const noexcept {
	return x_pOwner->Try() != RecursiveMutex::Result::R_TRY_FAILED;
}
template<>
void RecursiveMutex::UniqueLock::xDoLock() const noexcept {
	x_pOwner->Lock();
}
template<>
void RecursiveMutex::UniqueLock::xDoUnlock() const noexcept {
	x_pOwner->Unlock();
}

// 构造函数和析构函数。
RecursiveMutex::RecursiveMutex(std::size_t uSpinCount)
	: x_vMutex(uSpinCount), x_uRecursionCount(0)
{
}

// 其他非静态成员函数。
RecursiveMutex::Result RecursiveMutex::Try() noexcept {
	if(x_vMutex.IsLockedByCurrentThread()){
		++x_uRecursionCount;
		return R_RECURSIVE;
	}
	if(x_vMutex.Try()){
		++x_uRecursionCount;
		return R_STATE_CHANGED;
	}
	return R_TRY_FAILED;
}
RecursiveMutex::Result RecursiveMutex::Lock() noexcept {
	if(x_vMutex.IsLockedByCurrentThread()){
		++x_uRecursionCount;
		return R_RECURSIVE;
	}
	x_vMutex.Lock();
	++x_uRecursionCount;
	return R_STATE_CHANGED;
}
RecursiveMutex::Result RecursiveMutex::Unlock() noexcept {
	ASSERT(IsLockedByCurrentThread());

	if(--x_uRecursionCount != 0){
		return R_RECURSIVE;
	}
	x_vMutex.Unlock();
	return R_STATE_CHANGED;
}

}
