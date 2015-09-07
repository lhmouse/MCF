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
bool Mutex::UniqueLock::X_DoTry() const noexcept {
	return x_pOwner->Try();
}
template<>
void Mutex::UniqueLock::X_DoLock() const noexcept {
	x_pOwner->Lock();
}
template<>
void Mutex::UniqueLock::X_DoUnlock() const noexcept {
	x_pOwner->Unlock();
}

// 嵌套类定义。
struct Mutex::X_QueueNode {
	X_QueueNode *pNext;
};

// 构造函数和析构函数。
Mutex::Mutex(std::size_t uSpinCount)
	: x_uSpinCount(0)
	, x_uLockingThreadId(0), x_vSemaphore(0), x_pQueueHead(nullptr)
{
	SetSpinCount(uSpinCount);
}

// 其他非静态成员函数。
bool Mutex::X_IsQueueEmpty() const noexcept {
	return x_pQueueHead.Load(kAtomicConsume) == nullptr;
}
Mutex::X_QueueNode *Mutex::X_LockQueue() noexcept {
	for(;;){
		const auto pQueueHead = x_pQueueHead.Exchange((X_QueueNode *)-1, kAtomicRelease);
		if(pQueueHead != (X_QueueNode *)-1){
			return pQueueHead;
		}
		AtomicPause();
	}
}
void Mutex::X_UnlockQueue(Mutex::X_QueueNode *pQueueHead) noexcept {
	x_pQueueHead.Store(pQueueHead, kAtomicRelease);
}

std::size_t Mutex::GetSpinCount() const noexcept {
	return x_uSpinCount.Load(kAtomicRelaxed);
}
void Mutex::SetSpinCount(std::size_t uSpinCount) noexcept {
	if(GetLogicalProcessorCount() == 0){
		return;
	}
	x_uSpinCount.Store(uSpinCount, kAtomicRelaxed);
}

bool Mutex::Try() noexcept {
#ifndef NDEBUG
	const std::size_t uThreadId = ::GetCurrentThreadId();
	if(x_uLockingThreadId.Load(kAtomicConsume) == uThreadId){
		Bail(L"在不可重入的互斥锁中检测到死锁。");
	}
#else
	const std::size_t uThreadId = 1;
#endif

	if(X_IsQueueEmpty()){
		std::size_t uExpected = 0;
		if(x_uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
			return true;
		}
	}
	return false;
}
void Mutex::Lock() noexcept {
#ifndef NDEBUG
	const std::size_t uThreadId = ::GetCurrentThreadId();
	if(x_uLockingThreadId.Load(kAtomicConsume) == uThreadId){
		Bail(L"在不可重入的互斥锁中检测到死锁。");
	}
#else
	const std::size_t uThreadId = 1;
#endif

	if(X_IsQueueEmpty()){
		std::size_t uExpected = 0;
		if(x_uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
			return;
		}

		// 尝试忙等待。
		const auto uSpinCount = GetSpinCount();
		for(std::size_t i = 0; i < uSpinCount; ++i){
			AtomicPause();

			std::size_t uExpected = 0;
			if(x_uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
				return;
			}
		}
	} else {
		::SleepEx(1, false);
	}

	// 如果忙等待超过了自旋次数，就使用信号量同步。
	X_QueueNode vThisThread = { nullptr };

	auto pQueueHead = X_LockQueue();
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
		if(x_uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
			goto jLocked;
		}
		pQueueHead = &vThisThread;
	}
	for(;;){
		X_UnlockQueue(pQueueHead);
		x_vSemaphore.Wait();

		pQueueHead = X_LockQueue();
		if(pQueueHead == &vThisThread){
			std::size_t uExpected = 0;
			if(x_uLockingThreadId.CompareExchange(uExpected, uThreadId, kAtomicAcqRel, kAtomicConsume)){
				pQueueHead = pQueueHead->pNext;
				break;
			}
		}
		x_vSemaphore.Post();
	}
jLocked:
	X_UnlockQueue(pQueueHead);
}
void Mutex::Unlock() noexcept {
#ifndef NDEBUG
	if(x_uLockingThreadId.Load(kAtomicConsume) == 0){
		Bail(L"互斥锁没有被任何线程锁定。");
	}
#endif

	x_uLockingThreadId.Store(0, kAtomicRelease);

	auto pQueueHead = X_LockQueue();
	if(pQueueHead){
		x_vSemaphore.Post();
	}
	X_UnlockQueue(pQueueHead);
}

}
