// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReadWriteLock.hpp"
#include "CriticalSection.hpp"
#include "Semaphore.hpp"
using namespace MCF;

static_assert(ReadWriteLock::MAXIMUM_CONCURRENT_READS > 1, "MAXIMUM_CONCURRENT_READS <= 1 : There exist potential deadlocks.");

// 嵌套类定义。
class ReadWriteLock::xDelegate : NO_COPY {
private:
	struct xReentryInfo {
		volatile DWORD dwThreadId;
		std::size_t uReentryCount;
	};

private:
	// http://en.wikipedia.org/wiki/Readers–writers_problem#The_third_readers-writers_problem
	CriticalSection xm_csNoWaiting; // 用于内部同步。
	Semaphore xm_semNoAccessing; // 当且仅当没有读者且没有写者拥有锁时为激发态。
	volatile std::size_t xm_uReaders; // “试图”拥有锁的读者数。

	// 重入支持。
	Semaphore xm_semMostOneReader; // 至多有一个读者拥有锁时为激发态。
	volatile std::size_t xm_uCurrentReaders; // 拥有锁的读者数。

	xReentryInfo xm_aReaderInfos[MAXIMUM_CONCURRENT_READS]; // 读者重入计数。
	Semaphore xm_semReaderInfoCount;
	xReentryInfo xm_uWriterInfo; // 写者重入计数。

public:
	xDelegate(unsigned long ulSpinCount)
		: xm_csNoWaiting(ulSpinCount)
		, xm_semNoAccessing(1, 1)
		, xm_uReaders(0)
		, xm_semMostOneReader(1, 1)
		, xm_uCurrentReaders(0)
		, xm_aReaderInfos()
		, xm_semReaderInfoCount(MAXIMUM_CONCURRENT_READS, MAXIMUM_CONCURRENT_READS)
		, xm_uWriterInfo()
	{
	}

private:
	xReentryInfo *xGetReaderReentryInfo(DWORD dwThreadId) noexcept {
		for(auto &ReaderInfo : xm_aReaderInfos){
			if(__atomic_load_n(&ReaderInfo.dwThreadId, __ATOMIC_RELAXED) == dwThreadId){
				return &ReaderInfo;
			}
		}
		return nullptr;
	}
	xReentryInfo *xAllocReaderReentryInfo(DWORD dwThreadId) noexcept {
		const auto pExisting = xGetReaderReentryInfo(dwThreadId);
		if(pExisting){
			return pExisting;
		}
		for(;;){
			xm_semReaderInfoCount.Wait();
			for(auto &ReaderInfo : xm_aReaderInfos){
				DWORD dwExpected = 0;
				if(__atomic_compare_exchange_n(&ReaderInfo.dwThreadId, &dwExpected, dwThreadId, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
					return &ReaderInfo;
				}
			}
		}
	}
	void xDeleteReaderReentryInfo(xReentryInfo *pReentryInfo) noexcept {
		ASSERT(__atomic_load_n(&pReentryInfo->dwThreadId, __ATOMIC_RELAXED) == ::GetCurrentThreadId());

		__atomic_store_n(&pReentryInfo->dwThreadId, 0, __ATOMIC_RELAXED);
		xm_semReaderInfoCount.Signal();
	}

public:
	void LockRead() noexcept {
		const auto dwCurrentThreadId = ::GetCurrentThreadId();
		auto pReaderReentryInfo = xGetReaderReentryInfo(dwCurrentThreadId);
		if(!pReaderReentryInfo){
			if(__atomic_load_n(&xm_uWriterInfo.dwThreadId, __ATOMIC_ACQUIRE) == dwCurrentThreadId){
				__atomic_fetch_add(&xm_uReaders, 1, __ATOMIC_SEQ_CST);
			} else {
				CRITICAL_SECTION_SCOPE(xm_csNoWaiting){
					switch(__atomic_fetch_add(&xm_uReaders, 1, __ATOMIC_SEQ_CST)){
					case 0:
						xm_semNoAccessing.Wait();
						break;
					case 1:
						xm_semMostOneReader.Wait();
						break;
					}
				}
			}
			__atomic_fetch_add(&xm_uCurrentReaders, 1, __ATOMIC_RELAXED);

			pReaderReentryInfo = xAllocReaderReentryInfo(dwCurrentThreadId);
		}
		++pReaderReentryInfo->uReentryCount;
	}
	void UnlockRead() noexcept {
		const auto dwCurrentThreadId = ::GetCurrentThreadId();
		const auto pReaderReentryInfo = xGetReaderReentryInfo(dwCurrentThreadId);
		ASSERT(pReaderReentryInfo != nullptr);

		if(--pReaderReentryInfo->uReentryCount == 0){
			xDeleteReaderReentryInfo(pReaderReentryInfo);

			if(__atomic_load_n(&xm_uWriterInfo.dwThreadId, __ATOMIC_ACQUIRE) == dwCurrentThreadId){
				__atomic_sub_fetch(&xm_uCurrentReaders, 1, __ATOMIC_SEQ_CST);
			} else {
				switch(__atomic_sub_fetch(&xm_uCurrentReaders, 1, __ATOMIC_SEQ_CST)){
				case 0:
					xm_semNoAccessing.Signal();
					break;
				case 1:
					xm_semMostOneReader.Signal();
					break;
				}
			}
			__atomic_sub_fetch(&xm_uReaders, 1, __ATOMIC_RELAXED);
		}
	}

	void LockWrite() noexcept {
		const auto dwCurrentThreadId = ::GetCurrentThreadId();
		if(__atomic_load_n(&xm_uWriterInfo.dwThreadId, __ATOMIC_ACQUIRE) != dwCurrentThreadId){
			const auto pReaderReentryInfo = xGetReaderReentryInfo(dwCurrentThreadId);
			if(pReaderReentryInfo){
				xm_semMostOneReader.Wait();
				xm_semMostOneReader.Signal();
			} else {
				CRITICAL_SECTION_SCOPE(xm_csNoWaiting){
					xm_semNoAccessing.Wait();
				}
			}
			__atomic_store_n(&xm_uWriterInfo.dwThreadId, dwCurrentThreadId, __ATOMIC_RELEASE);

			ASSERT(xm_uWriterInfo.uReentryCount == 0);
		}
		++xm_uWriterInfo.uReentryCount;
	}
	void UnlockWrite() noexcept {
		ASSERT(__atomic_load_n(&xm_uWriterInfo.dwThreadId, __ATOMIC_ACQUIRE) == ::GetCurrentThreadId());

		if(--xm_uWriterInfo.uReentryCount == 0){
			__atomic_store_n(&xm_uWriterInfo.dwThreadId, 0, __ATOMIC_RELEASE);
			const auto dwCurrentThreadId = ::GetCurrentThreadId();
			const auto pReaderReentryInfo = xGetReaderReentryInfo(dwCurrentThreadId);
			if(!pReaderReentryInfo){
				xm_semNoAccessing.Signal();
			}
		}
	}
};

// 静态成员函数。
void ReadWriteLock::xUnlockRead(ReadWriteLock::xDelegate *pDelegate) noexcept {
	if(pDelegate){
		pDelegate->UnlockRead();
	}
}
void ReadWriteLock::xUnlockWrite(ReadWriteLock::xDelegate *pDelegate) noexcept {
	if(pDelegate){
		pDelegate->UnlockWrite();
	}
}

// 构造函数和析构函数。
ReadWriteLock::ReadWriteLock(unsigned long ulSpinCount)
	: xm_pDelegate(new xDelegate(ulSpinCount))
{
}
ReadWriteLock::ReadWriteLock(ReadWriteLock &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
ReadWriteLock &ReadWriteLock::operator=(ReadWriteLock &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
ReadWriteLock::~ReadWriteLock(){
}

// 其他非静态成员函数。
ReadWriteLock::ReadLockHolder ReadWriteLock::GetReadLock() const noexcept {
	ASSERT(xm_pDelegate);

	xm_pDelegate->LockRead();
	return ReadLockHolder(xm_pDelegate.get());
}
ReadWriteLock::WriteLockHolder ReadWriteLock::GetWriteLock() noexcept {
	ASSERT(xm_pDelegate);

	xm_pDelegate->LockWrite();
	return WriteLockHolder(xm_pDelegate.get());
}
