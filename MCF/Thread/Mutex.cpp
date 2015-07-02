// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/System.hpp"

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
	: x_vSemaphore(0), x_uSpinCount(0)
	, x_uLockingThreadId(0), x_pQueueHead(nullptr)
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
		::SwitchToThread();
	}
}
void Mutex::xUnlockQueue(Mutex::xQueueNode *pQueueHead) noexcept {
	AtomicStore(x_pQueueHead, pQueueHead, MemoryModel::kRelease);
}

void Mutex::SetSpinCount(std::size_t uSpinCount) noexcept {
	if(GetLogicalProcessorCount() == 0){
		return;
	}
	AtomicStore(x_uSpinCount, uSpinCount, MemoryModel::kRelaxed);
}

bool Mutex::IsLockedByCurrentThread() const noexcept {
	const auto dwThreadId = ::GetCurrentThreadId();

	return AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == dwThreadId;
}

bool Mutex::Try() noexcept {
	ASSERT_MSG(!IsLockedByCurrentThread(), L"在不可重入的互斥锁中检测到死锁。");

	const auto dwThreadId = ::GetCurrentThreadId();

	if(xIsQueueEmpty()){
		std::size_t uExpected = 0;
		return AtomicCompareExchange(x_uLockingThreadId, uExpected, dwThreadId, MemoryModel::kSeqCst, MemoryModel::kSeqCst);
	}
	return false;
}
void Mutex::Lock() noexcept {
	ASSERT_MSG(!IsLockedByCurrentThread(), L"在不可重入的互斥锁中检测到死锁。");

	const auto dwThreadId = ::GetCurrentThreadId();

	auto pQueueHead = xLockQueue();
	if(!pQueueHead){
		xUnlockQueue(pQueueHead);

		// 尝试忙等待。
		std::size_t uExpected = 0;
		if(AtomicCompareExchange(x_uLockingThreadId, uExpected, dwThreadId, MemoryModel::kSeqCst, MemoryModel::kSeqCst)){
			return;
		}

		const auto uSpinCount = GetSpinCount();
		if(uSpinCount != 0){
			::Sleep(1);

			std::size_t i = 0;
			do {
				std::size_t uExpected = 0;
				if(AtomicCompareExchange(x_uLockingThreadId, uExpected, dwThreadId, MemoryModel::kSeqCst, MemoryModel::kConsume)){
					return;
				}
				::SwitchToThread();
			} while(++i < uSpinCount);
		}

		pQueueHead = xLockQueue();
	}
	// 如果忙等待超过了自旋次数，就使用信号量同步。
	xQueueNode vThisThread = { nullptr };
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

		auto pQueueHead = xLockQueue();
		if(pQueueHead == &vThisThread){
			std::size_t uExpected = 0;
			if(AtomicCompareExchange(x_uLockingThreadId, uExpected, dwThreadId, MemoryModel::kSeqCst, MemoryModel::kConsume)){
				pQueueHead = pQueueHead->pNext;
				xUnlockQueue(pQueueHead);
				break;
			}
		}
		xUnlockQueue(pQueueHead);

		x_vSemaphore.Post();
	}
}
void Mutex::Unlock() noexcept {
	ASSERT_MSG(IsLockedByCurrentThread(), L"只能由持有互斥锁的线程释放该互斥锁。");

	auto pQueueHead = xLockQueue();
	if(pQueueHead){
		x_vSemaphore.Post();
	}
	xUnlockQueue(pQueueHead);

	AtomicStore(x_uLockingThreadId, 0, MemoryModel::kSeqCst);
}

}
