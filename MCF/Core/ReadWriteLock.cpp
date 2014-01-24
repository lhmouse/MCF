// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReadWriteLock.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
#include "CriticalSection.hpp"
#include "Semaphore.hpp"
using namespace MCF;

// 嵌套类定义。
class ReadWriteLock::xDelegate : NO_COPY {
private:
	struct xMutexCloser {
		constexpr HANDLE operator()() const {
			return NULL;
		}
		void operator()(HANDLE hMutex) const {
			::CloseHandle(hMutex);
		}
	};
private:
	const unsigned long xm_ulSpinCount;

	// http://en.wikipedia.org/wiki/Readers–writers_problem#The_third_readers-writers_problem
	CriticalSection xm_csNoWaiting;
	Semaphore xm_semNoAccessing;
	DWORD xm_dwWriterId;
	std::size_t xm_uReentryCount;
	volatile std::size_t xm_uReaders;
public:
	xDelegate(unsigned long ulSpinCount) noexcept
		: xm_ulSpinCount(ulSpinCount)
		, xm_csNoWaiting(ulSpinCount)
		, xm_semNoAccessing(1, 1)
		, xm_dwWriterId(0)
		, xm_uReentryCount(0)
		, xm_uReaders(0)
	{
	}
private:
	void xAcquireMutex() noexcept {
		xm_semNoAccessing.Wait();

		ASSERT(xm_dwWriterId == 0);
	}
	void xReleaseMutex() noexcept {
		ASSERT(xm_dwWriterId == 0);

		xm_semNoAccessing.Signal();
	}
public:
	void LockRead() noexcept {
		CRITICAL_SECTION_SCOPE(xm_csNoWaiting){
			if(__atomic_add_fetch(&xm_uReaders, 1, __ATOMIC_ACQ_REL) == 1){
				xAcquireMutex();
			}
		}
	}
	void UnlockRead() noexcept {
		if(__atomic_sub_fetch(&xm_uReaders, 1, __ATOMIC_SEQ_CST) == 0){
			xReleaseMutex();
		}
	}

	void LockWrite() noexcept {
		CRITICAL_SECTION_SCOPE(xm_csNoWaiting){
			xAcquireMutex();
			xm_dwWriterId = ::GetCurrentThreadId();
		}
		++xm_uReentryCount;
	}
	void UnlockWrite() noexcept {
		ASSERT(xm_dwWriterId == ::GetCurrentThreadId());

		if(--xm_uReentryCount == 0){
			xm_dwWriterId = 0;
			xReleaseMutex();
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
ReadWriteLock::~ReadWriteLock(){
}

// 其他非静态成员函数。
ReadWriteLock::ReadLockHolder ReadWriteLock::GetReadLock() const noexcept {
	xm_pDelegate->LockRead();
	return ReadLockHolder(xm_pDelegate.get());
}
ReadWriteLock::WriteLockHolder ReadWriteLock::GetWriteLock() noexcept {
	xm_pDelegate->LockWrite();
	return WriteLockHolder(xm_pDelegate.get());
}
