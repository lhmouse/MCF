// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include <process.h>
#include "AbstractThreadClass.hpp"
using namespace MCF;

// 静态成员变量。
TLSManagerTemplate<std::forward_list<AbstractThreadClass::xTHREAD_CONTROL_BLOCK *>> AbstractThreadClass::xs_tlsInstanceList __attribute__((init_priority(2000)));	// 参阅 StdMCF.hpp。

// 静态成员函数。
unsigned int __stdcall AbstractThreadClass::xThreadStartRoutine(void *pParam){
	xTHREAD_CONTROL_BLOCK *const pThreadControlBlock = (xTHREAD_CONTROL_BLOCK *)pParam;

	ASSERT(pThreadControlBlock != nullptr);

	xs_tlsInstanceList->emplace_front(pThreadControlBlock);
	const int nExitCode = pThreadControlBlock->pOwner->xThreadProc();
	pThreadControlBlock->nExitCode = nExitCode;
	xs_tlsInstanceList->remove(pThreadControlBlock);

	const bool bIsParasite = pThreadControlBlock->bIsParasite;
	for(auto iter = pThreadControlBlock->lstAtExitProcs.cbegin(); iter != pThreadControlBlock->lstAtExitProcs.cend(); ++iter){
		(*iter)(bIsParasite);
	}

	if(--(pThreadControlBlock->lRefCount) == 0){
		::CloseHandle(pThreadControlBlock->hThread);
		delete pThreadControlBlock;
	}

	return (unsigned int)nExitCode;
}

AbstractThreadClass *AbstractThreadClass::GetCurrentThread(){
	if(!xs_tlsInstanceList->empty()){
		return xs_tlsInstanceList->front()->pOwner;
	}
	return nullptr;
}

// 构造函数和析构函数。
AbstractThreadClass::AbstractThreadClass() :
	xm_pThreadControlBlock(nullptr)
{
}
AbstractThreadClass::~AbstractThreadClass(){
	WaitAndRelease();
}

// 其他非静态成员函数。
HANDLE AbstractThreadClass::GetHThread() const {
	return (xm_pThreadControlBlock == nullptr) ? NULL : (HANDLE)xm_pThreadControlBlock->hThread;
}
DWORD AbstractThreadClass::GetThreadID() const {
	return (xm_pThreadControlBlock == nullptr) ? 0 : xm_pThreadControlBlock->dwThreadID;
}
bool AbstractThreadClass::IsSuspended() const {
	return (xm_pThreadControlBlock == nullptr) ? false : xm_pThreadControlBlock->bIsSuspended;
}

void AbstractThreadClass::Create(bool bSuspended){
	WaitAndRelease();

	xm_pThreadControlBlock = new xTHREAD_CONTROL_BLOCK;

	unsigned int uThreadID;
	const HANDLE hThread = (HANDLE)VERIFY(::_beginthreadex(nullptr, 0, &xThreadStartRoutine, xm_pThreadControlBlock, CREATE_SUSPENDED, &uThreadID));

	xm_pThreadControlBlock->pOwner			= this;
	xm_pThreadControlBlock->hThread			= hThread;
	xm_pThreadControlBlock->dwThreadID		= (DWORD)uThreadID;
	xm_pThreadControlBlock->nExitCode		= STILL_ACTIVE;
	xm_pThreadControlBlock->bIsSuspended	= true;
	xm_pThreadControlBlock->lRefCount		= 2;
	xm_pThreadControlBlock->bIsParasite		= false;

	if(!bSuspended){
		Resume();
	}
}
void AbstractThreadClass::Release(){
	if(xm_pThreadControlBlock != nullptr){
		xm_pThreadControlBlock->pOwner = nullptr;

		if(--(xm_pThreadControlBlock->lRefCount) == 0){
			::CloseHandle(xm_pThreadControlBlock->hThread);
			delete xm_pThreadControlBlock;
		}
		xm_pThreadControlBlock = nullptr;
	}
}
void AbstractThreadClass::Wait() const {
	WaitTimeout(INFINITE);
}
bool AbstractThreadClass::WaitTimeout(DWORD dwMilliSeconds) const {
	if(xm_pThreadControlBlock == nullptr){
		return true;
	}
	if(xm_pThreadControlBlock->dwThreadID == ::GetCurrentThreadId()){
		return true;
	}
	return ::WaitForSingleObject(xm_pThreadControlBlock->hThread, dwMilliSeconds) != WAIT_TIMEOUT;
}
int AbstractThreadClass::WaitAndRelease(){
	if(xm_pThreadControlBlock == nullptr){
		return 0;
	}

	Wait();
	const int nExitCode = xm_pThreadControlBlock->nExitCode;
	Release();

	return nExitCode;
}
int AbstractThreadClass::ParasitizeCurrentThread(bool bSuspended, bool bUseRealHandle){
	WaitAndRelease();

	xm_pThreadControlBlock = new xTHREAD_CONTROL_BLOCK;

	HANDLE hThread = ::GetCurrentThread();
	if(bUseRealHandle){
		VERIFY(::DuplicateHandle(::GetCurrentProcess(), hThread, ::GetCurrentProcess(), &hThread, THREAD_ALL_ACCESS, FALSE, 0));
	}

	xm_pThreadControlBlock->pOwner			= this;
	xm_pThreadControlBlock->hThread			= hThread;
	xm_pThreadControlBlock->dwThreadID		= ::GetCurrentThreadId();
	xm_pThreadControlBlock->nExitCode		= STILL_ACTIVE;
	xm_pThreadControlBlock->bIsSuspended	= true;
	xm_pThreadControlBlock->lRefCount		= 2;
	xm_pThreadControlBlock->bIsParasite		= true;

	if(bSuspended){
		Suspend();
	}

	xThreadStartRoutine(xm_pThreadControlBlock);

	const int nExitCode = xm_pThreadControlBlock->nExitCode;
	Release();

	return nExitCode;
}

void AbstractThreadClass::Suspend(){
	if(xm_pThreadControlBlock != nullptr){
		::SuspendThread(xm_pThreadControlBlock->hThread);
		xm_pThreadControlBlock->bIsSuspended = true;
	}
}
void AbstractThreadClass::Resume(){
	if(xm_pThreadControlBlock != nullptr){
		xm_pThreadControlBlock->bIsSuspended = false;
		::ResumeThread(xm_pThreadControlBlock->hThread);
	}
}

bool AbstractThreadClass::AtExit(std::function<void(bool)> &&AtThreadExitProc){
	if(xm_pThreadControlBlock == nullptr){
		return false;
	}
	xm_pThreadControlBlock->lstAtExitProcs.emplace_front(std::move(AtThreadExitProc));
	return true;
}

// 外部函数定义。
namespace MCF {
	extern AbstractThreadClass *GetCurrentThread(){
		return AbstractThreadClass::GetCurrentThread();
	}
	extern bool AtThreadExit(std::function<void(bool)> &&AtThreadExitProc){
		const auto pCurrentThread = AbstractThreadClass::GetCurrentThread();
		if(pCurrentThread == nullptr){
			return false;
		}
		return pCurrentThread->AtExit(std::move(AtThreadExitProc));
	}
}
