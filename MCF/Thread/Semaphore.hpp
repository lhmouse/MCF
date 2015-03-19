// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_SEMAPHORE_HPP_
#define MCF_THREAD_SEMAPHORE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/String.hpp"
#include "_UniqueWin32Handle.hpp"
#include <cstddef>

namespace MCF {

class Semaphore : NONCOPYABLE {
private:
	const UniqueWin32Handle x_hSemaphore;

public:
	explicit Semaphore(std::size_t uInitCount, const wchar_t *pwszName = nullptr);
	Semaphore(std::size_t uInitCount, const WideString &wsName);

public:
	std::size_t Wait(unsigned long long ullMilliSeconds) noexcept;
	void Wait() noexcept;
	std::size_t Post(std::size_t uPostCount = 1) noexcept;

	std::size_t BatchWait(unsigned long long ullMilliSeconds, std::size_t uWaitCount) noexcept;
	void BatchWait(std::size_t uWaitCount) noexcept;
};

}

#endif
