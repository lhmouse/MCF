// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SYNCHRONIZERS_HPP__
#define __MCF_SYNCHRONIZERS_HPP__

#include <windows.h>
#include <map>

namespace MCF {
	class CriticalSection final : NO_COPY_OR_ASSIGN {
	private:
		LPCRITICAL_SECTION xm_pCriticalSection;
	public:
		CriticalSection(std::size_t uSpinCount = 0x400){
			xm_pCriticalSection = new CRITICAL_SECTION;
			VERIFY(::InitializeCriticalSectionAndSpinCount(xm_pCriticalSection, (DWORD)uSpinCount));
		}
		CriticalSection(CriticalSection &&src){
			ASSERT(src.xm_pCriticalSection != nullptr);

			xm_pCriticalSection = src.xm_pCriticalSection;
			src.xm_pCriticalSection = nullptr;
		}
		CriticalSection &operator=(CriticalSection &&src){
			ASSERT(src.xm_pCriticalSection != nullptr);

			xm_pCriticalSection = src.xm_pCriticalSection;
			src.xm_pCriticalSection = nullptr;
			return *this;
		}
		~CriticalSection(){
			if(xm_pCriticalSection != nullptr){
				::DeleteCriticalSection(xm_pCriticalSection);
				delete xm_pCriticalSection;
			}
		}
	public:
		bool Try() const {
			ASSERT(xm_pCriticalSection != nullptr);

			return ::TryEnterCriticalSection(xm_pCriticalSection) != FALSE;
		}
		void Enter() const {
			ASSERT(xm_pCriticalSection != nullptr);

			::EnterCriticalSection(xm_pCriticalSection);
		}
		void Leave() const {
			ASSERT(xm_pCriticalSection != nullptr);

			::LeaveCriticalSection(xm_pCriticalSection);
		}
	};

	class Event final : NO_COPY_OR_ASSIGN {
	private:
		HANDLE xm_hEvent;
	public:
		Event(bool bIsInitiallySet, bool bAutoReset = false){
			xm_hEvent = VERIFY(::CreateEvent(nullptr, bAutoReset ? FALSE : TRUE, bIsInitiallySet ? TRUE : FALSE, nullptr));
		}
		Event(Event &&src){
			ASSERT(src.xm_hEvent != NULL);

			xm_hEvent = src.xm_hEvent;
			src.xm_hEvent = NULL;
		}
		Event &operator=(Event &&src){
			ASSERT(src.xm_hEvent != NULL);

			xm_hEvent = src.xm_hEvent;
			src.xm_hEvent = NULL;
			return *this;
		}
		~Event(){
			if(xm_hEvent != NULL){
				::CloseHandle(xm_hEvent);
			}
		}
	public:
		HANDLE GetHEvent() const {
			ASSERT(xm_hEvent != NULL);

			return xm_hEvent;
		}

		bool IsClear() const {
			return WaitTimeout(0);
		}
		void Wait() const {
			WaitTimeout(INFINITE);
		}
		bool WaitTimeout(DWORD dwMilliSeconds) const {
			ASSERT(xm_hEvent != NULL);

			return ::WaitForSingleObject(xm_hEvent, dwMilliSeconds) != WAIT_TIMEOUT;
		}

		void Set(){
			ASSERT(xm_hEvent != NULL);

			::SetEvent(xm_hEvent);
		}
		void Clear(){
			ASSERT(xm_hEvent != NULL);

			::ResetEvent(xm_hEvent);
		}
	};

	class Semaphore final : NO_COPY_OR_ASSIGN {
	private:
		HANDLE xm_hSemaphore;
	public:
		Semaphore(long lInitCount, long lMaxCount){
			xm_hSemaphore = VERIFY(::CreateSemaphore(nullptr, lInitCount, lMaxCount, nullptr));
		}
		Semaphore(Semaphore &&src){
			ASSERT(src.xm_hSemaphore != NULL);

			xm_hSemaphore = src.xm_hSemaphore;
			src.xm_hSemaphore = NULL;
		}
		Semaphore &operator=(Semaphore &&src){
			ASSERT(src.xm_hSemaphore != NULL);

			xm_hSemaphore = src.xm_hSemaphore;
			src.xm_hSemaphore = NULL;
			return *this;
		}
		~Semaphore(){
			::CloseHandle(xm_hSemaphore);
		}
	public:
		HANDLE GetHSemaphore() const {
			ASSERT(xm_hSemaphore != NULL);

			return xm_hSemaphore;
		}

