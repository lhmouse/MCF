// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReaderWriterLock.hpp"
#include "ThreadLocal.hpp"
#include "../Core/Exception.hpp"
#include "../Core/UniqueHandle.hpp"
using namespace MCF;

namespace {

class ReaderWriterLockDelegate : CONCRETE(ReaderWriterLock) {
private:
	struct xThreadInfo {
		std::size_t m_uReaderRecur;
		std::size_t m_uWriterRecur;
	};

	struct xSemaphoreCloser {
		constexpr HANDLE operator()() const noexcept {
			return NULL;
		}
		void operator()(HANDLE hSemaphore) const noexcept {
			::CloseHandle(hSemaphore);
		}
	};

private:
	volatile std::size_t xm_uReaderCount;
	ThreadLocal<xThreadInfo, xThreadInfo> xm_pThreadInfo;

	CRITICAL_SECTION xm_csGuard;
	UniqueHandle<xSemaphoreCloser> xm_hExSemaphore;

public:
	ReaderWriterLockDelegate(unsigned long ulSpinCount) noexcept
		: xm_uReaderCount	(0)
		, xm_pThreadInfo	(xThreadInfo())
	{
#if defined(NDEBUG) && (_WIN32_WINNT >= 0x0600)
		::InitializeCriticalSectionEx(&xm_csGuard, ulSpinCount, CRITICAL_SECTION_NO_DEBUG_INFO);
#else
		::InitializeCriticalSectionAndSpinCount(&xm_csGuard, ulSpinCount);
#endif
		xm_hExSemaphore.Reset(::CreateSemaphoreW(nullptr, 1, 1, nullptr));
		if(!xm_hExSemaphore){
			MCF_THROW(::GetLastError(), L"CreateSemaphoreW() 失败。");
		}
	}
	~ReaderWriterLockDelegate() noexcept {
		::DeleteCriticalSection(&xm_csGuard);
	}

private:
	xThreadInfo &xGetThreadInfo() noexcept {
		for(;;){
			ASSERT_NOEXCEPT_BEGIN
			{
				const auto pThreadInfo = xm_pThreadInfo.GetPtr();
				if(pThreadInfo){
					return *pThreadInfo;
				}
			}
			ASSERT_NOEXCEPT_END

			::Sleep(1000);
		}
	}

public:
	void GetReaderLock() noexcept {
		auto &vThreadInfo = xGetThreadInfo();

		if(++vThreadInfo.m_uReaderRecur == 1){
			::EnterCriticalSection(&xm_csGuard);
			if(__atomic_add_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQUIRE) == 1){
				::WaitForSingleObject(xm_hExSemaphore.Get(), INFINITE);
			}
			::LeaveCriticalSection(&xm_csGuard);
		}
	}
	void ReleaseReaderLock() noexcept {
		auto &vThreadInfo = xGetThreadInfo();

		if(--vThreadInfo.m_uReaderRecur == 0){
			if(__atomic_sub_fetch(&xm_uReaderCount, 1, __ATOMIC_RELEASE) == 0){
				::ReleaseSemaphore(xm_hExSemaphore.Get(), 1, nullptr);
			}
		}
	}

	void GetWriterLock() noexcept {
		auto &vThreadInfo = xGetThreadInfo();

		if(++vThreadInfo.m_uWriterRecur == 1){
			ASSERT_MSG(vThreadInfo.m_uReaderRecur == 0, L"获取写锁前必须先释放读锁。");

			::EnterCriticalSection(&xm_csGuard);
			if(++vThreadInfo.m_uReaderRecur == 1){
				::WaitForSingleObject(xm_hExSemaphore.Get(), INFINITE);
				__atomic_add_fetch(&xm_uReaderCount, 1, __ATOMIC_RELAXED);
			}
		}
	}
	void ReleaseWriterLock() noexcept {
		auto &vThreadInfo = xGetThreadInfo();

		if(--vThreadInfo.m_uWriterRecur == 0){
			if(--vThreadInfo.m_uReaderRecur == 0){
				__atomic_sub_fetch(&xm_uReaderCount, 1, __ATOMIC_RELAXED);
				::ReleaseSemaphore(xm_hExSemaphore.Get(), 1, nullptr);
			}
			::LeaveCriticalSection(&xm_csGuard);
		}
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
