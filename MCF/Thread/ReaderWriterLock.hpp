// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_READER_WRITER_LOCK_HPP_
#define MCF_READER_WRITER_LOCK_HPP_

#include "_LockRaiiTemplate.hpp"
#include "../Core/Utilities.hpp"
#include <memory>

namespace MCF {

class ReaderWriterLock : NO_COPY, ABSTRACT {
public:
	typedef Impl::LockRaiiTemplate<ReaderWriterLock, 0u> ReaderLock;
	typedef Impl::LockRaiiTemplate<ReaderWriterLock, 1u> WriterLock;

public:
	static std::unique_ptr<ReaderWriterLock> Create(unsigned long ulSpinCount = 0x400);

public:
	unsigned long GetSpinCount() const noexcept;
	void SetSpinCount(unsigned long ulSpinCount) noexcept;

	ReaderLock GetReaderLock() noexcept;
	WriterLock GetWriterLock() noexcept;
};

}

#endif
