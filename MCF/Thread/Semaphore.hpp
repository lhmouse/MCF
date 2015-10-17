// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_SEMAPHORE_HPP_
#define MCF_THREAD_SEMAPHORE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "Mutex.hpp"
#include "ConditionVariable.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class Semaphore : NONCOPYABLE {
private:
	mutable Mutex x_mtxGuard;
	mutable ConditionVariable x_cvWaiter;
	std::size_t x_uCount;

public:
	explicit Semaphore(std::size_t uInitCount) noexcept;

public:
	bool Wait(std::uint64_t u64UntilFastMonoClock) noexcept;
	void Wait() noexcept;
	std::size_t Post(std::size_t uPostCount = 1) noexcept;
};

}

#endif
