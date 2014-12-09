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

namespace MCF {

class ReaderWriterMutex : NONCOPYABLE {
private:
	struct xTlsIndexDeleter {
		unsigned long operator()() const noexcept;
		void operator()(unsigned long ulTlsIndex) const noexcept;
	};

public:
	using Result = UserRecursiveMutexResult;
	using UniqueReaderLock = UniqueLockTemplate<ReaderWriterMutex, 0u>;
	using UniqueWriterLock = UniqueLockTemplate<ReaderWriterMutex, 1u>;

private:
	UserRecursiveMutex xm_mtxWriterGuard;
	Semaphore xm_semExclusive;
	volatile unsigned long xm_ulReaderCount;
	UniqueHandle<xTlsIndexDeleter> xm_ulTlsIndex;

public:
	explicit ReaderWriterMutex(unsigned long ulSpinCount = 0x400);

public:
	unsigned long GetSpinCount() const noexcept {
		return xm_mtxWriterGuard.GetSpinCount();
	}
	void SetSpinCount(unsigned long ulSpinCount) noexcept {
		xm_mtxWriterGuard.SetSpinCount(ulSpinCount);
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
