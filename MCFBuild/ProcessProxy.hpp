// Copyleft 2013, LH_Mouse. All wrongs reserved.

#ifndef __PROCESS_PROXY_HPP__
#define __PROCESS_PROXY_HPP__

#include <memory>
#include "RAIIWrapperTemplate.hpp"

namespace MCFBuild {
	class ProcessProxy {
	public:
		struct ExitInfo {
			int nExitCode;
			std::string strStdOut;
			std::string strStdErr;
		};
	private:
		struct xHandleCloser {
			constexpr static HANDLE Null(){ return NULL; }
			void operator()(HANDLE hObj) const { ::CloseHandle(hObj); }
		};
		typedef MCF::RAIIWrapper<HANDLE, xHandleCloser> xUniqueHandle;

		struct xContext {
			std::string strStdOut;
			std::string strStdErr;

			xUniqueHandle hStdOutRead;
			xUniqueHandle hStdErrRead;

			xUniqueHandle hStdOutDaemonThread;
			xUniqueHandle hStdErrDaemonThread;

			xUniqueHandle hProcess;
			DWORD dwProcessGroupID;
		};
	private:
		static DWORD WINAPI xStdOutDaemonProc(LPVOID pParam);
		static DWORD WINAPI xStdErrDaemonProc(LPVOID pParam);

		static std::pair<xUniqueHandle, xUniqueHandle> xCreateInputPipe();
	private:
		std::unique_ptr<xContext> xm_pContext;
	public:
		ProcessProxy();
		ProcessProxy(const std::wstring &wcsCmdLine);
		~ProcessProxy();
	public:
		void Fork(const std::wstring &wcsCmdLine);
		std::unique_ptr<ExitInfo> Join();

		void Break() const;
		void Kill() const;
	};
};

#endif
