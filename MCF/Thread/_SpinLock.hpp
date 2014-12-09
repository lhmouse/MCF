// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_SPIN_LOCK_HPP_
#define MCF_THREAD_SPIN_LOCK_HPP_

#include "../Utilities/Noncopyable.hpp"

namespace MCF {

class SpinLock : NONCOPYABLE {
private:
	volatile unsigned long xm_ulCount;

public:
	explicit SpinLock(unsigned long ulCount = 0) noexcept {
		__atomic_store_n(&xm_ulCount, ulCount, __ATOMIC_RELEASE);
	}

public:
	unsigned long Lock() volatile throw() { // FIXME: g++ 4.9.2 ICE.
		unsigned long ulOld;
		for(;;){
			ulOld = __atomic_exchange_n(&xm_ulCount, (unsigned long)-1, __ATOMIC_SEQ_CST);
			if(EXPECT_NOT(ulOld != (unsigned long)-1)){
				break;
			}
			__builtin_ia32_pause();
		}
		return ulOld;
	}
	void Unlock(unsigned long ulOld) volatile noexcept {
		__atomic_store_n(&xm_ulCount, ulOld, __ATOMIC_SEQ_CST);
	}
};

}

#endif
