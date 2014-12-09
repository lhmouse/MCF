#include <MCF/StdMCF.hpp>
#include <MCF/Core/File.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	File f(L"E:\\123.txt", File::TO_WRITE);
	f.Write(0, "meow\n", 5);
	return 0;
}
