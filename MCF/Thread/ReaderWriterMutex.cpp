// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReaderWriterMutex.hpp"
#include "Atomic.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
using namespace MCF;

std::size_t ReaderWriterMutex::xTlsIndexDeleter::operator()() const noexcept {
	return TLS_OUT_OF_INDEXES;
}
void ReaderWriterMutex::xTlsIndexDeleter::operator()(std::size_t uTlsIndex) const noexcept {
	::TlsFree(uTlsIndex);
}

namespace MCF {

template<>
bool ReaderWriterMutex::UniqueReaderLock::xDoTry() const noexcept {
	return xm_pOwner->TryAsReader() != ReaderWriterMutex::Result::R_TRY_FAILED;
}
template<>
void ReaderWriterMutex::UniqueReaderLock::xDoLock() const noexcept {
	xm_pOwner->LockAsReader();
}
template<>
void ReaderWriterMutex::UniqueReaderLock::xDoUnlock() const noexcept {
	xm_pOwner->UnlockAsReader();
}

template<>
bool ReaderWriterMutex::UniqueWriterLock::xDoTry() const noexcept {
	return xm_pOwner->TryAsWriter() != ReaderWriterMutex::Result::R_TRY_FAILED;
}
template<>
void ReaderWriterMutex::UniqueWriterLock::xDoLock() const noexcept {
	xm_pOwner->LockAsWriter();
}
template<>
void ReaderWriterMutex::UniqueWriterLock::xDoUnlock() const noexcept {
	xm_pOwner->UnlockAsWriter();
}

}

// 构造函数和析构函数。
ReaderWriterMutex::ReaderWriterMutex(std::size_t uSpinCount)
	: xm_mtxWriterGuard(uSpinCount), xm_semExclusive(1, nullptr), xm_uReaderCount(0)
{
	if(!xm_uTlsIndex.Reset(::TlsAlloc())){
		DEBUG_THROW(SystemError, "TlsAlloc");
	}
	AtomicFence(MemoryModel::RELEASE);
}

// 其他非静态成员函数。
ReaderWriterMutex::Result ReaderWriterMutex::TryAsReader() noexcept {
	Result eResult = R_RECURSIVE;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue(xm_uTlsIndex.Get());
	if(uReaderRecur == 0){
		if(xm_mtxWriterGuard.IsLockedByCurrentThread()){
			AtomicIncrement(xm_uReaderCount, MemoryModel::ACQ_REL);
		} else {
			if(xm_mtxWriterGuard.Try() == R_TRY_FAILED){
				eResult = R_TRY_FAILED;
				goto jDone;
			}
			if(AtomicIncrement(xm_uReaderCount, MemoryModel::ACQ_REL) == 1){
				if(!xm_semExclusive.Wait(0)){
					AtomicDecrement(xm_uReaderCount, MemoryModel::ACQ_REL);
					xm_mtxWriterGuard.Unlock();
					eResult = R_TRY_FAILED;
					goto jDone;
				}
				eResult = R_STATE_CHANGED;
			}
			xm_mtxWriterGuard.Unlock();
		}
	}
	::TlsSetValue(xm_uTlsIndex.Get(), (void *)(uReaderRecur + 1));
jDone:
	return eResult;
}
ReaderWriterMutex::Result ReaderWriterMutex::LockAsReader() noexcept {
	Result eResult = R_RECURSIVE;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue(xm_uTlsIndex.Get());
	if(uReaderRecur == 0){
		if(xm_mtxWriterGuard.IsLockedByCurrentThread()){
			AtomicIncrement(xm_uReaderCount, MemoryModel::ACQ_REL);
		} else {
			xm_mtxWriterGuard.Lock();
			if(AtomicIncrement(xm_uReaderCount, MemoryModel::ACQ_REL) == 1){
				xm_semExclusive.Wait();
				eResult = R_STATE_CHANGED;
			}
			xm_mtxWriterGuard.Unlock();
		}
	}
	::TlsSetValue(xm_uTlsIndex.Get(), (void *)(uReaderRecur + 1));
	return eResult;
}
ReaderWriterMutex::Result ReaderWriterMutex::UnlockAsReader() noexcept {
	Result eResult = R_RECURSIVE;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue(xm_uTlsIndex.Get());
	if(uReaderRecur == 1){
		if(xm_mtxWriterGuard.IsLockedByCurrentThread()){
			AtomicDecrement(xm_uReaderCount, MemoryModel::ACQ_REL);
		} else {
			if(AtomicDecrement(xm_uReaderCount, MemoryModel::ACQ_REL) == 0){
				xm_semExclusive.Post();
				eResult = R_STATE_CHANGED;
			}
		}
	}
	::TlsSetValue(xm_uTlsIndex.Get(), (void *)(uReaderRecur - 1));
	return eResult;
}

ReaderWriterMutex::Result ReaderWriterMutex::TryAsWriter() noexcept {
	// 假定有两个线程运行同样的函数：
	//
	//   GetReaderLock();
	//   ::Sleep(1000);
	//   GetWriterLock(); // 死锁，因为没有任何一个线程可以获得写锁。
	//
	// 这个问题并非无法解决，例如允许 GetWriterLock() 抛出异常。
	// 但是这样除了使问题复杂化以外没有什么好处。
	ASSERT_MSG((std::uintptr_t)::TlsGetValue(xm_uTlsIndex.Get()) == 0, L"获取写锁前必须先释放读锁。");

	const auto eResult = xm_mtxWriterGuard.Try();
	if(eResult != R_STATE_CHANGED){
		return eResult;
	}
	if(xm_semExclusive.Wait(0)){
		return R_STATE_CHANGED;
	}
	xm_mtxWriterGuard.Unlock();
	return R_TRY_FAILED;
}
ReaderWriterMutex::Result ReaderWriterMutex::LockAsWriter() noexcept {
	ASSERT_MSG((std::uintptr_t)::TlsGetValue(xm_uTlsIndex.Get()) == 0, L"获取写锁前必须先释放读锁。");

	const auto eResult = xm_mtxWriterGuard.Lock();
	if(eResult != R_STATE_CHANGED){
		return eResult;
	}
	xm_semExclusive.Wait();
	return R_STATE_CHANGED;
}
ReaderWriterMutex::Result ReaderWriterMutex::UnlockAsWriter() noexcept {
	const auto eResult = xm_mtxWriterGuard.Unlock();
	if(eResult != R_STATE_CHANGED){
		return eResult;
	}
	const auto uRecurReading = (std::uintptr_t)::TlsGetValue(xm_uTlsIndex.Get());
	if(uRecurReading == 0){
		xm_semExclusive.Post();
	}
	return R_STATE_CHANGED;
}

ReaderWriterMutex::UniqueReaderLock ReaderWriterMutex::TryReaderLock() noexcept {
	UniqueReaderLock vLock(*this, false);
	vLock.Try();
	return vLock;
}
ReaderWriterMutex::UniqueReaderLock ReaderWriterMutex::GetReaderLock() noexcept {
	return UniqueReaderLock(*this);
}

ReaderWriterMutex::UniqueWriterLock ReaderWriterMutex::TryWriterLock() noexcept {
	UniqueWriterLock vLock(*this, false);
	vLock.Try();
	return vLock;
}
ReaderWriterMutex::UniqueWriterLock ReaderWriterMutex::GetWriterLock() noexcept {
	return UniqueWriterLock(*this);
}
