#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexedMap.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	MultiIndexedMap<double, int> m;
	m.Insert(1.0 , 1);
	m.Insert(4.0 , 2);
	m.Insert(9.0 , 3);
	m.Insert(16.0, 4);
	m.Insert(25.0, 5);

	auto m2 = m;
	for(auto p = m.GetBegin<0>(); p; p = p->GetNext<0>()){
		std::printf("%d %f\n", p->GetIndex<0>(), p->GetElement());
	}

	return 0;
}
