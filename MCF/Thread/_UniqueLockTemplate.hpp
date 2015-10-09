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

namespace Impl_UniqueLockTemplate {
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
		std::size_t X_UnlockAll() noexcept {
			const auto uOldLockCount = x_uLockCount;
			if(uOldLockCount != 0){
				X_DoUnlock();
				x_uLockCount = 0;
			}
			return uOldLockCount;
		}

	private:
		virtual bool X_DoTry() const noexcept = 0;
		virtual void X_DoLock() const noexcept = 0;
		virtual void X_DoUnlock() const noexcept = 0;

	public:
		bool IsLocking() const noexcept {
			return x_uLockCount > 0;
		}
		std::size_t GetLockCount() const noexcept {
			return x_uLockCount;
		}

		bool Try() noexcept {
			const auto uOldCount = x_uLockCount;
			if(uOldCount == 0){
				if(!X_DoTry()){
					return false;
				}
			}
			x_uLockCount = uOldCount + 1;
			return true;
		}
		void Lock() noexcept {
			const auto uOldCount = x_uLockCount;
			if(uOldCount == 0){
				X_DoLock();
			}
			x_uLockCount = uOldCount + 1;
		}
		void Unlock() noexcept {
			ASSERT(x_uLockCount != 0);

			const auto uOldCount = x_uLockCount;
			x_uLockCount = uOldCount - 1;
			if(uOldCount == 1){
				X_DoUnlock();
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

			X_UnlockAll();
			Swap(rhs);
			return *this;
		}
		virtual ~UniqueLockTemplate(){
			X_UnlockAll();
		}

	private:
		bool X_DoTry() const noexcept override;
		void X_DoLock() const noexcept override;
		void X_DoUnlock() const noexcept override;

	public:
		MutexT &GetOwner() const noexcept {
			return *x_pOwner;
		}

		void Merge(UniqueLockTemplate &&rhs) noexcept {
			ASSERT(x_pOwner == rhs.x_pOwner);

			x_uLockCount += rhs.x_uLockCount;
			rhs.x_uLockCount = 0;
		}

		void Swap(UniqueLockTemplate &rhs) noexcept {
			using std::swap;
			swap(x_pOwner,     rhs.x_pOwner);
			swap(x_uLockCount, rhs.x_uLockCount);
		}
	};

	template<class MutexT, std::size_t kLockTypeT>
	void swap(Impl_UniqueLockTemplate::UniqueLockTemplate<MutexT, kLockTypeT> &lhs, Impl_UniqueLockTemplate::UniqueLockTemplate<MutexT, kLockTypeT> &rhs) noexcept {
		lhs.Swap(rhs);
	}
}

using Impl_UniqueLockTemplate::swap;

}

#endif
