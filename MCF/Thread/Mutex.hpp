// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MUTEX_HPP_
#define MCF_THREAD_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/String.hpp"
#include "Win32Handle.hpp"
#include "LockRaiiTemplate.hpp"

namespace MCF {

class Mutex : Noncopyable {
public:
	using Lock = LockRaiiTemplate<Mutex>;

private:
	const UniqueWin32Handle xm_hMutex;

public:
	explicit Mutex(const wchar_t *pwszName = nullptr);
	explicit Mutex(const WideString &wsName);

public:
	bool Try(unsigned long long ullMilliSeconds = 0) noexcept;
	void Acquire() noexcept;
	void Release() noexcept;

	Lock TryLock() noexcept;
	Lock GetLock() noexcept;
};

}

#endif
