// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/System.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Thread/Thread.hpp"

namespace MCF {

namespace Impl_UniqueLockTemplate {
	template<>
	bool Mutex::UniqueLock::X_DoTry() const noexcept {
		return x_pOwner->Try();
	}
	template<>
	void Mutex::UniqueLock::X_DoLock() const noexcept {
		x_pOwner->Lock();
	}
	template<>
	void Mutex::UniqueLock::X_DoUnlock() const noexcept {
		x_pOwner->Unlock();
	}
}

// 构造函数和析构函数。
Mutex::Mutex(std::size_t uSpinCount) noexcept
	: x_uSpinCount(0)
{
	static_assert(sizeof(x_aImpl) == sizeof(::SRWLOCK), "!");

	::InitializeSRWLock(reinterpret_cast<::SRWLOCK *>(x_aImpl));

	SetSpinCount(uSpinCount);
}

// 其他非静态成员函数。
std::size_t Mutex::GetSpinCount() const noexcept {
	return x_uSpinCount.Load(kAtomicRelaxed);
}
void Mutex::SetSpinCount(std::size_t uSpinCount) noexcept {
	if(GetLogicalProcessorCount() == 0){
		return;
	}
	x_uSpinCount.Store(uSpinCount, kAtomicRelaxed);
}

bool Mutex::Try() noexcept {
	return ::TryAcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}
void Mutex::Lock() noexcept {
	if(::TryAcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl))){
		return;
	}

	const auto uSpinCount = GetSpinCount() / 0x400; // FIXME: SRWLOCK 里面自己实现了一个自旋。
	for(std::size_t i = 0; i < uSpinCount; ++i){
		Thread::YieldExecution();

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
