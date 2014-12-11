#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Utilities.hpp>
using namespace MCF;

int count = 0;

struct foo {
	const int i;

	foo()
		: i(++count)
	{
		if(i == 5){
			throw 123;
		}
		std::printf("constructed: %d\n", i);
	}
	~foo(){
		std::printf("destructed: %d\n", i);
	}
};

extern "C" unsigned int MCFMain() noexcept {
	try {
		char buffer[sizeof(foo) * 30];
		auto p = (foo *)buffer;
		ConstructArray(p, 10);
		DestructArray(p, 10);
	} catch(int){
		std::puts("exception!");
	}
	return 0;
}
