#include <MCF/Hash/Sha1.hpp>
#include <cstdio>

using namespace MCF;

template class Array<int, 5>;

extern "C" unsigned MCFMain(){
	constexpr char data[] = "hello world!";
	Sha1 h;
	h.Update(data, sizeof(data) - 1);
	auto r = h.Finalize();
	std::printf(R"_(sha1("%s") = )_", data);
	for(auto by : r){
		std::printf("%02hhx", by);
	}
	std::putchar('\n');
	return 0;
}
