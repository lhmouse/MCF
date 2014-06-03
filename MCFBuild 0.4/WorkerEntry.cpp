// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Localization.hpp"
#include "ConsoleOutput.hpp"
#include "../MCF/Core/Utilities.hpp"
#include <cwchar>
using namespace MCFBuild;

namespace MCFBuild {

unsigned int WorkerEntry(std::size_t uArgC, const wchar_t *const *ppwszArgV){
	std::wprintf(L"argc %zu\n", uArgC);
	for(auto i = 0u; i < uArgC; ++i){
		std::wprintf(L"argv %zu = %ls\n", i, ppwszArgV[i]);
	}
	return 0;
}

}
