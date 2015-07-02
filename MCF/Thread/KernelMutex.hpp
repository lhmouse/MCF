// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MUTEX_HPP_
#define MCF_THREAD_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/String.hpp"
#include "../Core/UniqueWin32Handle.hpp"
#include "UniqueLockTemplate.hpp"

namespace MCF {

class KernelMutex : NONCOPYABLE {
public:
	using UniqueLock = UniqueLockTemplate<KernelMutex>;

private:
	const UniqueWin32Handle x_hMutex;

public:
	explicit KernelMutex(const wchar_t *pwszName = nullptr);
	explicit KernelMutex(const WideString &wsName);

public:
	bool Try(unsigned long long ullMilliSeconds = 0) noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;

	UniqueLock TryLock() noexcept {
		UniqueLock vLock(*this, false);
		vLock.Try();
		return vLock;
	}
	UniqueLock GetLock() noexcept {
		return UniqueLock(*this);
	}
};

}

#endif
