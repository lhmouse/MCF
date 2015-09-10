#include <MCF/Containers/RingQueue.hpp>
#include <MCF/Containers/Vector.hpp>
#include <MCF/Containers/StaticVector.hpp>
#include <MCF/Containers/List.hpp>
#include <MCF/Core/String.hpp>

using namespace MCF;

template class RingQueue<int>;
template class Vector<int>;
template class StaticVector<int, 8>;
template class List<int>;

template class RingQueue<Utf8String>;

extern "C" unsigned MCFMain(){
//	RingQueue<Utf8String> q;
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

	RingQueue<Utf8String> q;
//	RingQueue<int> q;

	q.Prepend(30, "00000");
//	q.Prepend(30, 0);
	q.Pop(30);

	for(int i = 0; i < 15; ++i){
		char temp[256];
		std::sprintf(temp, "---------------------------------- hello %2d ----------------------------------", i);
		q.Push(temp);
		std::sprintf(temp, "---------------------------------- world %2d ----------------------------------", i);
		q.Unshift(temp);
//		q.Push(i);
//		q.Unshift(-i);
	}

//	q.Reserve(100);
	auto p = q.GetFirst();
	for(int i = 0; i < 5; ++i){
		p = q.GetNext(p);
	}
	try {
		p = q.Insert(p, 9, "11111");
		p = q.Insert(p, {"s 0"_u8s,"s 1"_u8s,"s 2"_u8s,"s 3"_u8s,"s 4"_u8s,"s 5"_u8s,"s 6"_u8s});
//		q.Insert(p, 9, 11111);
//		q.Insert(p, {100,101,102,103,104,105,106});
	} catch(std::exception &e){
		std::printf("exception: %s\n", e.what());
	}

//	auto p1 = q.GetFirst();
//	for(int i = 0; p1 && i < 8; ++i){
//		p1 = q.GetNext(p1);
//	}
//	auto p2 = p1;
//	for(int i = 0; p2 && i < 20; ++i){
//		p2 = q.GetNext(p2);
//	}
//	q.Erase(p1, p2);

	auto &q2 = q;
	for(unsigned i = 0; i < q2.GetSize(); ++i){
		std::printf("element %2u = %s\n", i, q2[i].GetStr());
//		std::printf("element %2u = %d\n", i, q2[i]);
	}
//	for(auto e = q2.EnumerateLast(); e; --e){
//		std::printf("element %s\n", e->GetStr());
//	}


	RingQueue<int>       a0{0,1,2,3,4,5,6};
	Vector<int>          a1{0,1,2,3,4,5,6};
	StaticVector<int, 8> a2{0,1,2,3,4,5,6};
	List<int>            a3{0,1,2,3,4,5,6};

	return 0;
}
