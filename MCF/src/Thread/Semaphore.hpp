// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_SEMAPHORE_HPP_
#define MCF_THREAD_SEMAPHORE_HPP_

#include "Mutex.hpp"
#include "ConditionVariable.hpp"
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

class Semaphore {
private:
	mutable Mutex x_mtxGuard;
	mutable ConditionVariable x_cvWaiter;
	std::size_t x_uCount;

public:
	explicit constexpr Semaphore(std::size_t uInitCount) noexcept
		: x_mtxGuard(), x_cvWaiter(), x_uCount(uInitCount)
	{
	}

	Semaphore(const Semaphore &) = delete;
	Semaphore &operator=(const Semaphore &) = delete;

public:
	bool Wait(std::uint64_t u64UntilFastMonoClock) noexcept;
	void Wait() noexcept;
	std::size_t Post(std::size_t uPostCount = 1) noexcept;
};

static_assert(std::is_trivially_destructible<Semaphore>::value, "Hey!");

}

#endif
