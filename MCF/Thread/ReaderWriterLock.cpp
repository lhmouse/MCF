// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ReaderWriterLock.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

namespace MCF {

template<>
bool ReaderWriterLock::ReaderLock::xDoTry() const noexcept {
	return xm_pOwner->TryAsReader() != ReaderWriterLock::R_TRY_FAILED;
}
template<>
void ReaderWriterLock::ReaderLock::xDoLock() const noexcept {
	xm_pOwner->WaitAsReader();
}
template<>
void ReaderWriterLock::ReaderLock::xDoUnlock() const noexcept {
	xm_pOwner->ReleaseAsReader();
}

template<>
bool ReaderWriterLock::WriterLock::xDoTry() const noexcept {
	return xm_pOwner->TryAsWriter() != ReaderWriterLock::R_TRY_FAILED;
}
template<>
void ReaderWriterLock::WriterLock::xDoLock() const noexcept {
	xm_pOwner->WaitAsWriter();
}
template<>
void ReaderWriterLock::WriterLock::xDoUnlock() const noexcept {
	xm_pOwner->ReleaseAsWriter();
}

}

// 嵌套类成员函数定义。
unsigned long ReaderWriterLock::xTlsIndexDeleter::operator()() const noexcept {
	return TLS_OUT_OF_INDEXES;
}
void ReaderWriterLock::xTlsIndexDeleter::operator()(unsigned long ulTlsIndex) const noexcept {
	::TlsFree(ulTlsIndex);
}

// 构造函数和析构函数。
ReaderWriterLock::ReaderWriterLock(unsigned long ulSpinCount)
	: xm_csWriterGuard(ulSpinCount), xm_semExclusive(1, nullptr), xm_ulReaderCount(0)
{
	if(!xm_ulTlsIndex.Reset(::TlsAlloc())){
		DEBUG_THROW(SystemError, "TlsAlloc");
	}
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

// 其他非静态成员函数。
ReaderWriterLock::Result ReaderWriterLock::TryAsReader() noexcept {
	Result eResult = R_RECURSIVE;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue(xm_ulTlsIndex.Get());
	if(uReaderRecur == 0){
		if(xm_csWriterGuard.IsLockedByCurrentThread()){
			__atomic_add_fetch(&xm_ulReaderCount, 1, __ATOMIC_ACQ_REL);
		} else {
			if(xm_csWriterGuard.Try() == R_TRY_FAILED){
				eResult = R_TRY_FAILED;
				goto jDone;
			}
			if(__atomic_add_fetch(&xm_ulReaderCount, 1, __ATOMIC_ACQ_REL) == 1){
				if(!xm_semExclusive.Wait(0)){
					__atomic_sub_fetch(&xm_ulReaderCount, 1, __ATOMIC_ACQ_REL);
					xm_csWriterGuard.Release();
					eResult = R_TRY_FAILED;
					goto jDone;
				}
				eResult = R_STATE_CHANGED;
			}
			xm_csWriterGuard.Release();
		}
	}
	::TlsSetValue(xm_ulTlsIndex.Get(), (void *)(uReaderRecur + 1));
jDone:
	return eResult;
}
ReaderWriterLock::Result ReaderWriterLock::WaitAsReader() noexcept {
	Result eResult = R_RECURSIVE;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue(xm_ulTlsIndex.Get());
	if(uReaderRecur == 0){
		if(xm_csWriterGuard.IsLockedByCurrentThread()){
			__atomic_add_fetch(&xm_ulReaderCount, 1, __ATOMIC_ACQ_REL);
		} else {
			xm_csWriterGuard.Acquire();
			if(__atomic_add_fetch(&xm_ulReaderCount, 1, __ATOMIC_ACQ_REL) == 1){
				xm_semExclusive.Wait();
				eResult = R_STATE_CHANGED;
			}
			xm_csWriterGuard.Release();
		}
	}
	::TlsSetValue(xm_ulTlsIndex.Get(), (void *)(uReaderRecur + 1));
	return eResult;
}
ReaderWriterLock::Result ReaderWriterLock::ReleaseAsReader() noexcept {
	Result eResult = R_RECURSIVE;
	auto uReaderRecur = (std::uintptr_t)::TlsGetValue(xm_ulTlsIndex.Get());
	if(uReaderRecur == 1){
		if(xm_csWriterGuard.IsLockedByCurrentThread()){
			__atomic_sub_fetch(&xm_ulReaderCount, 1, __ATOMIC_ACQ_REL);
		} else {
			if(__atomic_sub_fetch(&xm_ulReaderCount, 1, __ATOMIC_ACQ_REL) == 0){
				xm_semExclusive.Post();
				eResult = R_STATE_CHANGED;
			}
		}
	}
	::TlsSetValue(xm_ulTlsIndex.Get(), (void *)(uReaderRecur - 1));
	return eResult;
}

ReaderWriterLock::Result ReaderWriterLock::TryAsWriter() noexcept {
	// 假定有两个线程运行同样的函数：
	//
	//   GetReaderLock();
	//   ::Sleep(1000);
	//   GetWriterLock(); // 死锁，因为没有任何一个线程可以获得写锁。
	//
	// 这个问题并非无法解决，例如允许 GetWriterLock() 抛出异常。
	// 但是这样除了使问题复杂化以外没有什么好处。
	ASSERT_MSG((std::uintptr_t)::TlsGetValue(xm_ulTlsIndex.Get()) == 0, L"获取写锁前必须先释放读锁。");

	const auto eResult = xm_csWriterGuard.Try();
	if(eResult != R_STATE_CHANGED){
		return eResult;
	}

	if(!xm_semExclusive.Wait(0)){
		xm_csWriterGuard.Release();
		return R_TRY_FAILED;
	}
	return R_STATE_CHANGED;
}
ReaderWriterLock::Result ReaderWriterLock::WaitAsWriter() noexcept {
	ASSERT_MSG((std::uintptr_t)::TlsGetValue(xm_ulTlsIndex.Get()) == 0, L"获取写锁前必须先释放读锁。");

	const auto eResult = xm_csWriterGuard.Acquire();
	if(eResult != R_STATE_CHANGED){
		return eResult;
	}

	xm_semExclusive.Wait();
	return R_STATE_CHANGED;
}
ReaderWriterLock::Result ReaderWriterLock::ReleaseAsWriter() noexcept {
	if(xm_csWriterGuard.Release() != R_STATE_CHANGED){
		return R_RECURSIVE;
	}

	const auto uRecurReading = (std::uintptr_t)::TlsGetValue(xm_ulTlsIndex.Get());
	if(uRecurReading == 0){
		xm_semExclusive.Post();
	}
	return R_STATE_CHANGED;
}

ReaderWriterLock::ReaderLock ReaderWriterLock::TryReaderLock() noexcept {
	ReaderLock vLock(*this, false);
	vLock.Try();
	return std::move(vLock);
}
ReaderWriterLock::ReaderLock ReaderWriterLock::GetReaderLock() noexcept {
	return ReaderLock(*this);
}

ReaderWriterLock::WriterLock ReaderWriterLock::TryWriterLock() noexcept {
	WriterLock vLock(*this, false);
	vLock.Try();
	return std::move(vLock);
}
ReaderWriterLock::WriterLock ReaderWriterLock::GetWriterLock() noexcept {
	return WriterLock(*this);
}
