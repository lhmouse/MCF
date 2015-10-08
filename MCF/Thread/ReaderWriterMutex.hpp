// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_READER_WRITER_MUTEX_HPP_
#define MCF_THREAD_READER_WRITER_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Atomic.hpp"
#include <cstddef>

namespace MCF {

class ConditionVariable;

class ReaderWriterMutex : NONCOPYABLE {
	friend ConditionVariable;

public:
	using UniqueReaderLock = Impl_UniqueLockTemplate::UniqueLockTemplate<ReaderWriterMutex, 0u>;
	using UniqueWriterLock = Impl_UniqueLockTemplate::UniqueLockTemplate<ReaderWriterMutex, 1u>;

private:
	Atomic<std::size_t> x_uSpinCount;

	std::intptr_t x_aImpl[1];

public:
	explicit ReaderWriterMutex(std::size_t uSpinCount = 0x400) noexcept;

public:
	std::size_t GetSpinCount() const noexcept;
	void SetSpinCount(std::size_t uSpinCount) noexcept;

	bool TryAsReader() noexcept;
	void LockAsReader() noexcept;
	void UnlockAsReader() noexcept;

	UniqueReaderLock TryReaderLock() noexcept {
		UniqueReaderLock vLock(*this, false);
		vLock.Try();
		return vLock;
	}
	UniqueReaderLock GetReaderLock() noexcept {
		return UniqueReaderLock(*this);
	}

	bool TryAsWriter() noexcept;
	void LockAsWriter() noexcept;
	void UnlockAsWriter() noexcept;

	UniqueWriterLock TryWriterLock() noexcept {
		UniqueWriterLock vLock(*this, false);
		vLock.Try();
		return vLock;
	}
	UniqueWriterLock GetWriterLock() noexcept {
		return UniqueWriterLock(*this);
	}
};

}

#endif
