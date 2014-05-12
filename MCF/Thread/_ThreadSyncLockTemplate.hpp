// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_SYNC_LOCK_TEMPLATE_HPP_
#define MCF_THREAD_SYNC_LOCK_TEMPLATE_HPP_

#include "../Core/Utilities.hpp"

namespace MCF {

namespace Impl {
	template<class Mutex_t, void (Mutex_t::*LOCK_FN)(), void (Mutex_t::*UNLOCK_FN)()>
	class ThreadSyncLockTemplate : NO_COPY {
	private:
		Mutex_t *xm_pOwner;

	public:
		constexpr explicit ThreadSyncLockTemplate() noexcept
			: xm_pOwner(nullptr)
		{
		}
		constexpr explicit ThreadSyncLockTemplate(std::nullptr_t) noexcept
			: ThreadSyncLockTemplate()
		{
		}
		explicit ThreadSyncLockTemplate(Mutex_t *pOwner) noexcept
			: ThreadSyncLockTemplate()
		{
			Lock(pOwner);
		}
		ThreadSyncLockTemplate &operator=(Mutex_t *pOwner) noexcept {
			Lock(pOwner);
			return *this;
		}
		~ThreadSyncLockTemplate() noexcept {
			Unlock();
		}

	public:
		bool IsLocking() const noexcept {
			return !!xm_pOwner;
		}
		void Lock(Mutex_t *pNewOwner) noexcept {
			ASSERT_NOEXCEPT_BEGIN
			{
				if(xm_pOwner){
					(xm_pOwner->*UNLOCK_FN)();
				}
				if(pNewOwner){
					(pNewOwner->*LOCK_FN)();
				}
				xm_pOwner = pNewOwner;
			}
			ASSERT_NOEXCEPT_END
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

}

#endif
