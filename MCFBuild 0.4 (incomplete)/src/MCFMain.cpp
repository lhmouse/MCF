// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2015, LH_Mouse. All wrongs reserved.

#include "Precompiled.hpp"
#include "System.hpp"
using namespace MCFBuild;

extern "C" unsigned MCFMain() noexcept
try {
	MCF::WideString out, err;
	auto ret = System::Shell(out, err, L"cop 喵"_wso);

	std::printf("ret = %u\n", ret);

	std::printf("out = ");
	auto p = out.GetStr();
	while(*p != 0){
		std::printf("%04X ", (unsigned)*p);
		++p;
	}
	std::printf("\n");

	std::printf("err = ");
	p = err.GetStr();
	while(*p != 0){
		std::printf("%04X ", (unsigned)*p);
		++p;
	}
	std::printf("\n");

	return EXIT_SUCCESS;
} catch(...){
	System::Print(L""_wso);

	return EXIT_FAILURE;
}
