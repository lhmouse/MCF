#include <MCF/StdMCF.hpp>
#include <MCF/Containers/FlatMap.hpp>
#include <MCF/Containers/FlatMultiMap.hpp>

using namespace MCF;

template class FlatMap<int, long>;
template class FlatMultiMap<int, long>;

extern "C" unsigned MCFMain(){
	FlatMultiMap<int, long> s{ {1,0}, {5,0}, {0,0}, {2,0}, {3,0}, {5,1}, {0,1}, {2,1}, {1,1}, {4,0}, {4,1}, {3,1} };
	auto r = s.GetEqualRange(2);
	std::printf("r = %td, %td\n", r.first - s.GetBegin(), r.second - s.GetBegin());
	for(auto it = s.GetBegin(); it != s.GetEnd(); ++it){
		std::printf("element: %d, %ld\n", it->first, it->second);
	}
	return 0;
}
