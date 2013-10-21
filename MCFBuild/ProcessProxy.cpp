// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "ProcessProxy.hpp"
#include <vector>
#include <algorithm>
using namespace MCFBuild;

DWORD WINAPI ProcessProxy::xStdOutDaemonProc(LPVOID pParam){
	xContext *const pContext = (xContext *)pParam;
	for(;;){
		char achBuffer[0x400];
		DWORD dwBytesRead;
		if(::ReadFile(pContext->hStdOutRead, achBuffer, sizeof(achBuffer), &dwBytesRead, nullptr) == FALSE){
			break;
		}
		pContext->strStdOut.append(achBuffer, dwBytesRead);
	}
	return 0;
}
DWORD WINAPI ProcessProxy::xStdErrDaemonProc(LPVOID pParam){
	xContext *const pContext = (xContext *)pParam;
	for(;;){
		char achBuffer[0x400];
		DWORD dwBytesRead;
		if(::ReadFile(pContext->hStdErrRead, achBuffer, sizeof(achBuffer), &dwBytesRead, nullptr) == FALSE){
			break;
		}
		pContext->strStdErr.append(achBuffer, dwBytesRead);
	}
	return 0;
}

std::pair<ProcessProxy::xUniqueHandle, ProcessProxy::xUniqueHandle> ProcessProxy::xCreateInputPipe(){
	HANDLE hRead, hWrite;
	if(::CreatePipe(&hRead, &hWrite, nullptr, 0) == FALSE){
		const DWORD dwError = ::GetLastError();
		throw Exception(dwError, L"创建无名管道失败。");
	}
	::SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	return std::make_pair(xUniqueHandle(hRead), xUniqueHandle(hWrite));
}

ProcessProxy::ProcessProxy(){
}
ProcessProxy::ProcessProxy(const std::wstring &wcsCmdLine){
	Fork(wcsCmdLine);
}
ProcessProxy::~ProcessProxy(){
	Break();
	Join();
}

void ProcessProxy::Fork(const std::wstring &wcsCmdLine){
	Kill();

	std::vector<wchar_t> vecCmdLine(wcsCmdLine.begin(), wcsCmdLine.end());
	vecCmdLine.push_back(0);

	std::unique_ptr<xContext> pNewContext(new xContext);

	auto StdOutPipe = xCreateInputPipe();
	pNewContext->hStdOutRead = std::move(StdOutPipe.first);
	auto StdErrPipe = xCreateInputPipe();
	pNewContext->hStdErrRead = std::move(StdErrPipe.first);

	pNewContext->hStdOutDaemonThread.Reset(::CreateThread(nullptr, 0, &xStdOutDaemonProc, pNewContext.get(), 0, nullptr));
	pNewContext->hStdErrDaemonThread.Reset( ::CreateThread(nullptr, 0, &xStdErrDaemonProc, pNewContext.get(), 0, nullptr));

	STARTUPINFOW StartupInfo;
	StartupInfo.cb			= sizeof(StartupInfo);
	StartupInfo.lpReserved	= nullptr;
	StartupInfo.lpDesktop	= nullptr;
	StartupInfo.lpTitle		= nullptr;
	StartupInfo.dwFlags		= STARTF_USESTDHANDLES;
	StartupInfo.cbReserved2	= 0;
	StartupInfo.lpReserved2	= nullptr;
	StartupInfo.hStdInput	= ::GetStdHandle(STD_INPUT_HANDLE);
	StartupInfo.hStdOutput	= StdOutPipe.second;
	StartupInfo.hStdError	= StdErrPipe.second;

	PROCESS_INFORMATION ProcessInfo;
	if(::CreateProcessW(nullptr, vecCmdLine.data(), nullptr, nullptr, TRUE, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &StartupInfo, &ProcessInfo) == FALSE){
		const DWORD dwError = ::GetLastError();
		throw Exception(dwError, L"使用命令行“" + wcsCmdLine + L"”创建进程失败。");
	}
	::CloseHandle(ProcessInfo.hThread);

	pNewContext->hProcess.Reset(ProcessInfo.hProcess);
	pNewContext->dwProcessGroupID = ProcessInfo.dwProcessId;

	xm_pContext = std::move(pNewContext);
}
std::unique_ptr<ProcessProxy::ExitInfo> ProcessProxy::Join(){
	std::unique_ptr<ExitInfo> pRet;
	if(xm_pContext){
		HANDLE arhToWait[3] = {
			xm_pContext->hProcess,
			xm_pContext->hStdOutDaemonThread,
			xm_pContext->hStdErrDaemonThread
		};
		::WaitForMultipleObjects(COUNTOF(arhToWait), arhToWait, TRUE, INFINITE);

		DWORD dwExitCode = 0;
		::GetExitCodeProcess(xm_pContext->hProcess, &dwExitCode);

		const auto ConvertCRLFs = [](std::string &u8s) -> std::string & {
			auto iterWrite = u8s.begin();
			auto iterRead = u8s.cbegin();
			char chNext = *iterRead;
			while(iterRead != u8s.cend()){
				const char chCur = chNext;
				chNext = *++iterRead;

				if(chCur == '\r'){
					if(chNext == '\n'){
						continue;
					}
					*(iterWrite++) = '\n';
				} else {
					*(iterWrite++) = chCur;
				}
			}
			u8s.erase(iterWrite, u8s.end());
			return u8s;
		};

		pRet.reset(new ExitInfo{
			(int)dwExitCode,
			std::move(ConvertCRLFs(xm_pContext->strStdOut)),
			std::move(ConvertCRLFs(xm_pContext->strStdErr))
		});

		xm_pContext.reset(nullptr);
	}
	return std::move(pRet);
}

void ProcessProxy::Break() const {
	if(xm_pContext){
		::GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, xm_pContext->dwProcessGroupID);
	}
}
void ProcessProxy::Kill() const {
	if(xm_pContext){
		::TerminateProcess(xm_pContext->hProcess, ERROR_PROCESS_ABORTED);
	}
}
