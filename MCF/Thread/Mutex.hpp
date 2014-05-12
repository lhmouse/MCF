// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_MUTEX_HPP_
#define MCF_MUTEX_HPP_

#include "_ThreadSyncLockTemplate.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/StringObserver.hpp"
#include <memory>

namespace MCF {

class Mutex : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<Mutex> Create(const WideStringObserver &wsoName = nullptr);

public:
	void Lock() noexcept;
	void Unlock() noexcept;
};

template class Impl::ThreadSyncLockTemplate<
	Mutex,
	&Mutex::Lock,
	&Mutex::Unlock
>;

typedef Impl::ThreadSyncLockTemplate<
	Mutex,
	&Mutex::Lock,
	&Mutex::Unlock
> CriticalSectionLock;

}

#define MCF_MUTEX_SCOPE(pmtx)	\
	for(const ::MCF::MutexLock MCF_vLock_(	\
			[&]() noexcept { return &*(pcs); }()	\
		), *MCF_pLock_ = &MCF_vLock_;	\
		MCF_pLock_;	\
		MCF_pLock_ = nullptr)

#endif
