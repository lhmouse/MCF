// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_LOCK_RAII_TEMPLATE_HPP_
#define MCF_LOCK_RAII_TEMPLATE_HPP_

#include "../Core/Utilities.hpp"
#include <cstddef>

namespace MCF {

class LockRaiiTemplateBase : NO_COPY, ABSTRACT {
protected:
	std::size_t xm_uLockCount;

protected:
	constexpr LockRaiiTemplateBase() noexcept
		: xm_uLockCount(0)
	{
	}

public:
	virtual ~LockRaiiTemplateBase() noexcept {
		ASSERT(xm_uLockCount == 0);
	}

private:
	virtual void xDoLock() const noexcept = 0;
	virtual void xDoUnlock() const noexcept = 0;

public:
	bool IsLocking() const noexcept {
		return xm_uLockCount > 0;
	}
	bool Lock(std::size_t uCount = 1) noexcept {
		bool bRet = false;
		if(xm_uLockCount == 0){
			xDoLock();
			bRet = true;
		}
		xm_uLockCount += uCount;
		return bRet;
	}
	bool Unlock() noexcept {
		ASSERT(xm_uLockCount > 0);

		bool bRet = false;
		if(--xm_uLockCount == 0){
			xDoUnlock();
			bRet = true;
		}
		return bRet;
	}
	std::size_t UnlockAll() noexcept {
		const auto uCount = xm_uLockCount;
		if(xm_uLockCount > 0){
			xDoUnlock();
			xm_uLockCount = 0;
		}
		return uCount;
	}

public:
	explicit operator bool() const noexcept {
		return IsLocking();
	}
};

namespace Impl {
	template<class Mutex_t, std::size_t LOCK_TYPE = 0>
	class LockRaiiTemplate : CONCRETE(LockRaiiTemplateBase) {
	private:
		Mutex_t *xm_pOwner;

	public:
		explicit LockRaiiTemplate(Mutex_t *pOwner, bool bInitLocked = true) noexcept
			: xm_pOwner(pOwner)
		{
			if(bInitLocked){
				Lock();
			}
		}
		LockRaiiTemplate(LockRaiiTemplate &&rhs) noexcept
			: xm_pOwner(rhs.xm_pOwner)
		{
			xm_uLockCount = std::exchange(rhs.xm_uLockCount, 0u);
		}
		LockRaiiTemplate &operator=(LockRaiiTemplate &&rhs) noexcept {
			if(this != &rhs){
				UnlockAll();

				xm_pOwner = rhs.xm_pOwner;
				xm_uLockCount = std::exchange(rhs.xm_uLockCount, 0u);
			}
			return *this;
		}
		virtual ~LockRaiiTemplate() noexcept {
			UnlockAll();
		}

	private:
		void xDoLock() const noexcept override;
		void xDoUnlock() const noexcept override;

	public:
		void Join(LockRaiiTemplate &&rhs) noexcept {
			ASSERT(xm_pOwner == rhs.xm_pOwner);

			xm_uLockCount += std::exchange(rhs.xm_uLockCount, 0u);
		}

		void Swap(LockRaiiTemplate &rhs) noexcept {
			std::swap(xm_pOwner, rhs.xm_pOwner);
			std::swap(xm_uLockCount, rhs.xm_uLockCount);
		}
	};
}

}

#endif
