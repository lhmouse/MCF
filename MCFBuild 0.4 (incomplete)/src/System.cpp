// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2015, LH_Mouse. All wrongs reserved.

#include "Precompiled.hpp"
#include "System.hpp"
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Core/UniqueHandle.hpp>
#include <MCF/Core/Utf8TextFile.hpp>
using namespace MCFBuild;

namespace {

// 控制台上使用 UTF-8 编码和 MSYS 兼容。
using ConsoleNarrowString = MCF::Utf8String;

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
		const ConsoleNarrowString cnsTemp(wsoText);

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
			Write(cnsTemp.GetStr(), cnsTemp.GetSize());
			if(bInsertsNewLine){
				Write("\n", 1);
			}
		}
	}
}
unsigned System::Shell(MCF::WideString &wcsStdOut, MCF::WideString &wcsStdErr, const MCF::WideStringObserver &wsoCommand){
	DWORD dwExitCode;
	ConsoleNarrowString cnsStdOut, cnsStdErr;

	{
		// CreateProcess() 要求路径是可写的。
		MCF::WideString wcsCommandLine(wsoCommand);

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
		if(!::CreateProcessW(nullptr, wcsCommandLine.GetStr(), nullptr, nullptr, true, 0, nullptr, nullptr, &vStartupInfo, &vProcessInfo)){
			DEBUG_THROW(MCF::SystemError, "CreateProcessW");
		}
		const WindowsHandle hPrimaryThread(vProcessInfo.hThread);
		const WindowsHandle hProcess(vProcessInfo.hProcess);

		bool bQuitNow = false;
		do {
			if(::WaitForSingleObject(hProcess.Get(), 15) != WAIT_TIMEOUT){
				::GetExitCodeProcess(hProcess.Get(), &dwExitCode);
				bQuitNow = true;
			}

			const auto ReadPipe = [](ConsoleNarrowString &cnsSink, HANDLE hPipe){
				char achTemp[0x400];
				DWORD dwBytesAvail;
				DWORD dwBytesRead;
				for(;;){
					if(!::PeekNamedPipe(hPipe, nullptr, 0, nullptr, &dwBytesAvail, nullptr)){
						break;
					}
					if(dwBytesAvail == 0){
						break;
					}
					if(!::ReadFile(hPipe, achTemp, sizeof(achTemp), &dwBytesRead, nullptr)){
						break;
					}
					cnsSink.Append(achTemp, dwBytesRead);
				}
			};
			ReadPipe(cnsStdOut, vStdOutPipe.first.Get());
			ReadPipe(cnsStdErr, vStdErrPipe.first.Get());
		} while(!bQuitNow);
	}

	const auto ZapCrlfs = [](ConsoleNarrowString &cnsInout){
		const auto uCrlfPos = cnsInout.Find(ConsoleNarrowString::ObserverType("\r\n", 2));
		if(uCrlfPos != ConsoleNarrowString::NPOS){
			auto pchWrite = cnsInout.GetBegin() + uCrlfPos + 1;
			pchWrite[-1] = '\n';

			const auto pchEnd = cnsInout.GetEnd();
			for(const char *pchRead = pchWrite + 1; pchRead != pchEnd; ++pchRead){
				if((pchRead[0] == '\r') && (pchRead[1] == '\n')){
					continue;
				}
				*pchWrite = *pchRead;
				++pchWrite;
			}
			cnsInout.Truncate((std::size_t)(pchEnd - pchWrite));
		}
	};
	ZapCrlfs(cnsStdOut);
	ZapCrlfs(cnsStdErr);

	wcsStdOut = MCF::WideString(cnsStdOut);
	wcsStdErr = MCF::WideString(cnsStdErr);
	return dwExitCode;
}
MCF::WideString System::NormalizePath(const wchar_t *pwcPath){
	MCF::WideString wcsRet;
	wcsRet.Reserve(MAX_PATH);
	wcsRet.Resize(wcsRet.GetCapacity());

	auto dwSize = ::GetFullPathNameW(pwcPath, wcsRet.GetSize(), wcsRet.GetStr(), nullptr);
	if(dwSize > wcsRet.GetSize()){
		// 缓冲区太小。
		wcsRet.Resize(dwSize);
		dwSize = ::GetFullPathNameW(pwcPath, wcsRet.GetSize(), wcsRet.GetStr(), nullptr);
	}
	wcsRet.Resize(dwSize);

	return wcsRet;
}

MCF::Vector<MCF::WideString> System::GetUtf8FileContents(const wchar_t *pwcPath){
	MCF::Vector<MCF::WideString> vecRet;
	MCF::Utf8TextFileReader vReader(MCF::File(pwcPath, MCF::File::TO_READ));
	MCF::Utf8String u8sLine;
	while(vReader.ReadLine(u8sLine)){
		vecRet.Push(MCF::WideString(u8sLine));
	}
	return vecRet;
}
void System::PutUtf8FileContents(const wchar_t *pwcPath, const MCF::Vector<MCF::WideString> &vecContents, bool bToAppend){
	std::uint32_t u32Flags = 0;
	if(bToAppend){
		u32Flags |= MCF::File::NO_TRUNC;
	}
	MCF::Utf8TextFileWriter vWriter(MCF::File(pwcPath, MCF::File::TO_WRITE | u32Flags));
	MCF::Utf8String u8sLine;
	for(auto pwcsLine = vecContents.GetBegin(); pwcsLine != vecContents.GetEnd(); ++pwcsLine){
		u8sLine.Assign(*pwcsLine);
		vWriter.WriteLine(u8sLine);
	}
}
