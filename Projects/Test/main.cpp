#include <MCF/Containers/RingQueue.hpp>
#include <MCF/Core/String.hpp>

template class MCF::RingQueue<int>;
template class MCF::RingQueue<MCF::Utf8String>;

extern "C" unsigned MCFMain(){
//	MCF::RingQueue<MCF::Utf8String> q;
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

	MCF::RingQueue<MCF::Utf8String> q;

	for(int i = 0; i < 15; ++i){
		char temp[256];
		std::sprintf(temp, "---------------------------------- hello %2d ----------------------------------", i);
		q.Push(temp);
		std::sprintf(temp, "---------------------------------- world %2d ----------------------------------", i);
		q.Unshift(temp);
	}
	q.Push("hello world!");

//	q.Shift(20);
//
//	auto p = q.GetFirst();
//	for(int i = 0; i < 5; ++i){
//		p = q.GetNext(p);
//	}
//	try {
//		q.Insert(p, 9, "10000");
//	} catch(std::exception &e){
//		std::printf("exception: %s\n", e.what());
//	}

	auto p = q.GetFirst();
	for(int i = 0; i < 5; ++i){
		p = q.GetNext(p);
	}
	auto p2 = p;
	for(int i = 0; i < 10; ++i){
		p2 = q.GetNext(p2);
	}
	q.Erase(p, p2);

	for(unsigned i = 0; i < q.GetSize(); ++i){
		std::printf("element %2u = %s\n", i, q[i].GetStr());
	}
//	for(auto &e : q){
//		std::printf("element %d\n", e);
//	}

	return 0;
}
