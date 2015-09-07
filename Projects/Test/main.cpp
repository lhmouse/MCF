#include <MCF/Containers/List.hpp>
#include <cstdio>

template class MCF::List<int>;

extern "C" unsigned MCFMain(){
	for(int begin = 0; begin < 10; ++begin){
		for(int end = begin; end < 10; ++end){
			MCF::List<int> l;
			for(int i = 0; i < 10; ++i){
				l.Push(i);
			}

			auto p1 = l.GetFirst();
			for(int i = 0; i < begin; ++i){
				p1 = l.GetNext(p1);
			}
			auto p2 = l.GetFirst();
			for(int i = 0; i < end; ++i){
				p2 = l.GetNext(p2);
			}
			MCF::List<int> sp;
			sp.Splice(nullptr, l, p1, p2);

			std::printf("source : ");
			for(auto e = l.EnumerateFirst(); e; ++e){
				std::printf("%d ", *e);
			}
			std::printf("\nspliced: ");
			for(auto e = sp.EnumerateFirst(); e; ++e){
				std::printf("%d ", *e);
			}
			std::printf("\n-----\n");
		}
	}
	return 0;
}
