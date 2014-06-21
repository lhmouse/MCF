// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Shell.hpp"
#include "../MCF/Core/StreamBuffer.hpp"
#include "../MCF/Core/UniqueHandle.hpp"
#include "../MCF/Core/Exception.hpp"
#include <cstddef>
using namespace MCFBuild;

namespace {

struct WinHandleCloser {
	constexpr HANDLE operator()() const noexcept {
		return NULL;
	}
	void operator()(HANDLE hObject) const noexcept {
		::CloseHandle(hObject);
	}
};

typedef MCF::UniqueHandle<WinHandleCloser> UniqueWinHandle;

std::pair<UniqueWinHandle, UniqueWinHandle> CreateReadablePipe(){
	HANDLE hRawRead, hRawWrite;
	if(!::CreatePipe(&hRawRead, &hRawWrite, nullptr, 0)){
		FORMAT_THROW(::GetLastError(), L"CREATE_PIPE_FAILED"_wso);
	}
	UniqueWinHandle hRead(hRawRead), hWrite(hRawWrite);
	if(!::SetHandleInformation(hWrite.Get(), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)){
		FORMAT_THROW(::GetLastError(), L"SET_PIPE_INFO_FAILED"_wso);
	}
	return std::make_pair(std::move(hRead), std::move(hWrite));
}

void ReadPipe(MCF::StreamBuffer &sbufSink, HANDLE hSource){
	DWORD dwBytesAvail;
	if(!::PeekNamedPipe(hSource, nullptr, 0, nullptr, &dwBytesAvail, nullptr)){
		return;
	}
	DWORD dwBytesRead = 0;
	for(;;){
		if(dwBytesRead >= dwBytesAvail){
			break;
		}
		unsigned char abyBuffer[0x100];
		DWORD dwBytesReadThisTime;
		if(!::ReadFile(hSource, abyBuffer, sizeof(abyBuffer), &dwBytesReadThisTime, nullptr)){
			break;
		}
		sbufSink.Insert(abyBuffer, dwBytesReadThisTime);
		dwBytesRead += dwBytesReadThisTime;
	}
}

void ConvertCrlfAppend(MCF::Vector<char> &vecAppendTo, MCF::StreamBuffer &sbufSource){
	vecAppendTo.ReserveMore(sbufSource.GetSize());
	int nNext = sbufSource.Get();
	while(nNext != -1){
		const char ch = (char)(unsigned char)nNext;
		nNext = sbufSource.Get();
		if(ch == '\r'){
			if(nNext == '\n'){
				nNext = sbufSource.Get();
			}
			vecAppendTo.Push('\n');
		} else {
			vecAppendTo.Push(ch);
		}
	}
}

}

namespace MCFBuild {

unsigned int Shell(
	MCF::Vector<char> &restrict vecStdOut,
	MCF::Vector<char> &restrict vecStdErr,
	const MCF::WideStringObserver &wsoCommandLine
){
	MCF::WideString wcsCommandLine;
	wcsCommandLine.Reserve(Min((std::size_t)MAX_PATH, wsoCommandLine.GetSize()));
	wcsCommandLine = wsoCommandLine;
	if(wcsCommandLine.GetSize() < MAX_PATH){
		wcsCommandLine.Resize(MAX_PATH);
	}

	auto vStdOut = CreateReadablePipe();
	auto vStdErr = CreateReadablePipe();

	STARTUPINFOW vStartupInfo;
	vStartupInfo.cb				= sizeof(vStartupInfo);
	vStartupInfo.lpReserved		= nullptr;
	vStartupInfo.lpDesktop		= nullptr;
	vStartupInfo.lpTitle		= nullptr;
	vStartupInfo.dwFlags		= STARTF_USESTDHANDLES | DETACHED_PROCESS;
	vStartupInfo.cbReserved2	= 0;
	vStartupInfo.lpReserved2	= nullptr;
	vStartupInfo.hStdInput		= INVALID_HANDLE_VALUE;
	vStartupInfo.hStdOutput		= vStdOut.second.Get();
	vStartupInfo.hStdError		= vStdErr.second.Get();

	PROCESS_INFORMATION vProcessInfo;
	if(!::CreateProcessW(nullptr, wcsCommandLine.GetStr(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &vStartupInfo, &vProcessInfo)){
		FORMAT_THROW(::GetLastError(), L"CREATE_PROCESS_FAILED|"_wso + wcsCommandLine);
	}
	::CloseHandle(vProcessInfo.hThread);
	const UniqueWinHandle hProcess(vProcessInfo.hProcess);

	MCF::StreamBuffer sbufStdOut, sbufStdErr;
	DWORD dwExitCode;

	bool bBreakNow = false;
	do {
		if(::WaitForSingleObject(hProcess.Get(), 100) != WAIT_TIMEOUT){
			::GetExitCodeProcess(hProcess.Get(), &dwExitCode);
			bBreakNow = true;
		}
		ReadPipe(sbufStdOut, vStdOut.first.Get());
		ReadPipe(sbufStdErr, vStdErr.first.Get());
	} while(!bBreakNow);

	ConvertCrlfAppend(vecStdOut, sbufStdOut);
	ConvertCrlfAppend(vecStdErr, sbufStdErr);

	return dwExitCode;
}

}
