#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	static constexpr unsigned char data[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	char temp[1024];
	std::size_t len;
	int c;

	for(unsigned i = 0; i < 400; ++i){
		StreamBuffer buf2;
		for(unsigned k = 0; k < 5; ++k){
//			for(unsigned j = 0; j < sizeof(data) - 1; ++j){
//				buf2.Put(data[j]);
//				// buf2.Unget(data[j]);
//			}
			buf2.Put((const char *)data);
		}
		auto buf1 = buf2.CutOff(i);

		len = buf1.Get(temp, sizeof(temp));
		temp[len] = 0;
		std::printf("buf1 = (%04zu) %s\n", buf1.GetSize(), temp);
//		len = buf2.Get(temp, sizeof(temp));
//		temp[len] = 0;
//		std::printf("buf2 = (%04zu) %s\n", buf2.GetSize(), temp);
		std::printf("buf2 = (%04zu) ", buf2.GetSize());
		while((c = buf2.Get()) >= 0){
//		while((c = buf2.Unput()) >= 0){
			std::putchar(c);
		}
		std::putchar('\n');
	}
	return 0;
}
