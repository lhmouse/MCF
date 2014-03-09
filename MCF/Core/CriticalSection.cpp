// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CriticalSection.hpp"
using namespace MCF;

// 嵌套类定义。
class CriticalSection::xDelegate : NO_COPY {
private:
	CRITICAL_SECTION xm_cs;

public:
	xDelegate(unsigned long ulSpinCount) noexcept {
#if defined(NDEBUG) && (_WIN32_WINNT >= 0x0600)
		::InitializeCriticalSectionEx(&xm_cs, ulSpinCount, CRITICAL_SECTION_NO_DEBUG_INFO);
#else
		::InitializeCriticalSectionAndSpinCount(&xm_cs, ulSpinCount);
#endif
	}
	~xDelegate(){
		::DeleteCriticalSection(&xm_cs);
	}

public:
	CRITICAL_SECTION *GetCriticalSecion() noexcept {
		return &xm_cs;
	}
};

// 静态成员函数。
void CriticalSection::xUnlock(CriticalSection::xDelegate *pDelegate) noexcept {
	if(pDelegate){
		::LeaveCriticalSection(pDelegate->GetCriticalSecion());
	}
}

// 构造函数和析构函数。
CriticalSection::CriticalSection(unsigned long ulSpinCount)
	: xm_pDelegate(new xDelegate(ulSpinCount))
{
}
CriticalSection::CriticalSection(CriticalSection &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
CriticalSection &CriticalSection::operator=(CriticalSection &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
CriticalSection::~CriticalSection(){
}

// 其他非静态成员函数。
CriticalSection::LockHolder CriticalSection::Try() noexcept {
	ASSERT(xm_pDelegate);

	if(!::TryEnterCriticalSection(xm_pDelegate->GetCriticalSecion())){
		return LockHolder();
	}
	return LockHolder(xm_pDelegate.get());
}
CriticalSection::LockHolder CriticalSection::Lock() noexcept {
	ASSERT(xm_pDelegate);

	::EnterCriticalSection(xm_pDelegate->GetCriticalSecion());
	return LockHolder(xm_pDelegate.get());
}
