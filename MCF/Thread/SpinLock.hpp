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
		kLockedCount = (std::uintptr_t)-1
	};

private:
	Atomic<std::uintptr_t> x_uCount;

public:
	explicit SpinLock(std::uintptr_t uCount = 0) noexcept {
		ASSERT(uCount != kLockedCount);

		x_uCount.Store(uCount, MemoryModel::kRelease);
	}

public:
	std::uintptr_t Lock() volatile noexcept {
		std::uintptr_t uOld;
		for(;;){
			uOld = x_uCount.Exchange(kLockedCount, MemoryModel::kSeqCst);
			if(EXPECT_NOT(uOld != kLockedCount)){
				break;
			}
			AtomicPause();
		}
		return uOld;
	}
	void Unlock(std::uintptr_t uOld) volatile noexcept {
		x_uCount.Store(uOld, MemoryModel::kSeqCst);
	}
};

}

#endif
