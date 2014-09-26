// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Shell.hpp"
#include "../MCF/Core/StreamBuffer.hpp"
#include "../MCF/Core/UniqueHandle.hpp"
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
		unsigned char abyBuffer[0x400];
		DWORD dwBytesReadThisTime;
		if(!::ReadFile(hSource, abyBuffer, sizeof(abyBuffer), &dwBytesReadThisTime, nullptr)){
			break;
		}
		sbufSink.Put(abyBuffer, dwBytesReadThisTime);
		dwBytesRead += dwBytesReadThisTime;
	}
}

long ExtractWchar(MCF::StreamBuffer &sbufSource){
	wchar_t wc;
	if(sbufSource.Get(&wc, sizeof(wc))){
		return (std::make_unsigned_t<wchar_t>)wc;
	}
	return -1;
}

void ConvertCrlfAndAppend(MCF::WideString &wcsSink, MCF::StreamBuffer &sbufSource){
	wcsSink.ReserveMore((sbufSource.GetSize() + 1) / sizeof(wchar_t));

	long lNext = ExtractWchar(sbufSource);
	while(lNext != -1){
		const auto wc = (wchar_t)lNext;
		lNext = ExtractWchar(sbufSource);

		if((wc == L'\r') && (lNext == L'\n')){
			continue;
		}
		wcsSink.PushNoCheck(wc);
	}
}

}

namespace MCFBuild {

unsigned int Shell(
	MCF::WideString &restrict wcsStdOut,
	MCF::WideString &restrict wcsStdErr,
	const MCF::WideStringObserver &wsoCommandLine
){
	MCF::WideString wcsComSpec;
	const auto dwComSpecLen = ::GetEnvironmentVariableW(L"COMSPEC", nullptr, 0);
	if(dwComSpecLen > 0){
		wcsComSpec.Resize(::GetEnvironmentVariableW(L"COMSPEC", wcsComSpec.Resize(dwComSpecLen), dwComSpecLen));
	}

	static const auto COMSPEC_ARG_PREFIX = L"/Q /U /C "_wso;
	MCF::WideString wcsArguments;
	wcsArguments.Resize(MCF::Max((std::size_t)MAX_PATH, wsoCommandLine.GetSize() + COMSPEC_ARG_PREFIX.GetSize()));
	MCF::Copy(
		MCF::Copy(wcsArguments.GetStr(), COMSPEC_ARG_PREFIX.GetBegin(), COMSPEC_ARG_PREFIX.GetEnd()),
		wsoCommandLine.GetBegin(), wsoCommandLine.GetEnd()
	)[0] = 0;

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
	if(!::CreateProcessW(
		wcsComSpec.GetCStr(), wcsArguments.GetStr(),
		nullptr, nullptr, TRUE, 0, nullptr, nullptr,
		&vStartupInfo, &vProcessInfo
	)){
		FORMAT_THROW(::GetLastError(), L"EXECUTE_COMMAND_FAILED\0"_ws + wsoCommandLine);
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

	ConvertCrlfAndAppend(wcsStdOut, sbufStdOut);
	ConvertCrlfAndAppend(wcsStdErr, sbufStdErr);

	return dwExitCode;
}

}
