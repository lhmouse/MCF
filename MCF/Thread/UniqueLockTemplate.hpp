// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_UNIQUE_LOCK_TEMPLATE_HPP_
#define MCF_THREAD_UNIQUE_LOCK_TEMPLATE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include <utility>
#include <cstddef>

namespace MCF {

class UniqueLockTemplateBase : NONCOPYABLE {
protected:
	std::size_t x_uLockCount;

protected:
	constexpr UniqueLockTemplateBase() noexcept
		: x_uLockCount(0)
	{
	}

public:
	virtual ~UniqueLockTemplateBase(){
		ASSERT(x_uLockCount == 0);
	}

protected:
	std::size_t XUnlockAll() noexcept {
		const auto uOldLockCount = x_uLockCount;
		if(uOldLockCount != 0){
			XDoUnlock();
			x_uLockCount = 0;
		}
		return uOldLockCount;
	}

private:
	virtual bool XDoTry() const noexcept = 0;
	virtual void XDoLock() const noexcept = 0;
	virtual void XDoUnlock() const noexcept = 0;

public:
	bool IsLocking() const noexcept {
		return x_uLockCount > 0;
	}
	std::size_t GetLockCount() const noexcept {
		return x_uLockCount;
	}

	bool Try() noexcept {
		if(x_uLockCount == 0){
			if(!XDoTry()){
				return false;
			}
		}
		++x_uLockCount;
		return true;
	}
	void Lock() noexcept {
		if(++x_uLockCount == 1){
			XDoLock();
		}
	}
	void Unlock() noexcept {
		ASSERT(x_uLockCount != 0);

		if(--x_uLockCount == 0){
			XDoUnlock();
		}
	}

public:
	explicit operator bool() const noexcept {
		return IsLocking();
	}
};

template<class MutexT, std::size_t kLockType = 0>
class UniqueLockTemplate final : public UniqueLockTemplateBase {
private:
	MutexT *x_pOwner;

public:
	explicit UniqueLockTemplate(MutexT &vOwner, bool bInitLocked = true) noexcept
		: x_pOwner(&vOwner)
	{
		if(bInitLocked){
			Lock();
		}
	}
	UniqueLockTemplate(UniqueLockTemplate &&rhs) noexcept
		: x_pOwner(rhs.x_pOwner)
	{
		Swap(rhs);
	}
	UniqueLockTemplate &operator=(UniqueLockTemplate &&rhs) noexcept {
		ASSERT(&rhs != this);

		XUnlockAll();
		Swap(rhs);
		return *this;
	}
	virtual ~UniqueLockTemplate(){
		XUnlockAll();
	}

private:
	bool XDoTry() const noexcept override;
	void XDoLock() const noexcept override;
	void XDoUnlock() const noexcept override;

public:
	void Join(UniqueLockTemplate &&rhs) noexcept {
		ASSERT(x_pOwner == rhs.x_pOwner);

		x_uLockCount += rhs.x_uLockCount;
		rhs.x_uLockCount = 0;
	}

	void Swap(UniqueLockTemplate &rhs) noexcept {
		std::swap(x_pOwner,     rhs.x_pOwner);
		std::swap(x_uLockCount, rhs.x_uLockCount);
	}
};

template<class MutexT, std::size_t kLockTypeT>
void swap(UniqueLockTemplate<MutexT, kLockTypeT> &lhs, UniqueLockTemplate<MutexT, kLockTypeT> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
