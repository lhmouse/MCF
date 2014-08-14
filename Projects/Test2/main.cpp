#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexedMap.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	MultiIndexedMap<double, int> m;
	m.Insert(1.0 , 1);
	m.Insert(4.0 , 2);
	m.Insert(9.0 , 3);
	auto h = m.GetBegin<0>();
	h = h->GetNext<0>();
	h = h->GetNext<0>();
std::puts("--- 1 ---");
	m.InsertHint(h, 16.0, 4);
std::puts("--- 2 ---");
	m.InsertHint(h, 25.0, 5);
std::puts("--- 3 ---");

	auto m2 = m;
	for(auto p = m.GetBegin<0>(); p; p = p->GetNext<0>()){
		std::printf("%d %f\n", p->GetIndex<0>(), p->GetElement());
	}

	return 0;
}
