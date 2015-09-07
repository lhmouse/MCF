#include <MCF/Containers/List.hpp>
#include <cstdio>

template class MCF::List<int>;

extern "C" unsigned MCFMain(){
	MCF::List<int> l;
	for(int i = 0; i < 10; ++i){
		l.Push(i);
	}

	auto begin = l.EnumerateFirst();
	auto end = begin;
	for(unsigned i = 0; i < 3; ++i){
		++begin;
	}
	for(unsigned i = 0; i < 6; ++i){
		++end;
	}
	l.Erase(l.GetFirst(), &*begin);
	l.Erase(&*begin, &*end);
//	l.Erase(&*end, nullptr);

	for(auto e = l.EnumerateFirst(); e; ++e){
		std::printf("element + : %d\n", *e);
	}
	for(auto e = l.EnumerateLast(); e; --e){
		std::printf("element - : %d\n", *e);
	}
	return 0;
}
