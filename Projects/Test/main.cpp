#include <MCF/StdMCF.hpp>
#include <MCF/Containers/StaticVector.hpp>

template class MCF::StaticVector<int, 5>;

extern "C" unsigned MCFMain(){
	MCF::StaticVector<int, 5> v;
	try {
		long a[] = { 0, 1, 2, 3, 4, 5 };
		v.Append(std::begin(a), std::end(a));
	} catch(std::exception &e){
		std::printf("exception: what = %s\n", e.what());
	}

	for(auto e = v.EnumerateFirst(); e; ++e){
		std::printf("element: %d\n", *e);
	}
	for(auto e = v.EnumerateLast(); e; --e){
		std::printf("element: %d\n", *e);
	}
	return 0;
}
