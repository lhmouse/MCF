// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReaderWriterMutex.hpp"
#include "Atomic.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

std::size_t ReaderWriterMutex::xTlsIndexDeleter::operator()() const noexcept {
	return TLS_OUT_OF_INDEXES;
}
void ReaderWriterMutex::xTlsIndexDeleter::operator()(std::size_t uTlsIndex) const noexcept {
	::TlsFree(uTlsIndex);
}

template<>
bool ReaderWriterMutex::UniqueReaderLock::xDoTry() const noexcept {
	return x_pOwner->TryAsReader() != ReaderWriterMutex::Result::kResTryFailed;
}
template<>
void ReaderWriterMutex::UniqueReaderLock::xDoLock() const noexcept {
	x_pOwner->LockAsReader();
}
template<>
void ReaderWriterMutex::UniqueReaderLock::xDoUnlock() const noexcept {
	x_pOwner->UnlockAsReader();
}

template<>
bool ReaderWriterMutex::UniqueWriterLock::xDoTry() const noexcept {
	return x_pOwner->TryAsWriter() != ReaderWriterMutex::Result::kResTryFailed;
}
template<>
void ReaderWriterMutex::UniqueWriterLock::xDoLock() const noexcept {
	x_pOwner->LockAsWriter();
}
template<>
void ReaderWriterMutex::UniqueWriterLock::xDoUnlock() const noexcept {
	x_pOwner->UnlockAsWriter();
}

// 构造函数和析构函数。
ReaderWriterMutex::ReaderWriterMutex(std::size_t uSpinCount)
	: x_mtxWriterGuard(uSpinCount), x_semExclusive(1, nullptr), x_uReaderCount(0)
{
	if(!x_uTlsIndex.Reset(::TlsAlloc())){
		DEBUG_THROW(SystemError, "TlsAlloc");
	}
	AtomicFence(MemoryModel::kRelease);
}

// 其他非静态成员函数。
ReaderWriterMutex::Result ReaderWriterMutex::TryAsReader() noexcept {
	Result eResult = kResRecursive;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue(x_uTlsIndex.Get());
	if(uReaderRecur == 0){
		if(x_mtxWriterGuard.IsLockedByCurrentThread()){
			AtomicIncrement(x_uReaderCount, MemoryModel::kAcqRel);
		} else {
			if(x_mtxWriterGuard.Try() == kResTryFailed){
				eResult = kResTryFailed;
				goto jDone;
			}
			if(AtomicIncrement(x_uReaderCount, MemoryModel::kAcqRel) == 1){
				if(!x_semExclusive.Wait(0)){
					AtomicDecrement(x_uReaderCount, MemoryModel::kAcqRel);
					x_mtxWriterGuard.Unlock();
					eResult = kResTryFailed;
					goto jDone;
				}
				eResult = kResStateChanged;
			}
			x_mtxWriterGuard.Unlock();
		}
	}
	::TlsSetValue(x_uTlsIndex.Get(), (void *)(uReaderRecur + 1));
jDone:
	return eResult;
}
ReaderWriterMutex::Result ReaderWriterMutex::LockAsReader() noexcept {
	Result eResult = kResRecursive;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue(x_uTlsIndex.Get());
	if(uReaderRecur == 0){
		if(x_mtxWriterGuard.IsLockedByCurrentThread()){
			AtomicIncrement(x_uReaderCount, MemoryModel::kAcqRel);
		} else {
			x_mtxWriterGuard.Lock();
			if(AtomicIncrement(x_uReaderCount, MemoryModel::kAcqRel) == 1){
				x_semExclusive.Wait();
				eResult = kResStateChanged;
			}
			x_mtxWriterGuard.Unlock();
		}
	}
	::TlsSetValue(x_uTlsIndex.Get(), (void *)(uReaderRecur + 1));
	return eResult;
}
ReaderWriterMutex::Result ReaderWriterMutex::UnlockAsReader() noexcept {
	Result eResult = kResRecursive;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue(x_uTlsIndex.Get());
	if(uReaderRecur == 1){
		if(x_mtxWriterGuard.IsLockedByCurrentThread()){
			AtomicDecrement(x_uReaderCount, MemoryModel::kAcqRel);
		} else {
			if(AtomicDecrement(x_uReaderCount, MemoryModel::kAcqRel) == 0){
				x_semExclusive.Post();
				eResult = kResStateChanged;
			}
		}
	}
	::TlsSetValue(x_uTlsIndex.Get(), (void *)(uReaderRecur - 1));
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
	ASSERT_MSG((std::uintptr_t)::TlsGetValue(x_uTlsIndex.Get()) == 0, L"获取写锁前必须先释放读锁。");

	const auto eResult = x_mtxWriterGuard.Try();
	if(eResult != kResStateChanged){
		return eResult;
	}
	if(!x_semExclusive.Wait(0)){
		x_mtxWriterGuard.Unlock();
		return kResTryFailed;
	}
	return kResStateChanged;
}
ReaderWriterMutex::Result ReaderWriterMutex::LockAsWriter() noexcept {
	ASSERT_MSG((std::uintptr_t)::TlsGetValue(x_uTlsIndex.Get()) == 0, L"获取写锁前必须先释放读锁。");

	const auto eResult = x_mtxWriterGuard.Lock();
	if(eResult != kResStateChanged){
		return eResult;
	}
	x_semExclusive.Wait();
	return kResStateChanged;
}
ReaderWriterMutex::Result ReaderWriterMutex::UnlockAsWriter() noexcept {
	const auto eResult = x_mtxWriterGuard.Unlock();
	if(eResult != kResStateChanged){
		return eResult;
	}
	const auto uRecurReading = (std::uintptr_t)::TlsGetValue(x_uTlsIndex.Get());
	if(uRecurReading == 0){
		x_semExclusive.Post();
	}
	return kResStateChanged;
}

}
