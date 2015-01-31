// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2015, LH_Mouse. All wrongs reserved.

#include "Precompiled.hpp"
#include <MCF/Core/Argv.hpp>
#include "System.hpp"
using namespace MCFBuild;

extern "C" unsigned MCFMain() noexcept
try {
	const MCF::Argv vArgs;
	for(unsigned i = 0; i < vArgs.GetSize(); ++i){
		System::Print(vArgs[i]);
	}

	return EXIT_SUCCESS;
} catch(...){
	System::Print(L""_wso);

	return EXIT_FAILURE;
}
