#include <MCF/Containers/RingQueue.hpp>
#include <MCF/Core/String.hpp>

template class MCF::RingQueue<int>;
template class MCF::RingQueue<MCF::AnsiString>;

extern "C" unsigned MCFMain(){
//	MCF::RingQueue<MCF::AnsiString> q;
//
//	for(int i = 0; i < 10; ++i){
//		char temp[256];
//		std::sprintf(temp, "---------------------------------- hello %d ----------------------------------", i);
//		q.Push(temp);
//		std::sprintf(temp, "---------------------------------- world %d ----------------------------------", i);
//		q.Unshift(temp);
//	}
//	q.Shift(15);
//	for(unsigned i = 0; i < q.GetSize(); ++i){
//		std::printf("element %2u = %s\n", i, q[i].GetStr());
//	}

	MCF::RingQueue<int> q;

	for(int i = 0; i < 10; ++i){
		q.Push(i);
		q.Unshift(-i);
	}

	auto p = q.GetFirst();
	for(int i = 0; i < 5; ++i){
		p = q.GetNext(p);
	}
	q.Insert(p, { 1000, 1001, 1002, 1003, 1004, 1005 });

	for(unsigned i = 0; i < q.GetSize(); ++i){
		std::printf("element %2u = %d\n", i, q[i]);
	}
//	for(auto &e : q){
//		std::printf("element %d\n", e);
//	}

	return 0;
}
