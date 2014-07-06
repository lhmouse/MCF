#include <MCF/StdMCF.hpp>
#include <MCF/File/Utf8TextFile.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	const auto pTestWrite = Utf8TextFile::Open(
		L"E:\\Desktop\\test2.txt"_wso,
		Utf8TextFile::TO_WRITE | Utf8TextFile::BOM_USE
	);
	pTestWrite->WriteLine(L"喵喵"_wso);
	pTestWrite->WriteLine(L"喵喵"_wso);

	const auto pUtf8TextFile = Utf8TextFile::Open(
		L"E:\\Desktop\\test.txt"_wso,
		Utf8TextFile::TO_READ | Utf8TextFile::BOM_DETECT
	);

	std::printf("bom = %d\n", pUtf8TextFile->GetFlags() & Utf8TextFile::BOM_USE);
	std::fflush(stdout);

	WideString wcsLine;
	while(pUtf8TextFile->ReadLine(wcsLine)){
		DWORD dwDummy;
		::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), wcsLine.GetStr(), wcsLine.GetSize(), &dwDummy, nullptr);
	}

	return 0;
}
