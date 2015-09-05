// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/System.hpp"
#include "../Utilities/Bail.hpp"

namespace MCF {

template<>
bool Mutex::UniqueLock::$DoTry() const noexcept {
	return $pOwner->Try();
}
template<>
void Mutex::UniqueLock::$DoLock() const noexcept {
	$pOwner->Lock();
}
template<>
void Mutex::UniqueLock::$DoUnlock() const noexcept {
	$pOwner->Unlock();
}

// 嵌套类定义。
struct Mutex::$QueueNode {
	$QueueNode *pNext;
};

// 构造函数和析构函数。
Mutex::Mutex(std::size_t uSpinCount)
	: $uSpinCount(0)
	, $uLockingThreadId(0), $vSemaphore(0), $pQueueHead(nullptr)
{
	SetSpinCount(uSpinCount);
}

// 其他非静态成员函数。
bool Mutex::$IsQueueEmpty() const noexcept {
	return $pQueueHead.Load(kAtomicConsume) == nullptr;
}
Mutex::$QueueNode *Mutex::$LockQueue() noexcept {
	for(;;){
		const auto pQueueHead = $pQueueHead.Exchange(($QueueNode *)-1, kAtomicRelease);
		if(pQueueHead != ($QueueNode *)-1){
			return pQueueHead;
		}
		AtomicPause();
	}
}
void Mutex::$UnlockQueue(Mutex::$QueueNode *pQueueHead) noexcept {
	$pQueueHead.Store(pQueueHead, kAtomicRelease);
}

std::size_t Mutex::GetSpinCount() const noexcept {
	return $uSpinCount.Load(kAtomicRelaxed);
}
void Mutex::SetSpinCount(std::size_t uSpinCount) noexcept {
	if(GetLogicalProcessorCount() == 0){
		return;
	}
	$uSpinCount.Store(uSpinCount, kAtomicRelaxed);
}

bool Mutex::Try() noexcept {
#ifndef NDEBUG
	const std::size_t uThreadId = ::GetCurrentThreadId();
	if($uLockingThreadId.Load(kAtomicConsume) == uThreadId){
		Bail(L"在不可重入的互斥锁中检测到死锁。");
	}
#else
	const std::size_t uThreadId = 1;
#endif

	if($IsQueueEmpty()){
		std::size_t uExpected = 0;
		if($uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
			return true;
		}
	}
	return false;
}
void Mutex::Lock() noexcept {
#ifndef NDEBUG
	const std::size_t uThreadId = ::GetCurrentThreadId();
	if($uLockingThreadId.Load(kAtomicConsume) == uThreadId){
		Bail(L"在不可重入的互斥锁中检测到死锁。");
	}
#else
	const std::size_t uThreadId = 1;
#endif

	if($IsQueueEmpty()){
		std::size_t uExpected = 0;
		if($uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
			return;
		}

		// 尝试忙等待。
		const auto uSpinCount = GetSpinCount();
		for(std::size_t i = 0; i < uSpinCount; ++i){
			AtomicPause();

			std::size_t uExpected = 0;
			if($uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
				return;
			}
		}
	} else {
		::SleepEx(1, false);
	}

	// 如果忙等待超过了自旋次数，就使用信号量同步。
	$QueueNode vThisThread = { nullptr };

	auto pQueueHead = $LockQueue();
	if(pQueueHead){
		auto pIns = pQueueHead;
		for(;;){
			const auto pNext = pIns->pNext;
			if(!pNext){
				pIns->pNext = &vThisThread;
				break;
			}
			pIns = pNext;
		}
	} else {
		std::size_t uExpected = 0;
		if($uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
			goto jLocked;
		}
		pQueueHead = &vThisThread;
	}
	for(;;){
		$UnlockQueue(pQueueHead);
		$vSemaphore.Wait();

		pQueueHead = $LockQueue();
		if(pQueueHead == &vThisThread){
			std::size_t uExpected = 0;
			if($uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
				pQueueHead = pQueueHead->pNext;
				break;
			}
		}
		$vSemaphore.Post();
	}
jLocked:
	$UnlockQueue(pQueueHead);
}
void Mutex::Unlock() noexcept {
#ifndef NDEBUG
	if($uLockingThreadId.Load(kAtomicConsume) == 0){
		Bail(L"互斥锁没有被任何线程锁定。");
	}
#endif

	$uLockingThreadId.Store(0, kAtomicRelease);

	auto pQueueHead = $LockQueue();
	if(pQueueHead){
		$vSemaphore.Post();
	}
	$UnlockQueue(pQueueHead);
}

}
