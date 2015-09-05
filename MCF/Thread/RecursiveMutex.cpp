// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RecursiveMutex.hpp"

namespace MCF {

template<>
bool RecursiveMutex::UniqueLock::$DoTry() const noexcept {
	return $pOwner->Try() != RecursiveMutex::Result::kResTryFailed;
}
template<>
void RecursiveMutex::UniqueLock::$DoLock() const noexcept {
	$pOwner->Lock();
}
template<>
void RecursiveMutex::UniqueLock::$DoUnlock() const noexcept {
	$pOwner->Unlock();
}

// 构造函数和析构函数。
RecursiveMutex::RecursiveMutex(std::size_t uSpinCount)
	: $vMutex(uSpinCount)
	, $uLockingThreadId(0), $uRecursionCount(0)
{
}

// 其他非静态成员函数。
bool RecursiveMutex::IsLockedByCurrentThread() const noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	return $uLockingThreadId.Load(kAtomicConsume) == dwThreadId;
}

RecursiveMutex::Result RecursiveMutex::Try() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if($uLockingThreadId.Load(kAtomicConsume) == dwThreadId){
		++$uRecursionCount;
		return kResRecursive;
	}
	if(!$vMutex.Try()){
		return kResTryFailed;
	}
	$uLockingThreadId.Store(dwThreadId, kAtomicRelease);
	++$uRecursionCount;
	return kResStateChanged;
}
RecursiveMutex::Result RecursiveMutex::Lock() noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();
	if($uLockingThreadId.Load(kAtomicConsume) == dwThreadId){
		++$uRecursionCount;
		return kResRecursive;
	}
	$vMutex.Lock();
	$uLockingThreadId.Store(dwThreadId, kAtomicRelease);
	++$uRecursionCount;
	return kResStateChanged;
}
RecursiveMutex::Result RecursiveMutex::Unlock() noexcept {
	ASSERT_MSG(IsLockedByCurrentThread(), L"试图使用当前不持有递归互斥体的线程释放递归互斥体。");

	if(--$uRecursionCount != 0){
		return kResRecursive;
	}
	$uLockingThreadId.Store(0, kAtomicRelaxed);
	$vMutex.Unlock();
	return kResStateChanged;
}

}
