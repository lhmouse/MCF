#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <MCF/Hash/Crc32.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	static constexpr unsigned char data[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	Crc32 hasher;
	std::string str;

	for(unsigned i = 0; i < 400; ++i){
		StreamBuffer buf2;
		for(unsigned k = 0; k < 100; ++k){
			for(unsigned j = 0; j < sizeof(data) - 1; ++j){
				buf2.Put(data[j]);
				// buf2.Unget(data[j]);
			}
			buf2.Put((const char *)data);
		}

		str.resize(buf2.GetSize());
		buf2.Peek(&str[0], str.size());
		hasher.Update(str.data(), str.size());
		unsigned long crc = hasher.Finalize();
		std::printf("crc   = %08lX\n", crc);

		auto buf1 = buf2.CutOff(i);
		buf1.Splice(buf2);

		str.resize(buf1.GetSize());
		buf1.Get(&str[0], str.size());
		hasher.Update(str.data(), str.size());
		crc = hasher.Finalize();
		std::printf("  crc = %08lX\n", crc);
	}
	return 0;
}
