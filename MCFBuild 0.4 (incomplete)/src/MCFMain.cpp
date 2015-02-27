// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2015, LH_Mouse. All wrongs reserved.

#include "Precompiled.hpp"
#include "System.hpp"
#include "Localization.hpp"
#include <MCF/Core/Argv.hpp>

extern "C" unsigned MCFMain() noexcept {
	using namespace MCFBuild;

	try {
		System::Print(L"MCF 构建实用工具"_wso);
		System::Print(L""_wso);

		const MCF::Argv vArgs;
		for(unsigned i = 0; i < vArgs.GetSize(); ++i){
			System::Print(vArgs[i]);
		}

		auto v = System::GetFileList(L"C:"_ws, true);
		for(auto &s : v){
			System::Print(s);
		}

		return EXIT_SUCCESS;
	} catch(MCF::Exception &e){
		//
		return EXIT_FAILURE;
	} catch(std::exception &e){
		//
		return EXIT_FAILURE;
	} catch(...){
		//
		return EXIT_FAILURE;
	}
}
