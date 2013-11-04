// Copyleft 2013, LH_Mouse. All wrongs reserved.

#ifndef __PROCESS_PROXY_HPP__
#define __PROCESS_PROXY_HPP__

#include <memory>
#include "RAIIWrapper.hpp"

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
			constexpr HANDLE Null(){
				return NULL;
			}
			void Close(HANDLE hObj){
				::CloseHandle(hObj);
			}
		};
		typedef MCF::UniqueHandle<HANDLE, xHandleCloser> xUniqueHandle;

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
		~ProcessProxy();
	public:
		void Fork(const std::wstring &wcsCmdLine);
		std::unique_ptr<ExitInfo> Join();

		void Break() const;
		void Kill() const;
	};
};

#endif
