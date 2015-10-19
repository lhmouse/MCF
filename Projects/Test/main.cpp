#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/LastError.hpp>
#include <MCF/File/File.hpp>

using namespace MCF;

extern "C" unsigned MCFMain()
try {
	File f(L"\\??\\PhysicalDrive0"_wsv, File::kToRead);
	char temp[256];
	auto len = f.Read(temp, sizeof(temp), 10);
	std::printf("read = %s\n", AnsiString(Utf8String(temp, len)).GetStr());
	return 0;
} catch(Exception &e){
	std::printf("MCF::Exception: code = %lu (%s), desc = %s\n", e.GetCode(), AnsiString(GetWin32ErrorDescription(e.GetCode())).GetStr(), e.GetDescription());
	return 3;
}
