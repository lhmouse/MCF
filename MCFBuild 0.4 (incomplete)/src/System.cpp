// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2015, LH_Mouse. All wrongs reserved.

#include "Precompiled.hpp"
#include "System.hpp"
#include <Thread/Mutex.hpp>
#include <Core/UniqueHandle.hpp>
#include <Containers/Vector.hpp>
using namespace MCFBuild;

namespace {

MCF::Mutex g_vConsoleMutex;

struct PipeCloser {
	constexpr HANDLE operator()() const noexcept {
		return nullptr;
	}
	void operator()(HANDLE hPipe) const noexcept {
		::CloseHandle(hPipe);
	}
};
using WindowsHandle = MCF::UniqueHandle<PipeCloser>;

}

void System::Print(const MCF::WideStringObserver &wsoText, bool bInsertsNewLine, bool bToStdErr) noexcept {
	const auto hOutput = ::GetStdHandle(bToStdErr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
	DWORD dwMode;
	if(::GetConsoleMode(hOutput, &dwMode)){
		// 写控制台。
		const auto Write = [&](const wchar_t *pwcStr, std::size_t uLen){
			DWORD dwTotal = 0, dwWritten;
			do {
				if(!::WriteConsoleW(hOutput, pwcStr + dwTotal, uLen - dwTotal, &dwWritten, nullptr)){
					break;
				}
				dwTotal += dwWritten;
			} while(dwTotal < uLen);
		};

		{
			const auto vLock = g_vConsoleMutex.GetLock();
			Write(wsoText.GetBegin(), wsoText.GetSize());
			if(bInsertsNewLine){
				Write(L"\n", 1);
			}
		}
	} else {
		// 写文件。
		const MCF::Utf8String u8sTemp(wsoText);

		const auto Write = [&](const char *pchStr, std::size_t uLen){
			DWORD dwTotal = 0, dwWritten;
			do {
				if(!::WriteFile(hOutput, pchStr + dwTotal, uLen - dwTotal, &dwWritten, nullptr)){
					break;
				}
				dwTotal += dwWritten;
			} while(dwTotal < uLen);
		};

		{
			const auto vLock = g_vConsoleMutex.GetLock();
			Write(u8sTemp.GetStr(), u8sTemp.GetSize());
			if(bInsertsNewLine){
				Write("\n", 1);
			}
		}
	}
}
unsigned System::Shell(MCF::WideString &wcsStdOut, MCF::WideString &wcsStdErr, const MCF::WideStringObserver &wsoCommand){
	const auto ConvertOutput = [](MCF::WideString &wcsSink, MCF::Vector<unsigned char> &vecSource){
		wcsSink.Assign((const wchar_t *)vecSource.GetData(), vecSource.GetSize() / sizeof(wchar_t));

		const auto uCrlfPos = wcsSink.Find(L"\r\n"_wso);
		if(uCrlfPos != MCF::WideString::NPOS){
			// 将 CRLF 转换为 LF。
			auto pwcWrite = wcsSink.GetBegin() + uCrlfPos + 1;
			pwcWrite[-1] = L'\n';

			const auto pwcEnd = wcsSink.GetEnd();
			for(const wchar_t *pwcRead = pwcWrite + 1; pwcRead != pwcEnd; ++pwcRead){
				if((pwcRead[0] == L'\r') && (pwcRead[1] == '\n')){
					continue;
				}
				*pwcWrite = *pwcRead;
				++pwcWrite;
			}
			wcsSink.Truncate((std::size_t)(pwcEnd - pwcWrite));
		}
	};

	DWORD dwExitCode;
	MCF::Vector<unsigned char> vecStdOut, vecStdErr;

	{
		MCF::WideString wcsFullCommand;
		wcsFullCommand.Reserve(MAX_PATH);
		wcsFullCommand.Resize(wcsFullCommand.GetCapacity());
		auto dwComSpecLen = ::GetEnvironmentVariableW(L"COMSPEC", wcsFullCommand.GetStr(), wcsFullCommand.GetSize());
		if(dwComSpecLen > wcsFullCommand.GetSize()){
			wcsFullCommand.Resize(dwComSpecLen);
			dwComSpecLen = ::GetEnvironmentVariableW(L"COMSPEC", wcsFullCommand.GetStr(), wcsFullCommand.GetSize());
		}
		if(dwComSpecLen != 0){
			wcsFullCommand.Resize(dwComSpecLen);
		} else {
			wcsFullCommand = L"CMD.EXE"_wso;
		}
		wcsFullCommand += L" /U /Q /C"_wso;
		wcsFullCommand += wsoCommand;

		const auto CreateInputPipe = []{
			HANDLE hRead, hWrite;
			if(!::CreatePipe(&hRead, &hWrite, nullptr, 0)){
				DEBUG_THROW(MCF::SystemError, "CreatePipe");
			}
			auto vRet = std::make_pair(WindowsHandle(hRead), WindowsHandle(hWrite));
			if(!::SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)){
				DEBUG_THROW(MCF::SystemError, "SetHandleInformation");
			}
			return vRet;
		};
		const auto ReadPipe = [](MCF::Vector<unsigned char> &vecSink, HANDLE hPipe){
			unsigned char abyTemp[0x400];
			DWORD dwBytesAvail;
			DWORD dwBytesRead;
			for(;;){
				if(!::PeekNamedPipe(hPipe, nullptr, 0, nullptr, &dwBytesAvail, nullptr)){
					break;
				}
				if(dwBytesAvail == 0){
					break;
				}
				if(::ReadFile(hPipe, abyTemp, sizeof(abyTemp), &dwBytesRead, nullptr) == FALSE){
					break;
				}
				vecSink.AppendCopy(abyTemp, dwBytesRead);
			}
		};

		const auto vStdOutPipe = CreateInputPipe();
		const auto vStdErrPipe = CreateInputPipe();

		::STARTUPINFOW vStartupInfo;
		vStartupInfo.cb				= sizeof(vStartupInfo);
		vStartupInfo.lpReserved		= nullptr;
		vStartupInfo.lpDesktop		= nullptr;
		vStartupInfo.lpTitle		= nullptr;
		vStartupInfo.dwFlags		= STARTF_USESTDHANDLES | DETACHED_PROCESS;
		vStartupInfo.cbReserved2	= 0;
		vStartupInfo.lpReserved2	= nullptr;
		vStartupInfo.hStdInput		= INVALID_HANDLE_VALUE;
		vStartupInfo.hStdOutput		= vStdOutPipe.second.Get();
		vStartupInfo.hStdError		= vStdErrPipe.second.Get();

		::PROCESS_INFORMATION vProcessInfo;
		if(!::CreateProcessW(nullptr, wcsFullCommand.GetStr(), nullptr, nullptr, true, 0, nullptr, nullptr, &vStartupInfo, &vProcessInfo)){
			DEBUG_THROW(MCF::SystemError, "CreateProcessW");
		}
		::CloseHandle(vProcessInfo.hThread);
		const WindowsHandle hProcess(vProcessInfo.hProcess);

		bool bQuitNow = false;
		do {
			if(::WaitForSingleObject(hProcess.Get(), 15) != WAIT_TIMEOUT){
				::GetExitCodeProcess(hProcess.Get(), &dwExitCode);
				bQuitNow = true;
			}
			ReadPipe(vecStdOut, vStdOutPipe.first.Get());
			ReadPipe(vecStdErr, vStdErrPipe.first.Get());
		} while(!bQuitNow);
	}

	ConvertOutput(wcsStdOut, vecStdOut);
	ConvertOutput(wcsStdErr, vecStdErr);

	return 0;
}
MCF::WideString System::NormalizePath(const MCF::WideString &wcsPath){
	MCF::WideString wcsRet;
	wcsRet.Reserve(MAX_PATH);
	wcsRet.Resize(wcsRet.GetCapacity());

	auto dwSize = ::GetFullPathNameW(wcsPath.GetStr(), wcsRet.GetSize(), wcsRet.GetStr(), nullptr);
	if(dwSize > wcsRet.GetSize()){
		// 缓冲区太小。
		wcsRet.Resize(dwSize);
		dwSize = ::GetFullPathNameW(wcsPath.GetStr(), wcsRet.GetSize(), wcsRet.GetStr(), nullptr);
	}
	wcsRet.Resize(dwSize);

	return wcsRet;
}
