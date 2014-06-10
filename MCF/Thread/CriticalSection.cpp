// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CriticalSection.hpp"
#include "_CriticalSectionImpl.hpp"
using namespace MCF;

namespace {

class CriticalSectionDelegate : CONCRETE(CriticalSection), public Impl::CriticalSectionImpl {
public:
	explicit CriticalSectionDelegate(unsigned long ulSpinCount)
		: CriticalSectionImpl(ulSpinCount)
	{
	}

public:
	using CriticalSectionImpl::GetSpinCount;
	using CriticalSectionImpl::SetSpinCount;

	using CriticalSectionImpl::IsLockedByCurrentThread;
};

}

namespace MCF {

namespace Impl {
	template<>
	void CriticalSection::Lock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		static_cast<CriticalSectionDelegate *>(xm_pOwner)->Enter();
	}
	template<>
	void CriticalSection::Lock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		static_cast<CriticalSectionDelegate *>(xm_pOwner)->Leave();
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

	return static_cast<const CriticalSectionDelegate *>(this)->GetSpinCount();
}
void CriticalSection::SetSpinCount(unsigned long ulSpinCount) noexcept {
	ASSERT(dynamic_cast<CriticalSectionDelegate *>(this));

	static_cast<CriticalSectionDelegate *>(this)->SetSpinCount(ulSpinCount);
}

bool CriticalSection::IsLockedByCurrentThread() const noexcept {
	ASSERT(dynamic_cast<const CriticalSectionDelegate *>(this));

	return static_cast<const CriticalSectionDelegate *>(this)->IsLockedByCurrentThread();
}

CriticalSection::Lock CriticalSection::GetLock() noexcept {
	return Lock(this);
}
