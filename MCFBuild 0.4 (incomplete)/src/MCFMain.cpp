// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2015, LH_Mouse. All wrongs reserved.

#include "Precompiled.hpp"
#include "System.hpp"
#include "Localization.hpp"
#include <MCF/Core/Argv.hpp>
using namespace MCFBuild;

extern "C" unsigned MCFMain() noexcept
try {
	MCF::WideString wcsBanner;
	Localization::Get(wcsBanner, "MCFBUILD");
	wcsBanner += L" ["_wso;
	Localization::Get(wcsBanner, "VERSION");
	wcsBanner += L" 0.4.0.14]"_wso;
	System::Print(wcsBanner);

	const MCF::Argv vArgs;
	for(unsigned i = 0; i < vArgs.GetSize(); ++i){
		System::Print(vArgs[i]);
	}

	return EXIT_SUCCESS;
} catch(...){
	System::Print(L"MCF"_wso);

	return EXIT_FAILURE;
}
