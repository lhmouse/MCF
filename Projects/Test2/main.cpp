#include <MCF/StdMCF.hpp>
#include <MCF/Serialization/Serdes.hpp>
#include <random>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;
	std::deque<bool> q1, q2;

	std::mt19937 mt;
	std::generate_n(std::back_inserter(q1), 161, [&]{ return mt() % 2; });
	for(auto b : q1){
		std::putchar(b + '0');
	}
	std::putchar('\n');

	Serialize(buf, q1);
	std::printf("serialized size = %zu\n", buf.GetSize());

	Deserialize(q2, buf);
	for(auto b : q2){
		std::putchar(b + '0');
	}
	std::putchar('\n');

	return 0;
}
