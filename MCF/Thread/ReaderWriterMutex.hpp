// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_READER_WRITER_MUTEX_HPP_
#define MCF_THREAD_READER_WRITER_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/UniqueHandle.hpp"
#include "UniqueLockTemplate.hpp"
#include "UserRecursiveMutex.hpp"
#include "Semaphore.hpp"
#include <cstddef>

namespace MCF {

class ReaderWriterMutex : NONCOPYABLE {
private:
	struct xTlsIndexDeleter {
		std::size_t operator()() const noexcept;
		void operator()(std::size_t uTlsIndex) const noexcept;
	};

public:
	using Result = UserRecursiveMutexResult;
	using UniqueReaderLock = UniqueLockTemplate<ReaderWriterMutex, 0u>;
	using UniqueWriterLock = UniqueLockTemplate<ReaderWriterMutex, 1u>;

private:
	UserRecursiveMutex xm_mtxWriterGuard;
	Semaphore xm_semExclusive;
	volatile std::size_t xm_uReaderCount;
	UniqueHandle<xTlsIndexDeleter> xm_uTlsIndex;

public:
	explicit ReaderWriterMutex(std::size_t uSpinCount = 0x400);

public:
	std::size_t GetSpinCount() const noexcept {
		return xm_mtxWriterGuard.GetSpinCount();
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		xm_mtxWriterGuard.SetSpinCount(uSpinCount);
	}

	Result TryAsReader() noexcept;
	Result LockAsReader() noexcept;
	Result UnlockAsReader() noexcept;

	Result TryAsWriter() noexcept;
	Result LockAsWriter() noexcept;
	Result UnlockAsWriter() noexcept;

	UniqueReaderLock TryReaderLock() noexcept;
	UniqueReaderLock GetReaderLock() noexcept;

	UniqueWriterLock TryWriterLock() noexcept;
	UniqueWriterLock GetWriterLock() noexcept;
};

}

#endif
