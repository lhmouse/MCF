// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRITICAL_SECTION_HPP__
#define __MCF_CRITICAL_SECTION_HPP__

#include "NoCopy.hpp"
#include <memory>

namespace MCF {

class CriticalSection : NO_COPY {
private:
	struct xUnlocker {
		void operator()(CriticalSection *pcsOwner) const {
			if(pcsOwner != nullptr){
				pcsOwner->xLeave();
			}
		}
	};

	class xDelegate;
public:
	typedef std::unique_ptr<CriticalSection, xUnlocker> LockHolder;
private:
	const std::unique_ptr<xDelegate> xm_pDelegate;
public:
	CriticalSection(unsigned long ulSpinCount = 0x400);
	~CriticalSection();
private:
	void xEnter();
	void xLeave();
public:
	LockHolder Lock() noexcept;
};

}

#endif
