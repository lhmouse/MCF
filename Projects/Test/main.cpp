#include <MCF/StdMCF.hpp>
#include <MCF/Containers/RingQueue.hpp>

using namespace MCF;

template class RingQueue<int>;

extern "C" unsigned MCFCRT_Main(){
	RingQueue<int> q;
	int a[] = {1,2,3,4,5};
	q.Prepend(5, std::end(a));
	for(auto i : q){
		std::printf("%d\n", i);
	}

	return 0;
}
