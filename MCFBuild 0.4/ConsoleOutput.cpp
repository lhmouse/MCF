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
	MCF::AnsiString ansTemp;
	ansTemp.Assign<MCF::StringEncoding::UTF16>(wsoString);
	ansTemp.Append('\n');

	const auto vLock = pLock->GetLock();
	const auto pchData = ansTemp.GetBegin();
	const auto uSize = ansTemp.GetSize();
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

namespace MCFBuild {

void PrintLn(const MCF::WideStringObserver &wsoString) noexcept {
	DoPrint(::GetStdHandle(STD_OUTPUT_HANDLE), wsoString);
}
void PrintLnErr(const MCF::WideStringObserver &wsoString) noexcept {
	DoPrint(::GetStdHandle(STD_ERROR_HANDLE), wsoString);
}

}
