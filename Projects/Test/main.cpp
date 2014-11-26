#include <MCF/StdMCF.hpp>
#include <MCF/Core/Utf8TextFile.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Utf8TextFileWriter wr(File::Open(L"E:\\Desktop\\test.txt", File::TO_WRITE), Utf8TextFileWriter::LES_CRLF);
	wr.WriteLine(u8"hello world!"_u8so);
	wr.WriteLine(u8"hello world!"_u8so);
	wr.WriteLine(u8"hello world!"_u8so);
	wr.WriteLine(u8"hello world!"_u8so);
	return 0;
}
