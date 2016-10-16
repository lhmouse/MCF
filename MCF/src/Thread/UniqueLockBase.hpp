// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_UNIQUE_LOCK_BASE_HPP_
#define MCF_THREAD_UNIQUE_LOCK_BASE_HPP_

#include "../Core/Noncopyable.hpp"
#include "../Core/Assert.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class ConditionVariable;

class UniqueLockBase : MCF_NONCOPYABLE {
	friend ConditionVariable;

protected:
	std::size_t y_uLockCount;

protected:
	constexpr UniqueLockBase() noexcept
		: y_uLockCount(0)
	{
	}

public:
	virtual ~UniqueLockBase();

protected:
	std::size_t Y_UnlockAll() noexcept {
		const auto uOldCount = y_uLockCount;
		if(uOldCount != 0){
			X_DoUnlock();
			y_uLockCount = 0;
		}
		return uOldCount;
	}
	void Y_RelockAll(std::size_t uNewCount) noexcept {
		MCF_ASSERT(y_uLockCount == 0);

		if(uNewCount != 0){
			X_DoLock();
			y_uLockCount = uNewCount;
		}
	}

	void Y_Swap(UniqueLockBase &rhs) noexcept {
		using std::swap;
		swap(y_uLockCount, rhs.y_uLockCount);
	}

private:
	virtual bool X_DoTry(std::uint64_t u64MilliSeconds) const noexcept = 0;
	virtual void X_DoLock() const noexcept = 0;
	virtual void X_DoUnlock() const noexcept = 0;

public:
	bool IsLocking() const noexcept {
		return y_uLockCount > 0;
	}
	std::size_t GetLockCount() const noexcept {
		return y_uLockCount;
	}

	bool Try(std::uint64_t u64MilliSeconds = 0) noexcept {
		const auto uOldCount = y_uLockCount;
		if(uOldCount == 0){
			if(!X_DoTry(u64MilliSeconds)){
				return false;
			}
		}
		y_uLockCount = uOldCount + 1;
		return true;
	}
	void Lock() noexcept {
		const auto uOldCount = y_uLockCount;
		if(uOldCount == 0){
			X_DoLock();
		}
		y_uLockCount = uOldCount + 1;
	}
	void Unlock() noexcept {
		MCF_ASSERT(y_uLockCount != 0);

		const auto uOldCount = y_uLockCount;
		y_uLockCount = uOldCount - 1;
		if(uOldCount == 1){
			X_DoUnlock();
		}
	}

public:
	explicit operator bool() const noexcept {
		return IsLocking();
	}
};

}

#endif
