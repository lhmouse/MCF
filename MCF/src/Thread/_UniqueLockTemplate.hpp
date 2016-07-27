// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_UNIQUE_LOCK_TEMPLATE_HPP_
#define MCF_THREAD_UNIQUE_LOCK_TEMPLATE_HPP_

#include "UniqueLockBase.hpp"
#include "../Core/Assert.hpp"
#include <utility>

namespace MCF {

namespace Impl_UniqueLockTemplate {
	template<class MutexT, int kLockType = 0>
	class UniqueLockTemplate final : public UniqueLockBase {
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
			MCF_ASSERT(&rhs != this);

			Y_UnlockAll();
			Swap(rhs);
			return *this;
		}
		virtual ~UniqueLockTemplate(){
			Y_UnlockAll();
		}

	private:
		bool X_DoTry(std::uint64_t u64MilliSeconds) const noexcept override;
		void X_DoLock() const noexcept override;
		void X_DoUnlock() const noexcept override;

	public:
		MutexT &GetOwner() const noexcept {
			return *x_pOwner;
		}

		void Merge(UniqueLockTemplate &&rhs) noexcept {
			MCF_ASSERT(x_pOwner == rhs.x_pOwner);

			y_uLockCount += rhs.y_uLockCount;
			rhs.y_uLockCount = 0;
		}

		void Swap(UniqueLockTemplate &rhs) noexcept {
			using std::swap;
			swap(x_pOwner,     rhs.x_pOwner);
			swap(y_uLockCount, rhs.y_uLockCount);
		}

	public:
		friend void swap(UniqueLockTemplate &lhs, UniqueLockTemplate &rhs) noexcept {
			lhs.Swap(rhs);
		}
	};
}

}

#endif
