#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Containers/Deque.hpp>
using namespace MCF;

struct foo {
	char big[1024];

	foo(){
		std::strcpy(big, "meow");
	}
	explicit foo(int i){
		throw i;
	}
};

template class Deque<foo>;

extern "C" unsigned int MCFMain() noexcept {
	Deque<foo> q;
	try {
		q.Unshift();
		q.Unshift(123);
	} catch(int e){
		std::printf("exception! e = %d\n", e);
	}
	std::printf("front = %s\n", q.GetFront().big);
	return 0;
}
