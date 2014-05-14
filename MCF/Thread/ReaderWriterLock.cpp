// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReaderWriterLock.hpp"
#include "../Core/Exception.hpp"
using namespace MCF;

namespace {

class ReaderWriterLockDelegate : CONCRETE(ReaderWriterLock) {
private:
	struct xSemaphoreCloser {
		constexpr HANDLE operator()() const noexcept {
			return NULL;
		}
		void operator()(HANDLE hSemaphore) const noexcept {
			::CloseHandle(hSemaphore);
		}
	};

private:
//	CRITICAL_SECTION xm_vCriticalSecion;

public:
	ReaderWriterLockDelegate(unsigned long ulSpinCount) noexcept {
/*#if defined(NDEBUG) && (_WIN32_WINNT >= 0x0600)
		::InitializeCriticalSecionEx(&xm_vCriticalSecion, ulSpinCount, CRITICAL_SECTION_NO_DEBUG_INFO);
#else
		::InitializeCriticalSecionAndSpinCount(&xm_vCriticalSecion, ulSpinCount);
#endif*/
	}
	~ReaderWriterLockDelegate() noexcept {
//		::DeleteCriticalSecion(&xm_vCriticalSecion);
	}

public:
	void GetReaderLock() noexcept {
		__builtin_puts("reader lock");
	}
	void ReleaseReaderLock() noexcept {
		__builtin_puts("reader unlock");
	}

	void GetWriterLock() noexcept {
		__builtin_puts("writer lock");
	}
	void ReleaseWriterLock() noexcept {
		__builtin_puts("writer unlock");
	}
};

}

namespace MCF {

namespace Impl {
	template<>
	void ReaderWriterLock::ReaderLock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		((ReaderWriterLockDelegate *)xm_pOwner)->GetReaderLock();
	}
	template<>
	void ReaderWriterLock::ReaderLock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		((ReaderWriterLockDelegate *)xm_pOwner)->ReleaseReaderLock();
	}

	template<>
	void ReaderWriterLock::WriterLock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		((ReaderWriterLockDelegate *)xm_pOwner)->GetWriterLock();
	}
	template<>
	void ReaderWriterLock::WriterLock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		((ReaderWriterLockDelegate *)xm_pOwner)->ReleaseWriterLock();
	}
}

}

// 静态成员函数。
std::unique_ptr<ReaderWriterLock> ReaderWriterLock::Create(unsigned long ulSpinCount){
	return std::make_unique<ReaderWriterLockDelegate>(ulSpinCount);
}

// 其他非静态成员函数。
ReaderWriterLock::ReaderLock ReaderWriterLock::GetReaderLock() noexcept {
	return ReaderLock(this);
}
ReaderWriterLock::WriterLock ReaderWriterLock::GetWriterLock() noexcept {
	return WriterLock(this);
}
