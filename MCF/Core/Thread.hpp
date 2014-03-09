// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_THREAD_HPP__
#define __MCF_THREAD_HPP__

#include "NoCopy.hpp"
#include <functional>
#include <memory>
#include <cstddef>

namespace MCF {

class Thread : NO_COPY {
private:
	class xDelegate;

private:
	std::shared_ptr<xDelegate> xm_pDelegate;

public:
	Thread() noexcept;
	Thread(Thread &&rhs) noexcept;
	Thread &operator=(Thread &&rhs) noexcept;
	~Thread();

public:
	void Start(std::function<void ()> fnProc, bool bSuspended = false);
	void WaitTimeout(unsigned long ulMilliSeconds) const noexcept;
	void Join() const; // 如果线程中有被捕获的异常，抛出异常。
	void Detach() noexcept;
	void JoinDetach() noexcept;

	void Suspend() noexcept;
	void Resume() noexcept;

	bool IsAlive() const noexcept;
	unsigned long GetThreadId() const noexcept;

public:
	explicit operator bool() const noexcept {
		return IsAlive();
	}
};

}

#endif
