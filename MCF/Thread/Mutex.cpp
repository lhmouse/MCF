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
	, x_uLockingThreadId(0), x_vSemaphore(0), x_pQueueHead(nullptr)
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
		const auto pQueueHead = AtomicExchange(x_pQueueHead, (xQueueNode *)-1, MemoryModel::kRelease);
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
	const std::size_t uThreadId = ::GetCurrentThreadId();
	if(AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == uThreadId){
		Bail(L"在不可重入的互斥锁中检测到死锁。");
	}
#else
	const std::size_t uThreadId = 1;
#endif

	if(xIsQueueEmpty()){
		std::size_t uExpected = 0;
		if(AtomicCompareExchange(x_uLockingThreadId, uExpected, uThreadId, MemoryModel::kAcqRel, MemoryModel::kConsume)){
			return true;
		}
	}
	return false;
}
void Mutex::Lock() noexcept {
#ifndef NDEBUG
	const std::size_t uThreadId = ::GetCurrentThreadId();
	if(AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == uThreadId){
		Bail(L"在不可重入的互斥锁中检测到死锁。");
	}
#else
	const std::size_t uThreadId = 1;
#endif

	if(xIsQueueEmpty()){
		std::size_t uExpected = 0;
		if(AtomicCompareExchange(x_uLockingThreadId, uExpected, uThreadId, MemoryModel::kAcqRel, MemoryModel::kConsume)){
			return;
		}

		// 尝试忙等待。
		const auto uSpinCount = GetSpinCount();
		for(std::size_t i = 0; i < uSpinCount; ++i){
			::SwitchToThread();

			std::size_t uExpected = 0;
			if(AtomicCompareExchange(x_uLockingThreadId, uExpected, uThreadId, MemoryModel::kAcqRel, MemoryModel::kConsume)){
				return;
			}
		}
	}

	::Sleep(1);

	// 如果忙等待超过了自旋次数，就使用信号量同步。
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
		std::size_t uExpected = 0;
		if(AtomicCompareExchange(x_uLockingThreadId, uExpected, uThreadId, MemoryModel::kAcqRel, MemoryModel::kConsume)){
			xUnlockQueue(pQueueHead);
			return;
		}
		pQueueHead = &vThisThread;
	}
	xUnlockQueue(pQueueHead);

	for(;;){
		x_vSemaphore.Wait();

		pQueueHead = xLockQueue();
		if(pQueueHead == &vThisThread){
			std::size_t uExpected = 0;
			if(AtomicCompareExchange(x_uLockingThreadId, uExpected, uThreadId, MemoryModel::kAcqRel, MemoryModel::kConsume)){
				pQueueHead = pQueueHead->pNext;
				xUnlockQueue(pQueueHead);
				return;
			}
		}
		x_vSemaphore.Post();
		xUnlockQueue(pQueueHead);
	}
}
void Mutex::Unlock() noexcept {
#ifndef NDEBUG
	if(AtomicLoad(x_uLockingThreadId, MemoryModel::kConsume) == 0){
		Bail(L"互斥锁没有被任何线程锁定。");
	}
#endif

	AtomicStore(x_uLockingThreadId, 0, MemoryModel::kRelease);

	auto pQueueHead = xLockQueue();
	if(pQueueHead){
		x_vSemaphore.Post();
	}
	xUnlockQueue(pQueueHead);
}

}
