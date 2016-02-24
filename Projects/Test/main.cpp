#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/FileInputStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	FileInputStream fs(File(WideString(NarrowStringView(__FILE__)), File::kToRead));

	char str[64];
	std::size_t len;
	while((len = fs.Get(str, sizeof(str))) != 0){
		std::fwrite(str, len, 1, stdout);
	}

	return 0;
}
