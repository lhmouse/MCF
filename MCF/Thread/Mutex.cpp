// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"

namespace MCF {

// 其他非静态成员函数。
bool Mutex::Try() noexcept {
	return ::TryAcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}
void Mutex::Lock() noexcept {
	if(::TryAcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl))){
		return;
	}

	const auto uSpinCount = GetSpinCount() / 0x400; // FIXME: SRWLOCK 里面自己实现了一个自旋。
	for(std::size_t i = 0; i < uSpinCount; ++i){
		if(::TryAcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl))){
			return;
		}
	}

	::AcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}
void Mutex::Unlock() noexcept {
	::ReleaseSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}

}
