// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_SPIN_LOCK_HPP_
#define MCF_THREAD_SPIN_LOCK_HPP_

#include "../Utilities/Noncopyable.hpp"
#include <cstddef>

namespace MCF {

class SpinLock : NONCOPYABLE {
private:
	volatile std::size_t xm_uCount;

public:
	explicit SpinLock(std::size_t uCount = 0) noexcept {
		__atomic_store_n(&xm_uCount, uCount, __ATOMIC_RELEASE);
	}

public:
	std::size_t Lock() volatile throw() { // FIXME: g++ 4.9.2 ICE.
		std::size_t uOld;
		for(;;){
			uOld = __atomic_exchange_n(&xm_uCount, (std::size_t)-1, __ATOMIC_SEQ_CST);
			if(EXPECT_NOT(uOld != (std::size_t)-1)){
				break;
			}
			__builtin_ia32_pause();
		}
		return uOld;
	}
	void Unlock(std::size_t uOld) volatile noexcept {
		__atomic_store_n(&xm_uCount, uOld, __ATOMIC_SEQ_CST);
	}
};

}

#endif
