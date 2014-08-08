// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CriticalSection.hpp"
#include "_CriticalSectionImpl.hpp"
using namespace MCF;

namespace {

typedef Impl::CriticalSectionImpl::Result Result;

class CriticalSectionDelegate
	: CONCRETE(CriticalSection)
	, public Impl::CriticalSectionImpl
{
public:
	using CriticalSectionImpl::CriticalSectionImpl;
};

}

namespace MCF {

namespace Impl {
	template<>
	bool CriticalSection::Lock::xDoTry() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		return static_cast<CriticalSectionDelegate *>(xm_pOwner)->ImplTry() != Result::TRY_FAILED;
	}
	template<>
	void CriticalSection::Lock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		static_cast<CriticalSectionDelegate *>(xm_pOwner)->ImplEnter();
	}
	template<>
	void CriticalSection::Lock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		static_cast<CriticalSectionDelegate *>(xm_pOwner)->ImplLeave();
	}
}

}

// 静态成员函数。
std::unique_ptr<CriticalSection> CriticalSection::Create(unsigned long ulSpinCount){
	return std::make_unique<CriticalSectionDelegate>(ulSpinCount);
}

// 其他非静态成员函数。
unsigned long CriticalSection::GetSpinCount() const noexcept {
	ASSERT(dynamic_cast<const CriticalSectionDelegate *>(this));

	return static_cast<const CriticalSectionDelegate *>(this)->ImplGetSpinCount();
}
void CriticalSection::SetSpinCount(unsigned long ulSpinCount) noexcept {
	ASSERT(dynamic_cast<CriticalSectionDelegate *>(this));

	static_cast<CriticalSectionDelegate *>(this)->ImplSetSpinCount(ulSpinCount);
}

bool CriticalSection::IsLockedByCurrentThread() const noexcept {
	ASSERT(dynamic_cast<const CriticalSectionDelegate *>(this));

	return static_cast<const CriticalSectionDelegate *>(this)->ImplIsLockedByCurrentThread();
}
CriticalSection::Lock CriticalSection::TryLock() noexcept {
	Lock vLock(this, 0);
	vLock.Try();
	return std::move(vLock);
}
CriticalSection::Lock CriticalSection::GetLock() noexcept {
	return Lock(this);
}
