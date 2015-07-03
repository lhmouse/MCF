// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "Atomic.hpp"
#include "../Core/Exception.hpp"
#include "../Core/System.hpp"
#include "../Utilities/Bail.hpp"

namespace MCF {

template<>
bool Mutex::UniqueLock::xDoTry() const noexcept {
	return x_pOwner->Try();
}
template<>
void Mutex::UniqueLock::xDoLock() const noexcept {
	x_pOwner->Lock();
}
template<>
void Mutex::UniqueLock::xDoUnlock() const noexcept {
	x_pOwner->Unlock();
}

// 嵌套类定义。
struct Mutex::xQueueNode {
	xQueueNode *pNext;
};

// 构造函数和析构函数。
Mutex::Mutex(std::size_t uSpinCount)
	: x_uSpinCount(0)
	, x_bLocked(false), x_vSemaphore(0), x_pQueueHead(nullptr)
	, x_uLockingThreadId(0)
{
	SetSpinCount(uSpinCount);

	AtomicFence(MemoryModel::kRelease);
}

// 其他非静态成员函数。
bool Mutex::xIsQueueEmpty() const noexcept {
	return AtomicLoad(x_pQueueHead, MemoryModel::kConsume) == nullptr;
}
Mutex::xQueueNode *Mutex::xLockQueue() noexcept {
	for(;;){
		const auto pQueueHead = AtomicExchange(x_pQueueHead, (xQueueNode *)-1, MemoryModel::kAcquire);
		if(pQueueHead != (xQueueNode *)-1){
			return pQueueHead;
		}
		AtomicPause();
	}
}
void Mutex::xUnlockQueue(Mutex::xQueueNode *pQueueHead) noexcept {
	AtomicStore(x_pQueueHead, pQueueHead, MemoryModel::kRelease);
}

std::size_t Mutex::GetSpinCount() const noexcept {
	return AtomicLoad(x_uSpinCount, MemoryModel::kRelaxed);
}
void Mutex::SetSpinCount(std::size_t uSpinCount) noexcept {
	if(GetLogicalProcessorCount() == 0){
		return;
	}
	AtomicStore(x_uSpinCount, uSpinCount, MemoryModel::kRelaxed);
}

bool Mutex::Try() noexcept {
#ifndef NDEBUG
	const auto dwThreadId = ::GetCurrentThreadId();
	if(AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == dwThreadId){
		Bail(L"在不可重入的互斥锁中检测到死锁。");
	}
#endif

	if(xIsQueueEmpty()){
		if(AtomicExchange(x_bLocked, true, MemoryModel::kSeqCst) == false){
			goto jLocked;
		}
	}
	return false;

jLocked:
#ifndef NDEBUG
	AtomicStore(x_uLockingThreadId, dwThreadId, MemoryModel::kRelease);
#endif
	return true;
}
void Mutex::Lock() noexcept {
#ifndef NDEBUG
	const auto dwThreadId = ::GetCurrentThreadId();
	if(AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == dwThreadId){
		Bail(L"在不可重入的互斥锁中检测到死锁。");
	}
#endif

	if(AtomicExchange(x_bLocked, true, MemoryModel::kSeqCst) == false){
		goto jLocked;
	}

	{
		// 尝试忙等待。
		const auto uSpinCount = GetSpinCount();
		for(std::size_t i = 0; i < uSpinCount; ++i){
			::SwitchToThread();

			if(AtomicExchange(x_bLocked, true, MemoryModel::kSeqCst) == false){
				goto jLocked;
			}
		}

		::Sleep(1);

		// 如果忙等待超过了自旋次数，就使用内核态互斥体同步。
		xQueueNode vThisThread = { nullptr };

		auto pQueueHead = xLockQueue();
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
			pQueueHead = &vThisThread;
		}
		xUnlockQueue(pQueueHead);

		for(;;){
			x_vSemaphore.Wait();

			pQueueHead = xLockQueue();
			if(pQueueHead == &vThisThread){
				if(AtomicExchange(x_bLocked, true, MemoryModel::kSeqCst) == false){
					pQueueHead = pQueueHead->pNext;
					xUnlockQueue(pQueueHead);
					goto jLocked;
				}
			}
			x_vSemaphore.Post();
			xUnlockQueue(pQueueHead);
		}
	}
	return;

jLocked:
#ifndef NDEBUG
	AtomicStore(x_uLockingThreadId, dwThreadId, MemoryModel::kRelease);
#endif
	return;
}
void Mutex::Unlock() noexcept {
#ifndef NDEBUG
	if(AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == 0){
		Bail(L"互斥锁没有被任何线程锁定。");
	}

	AtomicStore(x_uLockingThreadId, 0, MemoryModel::kRelease);
#endif

	auto pQueueHead = xLockQueue();
	if(pQueueHead){
		x_vSemaphore.Post();
	}
	xUnlockQueue(pQueueHead);

	AtomicStore(x_bLocked, 0, MemoryModel::kSeqCst);
}

}
