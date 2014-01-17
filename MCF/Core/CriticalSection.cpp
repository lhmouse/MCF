// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CriticalSection.hpp"
using namespace MCF;

// 嵌套类定义。
class CriticalSection::xDelegate : NO_COPY {
private:
	CRITICAL_SECTION xm_cs;
#ifndef NDEBUG
	std::size_t xm_uLockCount;
#endif
public:
	xDelegate(unsigned long ulSpinCount) noexcept {
		::InitializeCriticalSectionAndSpinCount(&xm_cs, ulSpinCount);
#ifndef NDEBUG
		xm_uLockCount = 0;
#endif
	}
	~xDelegate(){
#ifndef NDEBUG
		BAIL_IF_MSG(xm_uLockCount != 0, L"在一个临界区对象销毁时，还有至少一个线程的锁没有被释放。");
#endif
		::DeleteCriticalSection(&xm_cs);
	}
public:
	void Enter() noexcept {
		::EnterCriticalSection(&xm_cs);
#ifndef NDEBUG
		++xm_uLockCount;
#endif
	}
	void Leave() noexcept {
#ifndef NDEBUG
		BAIL_IF_MSG(xm_uLockCount == 0, L"试图在没有进入临界区以前退出临界区。");
		--xm_uLockCount;
#endif
		::LeaveCriticalSection(&xm_cs);
	}
};

// 构造函数和析构函数。
CriticalSection::CriticalSection(unsigned long ulSpinCount)
	: xm_pDelegate(new xDelegate(ulSpinCount))
{
}
CriticalSection::~CriticalSection(){
}

// 其他非静态成员函数。
void CriticalSection::xEnter(){
	xm_pDelegate->Enter();
}
void CriticalSection::xLeave(){
	xm_pDelegate->Leave();
}

CriticalSection::LockHolder CriticalSection::Lock() noexcept {
	xEnter();
	return LockHolder(this);
}
