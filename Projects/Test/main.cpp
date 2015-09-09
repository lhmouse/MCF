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

	q.Append(30, "00000");
	q.Shift(30);

	for(int i = 0; i < 15; ++i){
		char temp[256];
		std::sprintf(temp, "---------------------------------- hello %2d ----------------------------------", i);
		q.Push(temp);
		std::sprintf(temp, "---------------------------------- world %2d ----------------------------------", i);
		q.Unshift(temp);
	}

//	auto p = q.GetFirst();
//	for(int i = 0; i < 5; ++i){
//		p = q.GetNext(p);
//	}
//	try {
//		q.Insert(p, 9, "11111");
//	} catch(std::exception &e){
//		std::printf("exception: %s\n", e.what());
//	}

	auto p1 = q.GetFirst();
	for(int i = 0; p1 && i < 8; ++i){
		p1 = q.GetNext(p1);
	}
	auto p2 = p1;
	for(int i = 0; p2 && i < 20; ++i){
		p2 = q.GetNext(p2);
	}
	q.Erase(p1, p2);

	auto &q2 = q;
	for(unsigned i = 0; i < q2.GetSize(); ++i){
		std::printf("element %2u = %s\n", i, q2[i].GetStr());
	}
//	for(auto &e : q){
//		std::printf("element %d\n", e);
//	}

	return 0;
}
