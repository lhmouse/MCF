// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRITICAL_SECTION_HPP_
#define MCF_CRITICAL_SECTION_HPP_

#include "_ThreadSyncLockTemplate.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/StringObserver.hpp"
#include <memory>

namespace MCF {

class CriticalSection : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<CriticalSection> Create(unsigned long ulSpinCount = 0x400);

public:
	void Lock() noexcept;
	void Unlock() noexcept;
};

template class Impl::ThreadSyncLockTemplate<
	CriticalSection,
	&CriticalSection::Lock,
	&CriticalSection::Unlock
>;

typedef Impl::ThreadSyncLockTemplate<
	CriticalSection,
	&CriticalSection::Lock,
	&CriticalSection::Unlock
> CriticalSectionLock;

}

#define MCF_CRIT_SECT_SCOPE(pcs)	\
	for(const ::MCF::CriticalSectionLock MCF_vLock_(	\
			[&]() noexcept { return &*(pcs); }()	\
		), *MCF_pLock_ = &MCF_vLock_;	\
		MCF_pLock_;	\
		MCF_pLock_ = nullptr)

#endif
