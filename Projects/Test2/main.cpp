#include <MCF/StdMCF.hpp>
#include <MCF/Core/Utf8TextFile.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Utf8TextFileWriter vWriter(
		File::Open(L"E:\\Desktop\\test.txt"_wso, File::TO_WRITE),
		Utf8TextFileWriter::BOM_USE | Utf8TextFileWriter::LES_CRLF
	);
	vWriter.WriteLine(u8"喵喵喵"_u8so);

	return 0;
}
