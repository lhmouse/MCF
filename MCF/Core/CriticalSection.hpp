// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRITICAL_SECTION_HPP__
#define __MCF_CRITICAL_SECTION_HPP__

#include "Utilities.hpp"
#include "StringObserver.hpp"
#include <memory>

namespace MCF {

class CriticalSection : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<CriticalSection> Create(unsigned long ulSpinCount = 0x400);

public:
	bool Try() noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;
};


class CriticalSectionLock : NO_COPY {
private:
	CriticalSection *xm_pOwner;

public:
	constexpr explicit CriticalSectionLock() noexcept
		: xm_pOwner(nullptr)
	{
	}
	constexpr explicit CriticalSectionLock(std::nullptr_t) noexcept
		: xm_pOwner()
	{
	}
	explicit CriticalSectionLock(CriticalSection *pOwner) noexcept
		: xm_pOwner()
	{
		Lock(pOwner);
	}
	CriticalSectionLock &operator=(CriticalSection *pOwner) noexcept {
		Lock(pOwner);
		return *this;
	}
	~CriticalSectionLock(){
		Unlock();
	}

public:
	bool IsLocking() const noexcept {
		return !!xm_pOwner;
	}
	void Try(CriticalSection *pNewOwner) noexcept {
		if(xm_pOwner){
			xm_pOwner->Unlock();
		}
		if(pNewOwner){
			if(!pNewOwner->Try()){
				pNewOwner = nullptr;
			}
		}
		xm_pOwner = pNewOwner;
	}
	void Lock(CriticalSection *pNewOwner) noexcept {
		if(xm_pOwner){
			xm_pOwner->Unlock();
		}
		if(pNewOwner){
			pNewOwner->Lock();
		}
		xm_pOwner = pNewOwner;
	}
	void Unlock() noexcept {
		Lock(nullptr);
	}

public:
	explicit operator bool() const noexcept {
		return IsLocking();
	}
};

}

#define CRITICAL_SECTION_SCOPE(pcs)	\
	for(const ::MCF::CriticalSectionLock __MCF_LOCK(&*(pcs)), *__MCF_PLOCK = &__MCF_LOCK;	\
		__MCF_PLOCK;	\
		__MCF_PLOCK = nullptr)

#endif
