#include <MCF/StdMCF.hpp>
#include <MCF/Streams/Crc32OutputStream.hpp>
#include <MCF/Streams/Md5OutputStream.hpp>
#include <MCF/Streams/Sha1OutputStream.hpp>
#include <MCF/Streams/Sha256OutputStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	const char data[] = "hello world";

	Crc32OutputStream crc32;
	crc32.Put(data, sizeof(data) - 1);
	const auto crc32_val = crc32.Finalize();
	std::printf("crc32_val = %08lx", (unsigned long)crc32_val); // 0d4a1185
	std::putchar('\n');

	Md5OutputStream md5;
	md5.Put(data, sizeof(data) - 1);
	const auto md5_val = md5.Finalize();
	std::printf("md5_val = "); // 5eb63bbbe01eeed093cb22bb8f5acdc3
	for(unsigned by : md5_val){
		std::printf("%02x", by);
	}
	std::putchar('\n');

	Sha1OutputStream sha1;
	sha1.Put(data, sizeof(data) - 1);
	const auto sha1_val = sha1.Finalize();
	std::printf("sha1_val = "); // 2aae6c35c94fcfb415dbe95f408b9ce91ee846ed
	for(unsigned by : sha1_val){
		std::printf("%02x", by);
	}
	std::putchar('\n');

	Sha256OutputStream sha256;
	sha256.Put(data, sizeof(data) - 1);
	const auto sha256_val = sha256.Finalize();
	std::printf("sha256_val = "); // b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9
	for(unsigned by : sha256_val){
		std::printf("%02x", by);
	}
	std::putchar('\n');

	return 0;
}
