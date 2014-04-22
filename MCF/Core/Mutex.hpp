// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_MUTEX_HPP_
#define MCF_MUTEX_HPP_

#include "Utilities.hpp"
#include "StringObserver.hpp"
#include <memory>

namespace MCF {

class Mutex : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<Mutex> Create(const WideStringObserver &wsoName = nullptr);

public:
	bool Try(unsigned long ulMilliSeconds) noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;
};


class MutexLock : NO_COPY {
private:
	Mutex *xm_pOwner;

public:
	constexpr explicit MutexLock() noexcept
		: xm_pOwner(nullptr)
	{
	}
	constexpr explicit MutexLock(std::nullptr_t) noexcept
		: xm_pOwner()
	{
	}
	explicit MutexLock(Mutex *pOwner) noexcept
		: xm_pOwner()
	{
		Lock(pOwner);
	}
	MutexLock &operator=(Mutex *pOwner) noexcept {
		Lock(pOwner);
		return *this;
	}
	~MutexLock(){
		Unlock();
	}

public:
	bool IsLocking() const noexcept {
		return !!xm_pOwner;
	}
	void Try(Mutex *pNewOwner, unsigned long ulMilliSeconds) noexcept {
		if(xm_pOwner){
			xm_pOwner->Unlock();
		}
		if(pNewOwner){
			if(!pNewOwner->Try(ulMilliSeconds)){
				pNewOwner = nullptr;
			}
		}
		xm_pOwner = pNewOwner;
	}
	void Lock(Mutex *pNewOwner) noexcept {
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

#define MUTEX_SCOPE(pmtx)	\
	for(const ::MCF::MutexLock vMCF_Lock(&*(pmtx)), *pMCF_Lock = &vMCF_Lock;	\
		pMCF_Lock;	\
		pMCF_Lock = nullptr)

#endif
