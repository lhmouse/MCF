// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReaderWriterLock.hpp"
#include "CriticalSectionImpl.inl"
#include "WinHandle.inl"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

namespace {

using Result = Impl::CriticalSectionImpl::Result;

struct TlsDeleter {
	constexpr DWORD operator()() const noexcept {
		return TLS_OUT_OF_INDEXES;
	}
	void operator()(DWORD dwIndex) const noexcept {
		::TlsFree(dwIndex);
	}
};

class ReaderWriterLockDelegate : CONCRETE(ReaderWriterLock) {
private:
	Impl::CriticalSectionImpl xm_csGuard;
	Impl::UniqueWinHandle xm_hSemaphore;

	volatile std::size_t xm_uReaderCount;
	UniqueHandle<TlsDeleter> xm_hdwReaderRecur;

public:
	explicit ReaderWriterLockDelegate(unsigned long ulSpinCount)
		: xm_csGuard(ulSpinCount), xm_uReaderCount(0)
	{
		if(!xm_hSemaphore.Reset(::CreateSemaphoreW(nullptr, 1, 1, nullptr))){
			DEBUG_THROW(SystemError, "CreateSemaphoreW");
		}

		if(!xm_hdwReaderRecur.Reset(::TlsAlloc())){
			DEBUG_THROW(SystemError, "TlsAlloc");
		}
	}
	~ReaderWriterLockDelegate(){
		ASSERT(__atomic_load_n(&xm_uReaderCount, __ATOMIC_ACQUIRE) == 0);
	}

public:
	unsigned long GetSpinCount() const noexcept {
		return xm_csGuard.ImplGetSpinCount();
	}
	void SetSpinCount(unsigned long ulSpinCount) noexcept {
		xm_csGuard.ImplSetSpinCount(ulSpinCount);
	}

	bool TryReaderLock() noexcept {
		auto uReaderRecur = (std::size_t)::TlsGetValue(xm_hdwReaderRecur.Get());
		++uReaderRecur;
		if(uReaderRecur == 1){
			if(xm_csGuard.ImplIsLockedByCurrentThread()){
				__atomic_add_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL);
			} else {
				if(xm_csGuard.ImplTry() == Result::TRY_FAILED){
					return false;
				}
				if(__atomic_add_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL) == 1){
					const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), 0);
					if(dwResult == WAIT_FAILED){
						ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
					}
					if(dwResult == WAIT_TIMEOUT){
						__atomic_sub_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL);
						xm_csGuard.ImplLeave();
						return false;
					}
				}
				xm_csGuard.ImplLeave();
			}
		}
		::TlsSetValue(xm_hdwReaderRecur.Get(), (void *)uReaderRecur);
		return true;
	}
	void GetReaderLock() noexcept {
		auto uReaderRecur = (std::size_t)::TlsGetValue(xm_hdwReaderRecur.Get());
		++uReaderRecur;
		if(uReaderRecur == 1){
			if(xm_csGuard.ImplIsLockedByCurrentThread()){
				__atomic_add_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL);
			} else {
				xm_csGuard.ImplEnter();
				if(__atomic_add_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL) == 1){
					const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), INFINITE);
					if(dwResult == WAIT_FAILED){
						ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
					}
				}
				xm_csGuard.ImplLeave();
			}
		}
		::TlsSetValue(xm_hdwReaderRecur.Get(), (void *)uReaderRecur);
	}
	void ReleaseReaderLock() noexcept {
		auto uReaderRecur = (std::size_t)::TlsGetValue(xm_hdwReaderRecur.Get());
		--uReaderRecur;
		if(uReaderRecur == 0){
			if(xm_csGuard.ImplIsLockedByCurrentThread()){
				__atomic_sub_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL);
			} else {
				if(__atomic_sub_fetch(&xm_uReaderCount, 1, __ATOMIC_ACQ_REL) == 0){
					if(!::ReleaseSemaphore(xm_hSemaphore.Get(), 1, nullptr)){
						ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
					}
				}
			}
		}
		::TlsSetValue(xm_hdwReaderRecur.Get(), (void *)uReaderRecur);
	}

	bool TryWriterLock() noexcept {
		switch(xm_csGuard.ImplTry()){
		case Result::TRY_FAILED:
			return false;

		case Result::STATE_CHANGED:
			// 假定有两个线程运行同样的函数：
			//
			//   GetReaderLock();
			//   ::Sleep(1000);
			//   GetWriterLock(); // 死锁，因为没有任何一个线程可以获得写锁。
			//
			// 这个问题并非无法解决，例如允许 GetWriterLock() 抛出异常。
			// 但是这样除了使问题复杂化以外没有什么好处。
			ASSERT_MSG((std::size_t)::TlsGetValue(xm_hdwReaderRecur.Get()) == 0,
				L"获取写锁前必须先释放读锁。");

			{
				const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), 0);
				if(dwResult == WAIT_FAILED){
					ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
				}
				if(dwResult == WAIT_TIMEOUT){
					xm_csGuard.ImplLeave();
					return false;
				}
			}
			break;

		case Result::RECURSIVE:
			break;
		}
		return true;
	}
	void GetWriterLock() noexcept {
		if(xm_csGuard.ImplEnter() == Result::STATE_CHANGED){
			ASSERT_MSG((std::size_t)::TlsGetValue(xm_hdwReaderRecur.Get()) == 0,
				L"获取写锁前必须先释放读锁。");

			const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), INFINITE);
			if(dwResult == WAIT_FAILED){
				ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
			}
		}
	}
	void ReleaseWriterLock() noexcept {
		if(xm_csGuard.ImplLeave() == Result::STATE_CHANGED){
			if((std::size_t)::TlsGetValue(xm_hdwReaderRecur.Get()) == 0){
				if(!::ReleaseSemaphore(xm_hSemaphore.Get(), 1, nullptr)){
					ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
				}
			}
		}
	}
};

}

