#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <MCF/Containers/FlatMap.hpp>
#include <MCF/Containers/FlatMultiMap.hpp>
#include <MCF/Containers/Vector.hpp>
#include <MCF/Containers/StaticVector.hpp>
#include <MCF/Containers/List.hpp>
#include <MCF/Containers/RingQueue.hpp>
#include <cstdio>

using namespace MCF;

struct foo {
	UniquePtr<int> a;

	explicit foo(int r){
		a = MakeUnique<int>(r);
		std::printf("foo::foo(): a = %d\n", *a);
	}
	foo(const foo &r){
		if(!r.a){
			return;
		}

		a = MakeUnique<int>(*r.a);
		std::printf("foo::foo(const foo &): a = %d\n", *a);
	}
	foo(foo &&r) noexcept {
		if(!r.a){
			return;
		}

		a = std::move(r.a);
		std::printf("foo::foo(foo &&): a = %d\n", *a);
	}
	~foo(){
		if(!a){
			return;
		}

		std::printf("foo::~foo(): a = %d\n", *a);
	}

	void bark() const {
		if(a){
			std::printf("!! foo::bark(): a = %d\n", *a);
		} else {
			std::printf("!! foo::bark(): a = ??\n");
		}
	}
};

bool operator<(const foo &l, const foo &r) noexcept {
	std::printf("cmp(foo, foo) ");
	return *l.a < *r.a;
}
bool operator<(int l, const foo &r) noexcept {
	std::printf("cmp(int, foo) ");
	return l < *r.a;
}
bool operator<(const foo &l, int r) noexcept {
	std::printf("cmp(foo, int) ");
	return *l.a < r;
}

template class FlatMap<foo, int>;
template class FlatMap<int, foo>;
template class FlatMultiMap<foo, int>;
template class FlatMultiMap<int, foo>;
template class Vector<foo>;
template class StaticVector<foo, 100>;
template class List<foo>;
template class RingQueue<foo>;

extern "C" unsigned MCFCRT_Main(){
	RingQueue<foo> q;
	try {
		for(int i = 0; i < 10; ++i){
			q.Push(i);
		}

		q.Emplace(&*std::next(q.EnumerateFirst(), 1), 123);
		q.Emplace(&*std::next(q.EnumerateFirst(), 8), 456);
		q.Insert(&*std::next(q.EnumerateFirst(), 8), 8, 456);
		q.Erase(&*std::next(q.EnumerateFirst(), 3), &*std::next(q.EnumerateFirst(), 6));

		for(auto en = q.EnumerateFirst(); en; ++en){
			en->bark();
		}
	} catch(std::exception &e){
		std::printf("exception: %s\n", e.what());
	}
	return 0;
}
