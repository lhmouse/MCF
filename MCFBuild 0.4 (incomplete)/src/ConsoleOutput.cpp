// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "ConsoleOutput.hpp"
#include "Localization.hpp"
#include "../MCF/Thread/CriticalSection.hpp"
#include <cstddef>
using namespace MCFBuild;

namespace {

const auto g_pLock = MCF::CriticalSection::Create(0);

void DoPrint(HANDLE hFile, const MCF::WideStringObserver &wsoString) noexcept {
	DWORD dwMode;
	if(::GetConsoleMode(hFile, &dwMode)){
		const auto vLock = g_pLock->GetLock();
		std::size_t uTotalWritten = 0;
		const auto uSize = wsoString.GetSize();
		while(uTotalWritten < uSize){
			DWORD dwWrittenThisTime;
			if(!::WriteConsoleW(hFile,
				wsoString.GetBegin() + uTotalWritten, uSize - uTotalWritten,
				&dwWrittenThisTime, nullptr))
			{
				break;
			}
			uTotalWritten += dwWrittenThisTime;
		}
	} else {
		MCF::Utf8String u8sConverted;
		u8sConverted.Assign(wsoString);

		const auto vLock = g_pLock->GetLock();
		std::size_t uTotalWritten = 0;
		const auto uSize = u8sConverted.GetSize();
		while(uTotalWritten < uSize){
			DWORD dwWrittenThisTime;
			if(!::WriteFile(hFile,
				u8sConverted.GetBegin() + uTotalWritten, uSize - uTotalWritten,
				&dwWrittenThisTime, nullptr))
			{
				break;
			}
			uTotalWritten += dwWrittenThisTime;
		}
	}
}

}

namespace MCFBuild {

void Print(const MCF::WideStringObserver &wsoString) noexcept {
	DoPrint(::GetStdHandle(STD_OUTPUT_HANDLE), wsoString);
}
void PrintErr(const MCF::WideStringObserver &wsoString) noexcept {
	DoPrint(::GetStdHandle(STD_ERROR_HANDLE), wsoString);
}

void FormatPrint(const MCF::WideStringObserver &wsoString) noexcept {
	Print(FormatString(wsoString));
}
void FormatPrintErr(const MCF::WideStringObserver &wsoString) noexcept {
	PrintErr(FormatString(wsoString));
}

}