namespace MCF {

namespace Impl {
	template<>
	bool ReaderWriterLock::ReaderLock::xDoTry() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		return static_cast<ReaderWriterLockDelegate *>(xm_pOwner)->TryReaderLock();
	}
	template<>
	void ReaderWriterLock::ReaderLock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		static_cast<ReaderWriterLockDelegate *>(xm_pOwner)->GetReaderLock();
	}
	template<>
	void ReaderWriterLock::ReaderLock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		static_cast<ReaderWriterLockDelegate *>(xm_pOwner)->ReleaseReaderLock();
	}

	template<>
	bool ReaderWriterLock::WriterLock::xDoTry() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		return static_cast<ReaderWriterLockDelegate *>(xm_pOwner)->TryWriterLock();
	}
	template<>
	void ReaderWriterLock::WriterLock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		static_cast<ReaderWriterLockDelegate *>(xm_pOwner)->GetWriterLock();
	}
	template<>
	void ReaderWriterLock::WriterLock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(xm_pOwner));

		static_cast<ReaderWriterLockDelegate *>(xm_pOwner)->ReleaseWriterLock();
	}
}

}

// 静态成员函数。
std::unique_ptr<ReaderWriterLock> ReaderWriterLock::Create(unsigned long ulSpinCount){
	return std::make_unique<ReaderWriterLockDelegate>(ulSpinCount);
}

// 其他非静态成员函数。
unsigned long ReaderWriterLock::GetSpinCount() const noexcept {
	ASSERT(dynamic_cast<const ReaderWriterLockDelegate *>(this));

	return static_cast<const ReaderWriterLockDelegate *>(this)->GetSpinCount();
}
void ReaderWriterLock::SetSpinCount(unsigned long ulSpinCount) noexcept {
	ASSERT(dynamic_cast<ReaderWriterLockDelegate *>(this));

	static_cast<ReaderWriterLockDelegate *>(this)->SetSpinCount(ulSpinCount);
}

ReaderWriterLock::ReaderLock ReaderWriterLock::TryReaderLock() noexcept {
	ReaderLock vLock(this, 0);
	vLock.Try();
	return std::move(vLock);
}
ReaderWriterLock::ReaderLock ReaderWriterLock::GetReaderLock() noexcept {
	return ReaderLock(this);
}
ReaderWriterLock::WriterLock ReaderWriterLock::TryWriterLock() noexcept {
	WriterLock vLock(this, 0);
	vLock.Try();
	return std::move(vLock);
}
ReaderWriterLock::WriterLock ReaderWriterLock::GetWriterLock() noexcept {
	return WriterLock(this);
}
