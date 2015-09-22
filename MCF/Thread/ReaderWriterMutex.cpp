// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReaderWriterMutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/System.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

namespace Impl_UniqueLockTemplate {
	template<>
	bool ReaderWriterMutex::UniqueReaderLock::X_DoTry() const noexcept {
		return x_pOwner->TryAsReader();
	}
	template<>
	void ReaderWriterMutex::UniqueReaderLock::X_DoLock() const noexcept {
		x_pOwner->LockAsReader();
	}
	template<>
	void ReaderWriterMutex::UniqueReaderLock::X_DoUnlock() const noexcept {
		x_pOwner->UnlockAsReader();
	}

	template<>
	bool ReaderWriterMutex::UniqueWriterLock::X_DoTry() const noexcept {
		return x_pOwner->TryAsWriter();
	}
	template<>
	void ReaderWriterMutex::UniqueWriterLock::X_DoLock() const noexcept {
		x_pOwner->LockAsWriter();
	}
	template<>
	void ReaderWriterMutex::UniqueWriterLock::X_DoUnlock() const noexcept {
		x_pOwner->UnlockAsWriter();
	}
}

// 构造函数和析构函数。
ReaderWriterMutex::ReaderWriterMutex(std::size_t uSpinCount) noexcept
	: x_uSpinCount(0)
{
	static_assert(sizeof(x_aImpl) == sizeof(::SRWLOCK), "!");

	::InitializeSRWLock(reinterpret_cast<::SRWLOCK *>(x_aImpl));

	SetSpinCount(uSpinCount);
}

// 其他非静态成员函数。
std::size_t ReaderWriterMutex::GetSpinCount() const noexcept {
	return x_uSpinCount.Load(kAtomicRelaxed);
}
void ReaderWriterMutex::SetSpinCount(std::size_t uSpinCount) noexcept {
	if(GetLogicalProcessorCount() == 0){
		return;
	}
	x_uSpinCount.Store(uSpinCount, kAtomicRelaxed);
}

bool ReaderWriterMutex::TryAsReader() noexcept {
	return ::TryAcquireSRWLockShared(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}
void ReaderWriterMutex::LockAsReader() noexcept {
	if(::TryAcquireSRWLockShared(reinterpret_cast<::SRWLOCK *>(x_aImpl))){
		return;
	}

	const auto uSpinCount = GetSpinCount() / 0x400; // FIXME: SRWLOCK 里面自己实现了一个自旋。
	for(std::size_t i = 0; i < uSpinCount; ++i){
		::SwitchToThread();

		if(::TryAcquireSRWLockShared(reinterpret_cast<::SRWLOCK *>(x_aImpl))){
			return;
		}
	}

	::AcquireSRWLockShared(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}
void ReaderWriterMutex::UnlockAsReader() noexcept {
	::ReleaseSRWLockShared(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}

bool ReaderWriterMutex::TryAsWriter() noexcept {
	return ::TryAcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}
void ReaderWriterMutex::LockAsWriter() noexcept {
	if(::TryAcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl))){
		return;
	}

	const auto uSpinCount = GetSpinCount() / 0x400; // FIXME: SRWLOCK 里面自己实现了一个自旋。
	for(std::size_t i = 0; i < uSpinCount; ++i){
		::SwitchToThread();

		if(::TryAcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl))){
			return;
		}
	}

	::AcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}
void ReaderWriterMutex::UnlockAsWriter() noexcept {
	::ReleaseSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(x_aImpl));
}

}
