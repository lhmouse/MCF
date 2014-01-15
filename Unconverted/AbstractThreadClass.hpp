// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_ABSTRACT_THREAD_CLASS_HPP__
#define __MCF_ABSTRACT_THREAD_CLASS_HPP__

#include "StdMCF.hpp"
#include "Memory.hpp"
#include <forward_list>

namespace MCF {
	class AbstractThreadClass : NO_COPY_OR_ASSIGN {
	private:
		typedef struct xtagThreadControlBlock {
			AbstractThreadClass *volatile pOwner;
			HANDLE hThread;
			DWORD dwThreadID;
			int nExitCode;
			bool bIsSuspended;
			std::atomic<long> lRefCount;
			bool bIsParasite;
			std::forward_list<std::function<void(bool)>> lstAtExitProcs;
		} xTHREAD_CONTROL_BLOCK;
	private:
		static TLSManagerTemplate<std::forward_list<xTHREAD_CONTROL_BLOCK *>> xs_tlsInstanceList;
	private:
		static unsigned int __stdcall xThreadStartRoutine(void *pParam);
	public:
		static AbstractThreadClass *GetCurrentThread();
	private:
		xTHREAD_CONTROL_BLOCK *xm_pThreadControlBlock;
	public:
		AbstractThreadClass();
		virtual ~AbstractThreadClass();
	protected:
		virtual int xThreadProc() = 0;
	public:
		HANDLE GetHThread() const;
		DWORD GetThreadID() const;
		bool IsSuspended() const;

		void Create(bool bSuspended);
		void Release();
		void Wait() const;
		bool WaitTimeout(DWORD dwMilliSeconds) const;
		int WaitAndRelease();
		int ParasitizeCurrentThread(bool bSuspended, bool bUseRealHandle);

		void Suspend();
		void Resume();

		bool AtExit(std::function<void(bool)> &&AtThreadExitProc);
	};

	extern AbstractThreadClass *GetCurrentThread();
	extern bool AtThreadExit(std::function<void(bool)> &&AtThreadExitProc);
}

#endif
