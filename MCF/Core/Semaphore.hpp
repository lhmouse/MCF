// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SEMAPHORE_HPP__
#define __MCF_SEMAPHORE_HPP__

#include "NoCopy.hpp"
#include "../../MCFCRT/cpp/ext/vvector.hpp"
#include <memory>
#include <utility>

namespace MCF {

class Semaphore : NO_COPY {
private:
	class xDelegate;

	struct xUnlocker {
		void operator()(xDelegate *pDelegate) const noexcept {
			xUnlock(pDelegate);
		}
	};
private:
	static void xUnlock(xDelegate *pDelegate) noexcept;
public:
	typedef std::unique_ptr<xDelegate, xUnlocker> LockHolder;
private:
	const std::unique_ptr<xDelegate> xm_pDelegate;
public:
	Semaphore(long lInitCount, long lMaxCount);
	~Semaphore();
public:
	LockHolder WaitTimeOut(unsigned long ulMilliSeconds) noexcept;
	MCF::VVector<LockHolder> WaitTimeOut(long lWaitCount, unsigned long ulMilliSeconds) noexcept;
	LockHolder Wait() noexcept;
	MCF::VVector<LockHolder> Wait(long lWaitCount) noexcept;
};

}

#define SEMAPHORE_SCOPE(sem, cnt)	\
	for(auto __MCF_LOCK__ = ::std::make_pair((sem).Wait((cnt)), true);	\
		__MCF_LOCK__.second;	\
		__MCF_LOCK__.second = false	\
	)

#endif
