#include <MCF/StdMCF.hpp>

template class MCF::MultiIndexedMap<char, int, double>;

unsigned int MCFMain(){
	MCF::MultiIndexedMap<char, int, double> m, n;

	n.Insert('a', 1, 1.1);
	n.Insert('b', 1, 2.2);
	n.Insert('c', 1, 3.3);
	n.Insert('d', 1, 1.1);
	n.Insert('e', 2, 1.1);
	n.Insert('f', 3, 1.1);

	m = n;
/*
	auto q = m.Find<1>(3.3);
	m.SetIndex<1>(q, 0.4);
*/
	std::puts("--- iterate by int ---");
	auto p = m.Front<0>();
	while(p){
		std::printf("%c, %d, %f\n", p->GetElement(), p->GetIndex<0>(), p->GetIndex<1>());
		p = m.Next<0>(p);
	}

	std::puts("--- iterate by double ---");
	p = m.Front<1>();
	while(p){
		std::printf("%c, %d, %f\n", p->GetElement(), p->GetIndex<0>(), p->GetIndex<1>());
		p = m.Next<1>(p);
	}

	return 0;
}
