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

	struct xLockHolder {
		DWORD dwThreadId;
		DWORD dwReentryCount;
	};
private:
	const unsigned long xm_ulSpinCount;

	// http://en.wikipedia.org/wiki/Readers–writers_problem#The_third_readers-writers_problem
	CriticalSection xm_csNoWaiting;
	Semaphore xm_semNoAccessing;
	volatile std::size_t xm_uReaders;
public:
	xDelegate(unsigned long ulSpinCount) noexcept
		: xm_ulSpinCount(ulSpinCount)
		, xm_csNoWaiting(ulSpinCount)
		, xm_semNoAccessing(1, 1)
		, xm_uReaders(0)
	{
	}
public:
	void LockRead() noexcept {
		CRITICAL_SECTION_SCOPE(xm_csNoWaiting){
			if(__atomic_add_fetch(&xm_uReaders, 1, __ATOMIC_ACQ_REL) == 1){
				xm_semNoAccessing.Wait();
			}
		}
	}
	void UnlockRead() noexcept {
		if(__atomic_sub_fetch(&xm_uReaders, 1, __ATOMIC_SEQ_CST) == 0){
			xm_semNoAccessing.Signal();
		}
	}

	void LockWrite() noexcept {
		CRITICAL_SECTION_SCOPE(xm_csNoWaiting){
			xm_semNoAccessing.Wait();
		}
	}
	void UnlockWrite() noexcept {
		xm_semNoAccessing.Signal();
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