		void Wait(long lCount = 1){
			ASSERT(xm_hSemaphore != NULL);

			for(long i = 0; i < lCount; ++i){
				::WaitForSingleObject(xm_hSemaphore, INFINITE);
			}
		}
		long Try(long lCount = 1){
			ASSERT(xm_hSemaphore != NULL);

			long lSucceeded = 0;
			while((lSucceeded < lCount) && (::WaitForSingleObject(xm_hSemaphore, 0) != WAIT_TIMEOUT)){
				++lSucceeded;
			}
			return lSucceeded;
		}
		long Post(long lCount = 1){
			ASSERT(xm_hSemaphore != NULL);

			long lPrevCount;
			VERIFY(::ReleaseSemaphore(xm_hSemaphore, lCount, &lPrevCount));
			return lPrevCount;
		}
	};

	class ReadWriteLock final : NO_COPY_OR_ASSIGN {
	private:
		mutable volatile long xm_lReaderCount;
		CriticalSection xm_csReaderCountLock;
		mutable volatile long xm_lWriterCount;
		CriticalSection xm_csWriterCountLock;
		mutable Event xm_evnWriterDeactive;

		mutable Semaphore xm_semExclusiveLock;
		mutable volatile DWORD xm_dwLockOwner;
		mutable volatile long xm_lRecursionCount;
	public:
		ReadWriteLock() :
			xm_evnWriterDeactive(true),
			xm_semExclusiveLock(1, 1)
		{
			xm_lReaderCount		= 0;
			xm_lWriterCount		= 0;
			xm_dwLockOwner		= 0;
			xm_lRecursionCount	= 0;
		}
		ReadWriteLock(ReadWriteLock &&src) :
			xm_lReaderCount			(std::move(src.xm_lReaderCount)),
			xm_csReaderCountLock	(std::move(src.xm_csReaderCountLock)),
			xm_lWriterCount			(std::move(src.xm_lWriterCount)),
			xm_csWriterCountLock	(std::move(src.xm_csWriterCountLock)),
			xm_evnWriterDeactive	(std::move(src.xm_evnWriterDeactive)),

			xm_semExclusiveLock		(std::move(src.xm_semExclusiveLock)),
			xm_dwLockOwner			(std::move(src.xm_dwLockOwner)),
			xm_lRecursionCount		(std::move(src.xm_lRecursionCount))
		{
		}
		ReadWriteLock &operator=(ReadWriteLock &&src){
			xm_lReaderCount			= std::move(src.xm_lReaderCount);
			xm_csReaderCountLock	= std::move(src.xm_csReaderCountLock);
			xm_lWriterCount			= std::move(src.xm_lWriterCount);
			xm_csWriterCountLock	= std::move(src.xm_csWriterCountLock);
			xm_evnWriterDeactive	= std::move(src.xm_evnWriterDeactive);

			xm_semExclusiveLock		= std::move(src.xm_semExclusiveLock);
			xm_dwLockOwner			= std::move(src.xm_dwLockOwner);
			xm_lRecursionCount		= std::move(src.xm_lRecursionCount);

			return *this;
		}
	private:
		void xAcquireExclusiveLock() const {
			const DWORD dwCurrentThreadID = ::GetCurrentThreadId();
			if(xm_dwLockOwner != dwCurrentThreadID){
				xm_semExclusiveLock.Wait();
				xm_dwLockOwner = dwCurrentThreadID;
			}
			++xm_lRecursionCount;
		}
		void xReleaseExclusiveLock() const {
			if(--xm_lRecursionCount == 0){
				xm_dwLockOwner = 0;
				xm_semExclusiveLock.Post();
			}
		}
	public:
		void AcquireReadLock() const {
			if(xm_dwLockOwner != ::GetCurrentThreadId()){
				xm_evnWriterDeactive.Wait();
			}

			xm_csReaderCountLock.Enter();
				if(++xm_lReaderCount == 1){
					xAcquireExclusiveLock();
				}
			xm_csReaderCountLock.Leave();
		}
		void ReleaseReadLock() const {
			xm_csReaderCountLock.Enter();
				if(--xm_lReaderCount == 0){
					xReleaseExclusiveLock();
				}
			xm_csReaderCountLock.Leave();
		}

		void AcquireWriteLock() const {
			xm_csWriterCountLock.Enter();
				if(++xm_lWriterCount == 1){
					xm_evnWriterDeactive.Clear();
				}
			xm_csWriterCountLock.Leave();

			xAcquireExclusiveLock();
		}
		void ReleaseWriteLock() const {
			xReleaseExclusiveLock();

			xm_csWriterCountLock.Enter();
				if(--xm_lWriterCount == 0){
					xm_evnWriterDeactive.Set();
				}
			xm_csWriterCountLock.Leave();
		}
	};
}

#endif
