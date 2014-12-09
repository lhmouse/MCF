// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_UNIQUE_LOCK_TEMPLATE_HPP_
#define MCF_THREAD_UNIQUE_LOCK_TEMPLATE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Abstract.hpp"
#include "../Utilities/Assert.hpp"
#include <utility>

namespace MCF {

class UniqueLockTemplateBase : NONCOPYABLE, ABSTRACT {
protected:
	unsigned long xm_ulLockCount;

protected:
	constexpr UniqueLockTemplateBase() noexcept
		: xm_ulLockCount(0)
	{
	}

public:
	virtual ~UniqueLockTemplateBase(){
		ASSERT(xm_ulLockCount == 0);
	}

private:
	virtual bool xDoTry() const noexcept = 0;
	virtual void xDoLock() const noexcept = 0;
	virtual void xDoUnlock() const noexcept = 0;

public:
	bool IsLocking() const noexcept {
		return xm_ulLockCount > 0;
	}
	unsigned long GetLockCount() const noexcept {
		return xm_ulLockCount;
	}

	bool Try() noexcept {
		if(xm_ulLockCount == 0){
			if(!xDoTry()){
				return false;
			}
		}
		++xm_ulLockCount;
		return true;
	}
	void Lock() noexcept {
		if(++xm_ulLockCount == 1){
			xDoLock();
		}
	}
	void Unlock() noexcept {
		ASSERT(xm_ulLockCount != 0);

		if(--xm_ulLockCount == 0){
			xDoUnlock();
		}
	}

public:
	explicit operator bool() const noexcept {
		return IsLocking();
	}
};

template<class MutexT, unsigned LOCK_TYPE_T = 0>
class UniqueLockTemplate final : CONCRETE(UniqueLockTemplateBase) {
private:
	MutexT *xm_pOwner;

public:
	explicit UniqueLockTemplate(MutexT &vOwner, bool bInitLocked = true) noexcept
		: xm_pOwner(&vOwner)
	{
		if(bInitLocked){
			Lock();
		}
	}
	UniqueLockTemplate(UniqueLockTemplate &&rhs) noexcept
		: xm_pOwner(rhs.xm_pOwner)
	{
		xm_ulLockCount = std::exchange(rhs.xm_ulLockCount, 0u);
	}
	UniqueLockTemplate &operator=(UniqueLockTemplate &&rhs) noexcept {
		ASSERT(&rhs != this);

		if(xm_ulLockCount != 0){
			xDoUnlock();
		}
		xm_pOwner = rhs.xm_pOwner;
		xm_ulLockCount = std::exchange(rhs.xm_ulLockCount, 0u);
		return *this;
	}
	virtual ~UniqueLockTemplate(){
		if(xm_ulLockCount != 0){
			xDoUnlock();
		}
		xm_ulLockCount = 0;
	}

private:
	bool xDoTry() const noexcept override;
	void xDoLock() const noexcept override;
	void xDoUnlock() const noexcept override;

public:
	void Join(UniqueLockTemplate &&rhs) noexcept {
		ASSERT(xm_pOwner == rhs.xm_pOwner);

		xm_ulLockCount += std::exchange(rhs.xm_ulLockCount, 0u);
	}

	void Swap(UniqueLockTemplate &rhs) noexcept {
		std::swap(xm_pOwner, rhs.xm_pOwner);
		std::swap(xm_ulLockCount, rhs.xm_ulLockCount);
	}
};

template<class MutexT, unsigned long LOCK_TYPE_T>
void swap(UniqueLockTemplate<MutexT, LOCK_TYPE_T> &lhs,
	UniqueLockTemplate<MutexT, LOCK_TYPE_T> &rhs) noexcept
{
	lhs.Swap(rhs);
}

}

#endif
