// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReaderWriterMutex.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

std::size_t ReaderWriterMutex::$TlsIndexDeleter::operator()() const noexcept {
	return TLS_OUT_OF_INDEXES;
}
void ReaderWriterMutex::$TlsIndexDeleter::operator()(std::size_t uTlsIndex) const noexcept {
	::TlsFree(uTlsIndex);
}

template<>
bool ReaderWriterMutex::UniqueReaderLock::$DoTry() const noexcept {
	return $pOwner->TryAsReader() != ReaderWriterMutex::Result::kResTryFailed;
}
template<>
void ReaderWriterMutex::UniqueReaderLock::$DoLock() const noexcept {
	$pOwner->LockAsReader();
}
template<>
void ReaderWriterMutex::UniqueReaderLock::$DoUnlock() const noexcept {
	$pOwner->UnlockAsReader();
}

template<>
bool ReaderWriterMutex::UniqueWriterLock::$DoTry() const noexcept {
	return $pOwner->TryAsWriter() != ReaderWriterMutex::Result::kResTryFailed;
}
template<>
void ReaderWriterMutex::UniqueWriterLock::$DoLock() const noexcept {
	$pOwner->LockAsWriter();
}
template<>
void ReaderWriterMutex::UniqueWriterLock::$DoUnlock() const noexcept {
	$pOwner->UnlockAsWriter();
}

// 构造函数和析构函数。
ReaderWriterMutex::ReaderWriterMutex(std::size_t uSpinCount)
	: $mtxWriterGuard(uSpinCount), $mtxExclusive(uSpinCount), $uReaderCount(0)
{
	if(!$uTlsIndex.Reset(::TlsAlloc())){
		DEBUG_THROW(SystemError, "TlsAlloc");
	}
}

// 其他非静态成员函数。
ReaderWriterMutex::Result ReaderWriterMutex::TryAsReader() noexcept {
	Result eResult = kResRecursive;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue($uTlsIndex.Get());
	if(uReaderRecur == 0){
		if($mtxWriterGuard.IsLockedByCurrentThread()){
			$uReaderCount.Increment(kAtomicRelaxed);
		} else {
			if($mtxWriterGuard.Try() == kResTryFailed){
				eResult = kResTryFailed;
				goto jDone;
			}
			if($uReaderCount.Increment(kAtomicRelaxed) == 1){
				if(!$mtxExclusive.Try()){
					$uReaderCount.Decrement(kAtomicRelaxed);
					$mtxWriterGuard.Unlock();
					eResult = kResTryFailed;
					goto jDone;
				}
				eResult = kResStateChanged;
			}
			$mtxWriterGuard.Unlock();
		}
	}
	::TlsSetValue($uTlsIndex.Get(), (void *)(uReaderRecur + 1));
jDone:
	return eResult;
}
ReaderWriterMutex::Result ReaderWriterMutex::LockAsReader() noexcept {
	Result eResult = kResRecursive;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue($uTlsIndex.Get());
	if(uReaderRecur == 0){
		if($mtxWriterGuard.IsLockedByCurrentThread()){
			$uReaderCount.Increment(kAtomicRelaxed);
		} else {
			$mtxWriterGuard.Lock();
			if($uReaderCount.Increment(kAtomicRelaxed) == 1){
				$mtxExclusive.Lock();
				eResult = kResStateChanged;
			}
			$mtxWriterGuard.Unlock();
		}
	}
	::TlsSetValue($uTlsIndex.Get(), (void *)(uReaderRecur + 1));
	return eResult;
}
ReaderWriterMutex::Result ReaderWriterMutex::UnlockAsReader() noexcept {
	Result eResult = kResRecursive;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue($uTlsIndex.Get());
	if(uReaderRecur == 1){
		if($mtxWriterGuard.IsLockedByCurrentThread()){
			$uReaderCount.Decrement(kAtomicRelaxed);
		} else {
			if($uReaderCount.Decrement(kAtomicRelaxed) == 0){
				$mtxExclusive.Unlock();
				eResult = kResStateChanged;
			}
		}
	}
	::TlsSetValue($uTlsIndex.Get(), (void *)(uReaderRecur - 1));
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
	ASSERT_MSG((std::uintptr_t)::TlsGetValue($uTlsIndex.Get()) == 0, L"获取写锁前必须先释放读锁。");

	const auto eResult = $mtxWriterGuard.Try();
	if(eResult != kResStateChanged){
		return eResult;
	}
	if(!$mtxExclusive.Try()){
		$mtxWriterGuard.Unlock();
		return kResTryFailed;
	}
	return kResStateChanged;
}
ReaderWriterMutex::Result ReaderWriterMutex::LockAsWriter() noexcept {
	ASSERT_MSG((std::uintptr_t)::TlsGetValue($uTlsIndex.Get()) == 0, L"获取写锁前必须先释放读锁。");

	const auto eResult = $mtxWriterGuard.Lock();
	if(eResult != kResStateChanged){
		return eResult;
	}
	$mtxExclusive.Lock();
	return kResStateChanged;
}
ReaderWriterMutex::Result ReaderWriterMutex::UnlockAsWriter() noexcept {
	const auto eResult = $mtxWriterGuard.Unlock();
	if(eResult != kResStateChanged){
		return eResult;
	}
	const auto uRecurReading = (std::uintptr_t)::TlsGetValue($uTlsIndex.Get());
	if(uRecurReading == 0){
		$mtxExclusive.Unlock();
	}
	return kResStateChanged;
}

}
