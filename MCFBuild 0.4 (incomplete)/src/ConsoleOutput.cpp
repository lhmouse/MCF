// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "ConsoleOutput.hpp"
#include "Localization.hpp"
#include "../MCF/Containers/VVector.hpp"
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
			if(!::WriteConsoleW(
				hFile,
				wsoString.GetBegin() + uTotalWritten, uSize - uTotalWritten,
				&dwWrittenThisTime, nullptr
			)){
				break;
			}
			uTotalWritten += dwWrittenThisTime;
		}
	} else {
		MCF::VVector<char> vecConverted;
		const auto uCodePage = ::GetConsoleOutputCP();
		vecConverted.ResizeMore(wsoString.GetSize() * 2);
		const auto nWritten = MCF::Max(
			::WideCharToMultiByte(
				uCodePage, 0, wsoString.GetBegin(), (int)wsoString.GetSize(),
				vecConverted.GetData(), (int)vecConverted.GetSize(),
				nullptr, nullptr
			), 0
		);
		vecConverted.Resize((std::size_t)nWritten);

		const auto vLock = g_pLock->GetLock();
		std::size_t uTotalWritten = 0;
		const auto uSize = vecConverted.GetSize();
		while(uTotalWritten < uSize){
			DWORD dwWrittenThisTime;
			if(!::WriteFile(
				hFile,
				vecConverted.GetData() + uTotalWritten, uSize - uTotalWritten,
				&dwWrittenThisTime, nullptr
			)){
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
