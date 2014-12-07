// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_READER_WRITER_LOCK_HPP_
#define MCF_THREAD_READER_WRITER_LOCK_HPP_

#include "LockRaiiTemplate.hpp"
#include "../Utilities/NoCopy.hpp"
#include "CriticalSection.hpp"
#include "Semaphore.hpp"
#include "../Core/UniqueHandle.hpp"

namespace MCF {

class ReaderWriterLock : NO_COPY, public CriticalSectionResults {
private:
	struct xTlsIndexDeleter {
		unsigned long operator()() const noexcept;
		void operator()(unsigned long ulTlsIndex) const noexcept;
	};

public:
	using ReaderLock = LockRaiiTemplate<ReaderWriterLock, 0u>;
	using WriterLock = LockRaiiTemplate<ReaderWriterLock, 1u>;

private:
	CriticalSection xm_csWriterGuard;
	Semaphore xm_semExclusive;
	volatile unsigned long xm_ulReaderCount;
	UniqueHandle<xTlsIndexDeleter> xm_ulTlsIndex;

public:
	explicit ReaderWriterLock(unsigned long ulSpinCount = 0x400);

public:
	unsigned long GetSpinCount() const noexcept {
		return xm_csWriterGuard.GetSpinCount();
	}
	void SetSpinCount(unsigned long ulSpinCount) noexcept {
		xm_csWriterGuard.SetSpinCount(ulSpinCount);
	}

	Result TryAsReader() noexcept;
	Result WaitAsReader() noexcept;
	Result ReleaseAsReader() noexcept;

	Result TryAsWriter() noexcept;
	Result WaitAsWriter() noexcept;
	Result ReleaseAsWriter() noexcept;

	ReaderLock TryReaderLock() noexcept;
	ReaderLock GetReaderLock() noexcept;

	WriterLock TryWriterLock() noexcept;
	WriterLock GetWriterLock() noexcept;
};

}

#endif
