#include <MCF/StdMCF.hpp>
#include <MCF/Hash/Crc32.hpp>
#include <MCF/Hash/Md5.hpp>
#include <MCF/Hash/Sha1.hpp>
#include <MCF/Hash/Sha256.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	const char data[] = "hello world!";

	unsigned char result[32];
	Sha256 hash;
	hash.Update(data, sizeof(data) - 1);
	hash.Finalize(result);
	for(auto by : result){
		std::printf("%02hhX", by);
	}
	return 0;
}
