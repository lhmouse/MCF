#include <MCF/StdMCF.hpp>
#include <MCF/Containers/RingQueue.hpp>

using namespace MCF;

template class RingQueue<int>;

extern "C" unsigned MCFCRT_Main(){
	RingQueue<int> q;
	for(int i = 0; i < 10; ++i){
		q.Push(i);
	}
	for(int i = 0; i < 10; ++i){
		q.Shift();
	}
	for(int i = 0; i < 10; ++i){
		q.Push(i);
	}

	std::printf("capacity = %zu\n", q.GetCapacity());

	auto leading_seq = q.GetLongestLeadingSequence();
	std::printf("leading_seq  = %p, %zu\n", (void *)leading_seq.first,  leading_seq.second);

	auto trailing_seq = q.GetLongestTrailingSequence();
	std::printf("trailing_seq = %zu, %p\n", trailing_seq.first, (void *)trailing_seq.second);

	return 0;
}
