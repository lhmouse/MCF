// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_EXCLUSIVE_LOCK_TEMPLATE_HPP_
#define MCF_EXCLUSIVE_LOCK_TEMPLATE_HPP_

#include "../Core/Utilities.hpp"
#include <cstddef>

namespace MCF {

class LockRaiiTemplateBase {
public:
	virtual ~LockRaiiTemplateBase() noexcept {
	}

public:
	virtual void Lock() noexcept = 0;
	virtual void Unlock() noexcept = 0;
};

namespace Impl {
	template<class Mutex_t, std::size_t LOCK_TYPE = 0>
	class LockRaiiTemplateTemplate : public LockRaiiTemplateBase {
	private:
		Mutex_t *xm_pOwner;
		std::size_t xm_uLockCount;

	public:
		explicit LockRaiiTemplateTemplate(Mutex_t *pOwner, bool bInitLocked = true) noexcept
			: xm_pOwner		(pOwner)
			, xm_uLockCount	(0)
		{
			if(bInitLocked){
				Lock();
			}
		}
		LockRaiiTemplateTemplate(const LockRaiiTemplateTemplate &rhs) noexcept
			: xm_pOwner		(rhs.xm_pOwner)
			, xm_uLockCount	(0)
		{
			if(rhs.IsLocking()){
				Lock();
			}
		}
		LockRaiiTemplateTemplate(LockRaiiTemplateTemplate &&rhs) noexcept
			: xm_pOwner		(rhs.xm_pOwner)
			, xm_uLockCount	(rhs.xm_uLockCount)
		{
			rhs.xm_uLockCount = 0;
		}
		LockRaiiTemplateTemplate &operator=(const LockRaiiTemplateTemplate &rhs) noexcept {
			if(this != &rhs){
				UnlockAll();

				xm_pOwner = rhs.xm_pOwner;
				if(rhs.IsLocking()){
					Lock();
				}
			}
			return *this;
		}
		LockRaiiTemplateTemplate &operator=(LockRaiiTemplateTemplate &&rhs) noexcept {
			if(this != &rhs){
				UnlockAll();

				xm_pOwner = rhs.xm_pOwner;
				xm_uLockCount = rhs.xm_uLockCount;
				rhs.xm_uLockCount = 0;
			}
			return *this;
		}
		virtual ~LockRaiiTemplateTemplate() noexcept {
			UnlockAll();
		}

	private:
		void xDoLock() const noexcept;
		void xDoUnlock() const noexcept;

	public:
		bool IsLocking() const noexcept {
			return xm_uLockCount > 0;
		}
		void Lock(std::size_t uCount = 1) noexcept override {
			if(xm_uLockCount == 0){
				xDoLock();
			}
			xm_uLockCount += uCount;
		}
		void Unlock() noexcept override {
			ASSERT(xm_uLockCount > 0);

			if(--xm_uLockCount == 0){
				xDoUnlock();
			}
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
}

}

#endif
