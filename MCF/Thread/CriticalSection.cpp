// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CriticalSection.hpp"
using namespace MCF;

namespace {

class CriticalSectionDelegate : CONCRETE(CriticalSection) {
private:
	CRITICAL_SECTION xm_vCriticalSecion;

public:
	CriticalSectionDelegate(unsigned long ulSpinCount) noexcept {
#if defined(NDEBUG) && (_WIN32_WINNT >= 0x0600)
		::InitializeCriticalSectionEx(&xm_vCriticalSecion, ulSpinCount, CRITICAL_SECTION_NO_DEBUG_INFO);
#else
		::InitializeCriticalSectionAndSpinCount(&xm_vCriticalSecion, ulSpinCount);
#endif
	}
	~CriticalSectionDelegate() noexcept {
		::DeleteCriticalSection(&xm_vCriticalSecion);
	}

public:
	void Enter() noexcept {
		::EnterCriticalSection(&xm_vCriticalSecion);
	}
	void Leave() noexcept {
		::LeaveCriticalSection(&xm_vCriticalSecion);
	}
};

}

namespace MCF {

namespace Impl {
	template<>
	void CriticalSection::Lock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		((CriticalSectionDelegate *)xm_pOwner)->Enter();
	}
	template<>
	void CriticalSection::Lock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		((CriticalSectionDelegate *)xm_pOwner)->Leave();
	}
}

}

// 静态成员函数。
std::unique_ptr<CriticalSection> CriticalSection::Create(unsigned long ulSpinCount){
	return std::make_unique<CriticalSectionDelegate>(ulSpinCount);
}

// 其他非静态成员函数。
CriticalSection::Lock CriticalSection::GetLock() noexcept {
	return Lock(this);
}
