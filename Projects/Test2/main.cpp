#include <MCF/StdMCF.hpp>
#include <MCF/Core/VVector.hpp>
using namespace MCF;

static int cnt = 0;

struct S {
	int my;

	S(int i) : my(i) {
		std::printf("constructed %d\n", my);
		++cnt;
	}
	S(const S &r) : my(r.my) {
		if(my == 12){
			throw 123;
		}
		std::printf("copied      %d\n", my);
		++cnt;
	}
	S(S &&r) : my(r.my) {
		std::printf("moved       %d\n", my);
		++cnt;
	}
	~S(){
		std::printf("destroyed   %d\n", my);
		--cnt;
	}
};

unsigned int MCFMain(){
	VVector<S, 0> v;
	VVector<S, 3> v2;
	try {
		for(int i = 0; i < 20; ++i){
			v.Push(i);
		}
		for(int i = 0; i < 3; ++i){
			v2.Push(i);
		}
		v2 = std::move(v);
	} catch(int e){
		std::printf("-- exception %d\n", e);
	}
	std::printf("-- alive %d\n", cnt);

	std::printf("v : ");
	for(const auto &s : v){
		std::printf("%d ", s.my);
	}
	std::putchar('\n');

	std::printf("v2: ");
	for(const auto &s : v2){
		std::printf("%d ", s.my);
	}
	std::putchar('\n');

	return 0;
}
