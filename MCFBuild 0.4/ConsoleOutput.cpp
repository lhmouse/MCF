// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "ConsoleOutput.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Thread/CriticalSection.hpp"
#include <cstddef>
using namespace MCFBuild;

namespace {

static const auto pLock = MCF::CriticalSection::Create(0);

void DoPrint(HANDLE hFile, const MCF::WideStringObserver &wsoString) noexcept {
	DWORD dwMode;
	if(::GetConsoleMode(hFile, &dwMode)){
		const auto vLock = pLock->GetLock();

		const auto pwcData = wsoString.GetBegin();
		const auto uSize = wsoString.GetSize();
		std::size_t uTotalWritten = 0;

		DWORD dwCharsWritten;
		while(uTotalWritten < uSize){
			if(!::WriteConsoleW(hFile, pwcData + uTotalWritten, uSize - uTotalWritten, &dwCharsWritten, nullptr)){
				break;
			}
			uTotalWritten += dwCharsWritten;
		}
		::WriteConsoleW(hFile, L"\n", 1, &dwCharsWritten, nullptr);
	} else {
		MCF::Utf8String u8sTemp;
		u8sTemp.Assign<MCF::StringEncoding::UTF16>(wsoString);
		u8sTemp.Append('\n');

		const auto vLock = pLock->GetLock();

		const auto pchData = u8sTemp.GetBegin();
		const auto uSize = u8sTemp.GetSize();
		std::size_t uTotalWritten = 0;

		DWORD dwCharsWritten;
		while(uTotalWritten < uSize){
			if(!::WriteFile(hFile, pchData + uTotalWritten, uSize - uTotalWritten, &dwCharsWritten, nullptr)){
				break;
			}
			uTotalWritten += dwCharsWritten;
		}
	}
}

}

namespace MCFBuild {

void PrintLn(const MCF::WideStringObserver &wsoString) noexcept {
	DoPrint(::GetStdHandle(STD_OUTPUT_HANDLE), wsoString);
}
void PrintLnErr(const MCF::WideStringObserver &wsoString) noexcept {
	DoPrint(::GetStdHandle(STD_ERROR_HANDLE), wsoString);
}

}
