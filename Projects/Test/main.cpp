#include <MCF/StdMCF.hpp>
#include <MCF/Core/ArrayObserver.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto fn = [](auto &&v){
		ArrayObserver<const int> obs(v);
		for(auto p = obs.GetBegin(); p != obs.GetEnd(); ++p){
			std::printf("%d ", *p);
		}
		std::putchar('\n');
	};

	Vector<int> v{ 1, 2, 3, 4 };
	fn(v);

	std::initializer_list<int> il{ 5, 6, 7, 8 };
	fn(il);

	int a[] = { 4, 3, 2, 1 };
	fn(a);

	return 0;
}
