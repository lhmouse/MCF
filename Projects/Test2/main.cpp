#include <MCF/StdMCF.hpp>
#include <MCF/Core/Utf8TextFile.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Utf8TextFileWriter vWriter(File::Open(L"E:\\Desktop\\test2.txt"_wso, File::TO_WRITE), false, true);
	vWriter.WriteLine("喵喵"_u8so);
	vWriter.WriteLine("喵喵"_u8so);

	Utf8TextFileReader vReader(File::Open(L"E:\\Desktop\\test.txt"_wso, File::TO_READ));
	Utf8String u8sLine;
	while(vReader.ReadLine(u8sLine)){
		std::printf("%s$--\n", AnsiString(u8sLine).GetCStr());
	}

	return 0;
}
