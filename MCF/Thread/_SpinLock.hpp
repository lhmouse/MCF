// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_SPIN_LOCK_HPP_
#define MCF_THREAD_SPIN_LOCK_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include "../../MCFCRT/ext/expect.h"
#include "Atomic.hpp"
#include <cstdint>

namespace MCF {

class SpinLock : NONCOPYABLE {
public:
	enum : std::uintptr_t {
		LOCKED_COUNT = (std::uintptr_t)-1
	};

private:
	volatile std::uintptr_t xm_uCount;

public:
	explicit SpinLock(std::uintptr_t uCount = 0) noexcept {
		ASSERT(uCount != LOCKED_COUNT);
		AtomicStore(xm_uCount, uCount, MemoryModel::RELEASE);
	}

public:
	std::uintptr_t Lock() volatile noexcept {
		std::uintptr_t uOld;
		for(;;){
			uOld = AtomicExchange(xm_uCount, LOCKED_COUNT, MemoryModel::SEQ_CST);
			if(EXPECT_NOT(uOld != LOCKED_COUNT)){
				break;
			}
			AtomicPause();
		}
		return uOld;
	}
	void Unlock(std::uintptr_t uOld) volatile noexcept {
		AtomicStore(xm_uCount, uOld, MemoryModel::SEQ_CST);
	}
};

}

#endif
