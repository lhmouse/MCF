// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRITICAL_SECTION_HPP_
#define MCF_CRITICAL_SECTION_HPP_

#include "_LockRaiiTemplate.hpp"
#include "../Core/Utilities.hpp"
#include <memory>

namespace MCF {

class CriticalSection : NO_COPY, ABSTRACT {
public:
	typedef Impl::LockRaiiTemplate<CriticalSection> Lock;

public:
	static std::unique_ptr<CriticalSection> Create(unsigned long ulSpinCount = 0x400);

public:
	unsigned long GetSpinCount() const noexcept;
	void SetSpinCount(unsigned long ulSpinCount) noexcept;

	bool IsLockedByCurrentThread() const noexcept;

	Lock GetLock() noexcept;
};

}

#endif
