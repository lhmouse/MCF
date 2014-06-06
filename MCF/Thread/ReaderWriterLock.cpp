// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReaderWriterLock.hpp"
#include "ThreadLocalPtr.hpp"
#include "CriticalSection.hpp"
#include "Semaphore.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

namespace {

class ReaderWriterLockDelegate : CONCRETE(ReaderWriterLock) {
private:
	struct xThreadInfo {
		std::size_t m_uReaderRecur;
		std::size_t m_uWriterRecur;
	};

private:
	volatile std::size_t xm_uReaderCount;
	ThreadLocalPtr<xThreadInfo, xThreadInfo> xm_pThreadInfo;

	const std::unique_ptr<CriticalSection> xm_pcsGuard;
	CriticalSection::Lock xm_vWriterLock;
	const std::unique_ptr<Semaphore> xm_psemExclusive;

public:
	ReaderWriterLockDelegate(unsigned long ulSpinCount) noexcept
		: xm_uReaderCount	(0)
		, xm_pThreadInfo	(xThreadInfo())
		, xm_pcsGuard		(CriticalSection::Create(ulSpinCount))
		, xm_vWriterLock	(xm_pcsGuard.get(), false)
		, xm_psemExclusive	(Semaphore::Create(1, 1))
	{
	}

private:
	xThreadInfo &xGetThreadInfo() noexcept {
		const auto pThreadInfo = xm_pThreadInfo.GetPtr();
		if(!pThreadInfo){
			Bail(L"为读写锁创建 Tls 失败。\n\n注意不要在静态对象的构造函数或析构函数中使用读写锁。");
		}
		return *pThreadInfo;
	}

public:
	void GetReaderLock() noexcept {
		auto &vThreadInfo = xGetThreadInfo();

		if(++vThreadInfo.m_uReaderRecur == 1){
			if(vThreadInfo.m_uWriterRecur == 0){
				const auto vReaderLock = xm_pcsGuard->GetLock();
				if(__atomic_add_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL) == 1){
					xm_psemExclusive->Wait();
				}
			} else {
				__atomic_add_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL);
			}
		}
	}
	void ReleaseReaderLock() noexcept {
		auto &vThreadInfo = xGetThreadInfo();

		if(--vThreadInfo.m_uReaderRecur == 0){
			if(vThreadInfo.m_uWriterRecur == 0){
				if(__atomic_sub_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL) == 0){
					xm_psemExclusive->Signal();
				}
			} else {
				__atomic_sub_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL);
			}
		}
	}

	void GetWriterLock() noexcept {
		auto &vThreadInfo = xGetThreadInfo();

		if(++vThreadInfo.m_uWriterRecur == 1){
			// 假定有两个线程运行同样的函数：
			//
			//   GetReaderLock();
			//   ::Sleep(1000);
			//   GetWriterLock(); // 死锁，因为没有任何一个线程可以获得写锁。
			//
			// 这个问题并非无法解决，例如允许 GetWriterLock() 抛出异常。
			// 但是这样除了使问题复杂化以外没有什么好处。
			ASSERT_MSG(vThreadInfo.m_uReaderRecur == 0, L"获取写锁前必须先释放读锁。");

			auto vWriterLock = xm_pcsGuard->GetLock();
//			if(vThreadInfo.m_uReaderRecur == 0){
				xm_psemExclusive->Wait();
//			}
			xm_vWriterLock.Join(std::move(vWriterLock));
		}
	}
	void ReleaseWriterLock() noexcept {
		auto &vThreadInfo = xGetThreadInfo();

		if(--vThreadInfo.m_uWriterRecur == 0){
			if(vThreadInfo.m_uReaderRecur == 0){
				xm_psemExclusive->Signal();
			}
			xm_vWriterLock.Unlock();
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
