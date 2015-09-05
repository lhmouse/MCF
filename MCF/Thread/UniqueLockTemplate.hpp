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
	std::size_t $uLockCount;

protected:
	constexpr UniqueLockTemplateBase() noexcept
		: $uLockCount(0)
	{
	}

public:
	virtual ~UniqueLockTemplateBase(){
		ASSERT($uLockCount == 0);
	}

protected:
	std::size_t $UnlockAll() noexcept {
		const auto uOldLockCount = $uLockCount;
		if(uOldLockCount != 0){
			$DoUnlock();
			$uLockCount = 0;
		}
		return uOldLockCount;
	}

private:
	virtual bool $DoTry() const noexcept = 0;
	virtual void $DoLock() const noexcept = 0;
	virtual void $DoUnlock() const noexcept = 0;

public:
	bool IsLocking() const noexcept {
		return $uLockCount > 0;
	}
	std::size_t GetLockCount() const noexcept {
		return $uLockCount;
	}

	bool Try() noexcept {
		if($uLockCount == 0){
			if(!$DoTry()){
				return false;
			}
		}
		++$uLockCount;
		return true;
	}
	void Lock() noexcept {
		if(++$uLockCount == 1){
			$DoLock();
		}
	}
	void Unlock() noexcept {
		ASSERT($uLockCount != 0);

		if(--$uLockCount == 0){
			$DoUnlock();
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
	MutexT *$pOwner;

public:
	explicit UniqueLockTemplate(MutexT &vOwner, bool bInitLocked = true) noexcept
		: $pOwner(&vOwner)
	{
		if(bInitLocked){
			Lock();
		}
	}
	UniqueLockTemplate(UniqueLockTemplate &&rhs) noexcept
		: $pOwner(rhs.$pOwner)
	{
		Swap(rhs);
	}
	UniqueLockTemplate &operator=(UniqueLockTemplate &&rhs) noexcept {
		ASSERT(&rhs != this);

		$UnlockAll();
		Swap(rhs);
		return *this;
	}
	virtual ~UniqueLockTemplate(){
		$UnlockAll();
	}

private:
	bool $DoTry() const noexcept override;
	void $DoLock() const noexcept override;
	void $DoUnlock() const noexcept override;

public:
	void Join(UniqueLockTemplate &&rhs) noexcept {
		ASSERT($pOwner == rhs.$pOwner);

		$uLockCount += rhs.$uLockCount;
		rhs.$uLockCount = 0;
	}

	void Swap(UniqueLockTemplate &rhs) noexcept {
		std::swap($pOwner, rhs.$pOwner);
		std::swap($uLockCount, rhs.$uLockCount);
	}
};

template<class MutexT, std::size_t kLockTypeT>
void swap(UniqueLockTemplate<MutexT, kLockTypeT> &lhs,
	UniqueLockTemplate<MutexT, kLockTypeT> &rhs) noexcept
{
	lhs.Swap(rhs);
}

}

#endif
