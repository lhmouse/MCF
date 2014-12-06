// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_READER_WRITER_LOCK_HPP_
#define MCF_THREAD_READER_WRITER_LOCK_HPP_

#include "LockRaiiTemplate.hpp"
#include "../Utilities/NoCopy.hpp"
#include "../Utilities/Abstract.hpp"
#include <memory>

namespace MCF {

class ReaderWriterLock : NO_COPY, ABSTRACT {
public:
	using ReaderLock = Impl::LockRaiiTemplate<ReaderWriterLock, 0u>;
	using WriterLock = Impl::LockRaiiTemplate<ReaderWriterLock, 1u>;

public:
	static std::unique_ptr<ReaderWriterLock> Create(unsigned long ulSpinCount = 0x400);

public:
	unsigned long GetSpinCount() const noexcept;
	void SetSpinCount(unsigned long ulSpinCount) noexcept;

	ReaderLock TryReaderLock() noexcept;
	ReaderLock GetReaderLock() noexcept;
	WriterLock TryWriterLock() noexcept;
	WriterLock GetWriterLock() noexcept;
};

}

#endif
